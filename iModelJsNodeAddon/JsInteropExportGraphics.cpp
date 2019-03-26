/*--------------------------------------------------------------------------------------+
|
|     $Source: JsInteropExportGraphics.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "IModelJsNative.h"
#include <DgnPlatform/SimplifyGraphic.h>

using namespace IModelJsNative;

//=======================================================================================
// @bsistruct                                                   Matt.Gooding    02/19
//=======================================================================================
struct ExportGraphicsMesh
{
    bvector<int>        m_indices;
    bvector<DPoint3d>   m_points;
    bvector<FPoint3d>   m_normals;
    bvector<FPoint2d>   m_params;

    void Clear()
        {
        m_indices.clear();
        m_points.clear();
        m_normals.clear();
        m_params.clear();
        }

    void Add(ExportGraphicsMesh const& rhs)
        {
        int32_t pointOffset = (int32_t)m_points.size();
        if (pointOffset == 0)
            {
            m_indices = rhs.m_indices;
            m_points = rhs.m_points;
            m_normals = rhs.m_normals;
            m_params = rhs.m_params;
            return;
            }

        m_indices.reserve(m_indices.size() + rhs.m_indices.size());
        for (int i = 0; i < (int)rhs.m_indices.size(); ++i)
            m_indices.push_back(rhs.m_indices[i] + pointOffset);

        m_points.resize(m_points.size() + rhs.m_points.size());
        memcpy (&m_points[pointOffset], &rhs.m_points[0], sizeof(DPoint3d) * rhs.m_points.size());

        m_normals.resize(m_normals.size() + rhs.m_normals.size());
        memcpy (&m_normals[pointOffset], &rhs.m_normals[0], sizeof(FPoint3d) * rhs.m_normals.size());

        m_params.resize(m_params.size() + rhs.m_params.size());
        memcpy (&m_params[pointOffset], &rhs.m_params[0], sizeof(FPoint2d) * rhs.m_params.size());
        }
};

//=======================================================================================
// @bsistruct                                                   Matt.Gooding    02/19
//=======================================================================================
struct FastVertexUnifier
{
    struct NodeEntry
    {
        int32_t normal; int32_t newIndex; FPoint2d param;
        NodeEntry() : normal(-1), newIndex(-1) { }
    };

    struct Node // each node is 64 bytes for cache alignment
    {
        constexpr static uint32_t ENTRY_COUNT = 4;

        NodeEntry entries[ENTRY_COUNT];
    };

    bvector<Node>               m_remapper;

    // Indices are zero-based, no blocking, no sign for edge visibility
    bvector<DPoint3d>           m_inPoints;
    bvector<int32_t>            m_inPointIndices;
    bvector<FPoint3d>           m_inNormals;
    bvector<int32_t>            m_inNormalIndices;
    // No index array, already unified with point indices
    bvector<FPoint2d>           m_inParams;

    // Unified indices + corresponding data
    ExportGraphicsMesh          m_output;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    02/19
+---------------+---------------+---------------+---------------+---------------+------*/
void Clear()
    {
    m_remapper.clear();

    m_inPoints.clear();
    m_inPointIndices.clear();
    m_inNormals.clear();
    m_inNormalIndices.clear();
    m_inParams.clear();

    m_output.Clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    02/19
* Linear speed unification of vertex indices. Each point has ENTRY_COUNT remappings
* that can be saved. After that, just add additional vertices for each normal/param
* combination - ROI on further remapping is poor.
+---------------+---------------+---------------+---------------+---------------+------*/
void Unify()
    {
    m_remapper.resize(m_inPoints.size());

    // Reserve for at least best case compression
    m_output.m_points.reserve(m_inPoints.size());
    m_output.m_normals.reserve(m_inPoints.size());
    m_output.m_params.reserve(m_inPoints.size());

    // Index count will remain the same
    m_output.m_indices.resize(m_inPointIndices.size());

    const float UV_TOLERANCE = 0.01f;

    for (int i = 0; i < (int)m_inPointIndices.size(); ++i)
        {
        int32_t origPointIndex = m_inPointIndices[i];
        int32_t origNormalIndex = m_inNormalIndices[i];
        FPoint2d origParam = m_inParams[i];

        bool foundRemap = false;
        for (int j = 0; j < Node::ENTRY_COUNT; ++j)
            {
            NodeEntry& entry = m_remapper[origPointIndex].entries[j];
            if (entry.newIndex == -1) // unused entry
                {
                entry.normal = origNormalIndex;
                entry.param = origParam;
                entry.newIndex = (int32_t) m_output.m_points.size();
                m_output.m_indices[i] = entry.newIndex;
                m_output.m_points.push_back(m_inPoints[origPointIndex]);
                m_output.m_normals.push_back(m_inNormals[origNormalIndex]);
                m_output.m_params.push_back(origParam);
                foundRemap = true;
                break;
                }
            else if (entry.normal == origNormalIndex &&
                     abs(entry.param.x - origParam.x) < UV_TOLERANCE &&
                     abs(entry.param.y - origParam.y) < UV_TOLERANCE)
                {
                // reuse entry
                m_output.m_indices[i] = entry.newIndex;
                foundRemap = true;
                break;
                }
            }

        // Node for this point is full, just add another vertex.
        if (!foundRemap)
            {
            m_output.m_indices[i] = (int32_t) m_output.m_points.size();
            m_output.m_points.push_back(m_inPoints[origPointIndex]);
            m_output.m_normals.push_back(m_inNormals[origNormalIndex]);
            m_output.m_params.push_back(origParam);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    02/19
+---------------+---------------+---------------+---------------+---------------+------*/
void SetInput(PolyfaceQueryCR pf, TransformCR transform, FPoint2dCP srcParams)
    {
    bool isIdentity = transform.IsIdentity();
    bool isMirror = !isIdentity && transform.Determinant() < 0;

    uint32_t indexCount = (uint32_t) pf.GetPointIndexCount();
    uint32_t indexCountWithoutBlocking = (indexCount / 4) * 3; // no blocking in output
    m_inPointIndices.resize(indexCountWithoutBlocking); 
    m_inNormalIndices.resize(indexCountWithoutBlocking);

    // Account for potential winding order change with mirroring transform
    uint32_t indexOffset1 = isMirror ? 2 : 1;
    uint32_t indexOffset2 = isMirror ? 1 : 2;

    // Remove blocking, switch to zero-index, discard edge visibility
    int32_t const* srcPointIndices = pf.GetPointIndexCP();
    for (uint32_t dst = 0, src = 0; src < indexCount; src += 4, dst += 3)
        {
        m_inPointIndices[dst] = abs(srcPointIndices[src]) - 1;
        m_inPointIndices[dst + 1] = abs(srcPointIndices[src + indexOffset1]) - 1;
        m_inPointIndices[dst + 2] = abs(srcPointIndices[src + indexOffset2]) - 1;
        }

    // Remove blocking, switch to zero-index (no sign visibility on normals)
    int32_t const* srcNormalIndices = pf.GetNormalIndexCP();
    for (uint32_t dst = 0, src = 0; src < indexCount; src += 4, dst += 3)
        {
        m_inNormalIndices[dst] = srcNormalIndices[src] - 1;
        m_inNormalIndices[dst + 1] = srcNormalIndices[src + indexOffset1] - 1;
        m_inNormalIndices[dst + 2] = srcNormalIndices[src + indexOffset2] - 1;
        }

    m_inParams.resize(indexCountWithoutBlocking);
    for (uint32_t i = 0; i < indexCountWithoutBlocking; i += 3)
        {
        m_inParams[i] = srcParams[i];
        m_inParams[i + 1] = srcParams[i + indexOffset1];
        m_inParams[i + 2] = srcParams[i + indexOffset2];
        }

    DPoint3dCP srcPoints = pf.GetPointCP();
    uint32_t pointCount = (uint32_t) pf.GetPointCount();
    m_inPoints.resize(pointCount);
    memcpy (&m_inPoints[0], srcPoints, sizeof(DPoint3d) * pointCount);
    if (!isIdentity)
        transform.Multiply(&m_inPoints[0], (int)pointCount);

    RotMatrix normalMatrix = RotMatrix::From(transform);
    isIdentity = normalMatrix.IsIdentity(); // check if it was just translation
    if (!isIdentity)
        normalMatrix.Invert();
    DPoint3dCP srcNormals = pf.GetNormalCP();
    uint32_t normalCount = (uint32_t) pf.GetNormalCount();
    m_inNormals.resize(normalCount);
    for (uint32_t i = 0; i < normalCount; ++i)
        {
        DPoint3d tmpNormal = srcNormals[i];
        if (!isIdentity)
            {
            normalMatrix.MultiplyTranspose(tmpNormal);
            tmpNormal.Normalize();
            }
        m_inNormals[i] = FPoint3d::From(tmpNormal);
        }
    }
}; // FastVertexUnifier

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    02/19
+---------------+---------------+---------------+---------------+---------------+------*/
static Napi::Value convertMesh(Napi::Env env, ExportGraphicsMesh& mesh)
    {
    Napi::Object convertedMesh = Napi::Object::New(env);

    Napi::Int32Array indexArray = Napi::Int32Array::New(env, (uint32_t)mesh.m_indices.size());
    memcpy (indexArray.Data(), &mesh.m_indices[0], mesh.m_indices.size() * sizeof(int32_t));
    convertedMesh.Set("indices", indexArray);

    uint32_t nVertices = (uint32_t) mesh.m_points.size();
    Napi::Float64Array pointArray = Napi::Float64Array::New(env, nVertices * 3);
    for (uint32_t i = 0; i < nVertices; ++i)
        {
        pointArray[i * 3] = mesh.m_points[i].x;
        pointArray[i * 3 + 1] = mesh.m_points[i].y;
        pointArray[i * 3 + 2] = mesh.m_points[i].z;
        }
    convertedMesh.Set("points", pointArray);

    Napi::Float32Array normalArray = Napi::Float32Array::New(env, nVertices * 3);
    for (uint32_t i = 0; i < nVertices; ++i)
        {
        normalArray[i * 3] = mesh.m_normals[i].x;
        normalArray[i * 3 + 1] = mesh.m_normals[i].y;
        normalArray[i * 3 + 2] = mesh.m_normals[i].z;
        }
    convertedMesh.Set("normals", normalArray);

    Napi::Float64Array paramArray = Napi::Float64Array::New(env, nVertices * 2);
    for (uint32_t i = 0; i < nVertices; ++i)
        {
        paramArray[i * 2] = mesh.m_params[i].x;
        paramArray[i * 2 + 1] = mesh.m_params[i].y;
        }
    convertedMesh.Set("params", paramArray);

    return convertedMesh;
    }

struct ExportGraphicsMaterial : Render::Material
{
private:
    ExportGraphicsMaterial(CreateParams const& params) : Material(params) { }

public:
    static MaterialPtr Create(MaterialKey key) { return new ExportGraphicsMaterial(Material::CreateParams(key)); }
};

//=======================================================================================
// @bsistruct                                                   Matt.Gooding    02/19
//=======================================================================================
struct ExportGraphicsContext : NullContext
{
    DEFINE_T_SUPER(NullContext);

private:
    IGeometryProcessorR m_processor;

    Render::GraphicBuilderPtr _CreateGraphic(Render::GraphicBuilder::CreateParams const& params) override
        {
        return new SimplifyGraphic(params, m_processor, *this);
        }

public:
    ExportGraphicsContext(IGeometryProcessorR processor) : m_processor(processor)
        {
        m_purpose = processor._GetProcessPurpose();
        }
};

//=======================================================================================
// @bsistruct                                                   Matt.Gooding    02/19
//=======================================================================================
struct ExportGraphicsProcessor : IGeometryProcessor
{
public:
    struct CachedEntry
        {
        ColorDef            color;
        DgnTextureId        textureId;
        ExportGraphicsMesh  mesh; 
        CachedEntry(ColorDef c, DgnTextureId id) : color(c), textureId(id) { }
        };
    bvector<CachedEntry>            m_cachedEntries;

    ExportGraphicsProcessor(DgnDbR db, IFacetOptionsP facetOptions) : m_db(db), m_facetOptions(facetOptions) { }
    virtual ~ExportGraphicsProcessor() { }

private:
    IFacetOptionsPtr                m_facetOptions;
    DgnDbR                          m_db;
    FastVertexUnifier               m_unifier;

    struct CachedMaterial
        {
        ColorDef    color;
        bool        useColor;
        bool        useAlpha;
        Json::Value patternMap;
        };
    std::unordered_map<uint64_t, std::unique_ptr<CachedMaterial>> m_cachedMaterials;

    IFacetOptionsP _GetFacetOptionsP() override {return m_facetOptions.get();}

    UnhandledPreference _GetUnhandledPreference(CurveVectorCR, SimplifyGraphic&) const override {return UnhandledPreference::Facet;}
    UnhandledPreference _GetUnhandledPreference(ISolidPrimitiveCR, SimplifyGraphic&) const override {return UnhandledPreference::Facet;}
    UnhandledPreference _GetUnhandledPreference(MSBsplineSurfaceCR, SimplifyGraphic&) const override {return UnhandledPreference::Facet;}
    UnhandledPreference _GetUnhandledPreference(PolyfaceQueryCR, SimplifyGraphic&) const override {return UnhandledPreference::Ignore;}
    UnhandledPreference _GetUnhandledPreference(IBRepEntityCR, SimplifyGraphic&) const override {return UnhandledPreference::Facet;}
    UnhandledPreference _GetUnhandledPreference(Render::TriMeshArgsCR, SimplifyGraphic&) const override {return UnhandledPreference::Facet;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    02/19
+---------------+---------------+---------------+---------------+---------------+------*/
CachedMaterial& GetCachedMaterial(RenderMaterialId materialId)
    {
    std::unique_ptr<CachedMaterial>& result = m_cachedMaterials[materialId.GetValue()];
    if (result.get()) return *result;

    result.reset(new CachedMaterial());
    RenderMaterialCPtr matElem = RenderMaterial::Get(m_db, materialId);
    if (!matElem.IsValid())
        {
        result->useColor = result->useAlpha = false;
        return *result;
        }

    RenderingAssetCR asset = matElem->GetRenderingAsset();
    if (asset.GetBool(RENDER_MATERIAL_FlagHasBaseColor, false))
        {
        RgbFactor diffuseRgb = asset.GetColor(RENDER_MATERIAL_Color);
        result->color.SetRed((Byte)(diffuseRgb.red * 255.0));
        result->color.SetGreen((Byte)(diffuseRgb.green * 255.0));
        result->color.SetBlue((Byte)(diffuseRgb.blue * 255.0));
        result->useColor = true;
        }
    if (asset.GetBool(RENDER_MATERIAL_FlagHasTransmit, false))
        {
        double transparency = asset.GetDouble(RENDER_MATERIAL_Transmit, 0.0);
        result->color.SetAlpha((Byte)(transparency * 255.0));
        result->useAlpha = true;
        }

    result->patternMap = asset.GetPatternMap().m_value;

    return *result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    02/19
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef ResolveColor(SimplifyGraphic& sg, Json::Value& patternMapJson)
    {
    // Resolved fill color as baseline
    ColorDef color = sg.GetCurrentGraphicParams().GetFillColor();

    // GraphicParams material pointer will always be null since we don't have a Render::System.
    // Look up from ID on GeometryParams instead.
    RenderMaterialId materialId = sg.GetCurrentGeometryParams().GetMaterialId();
    if (!materialId.IsValid()) return color;

    CachedMaterial& cachedMaterial = GetCachedMaterial(materialId);
    if (cachedMaterial.useColor) color.SetColorNoAlpha(cachedMaterial.color);
    if (cachedMaterial.useAlpha) color.SetAlpha(cachedMaterial.color.GetAlpha());
    patternMapJson = cachedMaterial.patternMap;

    return color;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    02/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool _ProcessPolyface(PolyfaceQueryCR pfQuery, bool filled, SimplifyGraphic& sg) override
    {
    if (pfQuery.GetPointIndexCount() == 0 || pfQuery.GetPointCount() == 0) return true;
    if (pfQuery.GetNormalIndexCP() == nullptr || pfQuery.GetNormalCount() == 0) return true;
    if (pfQuery.GetParamIndexCP() == nullptr || pfQuery.GetParamCount() == 0) return true;
    if (pfQuery.GetMeshStyle() != MESH_ELM_STYLE_INDEXED_FACE_LOOPS) return true;

    DgnTextureId textureId;
    Json::Value patternMapJson;
    ColorDef color = ResolveColor(sg, patternMapJson);

    uint32_t indexCountWithoutBlocking = (uint32_t)((pfQuery.GetPointIndexCount() / 4) * 3);
    bvector<FPoint2d> effectiveParams;
    effectiveParams.reserve(indexCountWithoutBlocking);

    if (patternMapJson.isNull())
        {
        int32_t const* srcParamIndices = pfQuery.GetParamIndexCP();
        DPoint2dCP srcParams = pfQuery.GetParamCP();
        for (uint32_t i = 0, iLimit = (uint32_t)pfQuery.GetPointIndexCount(); i < iLimit; i += 4)
            {
            for (uint32_t j = 0; j < 3; ++j)
                {
                DPoint2d srcParam = srcParams[srcParamIndices[i + j] - 1];
                effectiveParams.push_back( { static_cast<float>(srcParam.x), static_cast<float>(srcParam.y) } );
                }
            }
        }
    else
        {
        RenderingAsset::TextureMap textureMap(patternMapJson, RenderingAsset::TextureMap::Type::Pattern);
        Render::TextureMapping::Params textureMapParams = textureMap.GetTextureMapParams();
        textureId = textureMap.GetTextureId();

        bvector<DPoint2d> perFaceParams(3, DPoint2d::FromZero());
        for (auto visitor = PolyfaceVisitor::Attach(pfQuery); visitor->AdvanceToNextFace(); )
            {
            textureMapParams.ComputeUVParams(perFaceParams, *visitor, nullptr);
            for (int i = 0; i < 3; ++i)
                effectiveParams.push_back( { static_cast<float>(perFaceParams[i].x), static_cast<float>(perFaceParams[i].y) } );
            }
        }

    m_unifier.SetInput(pfQuery, sg.GetLocalToWorldTransform(), &effectiveParams[0]);
    m_unifier.Unify();

    // Bucket polyfaces together by color to save cost on NAPI transition and give users a
    // minimal number of meshes for each element.
    ExportGraphicsMesh* cachedMesh = nullptr;
    for (auto& entry : m_cachedEntries)
        {
        if (entry.color != color || entry.textureId != textureId)
            continue;
        cachedMesh = &entry.mesh;
        break;
        }

    if (cachedMesh == nullptr)
        {
        m_cachedEntries.emplace_back(color, textureId);
        cachedMesh = &m_cachedEntries.back().mesh;
        }

    cachedMesh->Add(m_unifier.m_output);
    m_unifier.Clear();

    return true;
    }
}; // ExportGraphicsProcessor

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    02/19
+---------------+---------------+---------------+---------------+---------------+------*/
static IFacetOptionsPtr createFacetOptions(Napi::Object const& exportProps)
    {
    // The defaults for IFacetOptions::CreateForSurfaces when this function was written.
    double chordTol = 0.0;
    double angleTol = msGeomConst_piOver12;
    double maxEdgeLength = 0.0;

    Napi::Value chordTolVal = exportProps.Get("chordTol");
    if (chordTolVal.IsNumber())
        chordTol = chordTolVal.As<Napi::Number>().DoubleValue();

    Napi::Value angleTolVal = exportProps.Get("angleTol");
    if (angleTolVal.IsNumber())
        angleTol = angleTolVal.As<Napi::Number>().DoubleValue();

    Napi::Value maxEdgeLengthVal = exportProps.Get("maxEdgeLength");
    if (maxEdgeLengthVal.IsNumber())
        maxEdgeLength = maxEdgeLengthVal.As<Napi::Number>().DoubleValue();

    return IFacetOptions::CreateForSurfaces(chordTol, angleTol, maxEdgeLength, true, true, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    02/19
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus JsInterop::ExportGraphics(DgnDbR db, Napi::Object const& exportProps)
    {
    IFacetOptionsPtr facetOptions = createFacetOptions(exportProps);
    ExportGraphicsProcessor egProcessor(db, facetOptions.get());
    ExportGraphicsContext egContext(egProcessor);
    egContext.SetDgnDb(db);

    // TS API specifies that modifying the binding of this function in the callback will be ignored
    Napi::Function onGraphicsCb = exportProps.Get("onGraphics").As<Napi::Function>();
    Napi::Array elementIdArray = exportProps.Get("elementIdArray").As<Napi::Array>();

    for (uint32_t i = 0; i < elementIdArray.Length(); ++i)
        {
        Napi::String napiElementId = elementIdArray.Get(i).As<Napi::String>();
        std::string elementIdStr = napiElementId.Utf8Value();
        DgnElementId elementId(BeInt64Id::FromString(elementIdStr.c_str()).GetValue());
        if (!elementId.IsValid())
            continue;

        GeometrySourceCP geomSource;
        DgnElementCPtr element = db.Elements().GetElement(elementId);
        if (element.IsNull() || !(geomSource = element->ToGeometrySource()))
            continue;

        egProcessor.m_cachedEntries.clear();
        egContext.VisitGeometry(*geomSource);
        if (egProcessor.m_cachedEntries.empty())
            continue;

        for (auto& entry : egProcessor.m_cachedEntries)
            {
            Napi::Object cbArgument = Napi::Object::New(Env());
            cbArgument.Set("elementId", napiElementId);
            cbArgument.Set("mesh", convertMesh(Env(), entry.mesh));
            cbArgument.Set("color", Napi::Number::New(Env(), entry.color.GetValue()));
            if (entry.textureId.IsValid())
                {
                Utf8String textureIdString = entry.textureId.ToString(BeInt64Id::UseHex::Yes);
                cbArgument.Set("textureId", Napi::String::New(Env(), textureIdString.c_str()));
                }
            Napi::Value cbVal = onGraphicsCb.Call({ cbArgument });
            if (cbVal.IsBoolean() && !cbVal.As<Napi::Boolean>())
                return DgnDbStatus::Success;
            }
        }

    return DgnDbStatus::Success;
    }
