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

    constexpr static float UV_TOLERANCE = 0.01f;

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

// Ignore wireframe graphics, don't let SimplifyGraphic generate intermediates we always throw away
void _AddLineString(int, DPoint3dCP) override { }
void _AddLineString2d(int, DPoint2dCP, double) override { }
void _AddPointString(int, DPoint3dCP) override { }
void _AddPointString2d(int, DPoint2dCP, double) override { }
void _AddTextString(TextStringCR) override { }
void _AddTextString2d(TextStringCR, double) override { }
void _AddDgnOle(Render::DgnOleDraw*) override { }
bool _WantStrokeLineStyle(Render::LineStyleSymbCR, IFacetOptionsPtr&) override { return false; }
bool _WantStrokePattern(PatternParamsCR) override { return false; }
Render::GraphicPtr _Finish() override {m_isOpen = false; return nullptr;} // we don't use output, don't allocate!

void _AddBSplineCurve(MSBsplineCurveCR curve, bool filled) override
    { if (curve.params.closed) return T_Super::_AddBSplineCurve(curve, filled); }
void _AddBSplineCurve2d(MSBsplineCurveCR curve, bool filled, double zDepth) override
    { if (curve.params.closed) return T_Super::_AddBSplineCurve2d(curve, filled, zDepth); }

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
// @bsistruct                                                   Matt.Gooding    02/19
//=======================================================================================
struct ExportGraphicsContext : NullContext
{
DEFINE_T_SUPER(NullContext);
IGeometryProcessorR m_processor;

Render::GraphicBuilderPtr _CreateGraphic(Render::GraphicBuilder::CreateParams const& params) override
    { return new ExportGraphicsBuilder(params, m_processor, *this); }

ExportGraphicsContext(IGeometryProcessorR processor) : m_processor(processor) {m_purpose = processor._GetProcessPurpose();}
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
    bool                            m_gotBadPolyface;

    ExportGraphicsProcessor(DgnDbR db, IFacetOptionsR facetOptions) :
        m_db(db), m_facetOptions(facetOptions), m_gotBadPolyface(false) { }
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
* @bsimethod                                                    Matt.Gooding    02/19
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<Json::Value> ResolveColorAndMaterial(SimplifyGraphic& sg, ColorDef& outColor)
    {
    // Resolved fill color as baseline
    outColor = sg.GetCurrentGraphicParams().GetFillColor();

    // GraphicParams material pointer will always be null since we don't have a Render::System.
    // Look up from ID on GeometryParams instead.
    RenderMaterialId materialId = sg.GetCurrentGeometryParams().GetMaterialId();
    if (!materialId.IsValid()) return nullptr;

    RenderMaterialCPtr matElem = RenderMaterial::Get(m_db, materialId);
    if (!matElem.IsValid()) return nullptr;

    RenderingAssetCR asset = matElem->GetRenderingAsset();
    if (asset.GetBool(RENDER_MATERIAL_FlagHasBaseColor, false))
        {
        RgbFactor diffuseRgb = asset.GetColor(RENDER_MATERIAL_Color);
        outColor.SetRed((Byte)(diffuseRgb.red * 255.0));
        outColor.SetGreen((Byte)(diffuseRgb.green * 255.0));
        outColor.SetBlue((Byte)(diffuseRgb.blue * 255.0));
        }
    if (asset.GetBool(RENDER_MATERIAL_FlagHasTransmit, false))
        {
        double transparency = asset.GetDouble(RENDER_MATERIAL_Transmit, 0.0);
        outColor.SetAlpha((Byte)(transparency * 255.0));
        }

    auto patternMap = asset.GetPatternMap();
    if (!patternMap.IsValid()) return nullptr;

    return std::unique_ptr<Json::Value>(new Json::Value(patternMap.m_value));
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

    ColorDef color;
    std::unique_ptr<Json::Value> patternMapJson = ResolveColorAndMaterial(sg, color);

    uint32_t indexCount = (uint32_t)pfQuery.GetPointIndexCount();
    uint32_t indexCountWithoutBlocking = indexCount - (indexCount / 4);
    std::unique_ptr<FPoint2d[]> computedParams(new FPoint2d[indexCountWithoutBlocking]);

    TransformCR localToWorld = sg.GetLocalToWorldTransform();
    bool isMirrored = localToWorld.Determinant() < 0;

    DgnTextureId textureId;
    if (!patternMapJson.get())
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
                computedParams[computedParamCount + j].y = static_cast<float>(srcParam.y);
                }
            // Apply mirror transform while constructing so we don't have to iterate through again
            if (isMirrored) std::swap(computedParams[computedParamCount + 1], computedParams[computedParamCount + 2]);
            computedParamCount += 3;
            }
        }
    else
        {
        RenderingAsset::TextureMap textureMap(*patternMapJson.get(), RenderingAsset::TextureMap::Type::Pattern);
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
                computedParams[computedParamCount + i].y = static_cast<float>(perFaceParams[i].y);
                }
            // Apply mirror transform while constructing so we don't have to iterate through again
            if (isMirrored) std::swap(computedParams[computedParamCount + 1], computedParams[computedParamCount + 2]);
            computedParamCount += 3;
            }
        }

    // Bucket polyfaces together by color to save cost on NAPI transition and give users a
    // minimal number of meshes for each element.
    ExportGraphicsMesh* exportMesh = nullptr;
    for (auto& entry : m_cachedEntries)
        {
        if (entry.color != color || entry.textureId != textureId) continue;
        exportMesh = &entry.mesh;
        break;
        }
    if (exportMesh == nullptr)
        {
        m_cachedEntries.emplace_back(color, textureId);
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
    Napi::String                    m_elementId;
    DgnElementId                    m_debugElementId;

    ExportGraphicsJob(DgnDbR db, IFacetOptionsR fo, Napi::String elId, DgnElementId debugElId) :
        m_geom(db), m_db(db), m_processor(db, fo), m_context(m_processor), m_elementId(elId),
        m_debugElementId(debugElId) { }

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

    Napi::Value angleTolVal = exportProps.Get("angleTol");
    if (angleTolVal.IsNumber()) angleTol = angleTolVal.As<Napi::Number>().DoubleValue();

    Napi::Value maxEdgeLengthVal = exportProps.Get("maxEdgeLength");
    if (maxEdgeLengthVal.IsNumber()) maxEdgeLength = maxEdgeLengthVal.As<Napi::Number>().DoubleValue();

    auto result = IFacetOptions::CreateForSurfaces(chordTol, angleTol, maxEdgeLength, true, true, true);
    result->SetIgnoreHiddenBRepEntities(true); // act like tile generation, big perf improvement
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    02/19
+---------------+---------------+---------------+---------------+---------------+------*/
static Napi::Value convertMesh(Napi::Env env, ExportGraphicsMesh& mesh)
    {
    Napi::Object convertedMesh = Napi::Object::New(env);

    Napi::Int32Array indexArray = Napi::Int32Array::New(env, mesh.indices.size());
    memcpy (indexArray.Data(), &mesh.indices[0], mesh.indices.size() * sizeof(int32_t));
    convertedMesh.Set("indices", indexArray);

    Napi::Float64Array pointArray = Napi::Float64Array::New(env, mesh.points.size());
    memcpy (pointArray.Data(), &mesh.points[0], mesh.points.size() * sizeof(double));
    convertedMesh.Set("points", pointArray);

    Napi::Float32Array normalArray = Napi::Float32Array::New(env, mesh.normals.size());
    memcpy (normalArray.Data(), &mesh.normals[0], mesh.normals.size() * sizeof(float));
    convertedMesh.Set("normals", normalArray);

    Napi::Float32Array paramArray = Napi::Float32Array::New(env, mesh.params.size());
    memcpy (paramArray.Data(), &mesh.params[0], mesh.params.size() * sizeof(float));
    convertedMesh.Set("params", paramArray);

    return convertedMesh;
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

        ExportGraphicsJob* job = new ExportGraphicsJob(db, *facetOptions.get(), napiElementId, elementId);
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
            JsInterop::GetLogger().errorv(errorMsg, job->m_debugElementId.GetValueUnchecked());
            }

        for (auto& entry : job->m_processor.m_cachedEntries)
            {
            Napi::Object cbArgument = Napi::Object::New(Env());
            cbArgument.Set("elementId", job->m_elementId);
            cbArgument.Set("mesh", convertMesh(Env(), entry.mesh));
            cbArgument.Set("color", Napi::Number::New(Env(), entry.color.GetValue()));
            if (entry.textureId.IsValid())
                {
                Utf8String textureIdString = entry.textureId.ToString(BeInt64Id::UseHex::Yes);
                cbArgument.Set("textureId", Napi::String::New(Env(), textureIdString.c_str()));
                }
            onGraphicsCb.Call({ cbArgument });
            }

        jobs[i].reset(); // Cleaning these up now while they're still in cache is much faster
        }

    return DgnDbStatus::Success;
    }
