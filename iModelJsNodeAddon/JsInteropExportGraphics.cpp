/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "IModelJsNative.h"
#include <folly/BeFolly.h>
#include <DgnPlatform/SimplifyGraphic.h>

using namespace IModelJsNative;

//=======================================================================================
// @bsistruct                                                   Matt.Gooding    04/19
//=======================================================================================
struct IntermediateMesh
{
uint32_t                        m_indexCount;
uint32_t                        m_pointCount;
uint32_t                        m_normalCount;
// Indices are zero-based, no blocking, no sign for edge visibility
std::unique_ptr<DPoint3d[]>     m_points;
std::unique_ptr<int32_t[]>      m_pointIndices;
std::unique_ptr<FPoint3d[]>     m_normals;
std::unique_ptr<int32_t[]>      m_normalIndices;
std::unique_ptr<FPoint2d[]>     m_params; // No index array, already unified with point indices

IntermediateMesh(PolyfaceQueryCR pf, TransformCR transform, std::unique_ptr<FPoint2d[]>&& srcParams)
    :
    m_indexCount((uint32_t)(pf.GetPointIndexCount() - (pf.GetPointIndexCount() / 4))), // no blocking in output
    m_pointCount((uint32_t) pf.GetPointCount()),
    m_normalCount((uint32_t) pf.GetNormalCount()),
    m_points(new DPoint3d[m_pointCount]),
    m_pointIndices(new int32_t[m_indexCount]),
    m_normals(new FPoint3d[m_normalCount]),
    m_normalIndices(new int32_t[m_indexCount]),
    m_params(std::move(srcParams))
    {
    bool isIdentity = transform.IsIdentity();
    bool isMirrored = !isIdentity && transform.Determinant() < 0;
    // Account for potential winding order change with mirroring transform
    uint32_t indexOffset1 = isMirrored ? 2 : 1;
    uint32_t indexOffset2 = isMirrored ? 1 : 2;

    // Remove blocking, switch to zero-index, discard edge visibility
    uint32_t srcIndexCount = (uint32_t) pf.GetPointIndexCount();
    int32_t const* srcPointIndices = pf.GetPointIndexCP();
    for (uint32_t dst = 0, src = 0; src < srcIndexCount; src += 4, dst += 3)
        {
        m_pointIndices[dst]     = abs(srcPointIndices[src]) - 1;
        m_pointIndices[dst + 1] = abs(srcPointIndices[src + indexOffset1]) - 1;
        m_pointIndices[dst + 2] = abs(srcPointIndices[src + indexOffset2]) - 1;
        }

    // Remove blocking, switch to zero-index (no sign visibility on normals)
    int32_t const* srcNormalIndices = pf.GetNormalIndexCP();
    for (uint32_t dst = 0, src = 0; src < srcIndexCount; src += 4, dst += 3)
        {
        m_normalIndices[dst]        = srcNormalIndices[src] - 1;
        m_normalIndices[dst + 1]    = srcNormalIndices[src + indexOffset1] - 1;
        m_normalIndices[dst + 2]    = srcNormalIndices[src + indexOffset2] - 1;
        }

    // No work needed for params - if necessary, mirroring transform is applied at creation time in _ProcessPolyface

    memcpy (m_points.get(), pf.GetPointCP(), sizeof(DPoint3d) * m_pointCount);
    if (!isIdentity) transform.Multiply(m_points.get(), (int)m_pointCount);

    RotMatrix normalMatrix = RotMatrix::From(transform);
    isIdentity = normalMatrix.IsIdentity(); // check if it was just translation
    if (!isIdentity) normalMatrix.Invert();

    DPoint3dCP srcNormals = pf.GetNormalCP();
    for (uint32_t i = 0; i < m_normalCount; ++i)
        {
        DPoint3d tmpNormal = srcNormals[i];
        if (!isIdentity) { normalMatrix.MultiplyTranspose(tmpNormal); tmpNormal.Normalize(); }
        m_normals[i] = FPoint3d::From(tmpNormal);
        }
    }
}; // IntermediateMesh

//=======================================================================================
// @bsistruct                                                   Matt.Gooding    04/19
//=======================================================================================
struct ExportGraphicsMesh
{
bvector<int>        indices;
bvector<double>     points;
bvector<float>      normals;
bvector<float>      params;

void AddVertex(DPoint3dCR p, FPoint3dCR n, FPoint2dCR uv)
    {
    indices.push_back((int32_t)points.size() / 3);
    points.insert(points.end(), { p.x, p.y, p.z });
    normals.insert(normals.end(), { n.x, n.y, n.z });
    params.insert(params.end(), { uv.x, uv.y });
    }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    04/19
* Linear speed unification of vertex indices. Each point has ENTRY_COUNT remappings
* that can be saved. After that, just add additional vertices for each normal/param
* combination - ROI on further remapping is poor.
+---------------+---------------+---------------+---------------+---------------+------*/
static void unifyIndices(IntermediateMesh const& inMesh, ExportGraphicsMesh& outMesh)
    {
    struct NodeEntry { int32_t newIndex; int32_t normal; FPoint2d param; };
    constexpr static uint32_t NODE_ENTRY_COUNT = 8;
    struct Node { NodeEntry entries[NODE_ENTRY_COUNT]; }; // each node is 128 bytes for cache alignment

    std::unique_ptr<Node[]> remapper(new Node[inMesh.m_pointCount]);
    memset(remapper.get(), 0, sizeof(Node) * inMesh.m_pointCount);

    // Reserve for at least best case compression
    outMesh.points.reserve(outMesh.points.size() + inMesh.m_pointCount * 3);
    outMesh.normals.reserve(outMesh.normals.size() + inMesh.m_pointCount * 3);
    outMesh.params.reserve(outMesh.params.size() + inMesh.m_pointCount * 2);

    uint32_t indexCount = inMesh.m_indexCount;
    outMesh.indices.reserve(outMesh.indices.size() + indexCount);

    constexpr static float UV_TOLERANCE = 0.00025; // ~1 pixel at 4K

    for (uint32_t i = 0; i < indexCount; ++i)
        {
        int32_t origPointIndex = inMesh.m_pointIndices[i];
        int32_t origNormalIndex = inMesh.m_normalIndices[i];
        FPoint2d origParam = inMesh.m_params[i];

        bool foundRemap = false;
        for (int j = 0; j < NODE_ENTRY_COUNT; ++j)
            {
            NodeEntry& entry = remapper[origPointIndex].entries[j];
            if (entry.newIndex == 0) // unused entry
                {
                entry.normal = origNormalIndex;
                entry.param = origParam;
                entry.newIndex = (int32_t) (outMesh.points.size() / 3) + 1; // ONE-INDEX
                outMesh.AddVertex(inMesh.m_points[origPointIndex], inMesh.m_normals[origNormalIndex], origParam);
                foundRemap = true;
                break;
                }
            else if (entry.normal == origNormalIndex &&
                     std::abs(entry.param.x - origParam.x) < UV_TOLERANCE &&
                     std::abs(entry.param.y - origParam.y) < UV_TOLERANCE)
                {
                // reuse entry
                outMesh.indices.push_back(entry.newIndex - 1); // ONE-INDEX
                foundRemap = true;
                break;
                }
            }

        // Node for this point is full, just add another vertex.
        if (!foundRemap)
            outMesh.AddVertex(inMesh.m_points[origPointIndex], inMesh.m_normals[origNormalIndex], origParam);
        }
    }

//=======================================================================================
// @bsistruct                                                   Matt.Gooding    04/19
//=======================================================================================
struct ExportGraphicsBuilder : SimplifyGraphic
{
DEFINE_T_SUPER(SimplifyGraphic);

ExportGraphicsBuilder(Render::GraphicBuilder::CreateParams const& cp, IGeometryProcessorR gp, ViewContextR vc)
    : T_Super(cp, gp, vc) { }
virtual ~ExportGraphicsBuilder() { }

// Don't let SimplifyGraphic generate intermediates we always throw away
void _AddTextString(TextStringCR) override { }
void _AddTextString2d(TextStringCR, double) override { }
void _AddDgnOle(Render::DgnOleDraw*) override { }
bool _WantStrokeLineStyle(Render::LineStyleSymbCR, IFacetOptionsPtr&) override { return false; }
bool _WantStrokePattern(PatternParamsCR) override { return false; }
Render::GraphicPtr _Finish() override {m_isOpen = false; return nullptr;} // we don't use output, don't allocate!

// Copy of SimplifyGraphic::_CreateSubGraphic to generate derived class
Render::GraphicBuilderPtr _CreateSubGraphic(TransformCR subToGraphic, ClipVectorCP clip) const override
    {
    auto result = new ExportGraphicsBuilder(Render::GraphicBuilder::CreateParams::Scene(GetDgnDb(),
        Transform::FromProduct(GetLocalToWorldTransform(), subToGraphic)), m_processor, m_context);

    result->m_currGraphicParams  = m_currGraphicParams;
    result->m_currGeometryParams = m_currGeometryParams;
    result->m_currGeomEntryId    = m_currGeomEntryId;
    result->m_currClip           = (nullptr != clip ? clip->Clone(&GetLocalToWorldTransform()) : nullptr);
    return result;
    }
}; // ExportGraphicsBuilder

//=======================================================================================
// @bsistruct                                                   Matt.Gooding    06/19
//=======================================================================================
struct PartInstanceRecord
{
    DgnGeometryPartId       partId;
    Transform               transform;
    DgnCategoryId           categoryId;
    DgnSubCategoryId        subCategoryId;
    RenderMaterialId        materialId;
    double                  elmTransparency;
    ColorDef                lineColor;
};

//=======================================================================================
// @bsistruct                                                   Matt.Gooding    02/19
//=======================================================================================
struct ExportGraphicsContext : NullContext
{
    DEFINE_T_SUPER(NullContext);
    IGeometryProcessorR             m_processor;
    bvector<PartInstanceRecord>*    m_instances;

ExportGraphicsContext(IGeometryProcessorR processor, bvector<PartInstanceRecord>* instances)
    : m_processor(processor), m_instances(instances) {m_purpose = processor._GetProcessPurpose();}

Render::GraphicBuilderPtr _CreateGraphic(Render::GraphicBuilder::CreateParams const& params) override
    { return new ExportGraphicsBuilder(params, m_processor, *this); }

void _AddSubGraphic(Render::GraphicBuilderR graphic, DgnGeometryPartId partId, TransformCR transform, Render::GeometryParamsR geomParams) override
    {
    if (!m_instances) return T_Super::_AddSubGraphic(graphic, partId, transform, geomParams);

    PartInstanceRecord rec;
    rec.partId = partId;
    rec.transform = graphic.GetLocalToWorldTransform() * transform;

    geomParams.Resolve(GetDgnDb());
    rec.categoryId = geomParams.GetCategoryId();
    rec.subCategoryId = geomParams.GetSubCategoryId();
    rec.materialId = geomParams.GetMaterialId();
    rec.elmTransparency = geomParams.GetTransparency();
    rec.lineColor = geomParams.GetLineColor();

    m_instances->emplace_back(rec);
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
        RenderMaterialId    materialId;
        DgnTextureId        textureId;
        DgnSubCategoryId    subCategoryId;
        ExportGraphicsMesh  mesh; 
        CachedEntry(ColorDef c, RenderMaterialId matId, DgnTextureId texId, DgnSubCategoryId subCat)
            : color(c), materialId(matId), textureId(texId), subCategoryId(subCat) { }
        };
    bvector<CachedEntry>            m_cachedEntries;
    struct CachedLineString
        {
        ColorDef            color;
        DgnSubCategoryId    subCategoryId;
        bvector<DPoint3d>   points;
        CachedLineString(ColorDef c, DgnSubCategoryId subCat, bvector<DPoint3d>&& p)
            : color(c), subCategoryId(subCat), points(std::move(p)) { }
        };
    bvector<CachedLineString>       m_cachedLineStrings;
    bool                            m_gotBadPolyface;
    bool                            m_generateLines;

    ExportGraphicsProcessor(DgnDbR db, IFacetOptionsR facetOptions, bool generateLines) :
        m_db(db), m_facetOptions(facetOptions), m_gotBadPolyface(false), m_generateLines(generateLines) { }
    virtual ~ExportGraphicsProcessor() { }

private:
    IFacetOptionsR                  m_facetOptions;
    DgnDbR                          m_db;

    IFacetOptionsP _GetFacetOptionsP() override {return &m_facetOptions;}

    UnhandledPreference _GetUnhandledPreference(CurveVectorCR, SimplifyGraphic&) const override {return UnhandledPreference::Facet;}
    UnhandledPreference _GetUnhandledPreference(ISolidPrimitiveCR, SimplifyGraphic&) const override {return UnhandledPreference::Facet;}
    UnhandledPreference _GetUnhandledPreference(MSBsplineSurfaceCR, SimplifyGraphic&) const override {return UnhandledPreference::Facet;}
    UnhandledPreference _GetUnhandledPreference(PolyfaceQueryCR, SimplifyGraphic&) const override {return UnhandledPreference::Ignore;}
    UnhandledPreference _GetUnhandledPreference(IBRepEntityCR, SimplifyGraphic&) const override {return UnhandledPreference::Facet;}
    UnhandledPreference _GetUnhandledPreference(Render::TriMeshArgsCR, SimplifyGraphic&) const override {return UnhandledPreference::Facet;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    07/19
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _ProcessCurveVector(CurveVectorCR curve, bool filled, SimplifyGraphic& sg) override
    {
    if (curve.IsAnyRegionType()) return false; // throw back to polyface processing if can be closed shape
    if (!m_generateLines) return true;

    bvector<DPoint3d> strokePoints;
    curve.AddStrokePoints(strokePoints, m_facetOptions);
    if (strokePoints.empty()) return true;

    TransformCR transform = sg.GetLocalToWorldTransform();
    if (!transform.IsIdentity()) transform.Multiply(&strokePoints[0], (int)strokePoints.size());

    // Bucket together based on subcategory and color
    DgnSubCategoryId subCategoryId = sg.GetCurrentGeometryParams().GetSubCategoryId();
    ColorDef color = sg.GetCurrentGraphicParams().GetFillColor();
    for (auto& cached : m_cachedLineStrings)
        {
        if (cached.color == color && cached.subCategoryId == subCategoryId)
            {
            cached.points.push_back(DPoint3d::From(DISCONNECT, DISCONNECT, DISCONNECT));
            cached.points.insert(cached.points.end(), strokePoints.begin(), strokePoints.end());
            return true;
            }
        }

    m_cachedLineStrings.emplace_back(color, subCategoryId, std::move(strokePoints));
    return true;
    }

    struct MeshDisplayProps
    {
        ColorDef                        color;
        RenderMaterialId                materialId;
        // Usually no pattern map, but empty Json::Value constructor shows up in profile if instantiated
        // for display props lookup. Hide behind unique_ptr to only construct if needed.
        std::unique_ptr<Json::Value>    patternMap;
        DgnSubCategoryId                subCategoryId;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    02/19
+---------------+---------------+---------------+---------------+---------------+------*/
void ResolveMeshDisplayProps(SimplifyGraphic& sg, MeshDisplayProps& result)
    {
    result.subCategoryId = sg.GetCurrentGeometryParams().GetSubCategoryId();

    // Resolved fill color as baseline
    result.color = sg.GetCurrentGraphicParams().GetFillColor();

    // GraphicParams material pointer will always be null since we don't have a Render::System.
    // Look up from ID on GeometryParams instead.
    result.materialId = sg.GetCurrentGeometryParams().GetMaterialId();
    if (!result.materialId.IsValid()) return;

    RenderMaterialCPtr matElem = RenderMaterial::Get(m_db, result.materialId);
    if (!matElem.IsValid()) { result.materialId.Invalidate(); return; } // don't return invalid ID

    RenderingAssetCR asset = matElem->GetRenderingAsset();
    if (asset.GetBool(RENDER_MATERIAL_FlagHasBaseColor, false))
        {
        RgbFactor diffuseRgb = asset.GetColor(RENDER_MATERIAL_Color);
        result.color.SetRed((Byte)(diffuseRgb.red * 255.0));
        result.color.SetGreen((Byte)(diffuseRgb.green * 255.0));
        result.color.SetBlue((Byte)(diffuseRgb.blue * 255.0));
        }
    if (asset.GetBool(RENDER_MATERIAL_FlagHasTransmit, false))
        {
        double transparency = asset.GetDouble(RENDER_MATERIAL_Transmit, 0.0);
        result.color.SetAlpha((Byte)(transparency * 255.0));
        }

    auto patternMap = asset.GetPatternMap();
    if (patternMap.IsValid())
        result.patternMap = std::unique_ptr<Json::Value>(new Json::Value(patternMap.m_value));
    }

bool VerifyTriangulationAndZeroBlocking(PolyfaceQueryCR pfQuery)
    {
    const int32_t* indices = pfQuery.GetPointIndexCP();
    for (uint32_t i = 0, indexCount = (uint32_t)pfQuery.GetPointIndexCount(); i < indexCount; i += 4)
        {
        for (uint32_t j = 0; j < 3; ++j) { if (indices[i + j] == 0) { return false; } }
        if (indices[i + 3] != 0) { return false; }
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    02/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool _ProcessPolyface(PolyfaceQueryCR pfQuery, bool filled, SimplifyGraphic& sg) override
    {
    // Receiving empty polyfaces isn't ideal, but just ignore and don't count as real error condition
    if (pfQuery.GetPointIndexCount() == 0 || pfQuery.GetPointCount() == 0) return true;

    // Polyfaces missing requested information or in the wrong style are indicative of real problems upstream
    if (pfQuery.GetNormalIndexCP() == nullptr || pfQuery.GetNormalCount() == 0) { m_gotBadPolyface = true; return true; }
    if (pfQuery.GetParamIndexCP() == nullptr || pfQuery.GetParamCount() == 0) { m_gotBadPolyface = true; return true; }
    if (pfQuery.GetMeshStyle() != MESH_ELM_STYLE_INDEXED_FACE_LOOPS) { m_gotBadPolyface = true; return true; }
    if (!VerifyTriangulationAndZeroBlocking(pfQuery)) { m_gotBadPolyface = true; return true; }

    MeshDisplayProps displayProps;
    ResolveMeshDisplayProps(sg, displayProps);

    uint32_t indexCount = (uint32_t)pfQuery.GetPointIndexCount();
    uint32_t indexCountWithoutBlocking = indexCount - (indexCount / 4);
    std::unique_ptr<FPoint2d[]> computedParams(new FPoint2d[indexCountWithoutBlocking]);

    TransformCR localToWorld = sg.GetLocalToWorldTransform();
    bool isMirrored = localToWorld.Determinant() < 0;

    DgnTextureId textureId;
    if (!displayProps.patternMap.get())
        {
        int computedParamCount = 0;
        int32_t const* srcParamIndices = pfQuery.GetParamIndexCP();
        DPoint2dCP srcParams = pfQuery.GetParamCP();
        for (uint32_t i = 0, iLimit = (uint32_t)pfQuery.GetPointIndexCount(); i < iLimit; i += 4)
            {
            for (uint32_t j = 0; j < 3; ++j)
                {
                DPoint2d srcParam = srcParams[srcParamIndices[i + j] - 1];
                computedParams[computedParamCount + j].x = static_cast<float>(srcParam.x);
                // Unlike params computed from texture map (see below), V does not need to inverted for raw params.
                computedParams[computedParamCount + j].y = static_cast<float>(srcParam.y);
                }
            // Apply mirror transform while constructing so we don't have to iterate through again
            if (isMirrored) std::swap(computedParams[computedParamCount + 1], computedParams[computedParamCount + 2]);
            computedParamCount += 3;
            }
        }
    else
        {
        RenderingAsset::TextureMap textureMap(*displayProps.patternMap.get(), RenderingAsset::TextureMap::Type::Pattern);
        Render::TextureMapping::Params textureMapParams = textureMap.GetTextureMapParams();
        textureId = textureMap.GetTextureId();

        bvector<DPoint2d> perFaceParams(3, DPoint2d::FromZero());
        int computedParamCount = 0;
        for (auto visitor = PolyfaceVisitor::Attach(pfQuery); visitor->AdvanceToNextFace(); )
            {
            textureMapParams.ComputeUVParams(perFaceParams, *visitor, nullptr);
            for (int i = 0; i < 3; ++i)
                {
                computedParams[computedParamCount + i].x = static_cast<float>(perFaceParams[i].x);
                // TextureMapping::Trans2x3 inverts V so that the frontend doesn't need to flip textures
                // on load. We need to invert back to match interchange conventions.
                computedParams[computedParamCount + i].y = 1.0f - static_cast<float>(perFaceParams[i].y);
                }
            // Apply mirror transform while constructing so we don't have to iterate through again
            if (isMirrored) std::swap(computedParams[computedParamCount + 1], computedParams[computedParamCount + 2]);
            computedParamCount += 3;
            }
        }

    // Bucket polyfaces together by shared display props to save cost on NAPI transition and give users a
    // minimal number of meshes for each element.
    ExportGraphicsMesh* exportMesh = nullptr;
    for (auto& entry : m_cachedEntries)
        {
        if (entry.color != displayProps.color ||
            entry.materialId != displayProps.materialId ||
            entry.textureId != textureId ||
            entry.subCategoryId != displayProps.subCategoryId)
            continue;
        exportMesh = &entry.mesh;
        break;
        }
    if (exportMesh == nullptr)
        {
        m_cachedEntries.emplace_back(displayProps.color, displayProps.materialId, textureId, displayProps.subCategoryId);
        exportMesh = &m_cachedEntries.back().mesh;
        }

    unifyIndices(IntermediateMesh(pfQuery, localToWorld, std::move(computedParams)), *exportMesh);
    return true;
    }
}; // ExportGraphicsProcessor

//=======================================================================================
// @bsistruct                                                   Matt.Gooding    04/19
//=======================================================================================
struct ExportGeometrySource3d : GeometrySource3d
{
public:
    GeometryStream  m_geomStream;
    DgnCategoryId   m_categoryId;
    DgnDbR          m_db;
    Placement3d     m_placement;

    ExportGeometrySource3d(DgnDbR db) : m_db(db) { }
    virtual ~ExportGeometrySource3d() { }

    DgnDbR _GetSourceDgnDb() const override { return m_db; }
    DgnElementCP _ToElement() const override { return nullptr; }
    GeometrySource3dCP _GetAsGeometrySource3d() const override { return this; }
    DgnCategoryId _GetCategoryId() const override { return m_categoryId; }
    GeometryStreamCR _GetGeometryStream() const override { return m_geomStream; }
    Placement3dCR _GetPlacement() const override { return m_placement; }

    DgnDbStatus _SetCategoryId(DgnCategoryId categoryId) override { BeAssert(false); return DgnDbStatus::BadRequest; }
    DgnDbStatus _SetPlacement(Placement3dCR) override { BeAssert(false); return DgnDbStatus::BadRequest; }
};

//=======================================================================================
// @bsistruct                                                   Matt.Gooding    03/19
//=======================================================================================
struct ExportGraphicsJob
{
    ExportGeometrySource3d          m_geom;
    DgnDbR                          m_db;
    ExportGraphicsProcessor         m_processor;
    ExportGraphicsContext           m_context;
    DgnElementId                    m_elementId;
    bvector<PartInstanceRecord>     m_instances;

ExportGraphicsJob(DgnDbR db, IFacetOptionsR fo, DgnElementId elId, bool saveInstances, bool generateLines)
    : m_geom(db), m_db(db), m_processor(db, fo, generateLines), m_elementId(elId),
    m_context(m_processor, saveInstances ? &m_instances : nullptr)
    {
    }

void Execute()
    {
    m_context.SetDgnDb(m_db);
    m_geom.Draw(m_context, 0);
    }
};

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
    if (chordTolVal.IsNumber()) chordTol = chordTolVal.As<Napi::Number>().DoubleValue();

    // Per Earlin's advice on avoiding topology problems, restrict max angle tolerance to 45 deg
    Napi::Value angleTolVal = exportProps.Get("angleTol");
    if (angleTolVal.IsNumber())
        angleTol = std::min(msGeomConst_piOver4, angleTolVal.As<Napi::Number>().DoubleValue());

    Napi::Value maxEdgeLengthVal = exportProps.Get("maxEdgeLength");
    if (maxEdgeLengthVal.IsNumber()) maxEdgeLength = maxEdgeLengthVal.As<Napi::Number>().DoubleValue();

    auto result = IFacetOptions::CreateForSurfaces(chordTol, angleTol, maxEdgeLength, true, true, true);
    result->SetIgnoreHiddenBRepEntities(true); // act like tile generation, big perf improvement
    result->SetBRepConcurrentFacetting(false); // see Parasolid IR 8415939, always faster exclusive

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    07/19
+---------------+---------------+---------------+---------------+---------------+------*/
static Napi::Value convertLines(Napi::Env& env, bvector<DPoint3d> const& points)
    {
    bvector<int> exportIndices; exportIndices.reserve(points.size());
    bvector<double> exportPoints; exportPoints.reserve(points.size());

    // Only attempts to compress to indices for contiguous points in line string. Full search for
    // duplicate points seems like low ROI for runtime cost.

    bool canReuseLastPoint = false;
    for (int i = 1; i < (int)points.size(); ++i)
        {
        // Remove disconnects. Create index buffer set up for GL_LINES where every segment is two points.
        // If there are any individual points, this will skip them.
        while (i < (int)points.size() && points[i].IsDisconnect())
            {
            i += 2;
            canReuseLastPoint = false;
            }
        if (i >= (int)points.size()) break;

        if (canReuseLastPoint)
            exportIndices.push_back(exportIndices.back());
        else
            {
            exportIndices.push_back((int)exportPoints.size() / 3);
            DPoint3dCR p0 = points[i-1];
            exportPoints.insert(exportPoints.end(), { p0.x, p0.y, p0.z });
            }

        exportIndices.push_back((int)exportPoints.size() / 3);
        DPoint3dCR p1 = points[i];
        exportPoints.insert(exportPoints.end(), { p1.x, p1.y, p1.z });
        canReuseLastPoint = true;
        }

    Napi::Int32Array indexArray = Napi::Int32Array::New(env, exportIndices.size());
    memcpy (indexArray.Data(), &exportIndices[0], exportIndices.size() * sizeof(int));

    Napi::Float64Array pointArray = Napi::Float64Array::New(env, exportPoints.size());
    memcpy (pointArray.Data(), &exportPoints[0], exportPoints.size() * sizeof(double));

    Napi::Object convertedLines = Napi::Object::New(env);
    convertedLines.Set("indices", indexArray);
    convertedLines.Set("points", pointArray);
    return convertedLines;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    02/19
+---------------+---------------+---------------+---------------+---------------+------*/
static Napi::Value convertMesh(Napi::Env& env, ExportGraphicsMesh& mesh)
    {
    Napi::Int32Array indexArray = Napi::Int32Array::New(env, mesh.indices.size());
    memcpy (indexArray.Data(), &mesh.indices[0], mesh.indices.size() * sizeof(int32_t));
    Napi::Float64Array pointArray = Napi::Float64Array::New(env, mesh.points.size());
    memcpy (pointArray.Data(), &mesh.points[0], mesh.points.size() * sizeof(double));
    Napi::Float32Array normalArray = Napi::Float32Array::New(env, mesh.normals.size());
    memcpy (normalArray.Data(), &mesh.normals[0], mesh.normals.size() * sizeof(float));
    Napi::Float32Array paramArray = Napi::Float32Array::New(env, mesh.params.size());
    memcpy (paramArray.Data(), &mesh.params[0], mesh.params.size() * sizeof(float));

    Napi::Object convertedMesh = Napi::Object::New(env);
    convertedMesh.Set("indices", indexArray);
    convertedMesh.Set("points", pointArray);
    convertedMesh.Set("normals", normalArray);
    convertedMesh.Set("params", paramArray);
    return convertedMesh;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    06/19
+---------------+---------------+---------------+---------------+---------------+------*/
static Napi::String createIdString(Napi::Env& env, BeInt64Id const& id)
    {
    static Utf8Char idStrBuffer[BeInt64Id::ID_STRINGBUFFER_LENGTH];
    id.ToString(idStrBuffer, BeInt64Id::UseHex::Yes);
    return Napi::String::New(env, idStrBuffer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    06/19
+---------------+---------------+---------------+---------------+---------------+------*/
static void convertPartInstances(Napi::Env& env, Napi::Array& napiArray, DgnElementId partInstanceId,
    bvector<PartInstanceRecord> const& instances)
    {
    uint32_t napiArrayIndex = napiArray.Length();
    Napi::String partInstanceIdString = createIdString(env, partInstanceId);
    for (auto& rec : instances)
        {
        Napi::Object napiInstance = Napi::Object::New(env);
        napiArray.Set(napiArrayIndex, napiInstance);
        ++napiArrayIndex;

        napiInstance.Set("partId", createIdString(env, rec.partId));
        napiInstance.Set("partInstanceId", partInstanceIdString);
        if (!rec.transform.IsIdentity())
            {
            Napi::Float64Array transformArray = Napi::Float64Array::New(env, 12);
            memcpy (transformArray.Data(), rec.transform.form3d, 12 * sizeof(double));
            napiInstance.Set("transform", transformArray);
            }
        Napi::Object napiDisplayProps = Napi::Object::New(env);
        napiDisplayProps.Set("categoryId", createIdString(env, rec.categoryId));
        napiDisplayProps.Set("subCategoryId", createIdString(env, rec.subCategoryId));
        napiDisplayProps.Set("materialId", createIdString(env, rec.materialId));
        napiDisplayProps.Set("elmTransparency", Napi::Number::New(env, rec.elmTransparency));
        napiDisplayProps.Set("lineColor", Napi::Number::New(env, rec.lineColor.GetValue()));
        napiInstance.Set("displayProps", napiDisplayProps);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    04/19
+---------------+---------------+---------------+---------------+---------------+------*/
static BeSQLite::CachedStatementPtr getSelectStatement(DgnDbR db)
    {
    // Mimic GeometrySelector3d in Tile.cpp - just get the element bits we need and dodge
    // the mutex contention that comes with loading full elements.
    const Utf8CP sql = "SELECT CategoryId,GeometryStream,Yaw,Pitch,Roll,Origin_X,Origin_Y,Origin_Z,"
        "BBoxLow_X,BBoxLow_Y,BBoxLow_Z,BBoxHigh_X,BBoxHigh_Y,BBoxHigh_Z FROM "
        BIS_TABLE(BIS_CLASS_GeometricElement3d) " WHERE ElementId=?";
    return db.GetCachedStatement(sql);
    }
static Placement3d getPlacement(BeSQLite::CachedStatement& stmt)
    {
    Angle yaw   = Angle::FromDegrees(stmt.GetValueDouble(2));
    Angle pitch = Angle::FromDegrees(stmt.GetValueDouble(3));
    Angle roll  = Angle::FromDegrees(stmt.GetValueDouble(4));
    DPoint3d origin = DPoint3d::From(stmt.GetValueDouble(5), stmt.GetValueDouble(6), stmt.GetValueDouble(7));
    DPoint3d boxMin = DPoint3d::From(stmt.GetValueDouble(8), stmt.GetValueDouble(9), stmt.GetValueDouble(10));
    DPoint3d boxMax = DPoint3d::From(stmt.GetValueDouble(11), stmt.GetValueDouble(12), stmt.GetValueDouble(13));

    return Placement3d(origin, YawPitchRollAngles(yaw, pitch, roll),
        ElementAlignedBox3d(boxMin.x, boxMin.y, boxMin.z, boxMax.x, boxMax.y, boxMax.z));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    02/19
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus JsInterop::ExportGraphics(DgnDbR db, Napi::Object const& exportProps)
    {
    BeFolly::ThreadPool& threadPool = BeFolly::ThreadPool::GetCpuPool();
    BeSQLite::CachedStatementPtr stmt = getSelectStatement(db);
    IFacetOptionsPtr facetOptions = createFacetOptions(exportProps);
    Napi::Array elementIdArray = exportProps.Get("elementIdArray").As<Napi::Array>();

    bvector<std::unique_ptr<ExportGraphicsJob>> jobs; jobs.reserve(elementIdArray.Length());
    bvector<folly::Future<folly::Unit>> jobHandles; jobHandles.reserve(elementIdArray.Length());

    Napi::Array napiPartArray = exportProps.Get("partInstanceArray").As<Napi::Array>();
    bool saveInstances = napiPartArray.IsArray();

    Napi::Function onLineGraphicsCb = exportProps.Get("onLineGraphics").As<Napi::Function>();
    bool generateLines = onLineGraphicsCb.IsFunction();

    for (uint32_t i = 0; i < elementIdArray.Length(); ++i)
        {
        Napi::String napiElementId = elementIdArray.Get(i).As<Napi::String>();
        std::string elementIdStr = napiElementId.Utf8Value();
        DgnElementId elementId(BeInt64Id::FromString(elementIdStr.c_str()).GetValue());
        if (!elementId.IsValid()) continue;

        // Mimic GeometrySelector3d in Tile.cpp
        stmt->Reset();
        stmt->BindInt64(1, elementId.GetValueUnchecked());
        if (BeSQLite::BE_SQLITE_ROW != stmt->Step()) continue;

        GeometryStream geomStream;
        auto status = db.Elements().LoadGeometryStream(geomStream, stmt->GetValueBlob(1), stmt->GetColumnBytes(1));
        if (status != DgnDbStatus::Success) continue;

        auto job = new ExportGraphicsJob(db, *facetOptions.get(), elementId, saveInstances, generateLines);
        job->m_geom.m_categoryId = stmt->GetValueId<DgnCategoryId>(0);
        job->m_geom.m_placement = getPlacement(*stmt);
        job->m_geom.m_geomStream = std::move(geomStream);

        jobs.emplace_back(job);
        jobHandles.push_back(folly::via(&threadPool, [=]() { job->Execute(); }));
        }

    // TS API specifies that modifying the binding of this function in the callback will be ignored
    Napi::Function onGraphicsCb = exportProps.Get("onGraphics").As<Napi::Function>();

    for (uint32_t i = 0; i < (uint32_t)jobHandles.size(); ++i)
        {
        jobHandles[i].wait(); // Should use folly chaining? Good enough for now, jobs are FIFO

        ExportGraphicsJob* job = jobs[i].get();
        if (job->m_processor.m_gotBadPolyface)
            {
            Utf8CP errorMsg = "Element 0x%llx generated invalid geometry, this may indicate problems with the source data.";
            JsInterop::GetLogger().errorv(errorMsg, job->m_elementId.GetValueUnchecked());
            }

        Napi::String elementIdString = createIdString(Env(), job->m_elementId);
        for (auto& entry : job->m_processor.m_cachedEntries)
            {
            Napi::Object cbArgument = Napi::Object::New(Env());
            cbArgument.Set("elementId", elementIdString);
            cbArgument.Set("mesh", convertMesh(Env(), entry.mesh));
            cbArgument.Set("color", Napi::Number::New(Env(), entry.color.GetValue()));
            cbArgument.Set("subCategory", createIdString(Env(), entry.subCategoryId));
            if (entry.materialId.IsValid())
                cbArgument.Set("materialId", createIdString(Env(), entry.materialId));
            if (entry.textureId.IsValid())
                cbArgument.Set("textureId", createIdString(Env(), entry.textureId));
            onGraphicsCb.Call({ cbArgument });
            }

        for (auto& entry : job->m_processor.m_cachedLineStrings)
            {
            Napi::Object cbArgument = Napi::Object::New(Env());
            cbArgument.Set("elementId", elementIdString);
            cbArgument.Set("subCategory", createIdString(Env(), entry.subCategoryId));
            cbArgument.Set("color", Napi::Number::New(Env(), entry.color.GetValue()));
            cbArgument.Set("lines", convertLines(Env(), entry.points));
            onLineGraphicsCb.Call({ cbArgument });
            }

        if (saveInstances && !job->m_instances.empty())
            convertPartInstances(Env(), napiPartArray, job->m_elementId, job->m_instances);

        jobs[i].reset(); // Cleaning these up now while they're still in cache is much faster
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    06/19
+---------------+---------------+---------------+---------------+---------------+------*/
static uint64_t getIdFromNapiValue(Napi::Value const& napiVal)
    {
    auto napiString = napiVal.As<Napi::String>();
    std::string utf8String = napiString.Utf8Value();
    return BeInt64Id::FromString(utf8String.c_str()).GetValueUnchecked();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    06/19
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus JsInterop::ExportPartGraphics(DgnDbR db, Napi::Object const& exportProps)
    {
    DgnElementId elementId(getIdFromNapiValue(exportProps.Get("elementId")));
    if (!elementId.IsValid()) return DgnDbStatus::InvalidId;

    DgnGeometryPartCPtr partElement = db.Elements().Get<DgnGeometryPart>(elementId);
    if (!partElement.IsValid()) return DgnDbStatus::InvalidId;

    auto displayProps = exportProps.Get("displayProps").As<Napi::Object>();
    DgnCategoryId categoryId(getIdFromNapiValue(displayProps.Get("categoryId")));
    DgnSubCategoryId subCategoryId(getIdFromNapiValue(displayProps.Get("subCategoryId")));
    RenderMaterialId materialId(getIdFromNapiValue(displayProps.Get("materialId")));
    Render::GeometryParams geomParams(categoryId, subCategoryId);
    geomParams.SetMaterialId(materialId);
    geomParams.SetTransparency(displayProps.Get("elmTransparency").As<Napi::Number>());
    geomParams.SetLineColor(ColorDef(displayProps.Get("lineColor").As<Napi::Number>()));

    Napi::Function onLineGraphicsCb = exportProps.Get("onPartLineGraphics").As<Napi::Function>();
    bool generateLines = onLineGraphicsCb.IsFunction();

    IFacetOptionsPtr facetOptions = createFacetOptions(exportProps);
    ExportGraphicsProcessor processor(db, *facetOptions.get(), generateLines);
    ExportGraphicsContext context(processor, nullptr);
    context.SetDgnDb(db);

    auto graphic = context.CreateSceneGraphic(Transform::FromIdentity());
    GeometryStreamCR geomStream = partElement->GetGeometryStream();
    GeometryStreamIO::Collection geomCollection(geomStream.GetData(), geomStream.GetSize());
    geomCollection.Draw(*graphic, context, geomParams, true, partElement.get());

    Utf8CP errorMsg = "Element 0x%llx generated invalid geometry, this may indicate problems with the source data.";
    if (processor.m_gotBadPolyface)
        JsInterop::GetLogger().errorv(errorMsg, elementId.GetValueUnchecked());

    // TS API specifies that modifying the binding of this function in the callback will be ignored
    Napi::Function onGraphicsCb = exportProps.Get("onPartGraphics").As<Napi::Function>();
    for (auto& entry : processor.m_cachedEntries)
        {
        Napi::Object cbArgument = Napi::Object::New(Env());
        cbArgument.Set("color", Napi::Number::New(Env(), entry.color.GetValue()));
        cbArgument.Set("mesh", convertMesh(Env(), entry.mesh));
        if (entry.materialId.IsValid())
            cbArgument.Set("materialId", createIdString(Env(), entry.materialId));
        if (entry.textureId.IsValid())
            cbArgument.Set("textureId", createIdString(Env(), entry.textureId));
        onGraphicsCb.Call({ cbArgument });
        }

    for (auto& entry : processor.m_cachedLineStrings)
        {
        Napi::Object cbArgument = Napi::Object::New(Env());
        cbArgument.Set("color", Napi::Number::New(Env(), entry.color.GetValue()));
        cbArgument.Set("lines", convertLines(Env(), entry.points));
        onLineGraphicsCb.Call({ cbArgument });
        }

    return DgnDbStatus::Success;
    }
