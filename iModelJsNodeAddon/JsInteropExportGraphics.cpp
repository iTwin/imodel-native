/*--------------------------------------------------------------------------------------+
|
|     $Source: JsInteropExportGraphics.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "IModelJsNative.h"
#include <folly/BeFolly.h>
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

    void Add(ExportGraphicsMesh const& rhs)
        {
        int32_t pointOffset = (int32_t)m_points.size();
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
        int32_t newIndex; int32_t normal; FPoint2d param;
    };

    struct Node // each node is 64 bytes for cache alignment
    {
        constexpr static uint32_t ENTRY_COUNT = 4;

        NodeEntry entries[ENTRY_COUNT];
    };

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
* Linear speed unification of vertex indices. Each point has ENTRY_COUNT remappings
* that can be saved. After that, just add additional vertices for each normal/param
* combination - ROI on further remapping is poor.
+---------------+---------------+---------------+---------------+---------------+------*/
void Unify()
    {
    std::unique_ptr<Node[]> remapper(new Node[m_inPoints.size()]);
    memset(remapper.get(), 0, sizeof(Node) * m_inPoints.size());

    // Reserve for at least best case compression
    m_output.m_points.reserve(m_inPoints.size());
    m_output.m_normals.reserve(m_inPoints.size());
    m_output.m_params.reserve(m_inPoints.size());

    // Index count will remain the same
    uint32_t indexCount = (uint32_t) m_inPointIndices.size();
    m_output.m_indices.resize(indexCount);

    const float UV_TOLERANCE = 0.01f;

    for (uint32_t i = 0; i < indexCount; ++i)
        {
        int32_t origPointIndex = m_inPointIndices[i];
        int32_t origNormalIndex = m_inNormalIndices[i];
        FPoint2d origParam = m_inParams[i];

        bool foundRemap = false;
        for (int j = 0; j < Node::ENTRY_COUNT; ++j)
            {
            NodeEntry& entry = remapper[origPointIndex].entries[j];
            if (entry.newIndex == 0) // unused entry
                {
                entry.normal = origNormalIndex;
                entry.param = origParam;
                entry.newIndex = (int32_t) m_output.m_points.size() + 1; // ONE-INDEX
                m_output.m_indices[i] = (int32_t) m_output.m_points.size();
                m_output.m_points.push_back(m_inPoints[origPointIndex]);
                m_output.m_normals.push_back(m_inNormals[origNormalIndex]);
                m_output.m_params.push_back(origParam);
                foundRemap = true;
                break;
                }
            else if (entry.normal == origNormalIndex &&
                     std::abs(entry.param.x - origParam.x) < UV_TOLERANCE &&
                     std::abs(entry.param.y - origParam.y) < UV_TOLERANCE)
                {
                // reuse entry
                m_output.m_indices[i] = entry.newIndex - 1; // ONE-INDEX
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

    FastVertexUnifier unifier;
    unifier.SetInput(pfQuery, sg.GetLocalToWorldTransform(), &effectiveParams[0]);
    unifier.Unify();

    // Bucket polyfaces together by color to save cost on NAPI transition and give users a
    // minimal number of meshes for each element.
    for (auto& entry : m_cachedEntries)
        {
        if (entry.color != color || entry.textureId != textureId)
            continue;

        entry.mesh.Add(unifier.m_output);
        return true;
        }

    m_cachedEntries.emplace_back(color, textureId);
    ExportGraphicsMesh& cachedMesh = m_cachedEntries.back().mesh;
    cachedMesh.m_indices = std::move(unifier.m_output.m_indices);
    cachedMesh.m_points = std::move(unifier.m_output.m_points);
    cachedMesh.m_normals = std::move(unifier.m_output.m_normals);
    cachedMesh.m_params = std::move(unifier.m_output.m_params);
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

    ExportGraphicsJob(DgnDbR db, IFacetOptions* fo, Napi::String elementId) :
        m_geom(db), m_db(db), m_processor(db, fo), m_context(m_processor), m_elementId(elementId) { }

    void Execute()
        {
        m_context.SetDgnDb(m_db);
        m_context.VisitGeometry(m_geom);
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
    if (chordTolVal.IsNumber())
        chordTol = chordTolVal.As<Napi::Number>().DoubleValue();

    Napi::Value angleTolVal = exportProps.Get("angleTol");
    if (angleTolVal.IsNumber())
        angleTol = angleTolVal.As<Napi::Number>().DoubleValue();

    Napi::Value maxEdgeLengthVal = exportProps.Get("maxEdgeLength");
    if (maxEdgeLengthVal.IsNumber())
        maxEdgeLength = maxEdgeLengthVal.As<Napi::Number>().DoubleValue();

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

    Napi::Float32Array paramArray = Napi::Float32Array::New(env, nVertices * 2);
    for (uint32_t i = 0; i < nVertices; ++i)
        {
        paramArray[i * 2] = mesh.m_params[i].x;
        paramArray[i * 2 + 1] = mesh.m_params[i].y;
        }
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
    bvector<folly::Future<folly::Unit>> jobHandles; jobHandles.reserve(jobs.size());

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

        ExportGraphicsJob* job = new ExportGraphicsJob(db, facetOptions.get(), napiElementId);
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
