/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/Render.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Render::Target::VerifyRenderThread() {DgnDb::VerifyRenderThread();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
BSIRect Render::Target::SetAspectRatio(BSIRectCR requestedRect, double targetAspectRatio)
    {
    BSIRect rect = requestedRect;

    if (targetAspectRatio >= 1.0)
        {
        double requestedWidth = rect.Width();
        double requiredWidth = rect.Height() * targetAspectRatio;
        double adj = requiredWidth - requestedWidth;
        rect.Inset((int)(-adj/2.0), 0);
        }
    else
        {
        double requestedHeight = rect.Height();
        double requiredHeight = rect.Width() / targetAspectRatio;
        double adj = requiredHeight - requestedHeight;
        rect.Inset(0, (int)(-adj/2.0));
        }

    return rect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Render::Target::DestroyNow()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewport::SetRenderTarget(Target* newTarget)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2016
//---------------------------------------------------------------------------------------
uint32_t DgnViewport::SetMinimumTargetFrameRate(uint32_t frameRate)
    {
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void FrustumPlanes::Init(FrustumCR frustum)
    {
    m_isValid = true;
    ClipUtil::RangePlanesFromFrustum(m_planes, frustum, true, true, 1.0E-6);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/16
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
* @bsimethod                                    Keith.Bentley                   02/16
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
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void floatToDouble(double* pDouble, float const* pFloat, size_t n)
    {
    for (double* pEnd = pDouble + n; pDouble < pEnd; )
        *pDouble++ = *pFloat++;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static void toDPoints(T& dpts, QPoint3dCP qpts, QPoint3d::ParamsCR qparams, int32_t numPoints)
    {
    dpts.resize(numPoints);
    for (int32_t i = 0; i < numPoints; i++)
        dpts[i] = qpts[i].UnquantizeAsVector(qparams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void toNormals(BlockedVectorDVec3dR normals, OctEncodedNormalCP encoded, int32_t count)
    {
    normals.resize(count);
    for (int32_t i = 0; i < count; i++)
        normals[i] = encoded[i].Decode();
    }

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr TriMeshArgs::ToPolyface() const
    {
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

/*-----------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Transform Render::TextureMapping::Trans2x3::GetTransform() const
    {
    Transform transform = Transform::FromIdentity();

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
* @bsimethod                                    Keith.Bentley                   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value Render::HiddenLineParams::Style::ToJson() const
    {
    Json::Value val;
    val[json_ovrColor()] = m_ovrColor;
    val[json_color()] = m_color.GetValue();
    val[json_pattern()] = (uint32_t) m_pattern;
    val[json_width()] = (uint32_t) m_width;
    return val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Render::HiddenLineParams::Style::FromJson(JsonValueCR val)
    {
    m_ovrColor = val[json_ovrColor()].asBool(m_ovrColor);
    m_color = ColorDef(val[json_color()].asUInt(m_color.GetValue()));
    m_pattern = (LinePixels) val[json_pattern()].asUInt((uint32_t) m_pattern);
    m_width = val[json_width()].asUInt(m_width);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value Render::HiddenLineParams::ToJson() const
    {
    HiddenLineParams defaults;
    Json::Value val;

    if (m_visible != defaults.m_visible) val[json_visible()] = m_visible.ToJson();
    if (m_hidden != defaults.m_hidden) val[json_hidden()] = m_hidden.ToJson();
    if (m_transparencyThreshold != defaults.m_transparencyThreshold) val[json_transThreshold()] = m_transparencyThreshold;
    return val;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::HiddenLineParams Render::HiddenLineParams::FromJson(JsonValueCR val)
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
* @bsimethod                                                    Paul.Connelly   02/17
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
* @bsimethod                                                    Ray.Bentley     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool MeshEdge::operator<(MeshEdge const& rhs) const
    {
    return m_indices[0] == rhs.m_indices[0] ? (m_indices[1] < rhs.m_indices[1]) :  (m_indices[0] < rhs.m_indices[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool EdgeArgs::Init(MeshEdgesCR meshEdges)
    {
    auto const& visible = meshEdges.m_visible;
    if (visible.empty())
        return false;

    m_edges = visible.data();
    m_numEdges = visible.size();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool SilhouetteEdgeArgs::Init(MeshEdgesCR meshEdges)
    {
    auto const& silhouette = meshEdges.m_silhouette;
    if (silhouette.empty())
        return false;

    m_edges = silhouette.data();
    m_numEdges = silhouette.size();
    m_normals = meshEdges.m_silhouetteNormals.data();

    return true;
    }

#if !defined(NDEBUG)
/*---------------------------------------------------------------------------------**//**
* This lives here because (1) annoying out-of-sync headers on rebuild and (2) want to
* catch the actual delta when assertion triggered. Only used in non-optimized builds.
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
void OctEncodedNormal::VerifyNormalized(DVec3dCR vec)
    {
    auto magSq = vec.MagnitudeSquared();
    bool normalized = DoubleOps::WithinTolerance(magSq, 1.0, 0.001);
    BeAssert(normalized);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
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
* @bsimethod                                                    Paul.Connelly   09/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GraphicBuilder::_WantPreBakedBody(IBRepEntityCR body)
    {
#if defined (BENTLEYCONFIG_PARASOLID)
    return !BRepUtil::HasCurvedFaceOrEdge(body);
#else
    return true;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicBuilder::CreateParams::CreateParams(DgnViewportR vp, TransformCR tf, GraphicType type)
    : CreateParams(vp.GetViewController().GetDgnDb(), tf, &vp, type)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicBuilder::CreateParams::CreateParams(DgnDbR db, TransformCR tf, DgnViewportP vp, GraphicType type)
    : m_dgndb(db), m_placement(tf), m_viewport(vp), m_type(type)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
Material::CreateParams::CreateParams(MaterialKeyCR key, RenderingAssetCR asset, DgnDbR db, SystemCR sys, TextureP pTexture) : m_key(key)
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

    if (asset.GetBool(RENDER_MATERIAL_FlagHasTransmit, false))
        m_transparency = asset.GetDouble(RENDER_MATERIAL_Transmit, 0.0);

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

    auto const& texMap = asset.GetPatternMap();
    TexturePtr texture(pTexture);
    if (texture.IsNull() && texMap.IsValid() && texMap.GetTextureId().IsValid())
        texture = sys._GetTexture(texMap.GetTextureId(), db);

    if (texture.IsValid())
        m_textureMapping = TextureMapping(*texture, texMap.GetTextureMapParams());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
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
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialPtr System::_GetMaterial(RenderMaterialId id, DgnDbR db) const
    {
    MaterialKey name(id);
    MaterialPtr mat = _FindMaterial(name, db);
    if (mat.IsNull())
        {
        RenderMaterialCPtr matElem = RenderMaterial::Get(db, id);
        RenderingAssetCP asset = matElem.IsValid() ? &matElem->GetRenderingAsset() : nullptr;
        if (nullptr != asset)
            mat = _CreateMaterial(Material::CreateParams(name, *asset, db, *this), db);
        }

    return mat;
    }

