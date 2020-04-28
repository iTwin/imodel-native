/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "IModelJsNative.h"
#include <folly/BeFolly.h>
#include <DgnPlatform/SimplifyGraphic.h>
#include <BRepCore/PSolidUtil.h>

using namespace IModelJsNative;

#define LOG (*NativeLogging::LoggingManager::GetLogger(L"ExportGraphics"))

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

    auto addIndex = [&](int i)
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
        };
    auto isDegenerate= [](int i0, int i1, int i2) { return i0 == i1 || i0 == i2 || i1 == i2; };

    // Process as triangles so we can throw out degenerates along the way instead of needing to clean up later.
    for (uint32_t index = 0; index < indexCount; index += 3)
        {
        if (isDegenerate(inMesh.m_pointIndices[index], inMesh.m_pointIndices[index+1], inMesh.m_pointIndices[index+2]))
            continue;
        addIndex(index);
        addIndex(index+1);
        addIndex(index+2);
        }
    }

//=======================================================================================
// @bsistruct                                                   Matt.Gooding    02/19
//=======================================================================================
struct ExportGraphicsProcessor : IGeometryProcessor
{
public:
    struct CachedEntry
        {
        ColorDef            color;
        bool                isTwoSided;
        RenderMaterialId    materialId;
        DgnTextureId        textureId;
        DgnSubCategoryId    subCategoryId;
        ExportGraphicsMesh  mesh; 
        CachedEntry(ColorDef c, bool twoSided, RenderMaterialId matId, DgnTextureId texId, DgnSubCategoryId subCat)
            : color(c), isTwoSided(twoSided), materialId(matId), textureId(texId), subCategoryId(subCat) { }
        };
    bvector<CachedEntry>                    m_cachedEntries;
    struct CachedLineString
        {
        ColorDef            color;
        DgnSubCategoryId    subCategoryId;
        bvector<int>        indices;
        bvector<double>     points; // XYZXYZ, ready to go out as Float64Array
        CachedLineString(ColorDef c, DgnSubCategoryId subCat)
            : color(c), subCategoryId(subCat) { }
        };
    bvector<CachedLineString>               m_cachedLineStrings;
    bool                                    m_gotBadPolyface;
    bool                                    m_generateLines;

    struct LineStyleGeometryCacheEntry
        {
        ISolidPrimitivePtr  m_solid;
        PolyfaceHeaderPtr   m_polyface;

        LineStyleGeometryCacheEntry(ISolidPrimitiveCR solid, IFacetOptionsR facetOptions)
            {
            m_solid = solid.Clone();
            IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create(facetOptions);
            builder->AddSolidPrimitive(*m_solid.get());
            m_polyface = &builder->GetClientMeshR();
            }
        };
    bvector<LineStyleGeometryCacheEntry>    m_lineStyleGeometryCache;
    int32_t                                 m_drawingStyledCurveVectorStack;

    ExportGraphicsProcessor(DgnDbR db, IFacetOptionsR facetOptions, bool generateLines) :
        m_db(db), m_facetOptions(facetOptions), m_gotBadPolyface(false), m_generateLines(generateLines),
        m_drawingStyledCurveVectorStack(0) { }
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    07/19
+---------------+---------------+---------------+---------------+---------------+------*/
void ConvertStrokePoints(bvector<DPoint3d> const& strokePoints, bvector<int>& outIndices, bvector<double>& outPoints)
    {
    // Convert to points to XYZXYZ for Float64Array output and use indices for segments instead of disconnects.
    // See ExportGraphicsLines in iModel.js.

    // Only attempts to compress indices for contiguous points in line string.
    bool canReuseLastPoint = false;
    for (int i = 1; i < (int)strokePoints.size(); ++i)
        {
        // Remove disconnects. If there are any individual points, this will skip them.
        while (i < (int)strokePoints.size() && strokePoints[i].IsDisconnect())
            {
            i += 2;
            canReuseLastPoint = false;
            }
        if (i >= (int)strokePoints.size()) break;

        if (canReuseLastPoint)
            outIndices.push_back(outIndices.back());
        else
            {
            outIndices.push_back((int)outPoints.size() / 3);
            DPoint3dCR p0 = strokePoints[i-1];
            outPoints.insert(outPoints.end(), { p0.x, p0.y, p0.z });
            }

        outIndices.push_back((int)outPoints.size() / 3);
        DPoint3dCR p1 = strokePoints[i];
        outPoints.insert(outPoints.end(), { p1.x, p1.y, p1.z });
        canReuseLastPoint = true;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    07/19
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _ProcessCurveVector(CurveVectorCR curve, bool filled, SimplifyGraphic& sg) override
    {
    if (curve.IsAnyRegionType()) return false; // throw back to polyface processing if can be closed shape
    if (!m_generateLines) return true;

    // Prepare curve vector for output
    bvector<DPoint3d> strokePoints;
    curve.AddStrokePoints(strokePoints, m_facetOptions);
    if (strokePoints.empty()) return true;

    TransformCR transform = sg.GetLocalToWorldTransform();
    if (!transform.IsIdentity()) transform.Multiply(&strokePoints[0], (int)strokePoints.size());

    // Bucket output together based on subcategory and color
    bvector<int>* outIndices = nullptr;
    bvector<double>* outPoints = nullptr;
    DgnSubCategoryId subCategoryId = sg.GetCurrentGeometryParams().GetSubCategoryId();
    ColorDef color = sg.GetCurrentGraphicParams().GetFillColor();
    for (auto& cached : m_cachedLineStrings)
        {
        if (cached.color == color && cached.subCategoryId == subCategoryId)
            {
            outIndices = &cached.indices;
            outPoints = &cached.points;
            break;
            }
        }
    if (outIndices == nullptr)
        {
        m_cachedLineStrings.emplace_back(color, subCategoryId);
        outIndices = &m_cachedLineStrings.back().indices;
        outPoints = &m_cachedLineStrings.back().points;
        }
    
    ConvertStrokePoints(strokePoints, *outIndices, *outPoints);
    return true;
    }

    struct MeshDisplayProps
    {
        ColorDef                        color;
        bool                            isTwoSided;
        RenderMaterialId                materialId;
        // Usually no pattern map, but empty Json::Value constructor shows up in profile if instantiated
        // for display props lookup. Hide behind unique_ptr to only construct if needed.
        std::unique_ptr<Json::Value>    patternMap;
        DgnSubCategoryId                subCategoryId;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    02/19
+---------------+---------------+---------------+---------------+---------------+------*/
void ResolveMeshDisplayProps(SimplifyGraphic& sg, PolyfaceQueryCR pf, MeshDisplayProps& result)
    {
    result.isTwoSided = pf.GetTwoSided();
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
    uint32_t indexCount = (uint32_t)pfQuery.GetPointIndexCount();
    uint32_t pointCount = (uint32_t)pfQuery.GetPointCount();
    // Receiving empty polyfaces isn't ideal, but just ignore and don't count as real error condition
    // < 3 to account for valid inputs collapsed to a single sliver face.
    if (indexCount < 3 || pointCount < 3) return true;

    // Polyfaces missing requested information or in the wrong style are indicative of real problems upstream
    if (pfQuery.GetNormalIndexCP() == nullptr || pfQuery.GetNormalCount() == 0) { m_gotBadPolyface = true; return true; }
    if (pfQuery.GetParamIndexCP() == nullptr || pfQuery.GetParamCount() == 0) { m_gotBadPolyface = true; return true; }
    if (pfQuery.GetMeshStyle() != MESH_ELM_STYLE_INDEXED_FACE_LOOPS) { m_gotBadPolyface = true; return true; }
    if (!VerifyTriangulationAndZeroBlocking(pfQuery)) { m_gotBadPolyface = true; return true; }

    MeshDisplayProps displayProps;
    ResolveMeshDisplayProps(sg, pfQuery, displayProps);

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
            if (isMirrored)
                std::swap(computedParams[computedParamCount + 1], computedParams[computedParamCount + 2]);

            computedParamCount += 3;
            }
        }
    else
        {
        RenderingAsset::TextureMap textureMap(*displayProps.patternMap.get(), RenderingAsset::TextureMap::Type::Pattern);
        Render::TextureMapping::Params textureMapParams = textureMap.GetTextureMapParams();
        textureId = textureMap.GetTextureId();

        // Elevation drape needs localToWorld transform with iModel's global origin accounted for.
        DPoint3d globalOrigin = m_db.GeoLocation().GetGlobalOrigin();
        globalOrigin.Negate();
        ElevationDrapeParams drapeParams(Transform::FromProduct(localToWorld, Transform::From(globalOrigin)));

        bvector<DPoint2d> perFaceParams(3, DPoint2d::FromZero());
        int computedParamCount = 0;
        for (auto visitor = PolyfaceVisitor::Attach(pfQuery); visitor->AdvanceToNextFace(); )
            {
            textureMapParams.ComputeUVParams(perFaceParams, *visitor, &drapeParams);
            for (int i = 0; i < 3; ++i)
                {
                computedParams[computedParamCount + i].x = static_cast<float>(perFaceParams[i].x);
                // TextureMapping::Trans2x3 inverts V so that the frontend doesn't need to flip textures
                // on load. We need to invert back to match interchange conventions.
                computedParams[computedParamCount + i].y = 1.0f - static_cast<float>(perFaceParams[i].y);
                }

            // Apply mirror transform while constructing so we don't have to iterate through again
            if (isMirrored)
                std::swap(computedParams[computedParamCount + 1], computedParams[computedParamCount + 2]);

            computedParamCount += 3;
            }
        }

    // Bucket polyfaces together by shared display props to save cost on NAPI transition and give users a
    // minimal number of meshes for each element.
    ExportGraphicsMesh* exportMesh = nullptr;
    for (auto& entry : m_cachedEntries)
        {
        // Prevent bucketing into huge meshes and triggering too many reallocs
        static size_t MAX_INDICES_PER_EXPORT_GRAPHICS_MESH = 65000;
        if (entry.color != displayProps.color ||
            entry.mesh.indices.size() > MAX_INDICES_PER_EXPORT_GRAPHICS_MESH ||
            entry.isTwoSided != displayProps.isTwoSided ||
            entry.materialId != displayProps.materialId ||
            entry.textureId != textureId ||
            entry.subCategoryId != displayProps.subCategoryId)
            continue;
        exportMesh = &entry.mesh;
        break;
        }
    if (exportMesh == nullptr)
        {
        m_cachedEntries.emplace_back(displayProps.color, displayProps.isTwoSided, displayProps.materialId, textureId, displayProps.subCategoryId);
        exportMesh = &m_cachedEntries.back().mesh;
        }

    unifyIndices(IntermediateMesh(pfQuery, localToWorld, std::move(computedParams)), *exportMesh);
    return true;
    }

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    04/20
+---------------+---------------+---------------+---------------+---------------+------*/
// 3D linestyles will send through many, many instances of the same solid primitives. Some
// factors prevent these from being instanced as GeometryParts, but we can still reuse the
// constructed PolyfaceHeader for a given input for a massive perf savings.
// Only store inside a DrawStyledCurveVector call to avoid memory bloat.
// Track as a stack to avoid issues with re-entrant 3D linestyles, though hopefully no such thing exists.
void PushIsDrawingStyledCurveVector() { ++m_drawingStyledCurveVectorStack; }
void PopIsDrawingStyledCurveVector()
    {
    --m_drawingStyledCurveVectorStack;
    if (m_drawingStyledCurveVectorStack == 0)
        m_lineStyleGeometryCache.clear();
    }
bool IsDrawingStyledCurveVector() const { return 0 != m_drawingStyledCurveVectorStack; }

PolyfaceHeaderCR FindOrAddCachedLineStyleGeometry(ISolidPrimitiveCR solid)
    {
    BeAssert(IsDrawingStyledCurveVector());
    for (auto const& cacheEntry : m_lineStyleGeometryCache)
        {
        if (cacheEntry.m_solid->IsSameStructureAndGeometry(solid))
            return *cacheEntry.m_polyface.get();
        }
    
    m_lineStyleGeometryCache.emplace_back(solid, m_facetOptions);
    return *m_lineStyleGeometryCache.back().m_polyface.get();
    }
}; // ExportGraphicsProcessor


//=======================================================================================
// @bsistruct                                                   Matt.Gooding    04/19
//=======================================================================================
struct ExportGraphicsBuilder : SimplifyGraphic
{
public:
    DEFINE_T_SUPER(SimplifyGraphic);

    ExportGraphicsProcessor& m_exportGraphicsProcessor;

ExportGraphicsBuilder(Render::GraphicBuilder::CreateParams const& cp, ExportGraphicsProcessor& gp, ViewContextR vc)
    :
    T_Super(cp, gp, vc),
    m_exportGraphicsProcessor(gp)
    { }
virtual ~ExportGraphicsBuilder() { }

// Don't let SimplifyGraphic generate intermediates we always throw away
void _AddTextString(TextStringCR) override { }
void _AddTextString2d(TextStringCR, double) override { }
void AddImage(ImageGraphicCR) override { }
void AddImage2d(ImageGraphicCR, double) override { }
bool _WantStrokeLineStyle(Render::LineStyleSymbCR symb, IFacetOptionsPtr&) override { return !symb.IsCosmetic(); }
bool _WantStrokePattern(PatternParamsCR) override { return false; }
Render::GraphicPtr _Finish() override {m_isOpen = false; return nullptr;} // we don't use output, don't allocate!

// Copy of SimplifyGraphic::_CreateSubGraphic to generate derived class
Render::GraphicBuilderPtr _CreateSubGraphic(TransformCR subToGraphic, ClipVectorCP clip) const override
    {
    auto result = new ExportGraphicsBuilder(Render::GraphicBuilder::CreateParams::Scene(GetDgnDb(),
        Transform::FromProduct(GetLocalToWorldTransform(), subToGraphic)), (ExportGraphicsProcessor&)m_processor, m_context);

    result->m_currGraphicParams  = m_currGraphicParams;
    result->m_currGeometryParams = m_currGeometryParams;
    result->m_currGeomEntryId    = m_currGeomEntryId;
    result->m_currClip           = (nullptr != clip ? clip->Clone(&GetLocalToWorldTransform()) : nullptr);
    return result;
    }

void _AddSolidPrimitive(ISolidPrimitiveCR geom) override
    {
    if (!m_exportGraphicsProcessor.IsDrawingStyledCurveVector() || nullptr != GetCurrentClip())
        {
        T_Super::_AddSolidPrimitive(geom);
        return;
        }

    m_processor._ProcessPolyface(m_exportGraphicsProcessor.FindOrAddCachedLineStyleGeometry(geom), false, *this);
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
    ExportGraphicsProcessor&        m_processor;
    bvector<PartInstanceRecord>*    m_instances;

ExportGraphicsContext(ExportGraphicsProcessor& processor, bvector<PartInstanceRecord>* instances)
    : m_processor(processor), m_instances(instances)
    {
    m_purpose = processor._GetProcessPurpose();
    m_viewflags.SetShowConstructions(true);
    }

Render::GraphicBuilderPtr _CreateGraphic(Render::GraphicBuilder::CreateParams const& params) override
    { return new ExportGraphicsBuilder(params, m_processor, *this); }

void _DrawStyledCurveVector(Render::GraphicBuilderR builder, CurveVectorCR curve, Render::GeometryParamsR params, bool doCook) override
    {
    m_processor.PushIsDrawingStyledCurveVector();
    T_Super::_DrawStyledCurveVector(builder, curve, params, doCook);
    m_processor.PopIsDrawingStyledCurveVector();
    }

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
    bool                            m_caughtException;

ExportGraphicsJob(DgnDbR db, IFacetOptionsR fo, DgnElementId elId, bool saveInstances, bool generateLines)
    : m_geom(db), m_db(db), m_processor(db, fo, generateLines), m_elementId(elId),
    m_context(m_processor, saveInstances ? &m_instances : nullptr), m_caughtException(false)
    {
    }

void Execute()
    {
    try
        {
        PSolidThreadUtil::WorkerThreadErrorHandler errorHandler; // Needed to handle errors and clear thread exclusion.
        m_context.SetDgnDb(m_db);
        m_geom.Draw(m_context, 0);
        }
    catch (...)
        { // Mimic TileContext::ProcessElement bomb-proofing. Necessary for Parasolid error handling at a minimum.
        m_caughtException = true;
        }
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

    Napi::Value minBRepFeatureSize = exportProps.Get("minBRepFeatureSize");
    if (minBRepFeatureSize.IsNumber())
        result->SetBRepIgnoredFeatureSize(minBRepFeatureSize.As<Napi::Number>().DoubleValue());

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    07/19
+---------------+---------------+---------------+---------------+---------------+------*/
static Napi::Value convertLines(Napi::Env& env, bvector<int>& exportIndices, bvector<double>& exportPoints)
    {
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
static Napi::Value convertMesh(Napi::Env& env, ExportGraphicsMesh& mesh, bool isTwoSided)
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
    convertedMesh.Set("isTwoSided", Napi::Boolean::New(env, isTwoSided));
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

        if (job->m_caughtException)
            {
            Utf8CP errorMsg = "Element 0x%llx caused uncaught exception, this may indicate problems with the source data.";
            LOG.errorv(errorMsg, job->m_elementId.GetValueUnchecked());
            jobs[i].reset();
            continue; // State is invalid - clean up and ignore this element.
            }

        if (job->m_processor.m_gotBadPolyface)
            {
            Utf8CP errorMsg = "Element 0x%llx generated invalid geometry, this may indicate problems with the source data.";
            LOG.errorv(errorMsg, job->m_elementId.GetValueUnchecked());
            // Bad polyface is handled gracefully, OK to continue in case other valid geometry was generated
            }

        Napi::String elementIdString = createIdString(Env(), job->m_elementId);
        for (auto& entry : job->m_processor.m_cachedEntries)
            {
             // Can happen if all triangles are degenerate
            if (entry.mesh.indices.empty()) continue;
            Napi::Object cbArgument = Napi::Object::New(Env());
            cbArgument.Set("elementId", elementIdString);
            cbArgument.Set("mesh", convertMesh(Env(), entry.mesh, entry.isTwoSided));
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
            if (entry.indices.empty()) continue;
            Napi::Object cbArgument = Napi::Object::New(Env());
            cbArgument.Set("elementId", elementIdString);
            cbArgument.Set("subCategory", createIdString(Env(), entry.subCategoryId));
            cbArgument.Set("color", Napi::Number::New(Env(), entry.color.GetValue()));
            cbArgument.Set("lines", convertLines(Env(), entry.indices, entry.points));
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

    try
        {
        geomCollection.Draw(*graphic, context, geomParams, true, partElement.get());
        }
    catch (...)
        { // Protect against Parasolid exceptions
        Utf8CP errorMsg = "GeometryPart 0x%llx caused uncaught exception, this may indicate problems with the source data.";
        LOG.errorv(errorMsg, elementId.GetValueUnchecked());
        }

    Utf8CP errorMsg = "Element 0x%llx generated invalid geometry, this may indicate problems with the source data.";
    if (processor.m_gotBadPolyface)
        LOG.errorv(errorMsg, elementId.GetValueUnchecked());

    // TS API specifies that modifying the binding of this function in the callback will be ignored
    Napi::Function onGraphicsCb = exportProps.Get("onPartGraphics").As<Napi::Function>();
    for (auto& entry : processor.m_cachedEntries)
        {
        // Can happen if all triangles are degenerate
        if (entry.mesh.indices.empty()) continue;
        Napi::Object cbArgument = Napi::Object::New(Env());
        cbArgument.Set("color", Napi::Number::New(Env(), entry.color.GetValue()));
        cbArgument.Set("mesh", convertMesh(Env(), entry.mesh, entry.isTwoSided));
        if (entry.materialId.IsValid())
            cbArgument.Set("materialId", createIdString(Env(), entry.materialId));
        if (entry.textureId.IsValid())
            cbArgument.Set("textureId", createIdString(Env(), entry.textureId));
        onGraphicsCb.Call({ cbArgument });
        }

    for (auto& entry : processor.m_cachedLineStrings)
        {
        if (entry.indices.empty()) continue;
        Napi::Object cbArgument = Napi::Object::New(Env());
        cbArgument.Set("color", Napi::Number::New(Env(), entry.color.GetValue()));
        cbArgument.Set("lines", convertLines(Env(), entry.indices, entry.points));
        onLineGraphicsCb.Call({ cbArgument });
        }

    return DgnDbStatus::Success;
    }
