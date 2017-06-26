/*-------------------------------------------------------------------------------------+                                                                                           
|

|     $Source: DgnCore/TileWriter.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/RenderPrimitives.h>
#include <DgnPlatform/TileIO.h>

#include <TilePublisher/Lib/Constants.h>

USING_NAMESPACE_TILETREE
USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_BENTLEY_RENDER_PRIMITIVES
       

#define BEGIN_TILEWRITER_NAMESPACE    BEGIN_BENTLEY_DGN_NAMESPACE namespace TileWriter {
#define END_TILEWRITER_NAMESPACE      } END_BENTLEY_DGN_NAMESPACE

BEGIN_TILEWRITER_NAMESPACE


DEFINE_POINTER_SUFFIX_TYPEDEFS(TileTexture)
DEFINE_REF_COUNTED_PTR(TileTexture)


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void addTransparencyToTechnique (Json::Value& technique)
    {
    technique["states"]["enable"].append (3042);  // BLEND

    auto&   techniqueFunctions =    technique["states"]["functions"] = Json::objectValue;

    techniqueFunctions["blendEquationSeparate"] = Json::arrayValue;
    techniqueFunctions["blendFuncSeparate"]     = Json::arrayValue;

    techniqueFunctions["blendEquationSeparate"].append (32774);   // FUNC_ADD (rgb)
    techniqueFunctions["blendEquationSeparate"].append (32774);   // FUNC_ADD (alpha)

    techniqueFunctions["blendFuncSeparate"].append(1);            // ONE (srcRGB)
    techniqueFunctions["blendFuncSeparate"].append(771);          // ONE_MINUS_SRC_ALPHA (dstRGB)
    techniqueFunctions["blendFuncSeparate"].append(1);            // ONE (srcAlpha)
    techniqueFunctions["blendFuncSeparate"].append(771);          // ONE_MINUS_SRC_ALPHA (dstAlpha)

    techniqueFunctions["depthMask"] = "false";
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
static void addTechniqueParameter(Json::Value& technique, Utf8CP name, int type, Utf8CP semantic)
    {
    auto& param = technique["parameters"][name];
    param["type"] = type;
    if (nullptr != semantic)
        param["semantic"] = semantic;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
static void appendProgramAttribute(Json::Value& program, Utf8CP attrName)
    {
    program["attributes"].append(attrName);
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   02/17
//=======================================================================================
struct TileColorIndex
{
    enum class Dimension : uint8_t
    {
        Zero = 0,   // uniform color
        One,        // only one row
        Two,        // more than one row
        Background, // use background color.
        None,       // empty
    };
private:
    ByteStream      m_texture;
    uint16_t        m_width = 0;
    uint16_t        m_height = 0;

public:
    TileColorIndex(DisplayParamsCR displayParams, ColorTableCR colorTable) { Build(displayParams, colorTable); }

    static constexpr uint16_t GetMaxWidth() { return 256; }

    ByteStream const& GetTexture() const { return m_texture; }
    Image ExtractImage() { return Image(GetWidth(), GetHeight(), std::move(m_texture), Image::Format::Rgba); }

    uint16_t GetWidth() const { return m_width; }
    uint16_t GetHeight() const { return m_height; }
    bool empty() const { return m_texture.empty(); }

    Dimension GetDimension() const
        {
        if (empty())
            return Dimension::None;
        else if (GetHeight() > 1)
            return Dimension::Two;
        else
            return GetWidth() > 1 ? Dimension::One : Dimension::Zero;
        }

    static Dimension CalcDimension(uint16_t nColors)
        {
        switch (nColors)
            {
            case 0:     return Dimension::None;
            case 1:     return Dimension::Zero;
            default:    return nColors <= GetMaxWidth() ? Dimension::One : Dimension::Zero;
            }
        }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ComputeDimensions(uint16_t nColors)
    {
    // Minimum texture size in WebGL is 64x64. For 16-bit color indices, we need at least 256x256.
    // At the risk of pessimization, let's not assume more than that.
    // Let's assume non-power-of-2 textures are not a big deal (we're not mipmapping them or anything).
    uint16_t height = 1;
    uint16_t width = std::min(nColors, GetMaxWidth());
    if (width < nColors)
        {
        height = nColors / width;
        if (height*width < nColors)
            ++height;
        }

    BeAssert(height*width >= nColors);
    m_width = width;
    m_height = height;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static void fillColorIndex(ByteStream& texture, ColorTableCR colorTable, T getColorDef)
    {
    constexpr size_t bytesPerColor = 4;
    texture.Resize(bytesPerColor * colorTable.GetNumIndices());

    for (auto& color : colorTable)
        {
        ColorDef fill = getColorDef(color.first);

        auto pColor = texture.GetDataP() + color.second * bytesPerColor;
        pColor[0] = fill.GetRed();
        pColor[1] = fill.GetGreen();
        pColor[2] = fill.GetBlue();
        pColor[3] = fill.GetAlpha();    // already inverted...
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Build(DisplayParamsCR displayParams, ColorTableCR colorTable)
    {
    // Possibilities:
    //  - Material does not override color or alpha. Copy colors directly from ColorIndexMap (unless only one color - then use uniform color).
    //  - Material overrides both color and alpha. Every vertex has same color. Don't use indexed colors in that case.
    //  - Material overrides RGB only. Vertices may have differing alphas. ###TODO: If alpha does not vary, don't use indexed colors.
    //  - Material overrides alpha only. Vertices may have differing RGB values. ###TODO: Detect that case and don't use indexed colors?
    uint16_t        nColors = colorTable.GetNumIndices();

    ComputeDimensions(nColors);
    BeAssert(0 < m_width && 0 < m_height);
    
    bool  overridesRgb = false, overridesAlpha = false;

    if (nullptr !=  displayParams.GetRenderingAsset())
        {
        overridesRgb   = displayParams.GetRenderingAsset()->GetBool(RENDER_MATERIAL_FlagHasBaseColor, false);
        overridesAlpha = displayParams.GetRenderingAsset()->GetBool(RENDER_MATERIAL_FlagHasTransmit, false);
        }

    if (!overridesAlpha && !overridesRgb)
        {
        fillColorIndex(m_texture, colorTable, [](uint32_t color)
            {
            ColorDef fill(color);
            fill.SetAlpha(255 - fill.GetAlpha());
            return fill;
            });
        }
    else if (!overridesAlpha)
        {
        RgbFactor fRgb = displayParams.GetRenderingAsset()->GetColor(RENDER_MATERIAL_Color);
        ColorDef rgb(static_cast<uint8_t>(fRgb.red*255), static_cast<uint8_t>(fRgb.green*255), static_cast<uint8_t>(fRgb.blue*255));
        fillColorIndex(m_texture, colorTable, [=](uint32_t color)
            {
            ColorDef input(color);
            ColorDef output = rgb;
            output.SetAlpha(255 - input.GetAlpha());
            return output;
            });
        }
    else if (!overridesRgb)
        {
        uint8_t alpha = 255 - static_cast<uint8_t>(displayParams.GetRenderingAsset()->GetDouble(RENDER_MATERIAL_Transmit, 0.0) * 255);
        fillColorIndex(m_texture, colorTable, [=](uint32_t color)
            {
            ColorDef def(color);
            def.SetAlpha(alpha);
            return def;
            });
        }
    else
        {
        uint8_t alpha = 255 - static_cast<uint8_t>(displayParams.GetRenderingAsset()->GetDouble(RENDER_MATERIAL_Transmit, 0.0) * 255);
        RgbFactor fRgb = displayParams.GetRenderingAsset()->GetColor(RENDER_MATERIAL_Color);
        ColorDef rgba(static_cast<uint8_t>(fRgb.red*255), static_cast<uint8_t>(fRgb.green*255), static_cast<uint8_t>(fRgb.blue*255), alpha);
        fillColorIndex(m_texture, colorTable, [=](uint32_t color) { return rgba; });
        }
    }

};


/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     04/2017
+===============+===============+===============+===============+===============+======*/
struct TileTexture : Render::Texture
 {
    CreateParams        m_createParams;
    Image               m_image;

    TileTexture() { }
    TileTexture(ImageCR image, CreateParams const& createParams) : m_image(image), m_createParams(createParams) { }


    bool GetRepeat() const { return !m_createParams.m_isTileSection; }
 }; // TileTexture

/*=================================================================================**//**
* @bsiclass                                                     Ray.Bentley     04/2017
+===============+===============+===============+===============+===============+======*/
struct RenderSystem : Render::System
{

    RenderSystem()  { }
    ~RenderSystem() { }

    virtual MaterialPtr _GetMaterial(DgnMaterialId, DgnDbR) const override { return nullptr; }
    virtual MaterialPtr _CreateMaterial(Material::CreateParams const&) const override { return nullptr; } 
    virtual GraphicPtr _CreateSprite(ISprite& sprite, DPoint3dCR location, DPoint3dCR xVec, int transparency, DgnDbR db) const override { BeAssert(false); return nullptr; }
    virtual GraphicPtr _CreateBranch(GraphicBranch&& branch, DgnDbR dgndb, TransformCR transform, ClipVectorCP clips) const override { BeAssert(false); return nullptr; }
    virtual GraphicPtr _CreateViewlet(GraphicBranch& branch, PlanCR, ViewletPosition const&) const override { BeAssert(false); return nullptr; };
    virtual TexturePtr _GetTexture(DgnTextureId textureId, DgnDbR db) const override { BeAssert(false); return nullptr; }

    virtual TexturePtr _CreateGeometryTexture(GraphicCR graphic, DRange2dCR range, bool useGeometryColors, bool forAreaPattern) const override { BeAssert(false); return nullptr; }
    virtual LightPtr   _CreateLight(Lighting::Parameters const&, DVec3dCP direction, DPoint3dCP location) const override { BeAssert(false); return nullptr; }

    virtual int _Initialize(void* systemWindow, bool swRendering) override { return  0; }
    virtual Render::TargetPtr _CreateTarget(Render::Device& device, double tileSizeModifier) override { return nullptr; }
    virtual Render::TargetPtr _CreateOffscreenTarget(Render::Device& device, double tileSizeModifier) override { return nullptr; }
    virtual GraphicPtr _CreateVisibleEdges(MeshEdgeArgsCR args, DgnDbR dgndb)  const override { return nullptr; }
    virtual GraphicPtr _CreateSilhouetteEdges(SilhouetteEdgeArgsCR args, DgnDbR dgndb)  const override { return nullptr; }
    virtual GraphicPtr _CreateGraphicList(bvector<GraphicPtr>&& primitives, DgnDbR dgndb) const override { return nullptr; }
    virtual GraphicPtr _CreateBatch(GraphicR graphic, FeatureTable&& features) const override {return nullptr; }
    virtual uint32_t   _GetMaxFeaturesPerBatch() const override { return 0xffffffff; }

    virtual TexturePtr _GetTexture(GradientSymbCR gradient, DgnDbR db) const override {return nullptr; }
    virtual TexturePtr _CreateTexture(ImageCR image, Render::Texture::CreateParams const& params) const override {return new TileTexture(image, params);}
    virtual TexturePtr _CreateTexture(ImageSourceCR source, Image::BottomUp bottomUp, Texture::CreateParams const& params) const override { BeAssert(false); return nullptr; }
    virtual GraphicPtr _CreateIndexedPolylines(IndexedPolylineArgsCR args, DgnDbR dgndb) const override  { return nullptr; }
    virtual GraphicPtr _CreatePointCloud(PointCloudArgsCR args, DgnDbR dgndb)  const override {return nullptr; }
    virtual GraphicPtr _CreateTriMesh(TriMeshArgsCR args, DgnDbR dgndb) const override  { return  nullptr; }
    virtual GraphicBuilderPtr _CreateGraphic(GraphicBuilder::CreateParams const& params) const override { return nullptr; }

};  // RenderSystem


//=======================================================================================
// @bsistruct                                                   Paul.Connelly   02/17
//=======================================================================================
struct TileMaterial
{
protected:
    Utf8String                  m_name;
    TileColorIndex::Dimension   m_colorDimension;
    bool                        m_hasAlpha = false;
    TileTextureCPtr             m_texture;
    bool                        m_overridesAlpha = false;
    bool                        m_overridesRgb = false;
    RgbFactor                   m_rgbOverride;
    double                      m_alphaOverride;
    bool                        m_adjustColorForBackground = false;

    TileMaterial(Utf8StringCR name) : m_name(name) { }

    virtual std::string _GetVertexShaderString() const = 0;

public:
    Utf8StringCR GetName() const { return m_name; }
    TileColorIndex::Dimension GetColorIndexDimension() const { return m_colorDimension; }
    bool HasTransparency() const { return m_hasAlpha; }
    bool IsTextured() const { return m_texture.IsValid(); }
    TileTextureCPtr GetTexture() const { return m_texture.get(); }

    bool OverridesAlpha() const { return m_overridesAlpha; }
    bool OverridesRgb() const { return m_overridesRgb; }
    double GetAlphaOverride() const { return m_alphaOverride; }
    RgbFactor const& GetRgbOverride() const { return m_rgbOverride; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
std::string GetVertexShaderString() const
    {
    std::string vs = _GetVertexShaderString();

    if (!m_adjustColorForBackground)
        return vs;

    Utf8String vs2d(s_adjustBackgroundColorContrast.c_str());
    vs2d.append(vs.c_str());
    vs2d.ReplaceAll("v_color = computeColor()", "v_color = adjustContrast(computeColor())");
    return vs2d.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void AddColorIndexTechniqueParameters(Json::Value& technique, Json::Value& program, Json::Value& jsonRoot) const
    {
    auto dim = GetColorIndexDimension();
    auto& techniqueUniforms = technique["uniforms"];
    if (TileColorIndex::Dimension::Zero != dim)
        {
        addTechniqueParameter(technique, "tex", GLTF_SAMPLER_2D, nullptr);
        addTechniqueParameter(technique, "colorIndex", GLTF_FLOAT, "_COLORINDEX");

        techniqueUniforms["u_tex"] = "tex";
        techniqueUniforms["u_texStep"] = "texStep";

        technique["attributes"]["a_colorIndex"] = "colorIndex";
        appendProgramAttribute(program, "a_colorIndex");

        auto& sampler = jsonRoot["samplers"]["sampler_1"];
        sampler["minFilter"] = GLTF_NEAREST;
        sampler["maxFilter"] = GLTF_NEAREST;
        sampler["wrapS"] = GLTF_CLAMP_TO_EDGE;
        sampler["wrapT"] = GLTF_CLAMP_TO_EDGE;

        if (TileColorIndex::Dimension::Two == dim)
            {
            addTechniqueParameter(technique, "texWidth", GLTF_FLOAT, nullptr);
            addTechniqueParameter(technique, "texStep", GLTF_FLOAT_VEC4, nullptr);

            techniqueUniforms["u_texWidth"] = "texWidth";
            }
        else
            {
            addTechniqueParameter(technique, "texStep", GLTF_FLOAT_VEC2, nullptr);
            }
        }
    else
        {
        addTechniqueParameter(technique, "color", GLTF_FLOAT_VEC4, nullptr);
        techniqueUniforms["u_color"] = "color";
        }

    if (HasTransparency())
        addTransparencyToTechnique(technique);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void AddTextureTechniqueParameters(Json::Value& technique, Json::Value& program, Json::Value& jsonRoot) const

    {
    BeAssert (IsTextured());
    if (IsTextured())
        {
        addTechniqueParameter(technique, "tex", GLTF_SAMPLER_2D, nullptr);
        addTechniqueParameter(technique, "texc", GLTF_FLOAT_VEC2, "TEXCOORD_0");

        jsonRoot["samplers"]["sampler_0"] = Json::objectValue;
        jsonRoot["samplers"]["sampler_0"]["minFilter"] = GLTF_LINEAR;
        if (!m_texture->GetRepeat())
            {
            jsonRoot["samplers"]["sampler_0"]["wrapS"] = GLTF_CLAMP_TO_EDGE;
            jsonRoot["samplers"]["sampler_0"]["wrapT"] = GLTF_CLAMP_TO_EDGE;
            }
        technique["uniforms"]["u_tex"] = "tex";
        technique["attributes"]["a_texc"] = "texc";
        appendProgramAttribute(program, "a_texc");
        }
    }


};


//=======================================================================================
// @bsistruct                                                   Paul.Connelly   02/17
//=======================================================================================
struct PolylineMaterial : TileMaterial
{
private:
    double                  m_width;
    double                  m_textureLength;       // If positive, meters, if negative, pixels (Cosmetic).

public:

    std::string const& GetFragmentShaderString() const;
    double GetWidth() const { return m_width; }
    double GetTextureLength() const { return m_textureLength; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
PolylineMaterial(MeshCR mesh, bool is3d, Utf8CP suffix): TileMaterial(Utf8String("PolylineMaterial_")+suffix)
    {
    DisplayParamsCR displayParams = mesh.GetDisplayParams();
    ColorTableCR    colorTable = mesh.GetColorTable();

    m_hasAlpha = colorTable.HasTransparency();         // || IsTesselated(); // Turn this on if we use alpha in tesselated polylines.
    m_colorDimension = TileColorIndex::CalcDimension(colorTable.GetNumIndices());
    m_width = static_cast<double> (displayParams.GetLineWidth());

    if (LinePixels::Solid != displayParams.GetLinePixels())
        {
        static uint32_t         s_height = 2;
        ByteStream              bytes(32 * 4 * s_height);
        uint32_t*               dataP = (uint32_t*) bytes.GetDataP();

        for (uint32_t y=0, mask = 0x0001; y < s_height; y++)
            for (uint32_t x=0, mask = 0x0001; x < 32; x++, mask = mask << 1)
                *dataP++ = (0 == (mask & (uint32_t) displayParams.GetLinePixels())) ? 0 : 0xffffffff;

        Render::Image   image (32, s_height, std::move(bytes), Image::Format::Rgba);

        m_texture = new TileTexture (image, Texture::CreateParams());
        m_textureLength = -32;          // Negated to denote cosmetic.
        }
    m_adjustColorForBackground = !is3d;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void AddTechniqueParameters(Json::Value& tech, Json::Value& prog, Json::Value& jsonRoot) const
    {
    AddColorIndexTechniqueParameters(tech, prog, jsonRoot);
    if (IsTextured())
        AddTextureTechniqueParameters(tech, prog, jsonRoot);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String GetTechniqueNamePrefix() const
    {
    Utf8String prefix;

    prefix.append (IsTextured() ? "Textured" : "Solid");
    prefix.append("Polyline");
    prefix.append(std::to_string(static_cast<uint8_t>(GetColorIndexDimension())).c_str());
    return prefix;
    }
protected:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
std::string _GetVertexShaderString() const override
    {
    std::string const* list = nullptr;
    if (IsTextured())
        list = s_tesselatedTexturedPolylineVertexShaders;
    else
        list = s_tesselatedSolidPolylineVertexShaders;

    auto index = static_cast<uint8_t>(GetColorIndexDimension());

    return list[index];
    }

};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   02/17
//=======================================================================================
struct MeshMaterial : TileMaterial
{
    static constexpr double GetSpecularFinish() { return 0.9; }
    static constexpr double GetSpecularExponentMult() { return 48.0; }
private:
    DgnMaterialCPtr         m_material;
    RgbFactor               m_specularColor = { 1.0, 1.0, 1.0 };
    double                  m_specularExponent = GetSpecularFinish() * GetSpecularExponentMult();
    bool                    m_ignoreLighting;
public:

    bool HasTransparency() const { return m_hasAlpha; }
    bool IgnoresLighting() const { return m_ignoreLighting; }
    TileColorIndex::Dimension GetColorIndexDimension() const { return m_colorDimension; }
    DgnMaterialCP GetDgnMaterial() const { return m_material.get(); }
    double GetSpecularExponent() const { return m_specularExponent; }
    RgbFactor const& GetSpecularColor() const { return m_specularColor; }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
MeshMaterial(MeshCR mesh, bool is3d, Utf8StringCR suffix, DgnDbR db) : TileMaterial(Utf8String("Material_")+suffix)
    {
    DisplayParamsCR params = mesh.GetDisplayParams();

    m_ignoreLighting = params.IgnoresLighting();
    m_texture = static_cast<TileTexture*> (params.GetTexture());

    uint32_t rgbInt = params.GetFillColor();
    double alpha = 1.0 - ((uint8_t*)&rgbInt)[3] / 255.0;

    if (params.GetMaterialId().IsValid())
        {
        m_material = DgnMaterial::Get(db, params.GetMaterialId());
        if (m_material.IsValid())
            {
            auto jsonMat = &m_material->GetRenderingAsset();
            m_overridesRgb = jsonMat->GetBool(RENDER_MATERIAL_FlagHasBaseColor, false);
            m_overridesAlpha = jsonMat->GetBool(RENDER_MATERIAL_FlagHasTransmit, false);

            if (m_overridesRgb)
                m_rgbOverride = jsonMat->GetColor(RENDER_MATERIAL_Color);

            if (m_overridesAlpha)
                {
                m_alphaOverride = jsonMat->GetDouble(RENDER_MATERIAL_Transmit, 0.0);
                alpha = m_alphaOverride;
                }
            else if (m_overridesRgb)
                {
                // Apparently overriding RGB without specifying transmit => opaque.
                m_alphaOverride = 1.0;
                alpha = m_alphaOverride;
                m_overridesAlpha = true;
                }

            if (jsonMat->GetBool(RENDER_MATERIAL_FlagHasSpecularColor, false))
                m_specularColor = jsonMat->GetColor(RENDER_MATERIAL_SpecularColor);

            constexpr double s_finishScale = 15.0;
            if (jsonMat->GetBool(RENDER_MATERIAL_FlagHasFinish, false))
                m_specularExponent = jsonMat->GetDouble(RENDER_MATERIAL_Finish, s_qvSpecular) * s_finishScale;
            }
        }

    m_hasAlpha = mesh.GetColorTable().HasTransparency();
    m_adjustColorForBackground = !is3d && params.GetFillFlags() == FillFlags::Background;

    if (m_overridesAlpha && m_overridesRgb)
        m_colorDimension = TileColorIndex::Dimension::Zero;
    else
        m_colorDimension = m_adjustColorForBackground ? TileColorIndex::Dimension::Background : TileColorIndex::CalcDimension(mesh.GetColorTable().GetNumIndices());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
std::string _GetVertexShaderString() const override
    {
    if (IsTextured())
        return IgnoresLighting() ? s_unlitTextureVertexShader : s_texturedVertexShader;

    auto index = static_cast<uint8_t>(GetColorIndexDimension());
    BeAssert(index < _countof(s_untexturedVertexShaders));

    std::string const* list = IgnoresLighting() ? s_unlitVertexShaders : s_untexturedVertexShaders;
    return list[index];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
std::string const& GetFragmentShaderString() const
    {
    if (IsTextured())
        return IgnoresLighting() ? s_unlitTextureFragmentShader : s_texturedFragShader;
    else
        return IgnoresLighting() ? s_unlitFragmentShader : s_untexturedFragShader;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String GetTechniqueNamePrefix() const
    {
    Utf8String prefix = IsTextured() ? "Textured" : "Untextured";
    if (HasTransparency())
        prefix.append("Transparent");

    if (IgnoresLighting())
        prefix.append("Unlit");

    prefix.append(std::to_string(static_cast<uint8_t>(GetColorIndexDimension())).c_str());

    return prefix;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void AddTechniqueParameters(Json::Value& technique, Json::Value& program, Json::Value& jsonRoot) const
    {
    if (IsTextured())
        AddTextureTechniqueParameters(technique, program, jsonRoot);
    else
        AddColorIndexTechniqueParameters(technique, program, jsonRoot);

    if (!IgnoresLighting())
        {
        // Specular...
        addTechniqueParameter(technique, "specularColor", GLTF_FLOAT_VEC3, nullptr);
        technique["uniforms"]["u_specularColor"] = "specularColor";

        addTechniqueParameter(technique, "specularExponent", GLTF_FLOAT, nullptr);
        technique["uniforms"]["u_specularExponent"] = "specularExponent";
        }
    }


};

//=======================================================================================
// We use a hierarchical batch table to organize features by element and subcategory,
// and subcategories by category
// Each feature has a batch table class corresponding to its DgnGeometryClass.
// The feature classes have no properties, only parents for classification.
// The element, category, and subcategory classes each have an ID property.
// @bsistruct                                                   Paul.Connelly   02/17
//=======================================================================================
struct BatchTableBuilder
{
private:
    struct ElemInfo { uint32_t m_index; uint32_t m_parentIndex; };
    struct AssemInfo { uint32_t m_index; uint32_t m_catIndex; };
    struct Assembly { DgnElementId m_elemId; DgnCategoryId m_catId; };

    enum ClassIndex
    {
        kClass_Primary,
        kClass_Construction,
        kClass_Dimension,
        kClass_Pattern,
        kClass_Element,
        kClass_Assembly,
        kClass_SubCategory,
        kClass_Category,

        kClass_COUNT,
        kClass_FEATURE_COUNT = kClass_Pattern+1,
    };

    static constexpr Utf8CP s_classNames[kClass_COUNT] =
        {
        "Primary", "Construction", "Dimension", "Pattern", "Element", "Assembly", "SubCategory", "Category"
        };

    Json::Value                         m_json; // "HIERARCHY": object
    DgnDbR                              m_db;
    FeatureTableCR                      m_featureTable;
    bmap<DgnElementId, ElemInfo>        m_elems;
    bmap<DgnElementId, AssemInfo>       m_assemblies;
    bmap<DgnSubCategoryId, uint32_t>    m_subcats;
    bmap<DgnCategoryId, uint32_t>       m_cats;
    DgnCategoryId                       m_uncategorized;
    bool                                m_is3d;

    template<typename T, typename U> static auto Find(T& map, U const& key) -> typename T::iterator
        {
        return map.find(key);
        }
    template<typename T, typename U> static uint32_t FindOrInsert(T& map, U const& key)
        {
        auto iter = Find(map, key);
        if (iter != map.end())
            return iter->second;

        uint32_t index = static_cast<uint32_t>(map.size());
        map[key] = index;
        return index;
        }
    template<typename T, typename U> static uint32_t GetIndex(T& map, U const& key)
        {
        auto iter = Find(map, key);
        BeAssert(iter != map.end());
        return iter->second;
        }

    ElemInfo MapElementInfo(DgnElementId id);
    ElemInfo GetElementInfo(DgnElementId id);
    AssemInfo MapAssemblyInfo(Assembly assem);
    uint32_t MapCategoryIndex(DgnCategoryId id) { return FindOrInsert(m_cats, id); }
    uint32_t GetCategoryIndex(DgnCategoryId id) { return GetIndex(m_cats, id); }
    uint32_t MapSubCategoryIndex(DgnSubCategoryId id) { return FindOrInsert(m_subcats, id); }
    uint32_t GetSubCategoryIndex(DgnSubCategoryId id) { return GetIndex(m_subcats, id); }

    Json::Value& GetClass(ClassIndex idx) { return m_json["classes"][idx]; }
    static ClassIndex GetFeatureClassIndex(DgnGeometryClass geomClass);

    Assembly QueryAssembly(DgnElementId) const;
    void DefineClasses();
    void MapFeatures();
    void MapParents();
    void MapElements(uint32_t offset, uint32_t assembliesOffset);
    void MapAssemblies(uint32_t offset, uint32_t categoriesOffset);
    void MapSubCategories(uint32_t offset);
    void MapCategories(uint32_t offset);

    void Build();
    void InitUncategorizedCategory();
    bool IsUncategorized(DgnCategoryId id) const { return id.IsValid() && id == m_uncategorized; }
public:
    BatchTableBuilder(FeatureTableCR featureTable, DgnDbR db, bool is3d)
        : m_json(Json::objectValue), m_db(db), m_featureTable(featureTable), m_is3d(is3d)
        {
        InitUncategorizedCategory();
        Build();
        }

    Json::Value const& GetHierarchy() const { return m_json; }

    Utf8String ToString() const
        {
        Json::Value json;
        json["HIERARCHY"] = GetHierarchy();
        return Json::FastWriter().write(json);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BatchTableBuilder::InitUncategorizedCategory()
    {
    // This is dumb. See OfficeBuilding.dgn - cells have no level in V8, which translates to 'Uncategorized' (2d and 3d variants) in DgnDb
    // We don't want to create an 'Uncategorized' assembly if its children belong to a real category.
    // We only can detect this because for whatever reason, "Uncategorized" is not a localized string.
    DefinitionModelR dictionary = m_db.GetDictionaryModel();
    DgnCode code = m_is3d ? SpatialCategory::CreateCode(dictionary, "Uncategorized") : DrawingCategory::CreateCode(dictionary, "Uncategorized");
    m_uncategorized = DgnCategory::QueryCategoryId(m_db, code);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BatchTableBuilder::Build()
    {
    m_json["parentCounts"] = Json::arrayValue;
    m_json["classIds"] = Json::arrayValue;
    m_json["parentIds"] = Json::arrayValue;

    // Set up the "classes" array. For each member, defines "name" property only. "length" and "instances" property TBD.
    DefineClasses();

    // Makes sure every instance of every class is assigned an index into the "classIds" array (relative to the index of
    // the first instance of that class).
    // Adds index for each Feature into "classIds" (index == batch ID)
    // Sets "parentCounts" for each Feature (all == 2)
    // Sets "length" for each of the Feature classes
    MapFeatures();

    // Populates "classes" for all instances of abstract (parent) classes
    // Populates the "parentIds" and "parentCounts" arrays for all instances of all classes
    // Sets the "length" and "instances" property of "classes" member for each abstract (parent) class
    // Sets "instancesLength" to the total number of instances of all classes.
    MapParents();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
auto BatchTableBuilder::GetFeatureClassIndex(DgnGeometryClass geomClass) -> ClassIndex
    {
    switch (geomClass)
        {
        case DgnGeometryClass::Primary:         return kClass_Primary;
        case DgnGeometryClass::Construction:    return kClass_Construction;
        case DgnGeometryClass::Dimension:       return kClass_Dimension;
        case DgnGeometryClass::Pattern:         return kClass_Pattern;
        default:
            BeAssert(false);
            return kClass_Primary;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BatchTableBuilder::MapSubCategories(uint32_t offset)
    {
    Json::Value& subcats = GetClass(kClass_SubCategory);
    subcats["length"] = m_subcats.size();

    Json::Value &instances = (subcats["instances"] = Json::objectValue),
                &subcat_id = (instances["subcategory"] = Json::arrayValue),
                &classIds = m_json["classIds"],
                &parentCounts = m_json["parentCounts"];

    for (auto const& kvp : m_subcats)
        {
        classIds[offset + kvp.second] = kClass_SubCategory;
        parentCounts[offset + kvp.second] = 0;
        subcat_id[kvp.second] = kvp.first.ToString();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BatchTableBuilder::MapCategories(uint32_t offset)
    {
    Json::Value& cats = GetClass(kClass_Category);
    cats["length"] = m_cats.size();

    Json::Value &instances = (cats["instances"] = Json::objectValue),
                &cat_id = (instances["category"] = Json::arrayValue),
                &classIds = m_json["classIds"],
                &parentCounts = m_json["parentCounts"];

    for (auto const& kvp : m_cats)
        {
        classIds[offset + kvp.second] = kClass_Category;
        parentCounts[offset + kvp.second] = 0;
        cat_id[kvp.second] = kvp.first.ToString();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BatchTableBuilder::MapElements(uint32_t offset, uint32_t assembliesOffset)
    {
    Json::Value& elements = GetClass(kClass_Element);
    elements["length"] = m_elems.size();

    Json::Value& instances = (elements["instances"] = Json::objectValue);
    Json::Value& elem_id = (instances["element"] = Json::arrayValue);
    Json::Value& classIds = m_json["classIds"];
    Json::Value& parentCounts = m_json["parentCounts"];
    Json::Value& parentIds = m_json["parentIds"];

    for (auto const& kvp : m_elems)
        {
        Json::Value::ArrayIndex index = kvp.second.m_index;
        elem_id[index] = kvp.first.ToString();

        classIds[offset + index] = kClass_Element;
        parentCounts[offset + index] = 1;
        parentIds.append(kvp.second.m_parentIndex + assembliesOffset);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BatchTableBuilder::MapAssemblies(uint32_t offset, uint32_t categoriesOffset)
    {
    Json::Value& assems = GetClass(kClass_Assembly);
    assems["length"] = m_assemblies.size();

    Json::Value &instances = (assems["instances"] = Json::objectValue),
                &assem_id = (instances["assembly"] = Json::arrayValue),
                &classIds = m_json["classIds"],
                &parentCounts = m_json["parentCounts"],
                &parentIds = m_json["parentIds"];

    for (auto const& kvp : m_assemblies)
        {
        Json::Value::ArrayIndex index = kvp.second.m_index;
        classIds[offset+index] = kClass_Assembly;
        assem_id[index] = kvp.first.ToString();
        parentCounts[offset+index] = 1;
        parentIds.append(kvp.second.m_catIndex + categoriesOffset);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BatchTableBuilder::MapParents()
    {
    uint32_t elementsOffset = static_cast<uint32_t>(m_featureTable.size());
    uint32_t assembliesOffset = elementsOffset + static_cast<uint32_t>(m_elems.size());
    uint32_t subcatsOffset = assembliesOffset + static_cast<uint32_t>(m_assemblies.size());
    uint32_t catsOffset = subcatsOffset + static_cast<uint32_t>(m_subcats.size());
    uint32_t totalInstances = catsOffset + static_cast<uint32_t>(m_cats.size());

    m_json["instancesLength"] = totalInstances;

    // Now that every instance of every class has an index into "classIds", we can map parent IDs
    Json::Value& parentIds = m_json["parentIds"];
    for (auto const& kvp : m_featureTable)
        {
        FeatureCR attr = kvp.first;
        uint32_t index = kvp.second * 2; // 2 parents per feature

        parentIds[index] = elementsOffset + GetElementInfo(attr.GetElementId()).m_index;
        parentIds[index+1] = subcatsOffset + GetSubCategoryIndex(attr.GetSubCategoryId());
        }

    // Set "instances" and "length" to Element class, and add elements to "classIds" and assemblies to "parentIds"
    MapElements(elementsOffset, assembliesOffset);
    MapAssemblies(assembliesOffset, catsOffset);

    // Set "instances" and "length" to SubCategory class, and add subcategories to "classIds" and "parentIds"
    MapSubCategories(subcatsOffset);
    MapCategories(catsOffset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BatchTableBuilder::MapFeatures()
    {
    Json::Value& classIds = m_json["classIds"];
    Json::Value& parentCounts = m_json["parentCounts"];

    uint32_t instanceCounts[kClass_FEATURE_COUNT] = { 0 };

    for (auto const& kvp : m_featureTable)
        {
        FeatureCR attr = kvp.first;
        uint32_t index = kvp.second;
        ClassIndex classIndex = GetFeatureClassIndex(attr.GetClass());

        classIds[index] = classIndex;
        parentCounts[index] = 2; // element, subcategory

        ++instanceCounts[classIndex];

        // Ensure all parent instances are mapped
        MapElementInfo(attr.GetElementId());
        MapSubCategoryIndex(attr.GetSubCategoryId());
        }

    // Set the number of instances of each class
    for (uint8_t classIndex = 0; classIndex < kClass_FEATURE_COUNT; classIndex++)
        GetClass(static_cast<ClassIndex>(classIndex))["length"] = instanceCounts[classIndex];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void BatchTableBuilder::DefineClasses()
    {
    auto& classes = (m_json["classes"] = Json::arrayValue);
    for (uint8_t i = 0; i < kClass_COUNT; i++)
        {
        auto& cls = (classes[i] = Json::objectValue);
        cls["name"] = s_classNames[i];
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
auto BatchTableBuilder::MapElementInfo(DgnElementId id) -> ElemInfo
    {
    auto iter = Find(m_elems, id);
    if (iter != m_elems.end())
        return iter->second;

    Assembly assem = QueryAssembly(id);
    ElemInfo info;
    info.m_index = static_cast<uint32_t>(m_elems.size());
    info.m_parentIndex = MapAssemblyInfo(assem).m_index;
    m_elems[id] = info;
    return info;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
auto BatchTableBuilder::MapAssemblyInfo(Assembly assem) -> AssemInfo
    {
    auto iter = Find(m_assemblies, assem.m_elemId);
    if (iter != m_assemblies.end())
        return iter->second;

    AssemInfo info;
    info.m_index = static_cast<uint32_t>(m_assemblies.size());
    info.m_catIndex = MapCategoryIndex(assem.m_catId);
    m_assemblies[assem.m_elemId] = info;
    return info;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
auto BatchTableBuilder::GetElementInfo(DgnElementId id) -> ElemInfo
    {
    auto iter = Find(m_elems, id);
    BeAssert(iter != m_elems.end());
    return iter->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
auto BatchTableBuilder::QueryAssembly(DgnElementId childId) const -> Assembly
    {
    Assembly assem;
    assem.m_elemId = childId;
    if (!childId.IsValid())
        return assem;

    // Get this element's category and parent element
    // Recurse until no more parents (or we find a non-geometric parent)
    static constexpr Utf8CP s_3dsql = "SELECT Parent.Id,Category.Id FROM " BIS_SCHEMA(BIS_CLASS_GeometricElement3d) " WHERE ECInstanceId=?";
    static constexpr Utf8CP s_2dsql = "SELECT Parent.Id,Category.Id FROM " BIS_SCHEMA(BIS_CLASS_GeometricElement2d) " WHERE ECInstanceId=?";

    BeSQLite::EC::CachedECSqlStatementPtr stmt = m_db.GetPreparedECSqlStatement(m_is3d ? s_3dsql : s_2dsql);
    stmt->BindId(1, childId);

    while (BeSQLite::BE_SQLITE_ROW == stmt->Step())
        {
        auto thisCatId = stmt->GetValueId<DgnCategoryId>(1);
        if (assem.m_catId.IsValid() && IsUncategorized(thisCatId) && !IsUncategorized(assem.m_catId))
            break; // yuck. if have children with valid categories, stop before first uncategorized parent (V8 complex header).

        assem.m_catId = thisCatId;
        assem.m_elemId = childId;

        childId = stmt->GetValueId<DgnElementId>(0);
        if (!childId.IsValid())
            break;

        // Try to get the parent's category. If parent is not geometric, this will fail and we will treat current child as the assembly root.
        stmt->Reset();
        stmt->BindId(1, childId);
        }

    BeAssert(assem.m_catId.IsValid());
    return assem;
    }



//=======================================================================================
// @bsistruct                                                   Ray.Bentley     12/2016
//=======================================================================================
struct  Writer
{
    Json::Value         m_json;
    ByteStream          m_binaryData;
    StreamBufferR       m_buffer;
    RenderSystem        m_renderSystem;
    DgnModelCR          m_model;

    Writer(StreamBufferR buffer, DgnModelR model) : m_buffer(buffer), m_model(model) { }

    size_t BinaryDataSize() const { return m_binaryData.size(); }
    void const* BinaryData() const { return m_binaryData.data(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void WriteLength(uint32_t startPosition, uint32_t lengthDataPosition)
    {
    uint32_t    dataSize = static_cast<uint32_t> (m_buffer.GetSize() - startPosition);
    memcpy(m_buffer.GetDataP() + lengthDataPosition, &dataSize, sizeof(uint32_t));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void    AddBinaryData (void const* data, size_t size) 
    {
    m_binaryData.Append (static_cast<uint8_t const*> (data), size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void    PadBinaryDataToBoundary(size_t boundarySize = 4)
    {
    while (0 != (m_binaryData.GetSize() % boundarySize))
        m_binaryData.Append((uint8_t) 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void    PadToBoundary(size_t boundarySize = 4)
    {
    while (0 != (m_buffer.GetSize() % boundarySize))
        m_buffer.Append((uint8_t) 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void AddTechniqueParameter(Json::Value& technique, Utf8CP name, int type, Utf8CP semantic)
    {
    auto& param = technique["parameters"][name];
    param["type"] = type;
    if (nullptr != semantic)
        param["semantic"] = semantic;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void AppendProgramAttribute(Json::Value& program, Utf8CP attrName)
    {
    program["attributes"].append(attrName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void AddShader(Json::Value& shaders, Utf8CP name, int type, Utf8CP buffer)
    {
    auto& shader = (shaders[name] = Json::objectValue);
    shader["type"] = type;
    shader["extensions"]["KHR_binary_glTF"]["bufferView"] = buffer;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void AddColorIndexTechniqueParameters(Json::Value& technique, Json::Value& program, ColorIndex const& colorIndex, bool hasTransparency)
    {
    auto& techniqueUniforms = technique["uniforms"];
    if (0 != colorIndex.m_numColors)
        {
        AddTechniqueParameter(technique, "tex", GLTF_SAMPLER_2D, nullptr);
        AddTechniqueParameter(technique, "colorIndex", GLTF_FLOAT, "_COLORINDEX");

        techniqueUniforms["u_tex"] = "tex";
        techniqueUniforms["u_texStep"] = "texStep";

        technique["attributes"]["a_colorIndex"] = "colorIndex";
        AppendProgramAttribute(program, "a_colorIndex");

        auto& sampler = m_json["samplers"]["sampler_1"];
        sampler["minFilter"] = GLTF_NEAREST;
        sampler["maxFilter"] = GLTF_NEAREST;
        sampler["wrapS"] = GLTF_CLAMP_TO_EDGE;
        sampler["wrapT"] = GLTF_CLAMP_TO_EDGE;

        if (2 == colorIndex.m_numColors)
            {
            AddTechniqueParameter(technique, "texWidth", GLTF_FLOAT, nullptr);
            AddTechniqueParameter(technique, "texStep", GLTF_FLOAT_VEC4, nullptr);

            techniqueUniforms["u_texWidth"] = "texWidth";
            }
        else
            {
            AddTechniqueParameter(technique, "texStep", GLTF_FLOAT_VEC2, nullptr);
            }
        }
    else
        {
        AddTechniqueParameter(technique, "color", GLTF_FLOAT_VEC4, nullptr);
        techniqueUniforms["u_color"] = "color";
        }

    if (hasTransparency)
        AddTransparencyToTechnique(technique);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
 void AddTransparencyToTechnique (Json::Value& technique)
    {
    technique["states"]["enable"].append (3042);  // BLEND

    auto&   techniqueFunctions =    technique["states"]["functions"] = Json::objectValue;

    techniqueFunctions["blendEquationSeparate"] = Json::arrayValue;
    techniqueFunctions["blendFuncSeparate"]     = Json::arrayValue;

    techniqueFunctions["blendEquationSeparate"].append (32774);   // FUNC_ADD (rgb)
    techniqueFunctions["blendEquationSeparate"].append (32774);   // FUNC_ADD (alpha)

    techniqueFunctions["blendFuncSeparate"].append(1);            // ONE (srcRGB)
    techniqueFunctions["blendFuncSeparate"].append(771);          // ONE_MINUS_SRC_ALPHA (dstRGB)
    techniqueFunctions["blendFuncSeparate"].append(1);            // ONE (srcAlpha)
    techniqueFunctions["blendFuncSeparate"].append(771);          // ONE_MINUS_SRC_ALPHA (dstAlpha)

    techniqueFunctions["depthMask"] = "false";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void AddTextureTechniqueParameters(Json::Value& technique, Json::Value& program, TileTexture& texture)

    {
    AddTechniqueParameter(technique, "tex", GLTF_SAMPLER_2D, nullptr);
    AddTechniqueParameter(technique, "texc", GLTF_FLOAT_VEC2, "TEXCOORD_0");

    m_json["samplers"]["sampler_0"] = Json::objectValue;
    m_json["samplers"]["sampler_0"]["minFilter"] = GLTF_LINEAR;
    if (texture.m_createParams.m_isTileSection)
        {
        m_json["samplers"]["sampler_0"]["wrapS"] = GLTF_CLAMP_TO_EDGE;
        m_json["samplers"]["sampler_0"]["wrapT"] = GLTF_CLAMP_TO_EDGE;
        }
    technique["uniforms"]["u_tex"] = "tex";
    technique["attributes"]["a_texc"] = "texc";
    AppendProgramAttribute(program, "a_texc");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void AddMeshTechniqueParameters(Json::Value& technique, Json::Value& program, Render::Primitives::DisplayParamsR displayParams, ColorIndex const& colorIndex)
    {
    if (nullptr != displayParams.GetTexture())
        AddTextureTechniqueParameters(technique, program, *(static_cast <TileTexture*> (displayParams.GetTexture())));
    else
        AddColorIndexTechniqueParameters(technique, program, colorIndex, displayParams.HasFillTransparency());

    if (!displayParams.IgnoresLighting())
        {
        // Specular...
        AddTechniqueParameter(technique, "specularColor", GLTF_FLOAT_VEC3, nullptr);
        technique["uniforms"]["u_specularColor"] = "specularColor";

        AddTechniqueParameter(technique, "specularExponent", GLTF_FLOAT, nullptr);
        technique["uniforms"]["u_specularExponent"] = "specularExponent";
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> void AddBufferView(Utf8CP name, T const* bufferData, size_t count)
    {
    Json::Value&    views = m_json["bufferViews"];

    auto bufferDataSize = count * sizeof(*bufferData);
    auto& view = (views[name] = Json::objectValue);
    view["buffer"] = "binary_glTF";
    view["byteOffset"] = m_binaryData.size();
    view["byteLength"] = bufferDataSize;

    size_t binaryDataSize = m_binaryData.size();
    m_binaryData.resize(binaryDataSize + bufferDataSize);
    memcpy(m_binaryData.data() + binaryDataSize, bufferData, bufferDataSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> void AddBufferView(Utf8CP name, T const& bufferData)
    {
    AddBufferView (name, bufferData.data(), bufferData.size());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void AddExtensions(DPoint3dCR centroid)
    {
    m_json["extensionsUsed"] = Json::arrayValue;
    m_json["extensionsUsed"].append("KHR_binary_glTF");
    m_json["extensionsUsed"].append("CESIUM_RTC");
    m_json["extensionsUsed"].append("WEB3D_quantized_attributes");

    m_json["glExtensionsUsed"] = Json::arrayValue;
    m_json["glExtensionsUsed"].append("OES_element_index_uint");

    m_json["extensions"]["CESIUM_RTC"]["center"] = Json::arrayValue;
    m_json["extensions"]["CESIUM_RTC"]["center"].append(centroid.x);
    m_json["extensions"]["CESIUM_RTC"]["center"].append(centroid.y);
    m_json["extensions"]["CESIUM_RTC"]["center"].append(centroid.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void AddDefaultScene ()
    {
    m_json["scene"] = "defaultScene";
    m_json["scenes"]["defaultScene"]["nodes"] = Json::arrayValue;
    m_json["scenes"]["defaultScene"]["nodes"].append("rootNode");
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void AddMeshUInt16Attributes(Json::Value& primitive, uint16_t const* attributes16, size_t nAttributes, Utf8StringCR idStr, Utf8CP name, Utf8CP semantic)
    {
    Utf8String suffix(name);
    suffix.append(idStr);

    Utf8String bvId  = "bv" +  suffix;
    Utf8String accId = "acc" + suffix;

    primitive["attributes"][semantic] = accId;

    // Use uint8 if possible to save space in tiles and memory in browser
    bvector<uint8_t> attributes8;
    auto             componentType = GLTF_UNSIGNED_BYTE;

    for (size_t i=0; i<nAttributes; i++)
        {
        if (attributes16[i] > 0xff)
            {
            componentType = GLTF_UNSIGNED_SHORT;
            break;
            }
        }


    if (GLTF_UNSIGNED_BYTE == componentType)
        {
        attributes8.reserve(nAttributes);
        for (size_t i=0; i<nAttributes; i++)
            attributes8.push_back(static_cast<uint8_t>(attributes16[i]));

        AddBufferView(bvId.c_str(), attributes8.data(), nAttributes);
        }
    else
        {
        AddBufferView(bvId.c_str(), attributes16, nAttributes * sizeof(uint16_t));
        }

    AddAccessor(componentType, accId, bvId, nAttributes, "SCALAR");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void AddBatchIds(Json::Value& primitive, FeatureIndex const& featureIndex, size_t nVertices, Utf8StringCR idStr)
    {
    if (featureIndex.IsEmpty())
        {
        BeAssert(false);
        return;
        }

    bvector<uint16_t>   batchIds;

    for (size_t i=0; i<nVertices; i++)
        batchIds.push_back(featureIndex.IsUniform() ? featureIndex.m_featureID : featureIndex.m_featureIDs[i]);
   
    AddMeshUInt16Attributes(primitive, batchIds.data(), batchIds.size(), idStr, "Batch_", "BATCHID");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void AddColors(Json::Value& primitive, ColorIndex const& colorIndex, size_t nVertices, Utf8StringCR idStr)
    {
    BeAssert (colorIndex.m_numColors > 1);
    AddMeshUInt16Attributes(primitive, colorIndex.m_nonUniform.m_indices, nVertices, idStr, "ColorIndex_", "_COLORINDEX");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/02016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String AddTextureImage (TileTexture const& tileTexture, Utf8StringCR suffix)
    {
    Utf8String  textureId = Utf8String ("texture_") + suffix;

#ifdef TEXTURE_CACHE
    auto const& found = m_textureImages.find (textureImage);

    // For composite tiles, we must ensure that the texture is defined for each individual tile - cannot share
    bool textureExists = found != m_textureImages.end();
    if (textureExists && tilejsonRoot.isMember("textures") && tilejsonRoot["textures"].isMember(found->second.c_str()))
        return found->second;
#endif


#ifdef NEEDS_WORK_TEXTURE
    bool        hasAlpha = textureImage->GetImageSource().GetFormat() == ImageSource::Format::Png;

    Utf8String  textureId = Utf8String ("texture_") + suffix;
    Utf8String  imageId   = Utf8String ("image_")   + suffix;
    Utf8String  bvImageId = Utf8String ("imageBufferView") + suffix;

    tilejsonRoot["textures"][textureId] = Json::objectValue;
    tilejsonRoot["textures"][textureId]["format"] = hasAlpha ? GLTF_RGBA : GLTF_RGB;
    tilejsonRoot["textures"][textureId]["internalFormat"] = hasAlpha ? GLTF_RGBA : GLTF_RGB;
    tilejsonRoot["textures"][textureId]["sampler"] = "sampler_0";
    tilejsonRoot["textures"][textureId]["source"] = imageId;

    tilejsonRoot["images"][imageId] = Json::objectValue;


    DRange3d    range = mesh.GetRange(), uvRange = mesh.GetUVRange();
    Image       image (textureImage->GetImageSource(), hasAlpha ? Image::Format::Rgba : Image::Format::Rgb);

    // This calculation should actually be made for each triangle and maximum used.
    static      double      s_requiredSizeRatio = 2.0, s_sizeLimit = 1024.0;
    double      requiredSize = std::min (s_sizeLimit, s_requiredSizeRatio * range.DiagonalDistance () / (m_tile.GetTolerance() * std::min (1.0, uvRange.DiagonalDistance())));
    DPoint2d    imageSize = { (double) image.GetWidth(), (double) image.GetHeight() };

    tilejsonRoot["bufferViews"][bvImageId] = Json::objectValue;
    tilejsonRoot["bufferViews"][bvImageId]["buffer"] = "binary_glTF";

    Point2d     targetImageSize, currentImageSize = { (int32_t) image.GetWidth(), (int32_t) image.GetHeight() };

    if (requiredSize < std::min (currentImageSize.x, currentImageSize.y))
        {
        static      int32_t s_minImageSize = 64;
        static      int     s_imageQuality = 60;
        int32_t     targetImageMin = std::max(s_minImageSize, (int32_t) requiredSize);
        ByteStream  targetImageData;

        if (imageSize.x > imageSize.y)
            {
            targetImageSize.y = targetImageMin;
            targetImageSize.x = (int32_t) ((double) targetImageSize.y * imageSize.x / imageSize.y);
            }
        else
            {
            targetImageSize.x = targetImageMin;
            targetImageSize.y = (int32_t) ((double) targetImageSize.x * imageSize.y / imageSize.x);
            }
        targetImageSize.x = roundToMultipleOfTwo (targetImageSize.x);
        targetImageSize.y = roundToMultipleOfTwo (targetImageSize.y);
        }
    else
        {
        targetImageSize.x = roundToMultipleOfTwo (currentImageSize.x);
        targetImageSize.y = roundToMultipleOfTwo (currentImageSize.y);
        }

    ImageSource         imageSource = textureImage->GetImageSource();
    static const int    s_imageQuality = 50;


    if (targetImageSize.x != imageSize.x || targetImageSize.y != imageSize.y)
        {
        Image           targetImage = Image::FromResizedImage (targetImageSize.x, targetImageSize.y, image);

        imageSource = ImageSource (targetImage, textureImage->GetImageSource().GetFormat(), s_imageQuality);
        }

    tilejsonRoot["images"][imageId]["extensions"]["KHR_binary_glTF"] = Json::objectValue;
    tilejsonRoot["images"][imageId]["extensions"]["KHR_binary_glTF"]["bufferView"] = bvImageId;
    tilejsonRoot["images"][imageId]["extensions"]["KHR_binary_glTF"]["mimeType"] = "image/jpeg";


    tilejsonRoot["images"][imageId]["extensions"]["KHR_binary_glTF"]["height"] = targetImageSize.x;
    tilejsonRoot["images"][imageId]["extensions"]["KHR_binary_glTF"]["width"] = targetImageSize.y;

    ByteStream const& imageData = imageSource.GetByteStream();
    tilejsonRoot["bufferViews"][bvImageId]["byteOffset"] = BinaryDataSize();
    tilejsonRoot["bufferViews"][bvImageId]["byteLength"] = imageData.size();
    AddBinaryData (imageData.data(), imageData.size());

    if (!textureExists)
        m_textureImages.Insert (textureImage, textureId);
#endif

    return textureId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String AddColorIndex(TileColorIndex& colorIndex, MeshCR mesh, Utf8StringCR suffix)
    {
    Utf8String textureId("texture_"),
               imageId("image_"),
               bvImageId("imageBufferView");

    textureId.append(suffix);
    imageId.append(suffix);
    bvImageId.append(suffix);

    auto& texture = m_json["textures"][textureId] = Json::objectValue;
    texture["format"] = GLTF_RGBA;
    texture["internalFormat"] = GLTF_RGBA;
    texture["sampler"] = "sampler_1";
    texture["source"] = imageId;

    auto& bufferView = m_json["bufferViews"][bvImageId] = Json::objectValue;
    bufferView["buffer"] = "binary_glTF";

    auto& image = m_json["images"][imageId] = Json::objectValue;
    auto& imageExtensions = image["extensions"]["KHR_binary_glTF"] = Json::objectValue;
    imageExtensions["bufferView"] = bvImageId;
    imageExtensions["mimeType"] = "image/png";
    imageExtensions["width"] = colorIndex.GetWidth();
    imageExtensions["height"] = colorIndex.GetHeight();

    ImageSource imageSource(colorIndex.ExtractImage(), ImageSource::Format::Png);
    ByteStream const& imageData = imageSource.GetByteStream();
    bufferView["byteOffset"] = BinaryDataSize();
    bufferView["byteLength"] = imageData.size();
    AddBinaryData(imageData.data(), imageData.size());

#if defined(DEBUG_COLOR_INDEX)
    WString name = WString(imageId.c_str(), true) + L"_" + m_tile.GetNameSuffix(), extension;
    std::FILE* outputFile = _wfopen(BeFileName(nullptr, m_context.GetDataDirForModel(m_tile.GetModel()).c_str(), name.c_str(), L"png").c_str(), L"wb");
    fwrite(imageData.GetData(), 1, imageData.GetSize(), outputFile);
    fclose(outputFile);
#endif

    return textureId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void    AddMaterialColor(Json::Value& matJson,  TileMaterial const& mat, DisplayParamsCR displayParams, MeshCR mesh, Utf8StringCR suffix)
    {
    auto dim = TileColorIndex::CalcDimension(mesh.GetColorTable().GetNumIndices());

    if (TileColorIndex::Dimension::Zero != dim)
        {
        TileColorIndex colorIndex(displayParams, mesh.GetColorTable());

        matJson["values"]["tex"] = AddColorIndex(colorIndex, mesh, suffix);

        uint16_t width = colorIndex.GetWidth();
        double stepX = 1.0 / width;
        double stepY = 1.0 / colorIndex.GetHeight();

        auto& texStep = matJson["values"]["texStep"] = Json::arrayValue;
        texStep.append(stepX);
        texStep.append(stepX * 0.5);    // centerX

        if (TileColorIndex::Dimension::Two == colorIndex.GetDimension())
            {
            texStep.append(stepY);
            texStep.append(stepY * 0.5);    // centerY

            matJson["values"]["texWidth"] = width;
            }
        }
    else
        {
        BeAssert(1 == mesh.GetColorTable().GetNumIndices() || (mat.OverridesRgb() && mat.OverridesAlpha()));
        ColorDef baseDef(mesh.GetColorTable().begin()->first);
        RgbFactor rgb = mat.OverridesRgb() ? mat.GetRgbOverride() : RgbFactor::FromIntColor(baseDef.GetValue());
        double alpha = mat.OverridesAlpha() ? mat.GetAlphaOverride() : baseDef.GetAlpha()/255.0;

        auto& matColor = matJson["values"]["color"];
        matColor.append(rgb.red);
        matColor.append(rgb.green);
        matColor.append(rgb.blue);
        matColor.append(1.0 - alpha);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String AddMeshShaderTechnique(MeshMaterial const& mat, bool doBatchIds)
    {
    Utf8String prefix = mat.GetTechniqueNamePrefix();

    if (!doBatchIds)
        prefix.append("NoId");

    Utf8String techniqueName(prefix);
    techniqueName.append("Technique");

    if (m_json.isMember("techniques") && m_json["techniques"].isMember(techniqueName.c_str()))
        return techniqueName;

    Json::Value technique(Json::objectValue);

    AddTechniqueParameter(technique, "mv", GLTF_FLOAT_MAT4, "CESIUM_RTC_MODELVIEW");
    AddTechniqueParameter(technique, "proj", GLTF_FLOAT_MAT4, "PROJECTION");
    AddTechniqueParameter(technique, "pos", GLTF_FLOAT_VEC3, "POSITION");
    if (!mat.IgnoresLighting())
        {
        AddTechniqueParameter(technique, "n", GLTF_INT_VEC2, "NORMAL");
        AddTechniqueParameter(technique, "nmx", GLTF_FLOAT_MAT3, "MODELVIEWINVERSETRANSPOSE");
        }

    if (doBatchIds)
        AddTechniqueParameter(technique, "batch", GLTF_FLOAT, "BATCHID");

    if (!mat.IsTextured())
        AddTechniqueParameter(technique, "colorIndex", GLTF_FLOAT, "_COLORINDEX");

    Utf8String         programName               = prefix + "Program";
    Utf8String         vertexShader              = prefix + "VertexShader";
    Utf8String         fragmentShader            = prefix + "FragmentShader";
    Utf8String         vertexShaderBufferView    = vertexShader + "BufferView";
    Utf8String         fragmentShaderBufferView  = fragmentShader + "BufferView";

    technique["program"] = programName.c_str();

    auto&   techniqueStates = technique["states"];
    techniqueStates["enable"] = Json::arrayValue;
    techniqueStates["enable"].append(GLTF_DEPTH_TEST);
    techniqueStates["disable"].append(GLTF_CULL_FACE);

    auto& techniqueAttributes = technique["attributes"];
    techniqueAttributes["a_pos"] = "pos";
    if (doBatchIds)
        techniqueAttributes["a_batchId"] = "batch";

    if (!mat.IsTextured())
        techniqueAttributes["a_colorIndex"] = "colorIndex";

    if(!mat.IgnoresLighting())
        techniqueAttributes["a_n"] = "n";

    auto& techniqueUniforms = technique["uniforms"];
    techniqueUniforms["u_mv"] = "mv";
    techniqueUniforms["u_proj"] = "proj";
    if (!mat.IgnoresLighting())
        techniqueUniforms["u_nmx"] = "nmx";

    auto& rootProgramNode = (m_json["programs"][programName.c_str()] = Json::objectValue);
    rootProgramNode["attributes"] = Json::arrayValue;
    AppendProgramAttribute(rootProgramNode, "a_pos");

    if (doBatchIds)
        AppendProgramAttribute(rootProgramNode, "a_batchId");
    if (!mat.IgnoresLighting())
        AppendProgramAttribute(rootProgramNode, "a_n");

    rootProgramNode["vertexShader"]   = vertexShader.c_str();
    rootProgramNode["fragmentShader"] = fragmentShader.c_str();

    auto& shaders = m_json["shaders"];
    AddShader(shaders, vertexShader.c_str(), GLTF_VERTEX_SHADER, vertexShaderBufferView.c_str());
    AddShader(shaders, fragmentShader.c_str(), GLTF_FRAGMENT_SHADER, fragmentShaderBufferView.c_str());

    bool color2d = false;
    std::string vertexShaderString = s_shaderPrecision;
    if (doBatchIds)
        vertexShaderString.append(s_batchIdShaderAttribute);

    vertexShaderString.append(mat.GetVertexShaderString());

    AddBufferView(vertexShaderBufferView.c_str(),  vertexShaderString);
    AddBufferView(fragmentShaderBufferView.c_str(), mat.GetFragmentShaderString());

    mat.AddTechniqueParameters(technique, rootProgramNode, m_json);

    m_json["techniques"][techniqueName.c_str()] = technique;

    return techniqueName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static Json::Value CreateDecodeQuantizeValues(double const* min, double const* max, size_t nComponents)
    {
    Json::Value         decodeMatrix = Json::arrayValue;
    Json::Value         quantizeValue = Json::objectValue;

   for (size_t i=0; i<nComponents; i++)
        {
        for (size_t j=0; j<nComponents; j++)
            decodeMatrix.append ((i==j) ? ((max[i] - min[i]) / Quantization::RangeScale()) : 0.0);

        decodeMatrix.append (0.0);
        }

    for (size_t i=0; i<nComponents; i++)
        decodeMatrix.append (min[i]);

    decodeMatrix.append (1.0);
    quantizeValue["decodeMatrix"] = decodeMatrix;

    for (size_t i=0; i<3; i++)
        {
        quantizeValue["decodedMin"].append (min[i]);
        quantizeValue["decodedMax"].append (max[i]);
        }

    return quantizeValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void AddAccessor(uint32_t componentType, Utf8StringCR accessorId, Utf8StringCR bufferViewId, size_t count, Utf8CP type)
    {
    Json::Value         accessor   = Json::objectValue;

    accessor["componentType"] = componentType;
    accessor["bufferView"] = bufferViewId;
    accessor["byteOffset"] = 0;
    accessor["count"] = count;
    accessor["type"] = type;

    BeAssert(!m_json["accessors"].isMember(accessorId));
    m_json["accessors"][accessorId] = accessor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String    AddQuantizedPointsAttribute(QPoint3dCP qPoints, size_t nPoints, QPoint3d::Params params, Utf8StringCR name, Utf8StringCR id) 
    {
    Utf8String          nameId =  name + id,
                        accessorId = Utf8String("acc") + nameId,
                        bufferViewId = Utf8String("bv") + nameId;
    DRange3d            range = params.GetRange();

    AddBufferView(bufferViewId.c_str(), qPoints, nPoints);
    AddAccessor(GLTF_UNSIGNED_SHORT, accessorId, bufferViewId, nPoints, "VEC3");

    m_json["bufferViews"][bufferViewId]["target"] = GLTF_ARRAY_BUFFER;
    m_json["accessors"][accessorId]["extensions"]["WEB3D_quantized_attributes"] = CreateDecodeQuantizeValues(&range.low.x, &range.high.x, 3);

    return accessorId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String    AddQuantizedParamAttribute(FPoint2d const* params, size_t nParams, Utf8StringCR name, Utf8StringCR id) 
    {
    QPoint2dList        qParams;
    
    qParams.InitFrom(params, nParams);

    Utf8String          nameId =  Utf8String(name) + Utf8String(id),
                        accessorId = Utf8String("acc") + nameId,
                        bufferViewId = Utf8String("bv") + nameId;
    DRange2d            range = qParams.GetParams().GetRange();

    AddBufferView(bufferViewId.c_str(), qParams);
    AddAccessor(GLTF_UNSIGNED_SHORT, accessorId, bufferViewId, nParams, "VEC2");
    m_json["bufferViews"][bufferViewId]["target"] = GLTF_ARRAY_BUFFER;
    m_json["accessors"][accessorId]["extensions"]["WEB3D_quantized_attributes"] = CreateDecodeQuantizeValues(&range.low.x, &range.high.x, 2);

    return accessorId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String    AddParamAttribute(FPoint2d const* params, size_t nParams, Utf8StringCR name, Utf8StringCR id) 
    {
    Utf8String          nameId =  Utf8String(name) + Utf8String(id),
                        accessorId = Utf8String("acc") + nameId,
                        bufferViewId = Utf8String("bv") + nameId;

    AddBufferView(bufferViewId.c_str(), params, nParams);
    AddAccessor(GLTF_FLOAT, accessorId, bufferViewId, nParams, "VEC2");
    m_json["bufferViews"][bufferViewId]["target"] = GLTF_ARRAY_BUFFER;

    return accessorId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String AddMeshIndices(Utf8StringCR name, uint32_t const* indices, size_t numIndices, Utf8StringCR idStr, size_t maxIndex)
    {
    Utf8String          nameId           = name + idStr,
                        accIndexId       = "acc" + nameId,
                        bvIndexId        = "bv"  + nameId;
    bool                useShortIndices  = maxIndex < 0xffff;

 
    if (useShortIndices)
        {
        bvector<uint16_t>   shortIndices;

        shortIndices.reserve(numIndices);

        for (size_t i=0; i<numIndices; i++)
            shortIndices.push_back ((uint16_t) indices[i]);

        AddBufferView(bvIndexId.c_str(), shortIndices.data(), shortIndices.size());
        }
    else
        {
        AddBufferView(bvIndexId.c_str(), indices, numIndices);
        }
    m_json["bufferViews"][bvIndexId]["target"] =  GLTF_ELEMENT_ARRAY_BUFFER;
    AddAccessor(useShortIndices ? GLTF_UNSIGNED_SHORT : GLTF_UINT32, accIndexId, bvIndexId, numIndices, "SCALAR");

    return accIndexId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String AddMeshTriangleIndices(Utf8StringCR name, TriangleList const& triangles, Utf8StringCR idStr, size_t maxIndex)
    {
    bvector<uint32_t>       indices;
    
    indices.reserve(triangles.size() * 3);

    for (auto&  triangle : triangles)
        {
        indices.push_back(triangle.m_indices[0]);
        indices.push_back(triangle.m_indices[1]);
        indices.push_back(triangle.m_indices[2]);
        }
    return AddMeshIndices(name, indices.data(), indices.size(), idStr, maxIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void AddMeshPointRange (Json::Value& positionValue, DRange3dCR pointRange)
    {
    positionValue["min"] = Json::arrayValue;
    positionValue["min"].append(pointRange.low.x);
    positionValue["min"].append(pointRange.low.y);
    positionValue["min"].append(pointRange.low.z);
    positionValue["max"] = Json::arrayValue;
    positionValue["max"].append(pointRange.high.x);
    positionValue["max"].append(pointRange.high.y);
    positionValue["max"].append(pointRange.high.z);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static Json::Value     CreateColorJson(RgbFactorCR color)
    {
    Json::Value     colorJson = Json::objectValue;

    colorJson.append(color.red);
    colorJson.append(color.green);
    colorJson.append(color.blue);

    return colorJson;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  CreateMeshMaterialJson(Json::Value& matJson, MeshCR mesh, MeshMaterial const& meshMaterial, DisplayParamsCR displayParams, Utf8StringCR suffix) 
    {
#ifdef NEEDS_WORK
    if (nullptr != displayParams.GetMaterial())
        matJson["name"] = displayParams.GetMaterial()->GetMaterialName().c_str();
#endif

    if (meshMaterial.IsTextured())
        {
        TileTexture const*  texture = static_cast<TileTexture const*> (displayParams.GetTexture());
        matJson["values"]["tex"] = AddTextureImage(*texture, suffix);
        }
    else
        {
        AddMaterialColor (matJson, meshMaterial, displayParams, mesh, suffix);
        }

    matJson["technique"] = AddMeshShaderTechnique(meshMaterial, true).c_str();

    if (!displayParams.IgnoresLighting())
        {
        matJson["values"]["specularExponent"] = meshMaterial.GetSpecularExponent();
        matJson["values"]["specularColor"] = CreateColorJson(meshMaterial.GetSpecularColor());
        }
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value AddNormals (QPoint3dCP normals, size_t numNormals, Utf8CP name, Utf8CP id)
    {
    return AddQuantizedPointsAttribute(normals, numNormals, QPoint3d::Params::FromNormalizedRange(), name, id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CreateTriMesh(Json::Value& primitiveJson, MeshCR mesh, MeshArgs const& meshArgs, MeshMaterial const& meshMaterial, Utf8StringCR idStr)
    {
    primitiveJson["mode"] = GLTF_TRIANGLES;

    Utf8String      accPositionId =  AddQuantizedPointsAttribute(meshArgs.m_points, meshArgs.m_numPoints, meshArgs.m_pointParams, "Position", idStr.c_str());
    primitiveJson["attributes"]["POSITION"] = accPositionId;

    bool isTextured = meshMaterial.IsTextured();
    BeAssert (isTextured == (nullptr != meshArgs.m_textureUV));
    if (nullptr != meshArgs.m_textureUV && isTextured)
        primitiveJson["attributes"]["TEXCOORD_0"] = AddParamAttribute (meshArgs.m_textureUV, meshArgs.m_numPoints, "Param", idStr.c_str());
    if (meshArgs.m_colors.m_numColors > 1)
        AddColors(primitiveJson, meshArgs.m_colors, meshArgs.m_numPoints, idStr);

    BeAssert (meshMaterial.IgnoresLighting() || nullptr != meshArgs.m_normals);

    if (nullptr != meshArgs.m_normals && !meshMaterial.IgnoresLighting())        // No normals if ignoring lighting (reality meshes).
        primitiveJson["attributes"]["NORMAL"]  = AddQuantizedPointsAttribute(meshArgs.m_normals, meshArgs.m_numPoints, QPoint3d::Params::FromNormalizedRange(), "Normal", idStr.c_str());

    primitiveJson["indices"] = AddMeshIndices ("Indices", (uint32_t const*) meshArgs.m_vertIndex, meshArgs.m_numIndices, idStr, meshArgs.m_numPoints);
    AddMeshPointRange(m_json["accessors"][accPositionId], meshArgs.m_pointParams.GetRange());

    return SUCCESS;
    }
                                                                                     
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void AddTriMesh(Json::Value& primitivesNode, MeshArgs const& meshArgs, MeshCR mesh, size_t& index)
    {
    if (0 == meshArgs.m_numIndices)
        return;

    Utf8String          idStr(std::to_string(index++).c_str());
    Json::Value         materialJson = Json::objectValue, primitiveJson = Json::objectValue;

    MeshMaterial meshMaterial(mesh, m_model.Is3d(), idStr, m_model.GetDgnDb());
    
    if (SUCCESS == CreateMeshMaterialJson(materialJson, mesh, meshMaterial, mesh.GetDisplayParams(), idStr) &&
        SUCCESS == CreateTriMesh(primitiveJson, mesh, meshArgs, meshMaterial, idStr))
        {
        m_json["materials"][meshMaterial.GetName()] = materialJson;
        primitiveJson["material"] = meshMaterial.GetName();
        primitivesNode.append(primitiveJson);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void AppendPolylineToBufferView(MeshPolylineCR polyline, bool useShortIndices)
    {
    m_binaryData.Append(polyline.GetStartDistance());
    m_binaryData.Append(polyline.GetRangeCenter().x);
    m_binaryData.Append(polyline.GetRangeCenter().y);
    m_binaryData.Append(polyline.GetRangeCenter().z);
    m_binaryData.Append((uint32_t) polyline.GetIndices().size());
    for (auto& index : polyline.GetIndices())
        {
        if (useShortIndices)
            m_binaryData.Append((uint16_t) index);
        else
            m_binaryData.Append(index);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value AddPolylines(PolylineList const& polylines, size_t maxIndex, Utf8StringCR name, Utf8StringCR idStr) 
    {
    Utf8String          nameId = name + idStr;
    Utf8String          bufferViewId= "bv_" + nameId;
    Utf8String          accessorId   = "acc_" + nameId;
    Json::Value         bufferViewJson;
    size_t              bufferViewOffset = m_binaryData.size();
    bool                useShortIndices = maxIndex < 0xffff;

    bufferViewJson["buffer"] = "binary_glTF";
    bufferViewJson["byteOffset"] = (uint32_t) bufferViewOffset;

    for (auto& polyline : polylines)
        AppendPolylineToBufferView(polyline, useShortIndices);
    
    bufferViewJson["byteLength"] = m_binaryData.size() -  bufferViewOffset;
    m_json["bufferViews"][bufferViewId] = bufferViewJson;

    AddAccessor(useShortIndices ? GLTF_UNSIGNED_SHORT : GLTF_UINT32, accessorId, bufferViewId, polylines.size(), "PLINE");
    return accessorId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _AddMesh(Json::Value& primitivesNode, MeshCR mesh, size_t& index)
    { 
    GetMeshGraphicsArgs         graphicsArgs;
    bvector<Render::GraphicPtr> graphics;

    mesh.GetGraphics (graphics, m_renderSystem, graphicsArgs, m_model.GetDgnDb());

    AddTriMesh(primitivesNode, graphicsArgs.m_meshArgs, mesh, index);
//  AddPolyline(primitivesNode, graphicsArgs.m_polylineArgs, mesh, index);
    return SUCCESS;
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void AddMeshes(Render::Primitives::GeometryCollectionCR geometry)
    {
    Json::Value     meshes     = Json::objectValue;
    Json::Value     mesh       = Json::objectValue;
    Json::Value     nodes      = Json::objectValue;
    Json::Value     rootNode   = Json::objectValue;
    Json::Value     primitives = Json::arrayValue;
    Utf8String      meshName   = "Mesh";
    size_t          primitiveIndex = 0;

    for (auto& mesh : geometry.Meshes())
        _AddMesh(primitives, *mesh, primitiveIndex);

    mesh["primitives"] = primitives;
    meshes[meshName] = mesh;
    rootNode["meshes"].append (meshName);
    nodes["rootNode"] = rootNode;
    m_json["meshes"] = meshes;
    m_json["nodes"]  = nodes;
    }

};  // Writer

           
//=======================================================================================
// @bsistruct                                                   Ray.Bentley     06/2017
//=======================================================================================
struct GltfWriter : Writer
{

public:
    GltfWriter(StreamBufferR streamBuffer, DgnModelR model) : Writer(streamBuffer, model) { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WriteGltf(Render::Primitives::GeometryCollectionCR geometry, DPoint3dCR centroid)
    {
    AddExtensions(centroid);
    AddDefaultScene();
    AddMeshes (geometry);

    Utf8String  sceneStr = Json::FastWriter().write(m_json);
    uint32_t    sceneStrLength = static_cast<uint32_t>(sceneStr.size());

    long    startPosition =  m_buffer.GetSize();
    m_buffer.Append((const uint8_t*) s_gltfMagic, 4);
    m_buffer.Append(s_gltfVersion);
    long    lengthDataPosition = m_buffer.GetSize();
    m_buffer.Append((uint32_t) 0);
    m_buffer.Append((const uint8_t*) &sceneStrLength, sizeof(sceneStrLength));
    m_buffer.Append((const uint8_t*) &s_gltfSceneFormat, sizeof(s_gltfSceneFormat));
    m_buffer.Append((const uint8_t*) sceneStr.data(), sceneStrLength);
    if (!m_binaryData.empty())
        m_buffer.Append((const uint8_t*) m_binaryData.data(), BinaryDataSize());

    WriteLength(startPosition, lengthDataPosition);
    m_buffer.Append(m_binaryData.data(), m_binaryData.size());
    return SUCCESS;
    }


};  // GltfWriter

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     06/2017
//=======================================================================================
struct BatchedModelWriter : GltfWriter
{

public:
    BatchedModelWriter(StreamBufferR streamBuffer, DgnModelR model) : GltfWriter(streamBuffer, model) { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WriteTile(Render::Primitives::GeometryCollectionCR geometry, DPoint3dCR centroid)
    {
    Utf8String          batchTableStr = BatchTableBuilder (geometry.Meshes().FeatureTable(), m_model.GetDgnDb(), m_model.Is3d()).ToString();
    uint32_t            batchTableStrLen = static_cast<uint32_t>(batchTableStr.size());
    uint32_t            b3dmNumBatches = geometry.Meshes().FeatureTable().size();
    uint32_t            zero;

    uint32_t    startPosition = m_buffer.GetSize();
    m_buffer.Append((const uint8_t *) s_b3dmMagic, 4);
    m_buffer.Append(s_b3dmVersion);
    uint32_t    lengthDataPosition = m_buffer.GetSize();
    m_buffer.Append((uint32_t) 0);              // Filled in below.
    m_buffer.Append(batchTableStrLen);
    m_buffer.Append((uint32_t) 0); // length of binary portion of batch table - we have no binary batch table data
    m_buffer.Append(b3dmNumBatches);
    m_buffer.Append((const uint8_t *) batchTableStr.data(), batchTableStrLen);

    PadToBoundary();
    if (SUCCESS != WriteGltf (geometry, centroid))
        return ERROR;

    PadToBoundary ();
    WriteLength(startPosition, lengthDataPosition);

#ifdef DEBUG_TO_BATCHED_MODEL
    std::FILE* outputFile = fopen("d:\\tmp\\test.b3dm","wb");
    fwrite(m_buffer.GetDataP(), 1, m_buffer.GetSize(), outputFile);
    fclose(outputFile);
#endif
    return SUCCESS;
    }
};  // BatchedModelWriter


//=======================================================================================
// @bsistruct                                                   Ray.Bentley     06/2017
//=======================================================================================
struct DgnCacheTileWriter : GltfWriter
{

    DEFINE_T_SUPER(GltfWriter);

public:
    DgnCacheTileWriter(StreamBufferR streamBuffer, DgnModelR model) : GltfWriter(streamBuffer, model) { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value CreateColorTable(ColorTableCR colorTable)
    {
    Json::Value     jsonTable = Json::arrayValue;

    for (auto& entry : colorTable)
        jsonTable[entry.second] = entry.first;

    return jsonTable;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value CreateMeshEdges(MeshEdgesCR meshEdges, size_t maxIndex, Utf8StringCR idStr)
    {
    Json::Value     edgesValue= Json::objectValue;

    if (!meshEdges.m_visible.empty())
        edgesValue["visibles"] = AddMeshIndices("visibles", meshEdges.m_visible.front().m_indices, 2 * meshEdges.m_visible.size(), idStr, maxIndex);

    if (!meshEdges.m_silhouette.empty())
        {
        edgesValue["silhouettes"]["indices"]  = AddMeshIndices("silhouettes", meshEdges.m_silhouette.front().m_indices, 2 * meshEdges.m_silhouette.size(), idStr, maxIndex);
        edgesValue["silhouettes"]["normals0"] = AddNormals (meshEdges.m_silhouetteNormals0.data(), meshEdges.m_silhouetteNormals0.size(), "normals0", idStr.c_str());
        edgesValue["silhouettes"]["normals1"] = AddNormals (meshEdges.m_silhouetteNormals1.data(), meshEdges.m_silhouetteNormals1.size(), "normals1", idStr.c_str());
        }

    if (!meshEdges.m_polylines.empty())
        edgesValue["polylines"] = AddPolylines(meshEdges.m_polylines, maxIndex, "polyEdge", idStr);

    return edgesValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CreateTriMesh(Json::Value& primitiveJson, MeshCR mesh, Utf8StringCR idStr)
    {
    DisplayParamsCR     displayParams = mesh.GetDisplayParams();

    primitiveJson["mode"] = GLTF_TRIANGLES;

    if (!mesh.Params().empty() && displayParams.IsTextured())
        primitiveJson["attributes"]["TEXCOORD_0"] = AddParamAttribute (mesh.Params().data(), mesh.Params().size(), "Param", idStr.c_str());

    BeAssert(displayParams.IgnoresLighting() || !mesh.Normals().empty());

    if (!mesh.Normals().empty() && !displayParams.IgnoresLighting())        // No normals if ignoring lighting (reality meshes).
        primitiveJson["attributes"]["NORMAL"]  = AddQuantizedPointsAttribute(mesh.Normals().data(), mesh.Normals().size(), QPoint3d::Params::FromNormalizedRange(), "Normal", idStr.c_str());

    primitiveJson["indices"] = AddMeshTriangleIndices ("Indices", mesh.Triangles(), idStr, mesh.Points().size());

    if (mesh.GetEdges().IsValid())
        primitiveJson["edges"] = CreateMeshEdges(*mesh.GetEdges(), mesh.Points().size(), idStr);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CreatePolylines(Json::Value& primitiveJson, MeshCR mesh, Utf8StringCR idStr)
    {
    primitiveJson["mode"] = GLTF_LINES;
    primitiveJson["indices"] = AddPolylines(mesh.Polylines(), mesh.Points().size(), "polyline", idStr);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  CreateMaterialJson(Json::Value& matJson, MeshCR mesh,  DisplayParamsCR displayParams, Utf8StringCR suffix) 
    {
    matJson["type"] = (uint8_t) displayParams.GetType();

    // GeomParams...
    if (displayParams.GetCategoryId().IsValid())
        matJson["categoryId"] = displayParams.GetCategoryId().GetValue();
    
    if (displayParams.GetSubCategoryId().IsValid())
        matJson["subCategoryId"] = displayParams.GetSubCategoryId().GetValue();


    if (displayParams.GetMaterialId().IsValid())
        matJson["materialId"] = displayParams.GetMaterialId().GetValue();

    matJson["class"] = (uint16_t) displayParams.GetClass();

    // GraphicsParams...
    matJson["fillColor"] = displayParams.GetFillColor();
    matJson["fillFlags"] = static_cast<uint32_t> (displayParams.GetFillFlags());

    // Are these needed for meshes (Edges??)
    matJson["lineColor"]  = displayParams.GetLineColor();     // Edges?
    matJson["lineWidth"]  = displayParams.GetLineWidth();
    matJson["linePixels"] = (uint32_t) displayParams.GetLinePixels();     // Edges?
    

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void AddFeatures (MeshCR mesh, Json::Value& primitiveJson, Utf8StringCR idStr)
    {
    FeatureIndex    featureIndex;

    mesh.ToFeatureIndex(featureIndex);
    if(featureIndex.IsEmpty())
        BeAssert(false && "Empty feature index");
    else if (featureIndex.IsUniform())
        primitiveJson["featureID"]  = featureIndex.m_featureID;
    else
        primitiveJson["featureIDs"] = AddMeshIndices("featureIDs", featureIndex.m_featureIDs, mesh.Points().size(), idStr, mesh.Points().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _AddMesh(Json::Value& primitivesNode, MeshCR mesh, size_t& index)
    { 
    Utf8String          idStr(std::to_string(index++).c_str());
    Json::Value         materialJson = Json::objectValue, primitiveJson = Json::objectValue;

    if (SUCCESS != CreateMaterialJson(materialJson, mesh, mesh.GetDisplayParams(), idStr))
        return ERROR;

    Utf8String      accPositionId =  AddQuantizedPointsAttribute(mesh.Points().data(), mesh.Points().size(), mesh.Verts().GetParams(), "Position", idStr.c_str());
    primitiveJson["attributes"]["POSITION"] = accPositionId;

    if (!mesh.GetColorTable().IsUniform())
        AddMeshUInt16Attributes(primitiveJson, mesh.Colors().data(), mesh.Colors().size(), idStr, "ColorIndex_", "_COLORINDEX");

    AddFeatures (mesh, primitiveJson, idStr);
    AddMeshPointRange(m_json["accessors"][accPositionId], mesh.Verts().GetParams().GetRange());
 
    primitiveJson["colorTable"] = CreateColorTable(mesh.GetColorTable());

    if ((!mesh.Triangles().empty() && SUCCESS == CreateTriMesh(primitiveJson, mesh, idStr)) ||
        (!mesh.Polylines().empty() && SUCCESS == CreatePolylines(primitiveJson, mesh, idStr)))
        {
        Utf8String  materialName = "Material" + idStr;
        m_json["materials"][materialName] = materialJson;
        primitiveJson["material"] = materialName;
        primitivesNode.append(primitiveJson);
        
        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void WriteFeatureTable(FeatureTableCR featureTable)
    {
    uint32_t      startPosition = m_buffer.GetSize();
    m_buffer.Append((uint32_t) 0);              // Filled in below.

    m_buffer.Append(featureTable.GetMaxFeatures());
    m_buffer.Append((uint32_t) featureTable.size());
    for (auto& feature : featureTable)
        {
        m_buffer.Append(feature.first.GetElementId().GetValue());
        m_buffer.Append(feature.first.GetSubCategoryId().GetValue());
        m_buffer.Append(static_cast<uint32_t> (feature.first.GetClass()));
        m_buffer.Append(feature.second);
        }
    WriteLength(startPosition, startPosition);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus WriteTile(ElementAlignedBox3dCR contentRange, Render::Primitives::GeometryCollectionCR geometry, DPoint3dCR centroid)
    {
    uint32_t    startPosition = m_buffer.GetSize();
    uint32_t    flags = geometry.ContainsCurves() ? TileIO::ContainsCurves : TileIO::None;

    m_buffer.Append((const uint8_t *) s_dgnTileMagic, 4);
    m_buffer.Append(s_dgnTileVersion);
    m_buffer.Append(flags);
    m_buffer.Append(contentRange);

    uint32_t    lengthDataPosition = m_buffer.GetSize();
    m_buffer.Append((uint32_t) 0);              // Filled in below.
    WriteFeatureTable(geometry.Meshes().FeatureTable());
    PadToBoundary ();
    if (SUCCESS != WriteGltf (geometry, centroid))
        return ERROR;

    PadToBoundary ();
    WriteLength(startPosition, lengthDataPosition);

    return SUCCESS;
    }
};  // DgnCacheTileWriter

END_TILEWRITER_NAMESPACE
   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TileIO::Write3dTile(StreamBufferR streamBuffer, Render::Primitives::GeometryCollectionCR geometry, DgnModelR model, DPoint3dCR centroid)
    {
    return TileWriter::BatchedModelWriter(streamBuffer, model).WriteTile(geometry, centroid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TileIO::WriteDgnTile(StreamBufferR streamBuffer, ElementAlignedBox3dCR contentRange, Render::Primitives::GeometryCollectionCR geometry, DgnModelR model, DPoint3dCR centroid)
    {
    return TileWriter::DgnCacheTileWriter(streamBuffer, model).WriteTile(contentRange, geometry, centroid);
    }



