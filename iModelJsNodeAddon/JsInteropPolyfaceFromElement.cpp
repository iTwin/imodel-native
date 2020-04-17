/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "IModelJsNative.h"
#include <DgnPlatform/SimplifyGraphic.h>

using namespace IModelJsNative;

#define LOG (*NativeLogging::LoggingManager::GetLogger(L"PolyfaceFromElement"))

//=======================================================================================
// @bsistruct                                                   Matt.Gooding    04/20
//=======================================================================================
struct CreatePolyfaceProcessor : IGeometryProcessor
{
public:
    struct Entry
        {
        uint32_t                    m_lineColor;
        uint32_t                    m_fillColor;
        uint32_t                    m_materialDiffuseColor;
        bool                        m_hasMaterialDiffuse;
        PolyfaceHeaderPtr           m_polyface;
        };
    bvector<Entry>                  m_entries;
    bool                            m_gotBadPolyface;

    CreatePolyfaceProcessor(DgnDbR db, IFacetOptionsR facetOptions) :
        m_db(db), m_facetOptions(facetOptions), m_gotBadPolyface(false) { }
    virtual ~CreatePolyfaceProcessor() { }

private:
    IFacetOptionsR                  m_facetOptions;
    DgnDbR                          m_db;

    IFacetOptionsP _GetFacetOptionsP() override {return &m_facetOptions;}

    UnhandledPreference _GetUnhandledPreference(CurveVectorCR, SimplifyGraphic&) const override {return UnhandledPreference::Facet;}
    UnhandledPreference _GetUnhandledPreference(ISolidPrimitiveCR, SimplifyGraphic&) const override {return UnhandledPreference::Facet;}
    UnhandledPreference _GetUnhandledPreference(MSBsplineSurfaceCR, SimplifyGraphic&) const override {return UnhandledPreference::Facet;}
    UnhandledPreference _GetUnhandledPreference(PolyfaceQueryCR, SimplifyGraphic&) const override {return UnhandledPreference::Ignore;}
    UnhandledPreference _GetUnhandledPreference(IBRepEntityCR, SimplifyGraphic&) const override {return UnhandledPreference::Facet;}

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
* @bsimethod                                                    Matt.Gooding    04/20
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

    PolyfaceHeaderPtr newPolyface = pfQuery.Clone();
    newPolyface->Transform(sg.GetLocalToWorldTransform());

    auto graphicParams = sg.GetCurrentGraphicParams();
    uint32_t lineColor = graphicParams.GetLineColor().GetValue();
    uint32_t fillColor = graphicParams.GetFillColor().GetValue();

    uint32_t materialDiffuseColor = 0;
    bool hasMaterialDiffuse = false;
    auto materialId = sg.GetCurrentGeometryParams().GetMaterialId();
    if (materialId.IsValid())
        {
        RenderMaterialCPtr matElem = RenderMaterial::Get(m_db, materialId);
        if (matElem.IsValid())
            {
            auto asset = matElem->GetRenderingAsset();
            if (asset.GetBool(RENDER_MATERIAL_FlagHasBaseColor, false))
              {
              ColorDef diffuseColorDef;
              RgbFactor diffuseRgb = asset.GetColor(RENDER_MATERIAL_Color);
              diffuseColorDef.SetRed((Byte)(diffuseRgb.red * 255.0));
              diffuseColorDef.SetGreen((Byte)(diffuseRgb.green * 255.0));
              diffuseColorDef.SetBlue((Byte)(diffuseRgb.blue * 255.0));
              hasMaterialDiffuse = true;
              materialDiffuseColor = diffuseColorDef.GetValue();
              }
            }
        }

    m_entries.push_back({ lineColor, fillColor, materialDiffuseColor, hasMaterialDiffuse, newPolyface });

    return true;
    }
};

//=======================================================================================
// @bsistruct                                                   Matt.Gooding    04/20
//=======================================================================================
struct CreatePolyfaceGraphicsBuilder : SimplifyGraphic
{
DEFINE_T_SUPER(SimplifyGraphic);

CreatePolyfaceGraphicsBuilder(Render::GraphicBuilder::CreateParams const& cp, IGeometryProcessorR gp, ViewContextR vc)
    : T_Super(cp, gp, vc) { }
virtual ~CreatePolyfaceGraphicsBuilder() { }

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
    auto result = new CreatePolyfaceGraphicsBuilder(Render::GraphicBuilder::CreateParams::Scene(GetDgnDb(),
        Transform::FromProduct(GetLocalToWorldTransform(), subToGraphic)), m_processor, m_context);

    result->m_currGraphicParams  = m_currGraphicParams;
    result->m_currGeometryParams = m_currGeometryParams;
    result->m_currGeomEntryId    = m_currGeomEntryId;
    result->m_currClip           = (nullptr != clip ? clip->Clone(&GetLocalToWorldTransform()) : nullptr);
    return result;
    }
};

//=======================================================================================
// @bsistruct                                                   Matt.Gooding    04/20
//=======================================================================================
struct CreatePolyfaceContext : NullContext
{
    DEFINE_T_SUPER(NullContext);
    IGeometryProcessorR m_processor;

CreatePolyfaceContext(IGeometryProcessorR processor) : m_processor(processor)
    {
    m_purpose = processor._GetProcessPurpose();
    m_viewflags.SetShowConstructions(true);
    }

Render::GraphicBuilderPtr _CreateGraphic(Render::GraphicBuilder::CreateParams const& params) override
    { return new CreatePolyfaceGraphicsBuilder(params, m_processor, *this); }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    04/20
+---------------+---------------+---------------+---------------+---------------+------*/
static uint64_t getIdFromNapiValue(Napi::Value const& napiVal)
    {
    auto napiString = napiVal.As<Napi::String>();
    std::string utf8String = napiString.Utf8Value();
    return BeInt64Id::FromString(utf8String.c_str()).GetValueUnchecked();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    04/20
+---------------+---------------+---------------+---------------+---------------+------*/
static void setResponseStatus(Napi::Env& env, Napi::Object& response, DgnDbStatus status)
    {
    response.Set("status", Napi::Number::New(env, (int)status));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    04/20
+---------------+---------------+---------------+---------------+---------------+------*/
static IFacetOptionsPtr createFacetOptions(Napi::Object const& requestProps)
    {
    // The defaults for IFacetOptions::CreateForSurfaces when this function was written.
    double chordTol = 0.0;
    double angleTol = msGeomConst_piOver12;
    double maxEdgeLength = 0.0;

    Napi::Value chordTolVal = requestProps.Get("chordTol");
    if (chordTolVal.IsNumber())
        chordTol = chordTolVal.As<Napi::Number>().DoubleValue();

    // Per Earlin's advice on avoiding topology problems, restrict max angle tolerance to 45 deg
    Napi::Value angleTolVal = requestProps.Get("angleTol");
    if (angleTolVal.IsNumber())
        angleTol = std::min(msGeomConst_piOver4, angleTolVal.As<Napi::Number>().DoubleValue());

    Napi::Value maxEdgeLengthVal = requestProps.Get("maxEdgeLength");
    if (maxEdgeLengthVal.IsNumber())
        maxEdgeLength = maxEdgeLengthVal.As<Napi::Number>().DoubleValue();

    auto result = IFacetOptions::CreateForSurfaces(chordTol, angleTol, maxEdgeLength, true, true, true);
    result->SetIgnoreHiddenBRepEntities(true); // act like tile generation, big perf improvement
    result->SetBRepConcurrentFacetting(false); // see Parasolid IR 8415939, always faster exclusive

    Napi::Value minBRepFeatureSize = requestProps.Get("minBRepFeatureSize");
    if (minBRepFeatureSize.IsNumber())
        result->SetBRepIgnoredFeatureSize(minBRepFeatureSize.As<Napi::Number>().DoubleValue());

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    04/20
+---------------+---------------+---------------+---------------+---------------+------*/
static Napi::Array convertPfArray(Napi::Env& env, DPoint3dCP points, int32_t pointCount)
    {
    Napi::Array napiPoints = Napi::Array::New(env);
    for (int32_t i = 0; i < pointCount; ++i)
        {
        napiPoints.Set(i * 3, Napi::Number::New(env, points[i].x));
        napiPoints.Set(i * 3 + 1, Napi::Number::New(env, points[i].y));
        napiPoints.Set(i * 3 + 2, Napi::Number::New(env, points[i].z));
        }
    return napiPoints;
    }
static Napi::Array convertPfArray(Napi::Env& env, DPoint2dCP points, int32_t pointCount)
    {
    Napi::Array napiPoints = Napi::Array::New(env);
    for (int32_t i = 0; i < pointCount; ++i)
        {
        napiPoints.Set(i * 2, Napi::Number::New(env, points[i].x));
        napiPoints.Set(i * 2 + 1, Napi::Number::New(env, points[i].y));
        }
    return napiPoints;
    }
static Napi::Array convertPfArray(Napi::Env& env, int32_t const* indices, int32_t indexCount)
    {
    Napi::Array napiIndices = Napi::Array::New(env);
    for (int32_t i = 0; i < indexCount; ++i)
        napiIndices.Set(i, Napi::Number::New(env, indices[i]));
    return napiIndices;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    04/20
+---------------+---------------+---------------+---------------+---------------+------*/
static Napi::Value convertPfToNapi(Napi::Env& env, PolyfaceQueryCR pf)
    {
    Napi::Object indexedMesh = Napi::Object::New(env);

    indexedMesh.Set("point", convertPfArray(env, pf.GetPointCP(), (int32_t)pf.GetPointCount()));
    indexedMesh.Set("normal", convertPfArray(env, pf.GetNormalCP(), (int32_t)pf.GetNormalCount()));
    indexedMesh.Set("param", convertPfArray(env, pf.GetParamCP(), (int32_t)pf.GetParamCount()));

    indexedMesh.Set("pointIndex", convertPfArray(env, pf.GetPointIndexCP(), (int32_t)pf.GetPointIndexCount()));
    indexedMesh.Set("normalIndex", convertPfArray(env, pf.GetNormalIndexCP(), (int32_t)pf.GetPointIndexCount()));
    indexedMesh.Set("paramIndex", convertPfArray(env, pf.GetParamIndexCP(), (int32_t)pf.GetPointIndexCount()));

    return indexedMesh;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    04/20
+---------------+---------------+---------------+---------------+---------------+------*/
static void setResponseResults(Napi::Env& env, Napi::Object& responseProps, bvector<CreatePolyfaceProcessor::Entry>& entries)
    {
    Napi::Array resultsArray = Napi::Array::New(env);

    for (int32_t i = 0; i < (int32_t)entries.size(); ++i)
        {
        Napi::Object resultsObject = Napi::Object::New(env);
        resultsObject.Set("indexedMesh", convertPfToNapi(env, *entries[i].m_polyface.get()));
        resultsObject.Set("lineColor", Napi::Number::New(env, entries[i].m_lineColor));
        resultsObject.Set("fillColor", Napi::Number::New(env, entries[i].m_fillColor));
        if (entries[i].m_hasMaterialDiffuse)
          resultsObject.Set("materialDiffuseColor", Napi::Number::New(env, entries[i].m_materialDiffuseColor));
        resultsArray.Set(i, resultsObject);
        }

    responseProps.Set("results", resultsArray);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Matt.Gooding    04/20
+---------------+---------------+---------------+---------------+---------------+------*/
Napi::Value JsInterop::CreatePolyfaceFromElement(DgnDbR db, Napi::Object const& requestProps)
    {
    Napi::Object responseProps = Napi::Object::New(Env());

    DgnElementId elementId(getIdFromNapiValue(requestProps.Get("elementId")));
    if (!elementId.IsValid())
        {
        setResponseStatus(Env(), responseProps, DgnDbStatus::InvalidId);
        return responseProps;
        }

    DgnElementCPtr element = db.Elements().GetElement(elementId);
    if (!element.IsValid())
        {
        setResponseStatus(Env(), responseProps, DgnDbStatus::NotFound);
        return responseProps;
        }

    GeometrySourceCP geomSource = element->ToGeometrySource();
    if (!geomSource)
        {
        setResponseStatus(Env(), responseProps, DgnDbStatus::NoGeometry);
        return responseProps;
        }

    IFacetOptionsPtr facetOptions = createFacetOptions(requestProps);
    CreatePolyfaceProcessor processor(db, *facetOptions.get());
    CreatePolyfaceContext context(processor);
    context.SetDgnDb(db);

    try
        {
        geomSource->Draw(context, 0.0);
        }
    catch (...)
        { // Mimic TileContext::ProcessElement bomb-proofing
        Utf8CP errorMsg = "Element 0x%llx caused uncaught exception, this may indicate problems with the source data.";
        LOG.errorv(errorMsg, elementId.GetValueUnchecked());
        setResponseStatus(Env(), responseProps, DgnDbStatus::BadElement);
        return responseProps;
        }

    if (processor.m_entries.empty())
        {
        setResponseStatus(Env(), responseProps, DgnDbStatus::NoGeometry);
        return responseProps;
        }

    setResponseResults(Env(), responseProps, processor.m_entries);
    setResponseStatus(Env(), responseProps, DgnDbStatus::Success);

    return responseProps;
    }
