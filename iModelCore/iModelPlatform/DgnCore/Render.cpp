/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"

// for Image Scale
#define STBIR_DEFAULT_FILTER_DOWNSAMPLE STBIR_FILTER_CATMULLROM // this works better than default (STBIR_FILTER_MITCHELL)
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "DgnCore/stb_image_resize.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void FrustumPlanes::Init(FrustumCR frustum)
    {
    m_isValid = true;
    ClipUtil::RangePlanesFromFrustum(m_planes, frustum, true, true, 1.0E-6);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
FrustumPlanes::Contained FrustumPlanes::Contains(DPoint3dCP points, int nPts, double tolerance) const
    {
    BeAssert(IsValid());

    bool allInside = true;
    for (ClipPlaneCR plane : m_planes)
        {
        int nOutside = 0;
        for (int j=0; j < nPts; ++j)
            {
            if (plane.EvaluatePoint(points[j]) + tolerance < 0.0)
                {
                ++nOutside;
                allInside = false;
                }
            }

        if (nOutside == nPts)
            return Contained::Outside;
        }

    return allInside ? Contained::Inside : Contained::Partly;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool FrustumPlanes::IntersectsRay(DPoint3dCR origin, DVec3dCR direction)
    {
    double tFar  =  1e37;
    double tNear = -1e37;

    for (ClipPlaneCR plane : m_planes)
        {
        double vD = plane.DotProduct(direction), vN = plane.EvaluatePoint(origin);

        if (0.0 == vD)
            {
            // Ray is parallel... No need to continue testing if outside halfspace.
            if (vN < 0.0)
                return false;
            }
        else
            {
            double      rayDistance = -vN / vD;

            if (vD < 0.0)
                {
                if (rayDistance < tFar)
                   tFar = rayDistance;
                }
            else
                {
                if (rayDistance > tNear)
                    tNear = rayDistance;
                }
            }
        }

    return tNear <= tFar;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void floatToDouble(double* pDouble, float const* pFloat, size_t n)
    {
    for (double* pEnd = pDouble + n; pDouble < pEnd; )
        *pDouble++ = *pFloat++;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static void toDPoints(T& dpts, QPoint3dCP qpts, QPoint3d::ParamsCR qparams, int32_t numPoints)
    {
    dpts.resize(numPoints);
    for (int32_t i = 0; i < numPoints; i++)
        dpts[i] = qpts[i].UnquantizeAsVector(qparams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void toNormals(BlockedVectorDVec3dR normals, OctEncodedNormalCP encoded, int32_t count)
    {
    normals.resize(count);
    for (int32_t i = 0; i < count; i++)
        normals[i] = encoded[i].Decode();
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr TriMeshArgs::ToPolyface() const
    {
    if (!IsValid())
        return nullptr;

    PolyfaceHeaderPtr polyFace = PolyfaceHeader::CreateFixedBlockIndexed(3);

    BlockedVectorIntR pointIndex = polyFace->PointIndex();
    pointIndex.resize(m_numIndices);
    uint32_t const* pIndex = m_vertIndex;
    uint32_t const* pEnd = pIndex + m_numIndices;
    int* pOut = &pointIndex.front();

    for (; pIndex < pEnd; )
        *pOut++ = 1 + *pIndex++;

    if (nullptr != m_points)
        toDPoints(polyFace->Point(), m_points, m_pointParams, m_numPoints);

    if (nullptr != m_normals)
        toNormals(polyFace->Normal(), m_normals, m_numPoints);

    if (nullptr != m_textureUV)
        {
        polyFace->Param().resize(m_numPoints);
        floatToDouble(&polyFace->Param().front().x, &m_textureUV->x, 2 * m_numPoints);
        polyFace->ParamIndex() = pointIndex;
        }

    return polyFace;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool TriMeshArgs::IsValid() const
    {
    return 0 != m_numIndices && nullptr != m_vertIndex
        && 0 != m_numPoints && nullptr != m_points;
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Transform Render::TextureMapping::Trans2x3::GetTransform() const
    {
    auto transform = Transform::FromIdentity();

    for (size_t i=0; i<2; ++i)
        {
        for (size_t j=0; j<2; ++j)
            transform.form3d[i][j] = m_val[i][j];
        }

    transform.form3d[0][3] = m_val[0][2];
    transform.form3d[1][3] = m_val[1][2];
    return transform;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool Render::TextureMapping::Trans2x3::AlmostEqual(Trans2x3 const& rhs) const
    {
    auto tol = DoubleOps::SmallCoordinateRelTol();
    for (auto i = 0; i < 2; i++)
        for (auto j = 0; j < 3; j++)
            if (fabs(m_val[i][j] - rhs.m_val[i][j]) < tol)
                return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Render::TextureMapping::Trans2x3::ToJson(BeJsValue json) const
    {
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 3; j++)
            json[i][j] = m_val[i][j];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Render::TextureMapping::Trans2x3 Render::TextureMapping::Trans2x3::FromJson(BeJsConst json)
    {
    return Trans2x3(json[0][0].asDouble(), json[0][1].asDouble(), json[0][2].asDouble(),
                    json[1][0].asDouble(), json[1][1].asDouble(), json[1][2].asDouble());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Render::TextureMapping::Trans2x3 Render::TextureMapping::Trans2x3::FromTransform(TransformCR trans)
    {
    return Trans2x3(trans.form3d[0][0], trans.form3d[0][1], trans.form3d[0][3],
                    trans.form3d[1][0], trans.form3d[1][1], trans.form3d[1][3]);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool Render::MaterialUVDetail::IsEquivalent(MaterialUVDetailCR rhs) const
    {
    if (m_type != rhs.m_type)
        return false;

    switch (m_type)
        {
        case Type::None: return true;
        case Type::Projection: return true; // ###TODO...
        case Type::Transform: return m_transform.AlmostEqual(rhs.m_transform);
        default: BeAssert(false); return true;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Render::HiddenLineParams::Style::ToJson(BeJsValue val) const
    {
    val.SetEmptyObject();
    val[json_ovrColor()] = m_ovrColor;
    val[json_color()] = m_color.GetValue();
    val[json_pattern()] = (uint32_t) m_pattern;
    val[json_width()] = (uint32_t) m_width;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Render::HiddenLineParams::Style::FromJson(BeJsConst val)
    {
    m_ovrColor = val[json_ovrColor()].asBool(m_ovrColor);
    m_color = ColorDef(val[json_color()].asUInt(m_color.GetValue()));
    m_pattern = (LinePixels) val[json_pattern()].asUInt((uint32_t) m_pattern);
    m_width = val[json_width()].asUInt(m_width);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Render::HiddenLineParams::ToJson(BeJsValue val) const
    {
    val.SetNull();

    HiddenLineParams defaults;
    if (m_visible != defaults.m_visible) m_visible.ToJson(val[json_visible()]);
    if (m_hidden != defaults.m_hidden) m_hidden.ToJson(val[json_hidden()]);
    if (m_transparencyThreshold != defaults.m_transparencyThreshold) val[json_transThreshold()] = m_transparencyThreshold;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Render::HiddenLineParams Render::HiddenLineParams::FromJson(BeJsConst val)
    {
    HiddenLineParams params;

    if (val.isObject())
        {
        params.m_visible.FromJson(val[json_visible()]);
        params.m_hidden.FromJson(val[json_hidden()]);
        params.m_transparencyThreshold = val[json_transThreshold()].asDouble(params.m_transparencyThreshold);
        }
    return params;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Render::AmbientOcclusionParams::ToJson(BeJsValue json) const
    {
    json.SetNull();
    AmbientOcclusionParams defaults;

    if (m_bias != defaults.m_bias) json[json_bias()] = m_bias;
    if (m_zLengthCap != defaults.m_zLengthCap) json[json_zLengthCap()] = m_zLengthCap;
    if (m_intensity != defaults.m_intensity) json[json_intensity()] = m_intensity;
    if (m_texelStepSize != defaults.m_texelStepSize) json[json_texelStepSize()] = m_texelStepSize;
    if (m_blurDelta != defaults.m_blurDelta) json[json_blurDelta()] = m_blurDelta;
    if (m_blurSigma != defaults.m_blurSigma) json[json_blurSigma()] = m_blurSigma;
    if (m_blurTexelStepSize != defaults.m_blurTexelStepSize) json[json_blurTexelStepSize()] = m_blurTexelStepSize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Render::AmbientOcclusionParams Render::AmbientOcclusionParams::FromJson(BeJsConst json)
    {
    AmbientOcclusionParams params;
    if (json.isObject())
        {
        params.m_bias = json[json_bias()].asDouble(params.m_bias);
        params.m_zLengthCap = json[json_zLengthCap()].asDouble(params.m_zLengthCap);
        params.m_intensity = json[json_intensity()].asDouble(params.m_intensity);
        params.m_texelStepSize = json[json_texelStepSize()].asDouble(params.m_texelStepSize);
        params.m_blurDelta = json[json_blurDelta()].asDouble(params.m_blurDelta);
        params.m_blurSigma = json[json_blurSigma()].asDouble(params.m_blurSigma);
        params.m_blurTexelStepSize = json[json_blurTexelStepSize()].asDouble(params.m_blurTexelStepSize);
        }

    return params;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool Feature::operator<(FeatureCR rhs) const
    {
    if (GetElementId() != rhs.GetElementId())
        return GetElementId() < rhs.GetElementId();
    else if (GetSubCategoryId() != rhs.GetSubCategoryId())
        return GetSubCategoryId() < rhs.GetSubCategoryId();
    else if (GetClass() != rhs.GetClass())
        return static_cast<uint8_t>(GetClass()) < static_cast<uint8_t>(rhs.GetClass());
    else
        return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool MeshEdge::operator<(MeshEdge const& rhs) const
    {
    return m_indices[0] == rhs.m_indices[0] ? (m_indices[1] < rhs.m_indices[1]) :  (m_indices[0] < rhs.m_indices[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool EdgeArgs::Init(MeshEdgesCR meshEdges)
    {
    auto const& visible = meshEdges.m_visible;
    if (visible.empty())
        return false;

    m_edges = visible.data();
    m_numEdges = static_cast<uint32_t>(visible.size());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SilhouetteEdgeArgs::Init(MeshEdgesCR meshEdges)
    {
    auto const& silhouette = meshEdges.m_silhouette;
    if (silhouette.empty())
        return false;

    m_edges = silhouette.data();
    m_numEdges = static_cast<uint32_t>(silhouette.size());
    m_normals = meshEdges.m_silhouetteNormals.data();

    return true;
    }

#if !defined(NDEBUG)
/*---------------------------------------------------------------------------------**//**
* This lives here because (1) annoying out-of-sync headers on rebuild and (2) want to
* catch the actual delta when assertion triggered. Only used in non-optimized builds.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void OctEncodedNormal::VerifyNormalized(DVec3dCR vec)
    {
    auto magSq = vec.MagnitudeSquared();
    bool normalized = DoubleOps::WithinTolerance(magSq, 1.0, 0.001);
    BeAssert(normalized);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void OctEncodedNormal::VerifyEncoded(uint16_t val, DVec3dCR in)
    {
    OctEncodedNormal enc = OctEncodedNormal::From(val);
    DVec3d out = enc.Decode();
    bool vecEqual = in.IsEqual(out, 0.05);
    BeAssert(vecEqual);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicBuilder::CreateParams::CreateParams(DgnDbR db, TransformCR tf, GraphicType type)
    : m_dgndb(db), m_placement(tf), m_type(type)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Material::CreateParams::CreateParams(MaterialKeyCR key, RenderingAssetCR asset, DgnDbR db, SystemCR sys) : m_key(key)
    {
// #define DEBUG_JSON_CONTENT
#if defined(DEBUG_JSON_CONTENT)
    Utf8String string = Json::FastWriter().write(asset);
    UNUSED_VARIABLE(string);
#endif

    if (asset.GetBool(RENDER_MATERIAL_FlagHasBaseColor, false))
        {
        RgbFactor rgb = asset.GetColor(RENDER_MATERIAL_Color);
        m_diffuseColor = MatColor(ColorUtil::FromRgbFactor(rgb));
        }

    if (asset.GetBool(RENDER_MATERIAL_FlagHasSpecularColor, false))
        {
        RgbFactor rgb = asset.GetColor(RENDER_MATERIAL_SpecularColor);
        m_specularColor = MatColor(ColorUtil::FromRgbFactor(rgb));
        }

    if (asset.GetBool(RENDER_MATERIAL_FlagHasFinish, false))
        m_specularExponent = asset.GetDouble(RENDER_MATERIAL_Finish, Defaults::SpecularExponent());

    auto transparency = asset.GetBool(RENDER_MATERIAL_FlagHasTransmit, false) ? asset.GetDouble(RENDER_MATERIAL_Transmit, 0.0) : 0.0;
    m_transparency.SetValue(transparency);

    if (asset.GetBool(RENDER_MATERIAL_FlagHasDiffuse, false))
        m_diffuse = asset.GetDouble(RENDER_MATERIAL_Diffuse, Defaults::Diffuse());

    if (asset.GetBool(RENDER_MATERIAL_FlagHasSpecular, false))
        m_specular = asset.GetDouble(RENDER_MATERIAL_Specular, Defaults::Specular());
    else
        m_specular = 0.0;     // Lack of specular means 0.0 -- not default (painting overspecular in PhotoRealistic Rendering

    if (asset.GetBool(RENDER_MATERIAL_FlagHasReflect, false))
        {
        // Reflectance stored as fraction of specular in V8 material settings.
        m_reflect = m_specular * asset.GetDouble(RENDER_MATERIAL_Reflect, Defaults::Reflect());
        }

    if (asset.GetBool(RENDER_MATERIAL_FlagHasReflectColor, false))
        {
        RgbFactor rgb = asset.GetColor(RENDER_MATERIAL_ReflectColor);
        m_reflectColor = MatColor(ColorUtil::FromRgbFactor(rgb));
        }
    else
        {
        m_reflectColor = m_specularColor;       // Linked...
        }

    auto const& patternMap = asset.GetPatternMap();
    auto const& normalMap = asset.GetNormalMap();
    auto havePatternMap = patternMap.IsValid() && patternMap.IsPatternEnabled() && patternMap.GetTextureId().IsValid();
    auto haveNormalMap = normalMap.IsValid() && normalMap.IsPatternEnabled() && normalMap.GetTextureId().IsValid();
    if (!havePatternMap && !haveNormalMap)
      return;

    // Make sure we can obtain the needed texture(s).
    TexturePtr patternTexture;
    if (havePatternMap) {
      patternTexture = sys._GetTexture(patternMap.GetTextureId(), db);
      havePatternMap = patternTexture.IsValid();
    }

    TexturePtr normalTexture;
    if (haveNormalMap) {
      normalTexture = sys._GetTexture(normalMap.GetTextureId(), db);
      haveNormalMap = normalTexture.IsValid();
    }

    if (!haveNormalMap && !havePatternMap)
      return;

    Nullable<TextureMapping::NormalMapParams> normalMapParams;
    if (haveNormalMap) {
      double normalScale = asset.GetNormalScale();
      bool invertGreen = 0 != (normalMap.GetUint32(RENDER_MATERIAL_NormalFlags) & RenderingAsset::TextureMap::NormalFlags::InvertGreen);
      TextureCP pNormalTexture = havePatternMap ? normalTexture.get() : nullptr;
      normalMapParams = TextureMapping::NormalMapParams(pNormalTexture, normalScale, invertGreen);
    }

    // MicroStation's material editor permits the texture mapping mode, scales, etc etc to be configured per map, but
    // making changes to any one map affects all maps.
    // We will assume all of the maps have the same parameters. We've only got one set of UVs anyway.
    auto textureMapParams = havePatternMap ? patternMap.GetTextureMapParams() : normalMap.GetTextureMapParams();
    m_textureMapping = TextureMapping(havePatternMap ? *patternTexture : *normalTexture, textureMapParams, normalMapParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TexturePtr System::_GetTexture(DgnTextureId id, DgnDbR db) const
    {
    TextureKey name(id);
    TexturePtr tx = _FindTexture(name, db);
    if (tx.IsNull())
        {
        DgnTextureCPtr txElem = DgnTexture::Get(db, id);
        if (txElem.IsValid())
            tx = _CreateTexture(txElem->GetImageSource(), Image::BottomUp::No, db, Texture::CreateParams(name));
        }

    return tx;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialPtr System::_GetMaterial(RenderMaterialId id, DgnDbR db) const
    {
    MaterialKey name(id);
    MaterialPtr mat = _FindMaterial(name, db);
    if (mat.IsNull())    {
        RenderMaterialCPtr matElem = RenderMaterial::Get(db, id);
        if (matElem.IsValid()) {
            auto asset = matElem->GetRenderingAsset();
            mat = _CreateMaterial(Material::CreateParams(name, asset, db, *this), db);
        }
    }

    return mat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PackedFeatureTable FeatureTable::Pack() const
    {
    ByteStream bytes(static_cast<uint32_t>(PackedFeature::PackedSize() * m_map.size()));
    bmap<DgnSubCategoryId, uint32_t> subCategories;
    auto getSubCategoryIndex = [&](DgnSubCategoryId id)
        {
        auto nextIndex = static_cast<uint32_t>(subCategories.size());
        auto iter = subCategories.Insert(id, nextIndex);
        return iter.first->second;
        };

    for (auto const& kvp : m_map)
        {
        FeatureCR feature = kvp.first;
        uint32_t featureIndex = kvp.second;
        PackedFeature packedFeature(feature.GetElementId(), getSubCategoryIndex(feature.GetSubCategoryId()), feature.GetClass());
        size_t byteOffset = featureIndex * PackedFeature::PackedSize();
        memcpy(bytes.data() + byteOffset, &packedFeature, PackedFeature::PackedSize());
        }

    size_t subCategoriesOffset = bytes.size();
    size_t nSubCategoryBytes = subCategories.size() * sizeof(uint64_t);
    bytes.resize(bytes.size() + nSubCategoryBytes);
    for (auto const& kvp : subCategories)
        {
        uint64_t id = kvp.first.GetValueUnchecked();
        size_t byteIndex = kvp.second * sizeof(id);
        memcpy(bytes.data() + subCategoriesOffset + byteIndex, &id, sizeof(id));
        }

    return PackedFeatureTable(std::move(bytes), GetNumIndices(), GetModelId(), GetMaxFeatures());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
FeatureTable PackedFeatureTable::Unpack() const
    {
    FeatureTable table(GetModelId(), GetMaxFeatures());
    for (uint32_t i = 0; i < GetNumFeatures(); i++)
        table.Insert(GetFeature(i), i);

    return table;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CompareResult Material::CreateParams::MatColor::Compare(MatColor const& other) const
    {
    if (IsValid() != other.IsValid())
        return IsValid() ? CompareResult::Greater : CompareResult::Less;
    else if (!IsValid() || m_value.GetValueNoAlpha() == other.m_value.GetValueNoAlpha())
        return CompareResult::Equal;
    else
        return m_value.GetValueNoAlpha() < other.m_value.GetValueNoAlpha() ? CompareResult::Less : CompareResult::Greater;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Image Image::Scale(ImageCR image, uint32_t outWidth, uint32_t outHeight)
    {
    outWidth = std::max((uint32_t) 1, outWidth);    // Zero dimensions should never exist - but avoid crash if they do.
    outHeight = std::max((uint32_t) 1, outHeight);

    int            bytesPerPixel = image.GetBytesPerPixel();
    uint8_t const* inP  = image.GetByteStream().GetData();
    ByteStream     outImageBytes(outWidth * outHeight * bytesPerPixel);
    uint8_t*       outP = reinterpret_cast <uint8_t *> (outImageBytes.GetDataP());

    stbir_resize_uint8 (inP, image.GetWidth(), image.GetHeight(), 0, outP, outWidth, outHeight, 0, bytesPerPixel);

    return Image(outWidth, outHeight, std::move(outImageBytes), image.GetFormat());
    }

