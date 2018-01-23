/*--------------------------------------------------------------------------------------+                                                                                           
|
|     $Source: DgnCore/RenderPrimitives.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/RenderPrimitives.h>
#if defined(BENTLEYCONFIG_PARASOLID)
#include <DgnPlatform/DgnBRep/PSolidUtil.h>
#endif

USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_BENTLEY_RENDER_PRIMITIVES

#define COMPARE_VALUES_TOLERANCE(val0, val1, tol)   if (val0 < val1 - tol) return true; if (val0 > val1 + tol) return false;
#define COMPARE_VALUES(val0, val1) if (val0 < val1) { return true; } if (val0 > val1) { return false; }

// #define DEBUG_DISPLAY_PARAMS_CACHE

BEGIN_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void copy2dTo3d(int numPoints, DPoint3dP pts3d, DPoint2dCP pts2d, double priority)
    {
    double depth = Render::Target::DepthFromDisplayPriority(static_cast<int32_t>(priority));
    for (int i = 0; i < numPoints; i++, pts3d++, pts2d++)
        pts3d->Init(pts2d->x, pts2d->y, depth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void copy2dTo3d(std::valarray<DPoint3d>& pts3d, DPoint2dCP pts2d, double priority)
    {
    copy2dTo3d(static_cast<int>(pts3d.size()), &pts3d[0], pts2d, priority);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
static FPoint2d toFPoint2d(DPoint2dCR dpoint)
    {
    FPoint2d fpoint;
    fpoint.x = dpoint.x;
    fpoint.y = dpoint.y;
    return fpoint;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void collectCurveStrokes (Strokes::PointLists& strokes, CurveVectorCR curve, IFacetOptionsR facetOptions, TransformCP transform)
    {
    bvector <bvector<bvector<DPoint3d>>> strokesArray;

    curve.CollectLinearGeometry (strokesArray, &facetOptions);

    for (auto& loop : strokesArray)
        {
        for (auto& loopStrokes : loop)
            {
            if (nullptr != transform)
                transform->Multiply(loopStrokes, loopStrokes);

            DRange3d    range = DRange3d::From(loopStrokes);
            strokes.push_back (Strokes::PointList(std::move(loopStrokes), DPoint3d::FromInterpolate(range.low, .5, range.high)));
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void collectCurveStrokes (Strokes::PointLists& strokes, CurveVectorCR curve, IFacetOptionsR facetOptions, TransformCR transform)
    {
    return collectCurveStrokes(strokes, curve, facetOptions, &transform);
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct PrimitiveGeometry : Geometry
{
private:
    IGeometryPtr        m_geometry;
    bool                m_inCache = false;
    bool                m_disjoint;

    PrimitiveGeometry(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsCR params, bool isCurved, DgnDbR db, bool disjoint)
        : Geometry(tf, range, elemId, params, isCurved, db), m_geometry(&geometry), m_disjoint(disjoint) { }

    PolyfaceList _GetPolyfaces(IFacetOptionsR facetOptions, ViewContextR context) override;
    StrokesList _GetStrokes (IFacetOptionsR facetOptions, ViewContextR context) override;
    bool _DoDecimate () const override { return m_geometry->GetAsPolyfaceHeader().IsValid(); }
    size_t _GetFacetCount(FacetCounter& counter) const override { return counter.GetFacetCount(*m_geometry); }
    void _SetInCache(bool inCache) override { m_inCache = inCache; }

    static PolyfaceHeaderPtr FixPolyface(PolyfaceHeaderR, IFacetOptionsR);
    static void AddNormals(PolyfaceHeaderR, IFacetOptionsR);
    static void AddParams(PolyfaceHeaderR, IFacetOptionsR);
public:
    static GeometryPtr Create(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsCR params, bool isCurved, DgnDbR db, bool disjoint)
        {
        BeAssert(!disjoint || geometry.GetAsCurveVector().IsValid());
        return new PrimitiveGeometry(geometry, tf, range, elemId, params, isCurved, db, disjoint);
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct SolidKernelGeometry : Geometry
{
private:
    IBRepEntityPtr      m_entity;

    SolidKernelGeometry(IBRepEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsCR params, DgnDbR db);

    PolyfaceList _GetPolyfaces(IFacetOptionsR facetOptions, ViewContextR context) override;
    size_t _GetFacetCount(FacetCounter& counter) const override { return counter.GetFacetCount(*m_entity); }
public:
    static GeometryPtr Create(IBRepEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsCR params, DgnDbR db)
        {
        return new SolidKernelGeometry(solid, tf, range, elemId, params, db);
        }
};

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     11/2016
//=======================================================================================
struct TextStringGeometry : Geometry
{
private:
    TextStringPtr                   m_text;
    mutable bvector<CurveVectorPtr> m_glyphCurves;
    DgnDbR                          m_db;
    bool                            m_checkGlyphBoxes;

    TextStringGeometry(TextStringR text, TransformCR transform, DRange3dCR range, DgnElementId elemId, DisplayParamsCR params, DgnDbR db, bool checkGlyphBoxes)
        : Geometry(transform, range, elemId, params, true, db), m_text(&text), m_checkGlyphBoxes(checkGlyphBoxes), m_db(db)
        { 
        //
        }

    bool _DoVertexCluster() const override { return false; }

public:
    static GeometryPtr Create(TextStringR textString, TransformCR transform, DRange3dCR range, DgnElementId elemId, DisplayParamsCR params, DgnDbR db, bool checkGlyphBoxes)
        {
        return new TextStringGeometry(textString, transform, range, elemId, params, db, checkGlyphBoxes);
        }
    
    bool DoGlyphBoxes (IFacetOptionsR facetOptions);
    PolyfaceList _GetPolyfaces(IFacetOptionsR facetOptions, ViewContextR context) override;
    StrokesList _GetStrokes (IFacetOptionsR facetOptions, ViewContextR context) override;
    size_t _GetFacetCount(FacetCounter& counter) const override;

    DgnDbR GetDb() const { return m_db; }
    TextStringCR GetText() const { return *m_text; }
};  // TextStringGeometry

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     11/2016
//=======================================================================================
struct GeomPartInstanceGeometry : Geometry
{
private:
    GeomPartPtr     m_part;

    GeomPartInstanceGeometry(GeomPartR  part, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsCR params, DgnDbR db)
        : Geometry(tf, range, elemId, params, part.IsCurved(), db), m_part(&part) { }

public:
    static GeometryPtr Create(GeomPartR  part, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsCR params, DgnDbR db)  { return new GeomPartInstanceGeometry(part, tf, range, elemId, params, db); }

    PolyfaceList _GetPolyfaces(IFacetOptionsR facetOptions, ViewContextR context) override { return m_part->GetPolyfaces(facetOptions, *this, context); }
    StrokesList _GetStrokes (IFacetOptionsR facetOptions, ViewContextR context) override { return m_part->GetStrokes(facetOptions, *this, context); }
    size_t _GetFacetCount(FacetCounter& counter) const override { return m_part->GetFacetCount (counter, *this); }
    GeomPartCPtr _GetPart() const override { return m_part; }

};  // GeomPartInstanceTileGeometry 

//=======================================================================================
//! Text is frequently used for decorations, which are regenerated constantly; and
//! glyphs can be too expensive to regenerate and re-facet from scratch every frame.
//! Cache them a la qv_addGlyphToFont(), but tie their lifetimes to that of the DgnDb.
// @bsistruct                                                   Paul.Connelly   11/17
//=======================================================================================
struct GlyphCache : DgnDb::AppData
{
    enum Tolerance
    {
        kTolerance_Largest,
        kTolerance_Large,
        kTolerance_Small,
        kTolerance_Smallest,
        kTolerance_COUNT
    };

    static double GetTolerance(Tolerance tol) { return s_tolerances[tol]; }
private:
    //=======================================================================================
    // @bsistruct                                                   Paul.Connelly   11/17
    //=======================================================================================
    struct Geom
    {
    private:
        Strokes::PointLists m_strokes;
        PolyfaceHeaderPtr   m_polyface;
        PolyfaceHeaderPtr   m_rasterPolyface;
    public:
        Geom() = default;

        Strokes::PointLists GetStrokes() const { return m_strokes; }
        PolyfaceHeaderPtr GetPolyface() const { return m_polyface.IsValid() ? m_polyface->Clone() : nullptr; }
        PolyfaceHeaderPtr GetRasterPolyface() const { return m_rasterPolyface.IsValid() ? m_rasterPolyface->Clone() : nullptr; }
        bool IsValid() const { return m_polyface.IsValid() || !m_strokes.empty(); }
        void Invalidate() { m_polyface = nullptr; m_strokes.clear(); }
        void InitStrokes(CurveVectorCR curves, IFacetOptionsR facetOptions)
            {
            m_strokes.clear();
            collectCurveStrokes(m_strokes, curves, facetOptions, nullptr);
            }
        void InitPolyface(CurveVectorCR curves, IFacetOptionsR facetOptions)
            {
            IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create(facetOptions);
            builder->AddRegion(curves);
            m_polyface = builder->GetClientMeshPtr();
            }
        bool IsValidRaster() const { return m_rasterPolyface.IsValid(); }
        void InvalidateRaster() { m_rasterPolyface = nullptr; }
        void InitRasterPolyface(CurveVectorCR curves, IFacetOptionsR facetOptions);
    };

    static double s_tolerances[kTolerance_COUNT];

    //=======================================================================================
    // @bsistruct                                                   Paul.Connelly   11/17
    //=======================================================================================
    struct Glyph
    {
        struct Raster
        {
            Image       m_image;
            Utf8String  m_name;
            TexturePtr  m_texture;

            Raster() { } // for bmap...
            Raster(DgnGlyphCR glyph, DgnFontCR font)
                {
                m_name.Sprintf("%s-%u", font.GetName().c_str(), glyph.GetId());

                /*DgnGlyph::RasterStatus status = */ glyph.GetRaster(m_image);
                //if (DgnGlyph::RasterStatus::Success != status)
                //    DEBUG_PRINTF("Error %u retrieving raster", status);

                //UNUSED_VARIABLE(status);

#if defined(DEBUG_PRINT_GLYPH_RASTERS)
                uint32_t imgWidth = m_image.GetWidth();
                uint32_t imgHeight = m_image.GetHeight();
                int imgBytesPerPixel = m_image.GetBytesPerPixel();
                ByteStream const& imgByteStream = m_image.GetByteStream();
                uint8_t const* imgData = imgByteStream.GetData();

                DEBUG_PRINTF("");
                DEBUG_PRINTF("Retrieving raster for glyph %d (%dx%d)", glyph.GetId(), imgWidth, imgHeight);

                for (uint32_t y = 0; y < imgHeight; y++)
                    {
                    std::string str = "";
                    for (uint32_t x = 0; x < imgWidth; x++)
                        {
                        uint32_t imgNdx = y * imgWidth * imgBytesPerPixel + x * imgBytesPerPixel;
                        if (imgData[imgNdx + 3] > 0)
                            str += "X";
                        else
                            str += " ";
                        }
                    DEBUG_PRINTF("%s", str.c_str());
                    }
#endif
                }

            bool IsValid() const { return m_texture.IsValid() || m_image.IsValid(); }
            TexturePtr GetTexture(SystemR system, DgnDbR db)
                {
                if (m_texture.IsNull() && m_image.IsValid())
                    {
                    TextureKey key(m_name);
                    m_texture = system._FindTexture(key, db);
                    if (m_texture.IsNull())
                        {
                        Texture::CreateParams params(key);
                        params.m_isGlyph = true;
                        m_texture = system._CreateTexture(m_image, db, params);
                        }

                    m_image.Invalidate();
                    BeAssert(m_texture.IsValid());
                    }

                return m_texture;
                }
        };

    struct RasterPolyface
    {
        PolyfaceHeaderPtr   m_polyface;
        TexturePtr          m_texture;

        bool IsValid() const { return m_polyface.IsValid() && m_texture.IsValid(); }
    };

    private:
        using Tolerance = GlyphCache::Tolerance;

        CurveVectorPtr      m_curves;
        Raster              m_raster;
        Geom                m_geom[kTolerance_COUNT];
        double              m_smallestTolerance;
        bool                m_isCurved;
        bool                m_isStrokes;

        template<typename T, typename U> void GetGeometry(IFacetOptionsR, bool requestRaster, T createGeom, U acceptGeom);
    public:
        Glyph() = default; // for bmap...
        Glyph(DgnGlyphCR glyph, DgnFontCR font) : m_raster(glyph, font)
            {
            bool unusedArg;
            if ((m_curves = glyph.GetCurveVector(unusedArg)).IsValid())
                {
                m_isCurved = m_curves->ContainsNonLinearPrimitive();
                m_isStrokes = !m_curves->IsAnyRegionType();

                // If we receive a request for a tolerance smaller than our 'smallest', we'll re-facet and replace our cached geometry.
                m_smallestTolerance = GetTolerance(kTolerance_Smallest);
                }
            }

        bool IsValid() const { return m_curves.IsValid(); }
        bool IsStrokes() const { return m_isStrokes; }
        bool IsCurved() const { return m_isCurved; }
        bool HasRaster() const { return m_raster.IsValid(); }

        Strokes::PointLists GetStrokes(IFacetOptionsR facetOptions)
            {
            BeAssert(IsStrokes());
            Strokes::PointLists strokes;
            GetGeometry(facetOptions, false,
                    [&](Geom& geom) { geom.InitStrokes(*m_curves, facetOptions); },
                    [&](Geom& geom) { strokes = geom.GetStrokes(); });
            return strokes;
            }
        PolyfaceHeaderPtr GetPolyface(IFacetOptionsR facetOptions)
            {
            BeAssert(!IsStrokes());
            PolyfaceHeaderPtr polyface;
            GetGeometry(facetOptions, false,
                    [&](Geom& geom) { geom.InitPolyface(*m_curves, facetOptions); },
                    [&](Geom& geom) { polyface = geom.GetPolyface(); });
            return polyface;
            }
        RasterPolyface GetRasterPolyface(IFacetOptionsR facetOptions, SystemR system, DgnDbR db)
            {
            BeAssert(!IsStrokes()); // ###TODO: support rasterizing strokes (stick text)
            RasterPolyface polyface;
            GetGeometry(facetOptions, true,
                    [&](Geom& geom) { geom.InitRasterPolyface(*m_curves, facetOptions); },
                    [&](Geom& geom) { polyface.m_polyface = geom.GetRasterPolyface(); polyface.m_texture = m_raster.GetTexture(system, db); });
            return polyface;
            }

        Tolerance GetTolerance(double tol) const
            {
            if (m_isCurved)
                {
                for (uint8_t iTol = kTolerance_Largest; iTol < kTolerance_Smallest; iTol++)
                    {
                    auto kTol = static_cast<Tolerance>(iTol);
                    if (tol >= GlyphCache::GetTolerance(kTol))
                        return static_cast<Tolerance>(kTol);
                    }
                }

            return kTolerance_Smallest;
            }
        double GetChordTolerance(Tolerance tol, double requestedTolerance) const
            {
            if (!IsCurved())
                return 0.0;
            else if (kTolerance_Smallest == tol && requestedTolerance < m_smallestTolerance)
                return requestedTolerance;
            else
                return GlyphCache::GetTolerance(tol);
            }
    };

    static Key const& GetKey() { static Key s_key; return s_key; }

    using Map = bmap<DgnGlyphCP, Glyph>;

    Map m_map;

    GlyphCache() { }

    static GlyphCache& Get(DgnDbR db)
        {
        return static_cast<GlyphCache&>(*db.FindOrAddAppData(GetKey(), []() { return new GlyphCache(); }));
        }

    static IFacetOptionsPtr CreateFacetOptions(double tolerance);

    Glyph* FindOrInsert(DgnGlyphCR, DgnFontCR font);
    void GetGeometry(StrokesList* strokes, PolyfaceList* polyfaces, TextStringGeometry const& text, double tolerance, ViewContextR context);
    static void GetTextGeometry(StrokesList* strokes, PolyfaceList* polyfaces, TextStringGeometry const& text, double tolerance, ViewContextR context)
        {
        // Because we need the font mutex in order to operate on the glyphs, also use it to protect our internal data.
        BeMutexHolder lock(DgnFonts::GetMutex());
        Get(text.GetDb()).GetGeometry(strokes, polyfaces, text, tolerance, context);
        }
public:
    static void GetGeometry(StrokesList& strokes, PolyfaceList& polyfaces, TextStringGeometry const& text, double tolerance, ViewContextR context)
        {
        return GetTextGeometry(&strokes, &polyfaces, text, tolerance, context);
        }
    static PolyfaceList GetPolyfaces(TextStringGeometry const& text, double tolerance, ViewContextR context)
        {
        PolyfaceList polyfaces;
        GetTextGeometry(nullptr, &polyfaces, text, tolerance, context);
        return polyfaces;
        }
    static StrokesList GetStrokes(TextStringGeometry const& text, double tolerance, ViewContextR context)
        {
        StrokesList strokes;
        GetTextGeometry(&strokes, nullptr, text, tolerance, context);
        return strokes;
        }
};

double GlyphCache::s_tolerances[GlyphCache::kTolerance_COUNT] =
    {
    1.0,
    0.1,
    0.01,
    0.0001,
    };

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayParams::InitGeomParams(DgnCategoryId catId, DgnSubCategoryId subCatId, DgnGeometryClass geomClass)
    {
    m_categoryId = catId;
    m_subCategoryId = subCatId;
    m_class = geomClass;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayParams DisplayParams::ForType(Type type, GraphicParamsCR gf, GeometryParamsCP geom, bool filled, DgnDbR db, System& sys)
    {
    switch (type)
        {
        case Type::Mesh:    return ForMesh(gf, geom, filled, db, sys);
        case Type::Text:    return ForText(gf, geom);
        case Type::Linear:  return ForLinear(gf, geom);
        default:            BeAssert(false); return ForText(gf, geom);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayParams::InitText(ColorDef lineColor, DgnCategoryId catId, DgnSubCategoryId subCatId, DgnGeometryClass geomClass)
    {
    InitGeomParams(catId, subCatId, geomClass);
    m_type = Type::Text;
    m_lineColor = m_fillColor = lineColor;
    m_ignoreLighting = true;
    m_fillFlags = FillFlags::Always;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayParams DisplayParams::ForText(GraphicParamsCR gf, GeometryParamsCP geom)
    {
    DgnCategoryId catId;
    DgnSubCategoryId subCatId;
    DgnGeometryClass geomClass = DgnGeometryClass::Primary;
    if (nullptr != geom)
        {
        catId = geom->GetCategoryId();
        subCatId = geom->GetSubCategoryId();
        geomClass = geom->GetGeometryClass();
        }

    return DisplayParams(gf.GetLineColor(), catId, subCatId, geomClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayParams::InitLinear(ColorDef lineColor, uint32_t width, LinePixels pixels, DgnCategoryId catId, DgnSubCategoryId subCatId, DgnGeometryClass geomClass)
    {
    InitGeomParams(catId, subCatId, geomClass);
    m_type = Type::Linear;
    m_lineColor = m_fillColor = lineColor;
    m_width = width;
    m_linePixels = pixels;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayParams DisplayParams::ForLinear(GraphicParamsCR gf, GeometryParamsCP geom)
    {
    DgnCategoryId catId;
    DgnSubCategoryId subCatId;
    DgnGeometryClass geomClass = DgnGeometryClass::Primary;
    if (nullptr != geom)
        {
        catId = geom->GetCategoryId();
        subCatId = geom->GetSubCategoryId();
        geomClass = geom->GetGeometryClass();
        }

    return DisplayParams(gf.GetLineColor(), gf.GetWidth(), static_cast<LinePixels>(gf.GetLinePixels()), catId, subCatId, geomClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayParams::InitMesh(ColorDef lineColor, ColorDef fillColor, uint32_t width, LinePixels pixels, MaterialP mat, GradientSymbCP grad, TextureMappingCP tex,
    FillFlags fillFlags, DgnCategoryId catId, DgnSubCategoryId subCatId, DgnGeometryClass geomClass)
    {
    InitGeomParams(catId, subCatId, geomClass);
    m_type = Type::Mesh;
    m_lineColor = lineColor;
    m_fillColor = fillColor;
    m_fillFlags = fillFlags;
    m_material = mat;
    m_gradient = grad;
    m_width = width;
    m_linePixels = pixels;
    m_hasRegionOutline = ComputeHasRegionOutline();
    
    if (nullptr == mat && nullptr != tex)
        m_textureMapping = *tex;

    BeAssert(m_gradient.IsNull() || m_textureMapping.IsValid());
    BeAssert(m_gradient.IsNull() || m_gradient->GetRefCount() > 1); // assume caller allocated on heap...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayParams DisplayParams::ForMesh(GraphicParamsCR gf, GeometryParamsCP geom, bool filled, DgnDbR db, System& sys)
    {
    DgnCategoryId catId;
    DgnSubCategoryId subCatId;
    DgnGeometryClass geomClass = DgnGeometryClass::Primary;
    FillFlags fillFlags = filled ? FillFlags::ByView : FillFlags::None;
    if (gf.IsBlankingRegion())
        fillFlags |= FillFlags::Blanking;

    // TFS#786614: BRep with multiple face attachments - params may no longer be resolved.
    // Doesn't matter - we will create GeometryParams for each of the face attachments - only need the
    // class, category, and sub-category here.
    if (nullptr != geom && geom->IsResolved())
        {
        catId = geom->GetCategoryId();
        subCatId = geom->GetSubCategoryId();
        geomClass = geom->GetGeometryClass();

        if (nullptr != geom->GetPatternParams())
            fillFlags |= FillFlags::Behind;

        if (filled)
            {
            if (FillDisplay::Always == geom->GetFillDisplay())
                {
                fillFlags |= FillFlags::Always;
                fillFlags&= ~FillFlags::ByView;
                }

            if (geom->IsFillColorFromViewBackground())
                fillFlags |= FillFlags::Background;
            }
        }

    auto grad = gf.GetGradientSymb();
    TextureMapping tex;
    TextureMappingP pTex = nullptr;
    if (nullptr != grad)
        {
        tex = TextureMapping(*sys._GetTexture(*grad, db));
        pTex = &tex;
        }

    return DisplayParams(gf.GetLineColor(), gf.GetFillColor(), gf.GetWidth(), static_cast<LinePixels>(gf.GetLinePixels()), gf.GetMaterial(), grad, pTex, fillFlags, catId, subCatId, geomClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/17
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayParamsCPtr DisplayParams::CreateForGeomPartInstance(DisplayParamsCR part, DisplayParamsCR inst)
    {
    if (part.GetType() == inst.GetType())
        return &inst;

    // We initially create the instance params via CreateForMesh(), because we don't know what type(s) of geometry the part will contain.
    // Need to fix it up for linear/text
    BeAssert(Type::Mesh == inst.GetType());
    DisplayParamsPtr clone(new DisplayParams(part));
    clone->m_fillColor = clone->m_lineColor = inst.m_lineColor;
    clone->m_categoryId = inst.m_categoryId;
    clone->m_subCategoryId = inst.m_subCategoryId;
    clone->m_class = inst.m_class;

    if (Type::Linear == clone->GetType())
        {
        clone->m_width = inst.m_width;
        clone->m_linePixels = inst.m_linePixels;
        }

    return clone.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisplayParams::ComputeHasRegionOutline() const
    {
    if (m_gradient.IsValid())
        return m_gradient->GetIsOutlined();
    else
        return !NeverRegionOutline() && m_fillColor != m_lineColor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisplayParams::HasRegionOutline() const
    {
    return m_hasRegionOutline;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayParamsCPtr DisplayParams::Clone() const
    {
    return new DisplayParams(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayParamsCPtr DisplayParams::CloneForRasterText(TextureR texture) const
    {
    BeAssert(Type::Text == GetType());
    auto clone = new DisplayParams(*this);

    TextureMapping::Trans2x3 tf(0.0, 1.0, 0.0, 1.0, 0.0, 0.0);
    TextureMapping::Params params;
    params.SetWeight(0.0);
    params.SetTransform(&tf);
    clone->m_textureMapping = TextureMapping(texture, params);
    return clone;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static int compareValues(T const& lhs, T const& rhs)
    {
    return lhs == rhs ? 0 : (lhs < rhs ? -1 : 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
template<> int compareValues(bool const& lhs, bool const& rhs)
    {
    return compareValues(static_cast<uint8_t>(lhs), static_cast<uint8_t>(rhs));
    }

#define TEST_LESS_THAN(MEMBER) \
    { \
    int cmp = compareValues(MEMBER, rhs.MEMBER); \
    if (0 != cmp) \
        return cmp < 0; \
    }

#define TEST_EQUAL(MEMBER) if (MEMBER != rhs.MEMBER) return false

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisplayParams::IsLessThan(DisplayParamsCR rhs, ComparePurpose purpose) const
    {
    if (&rhs == this)
        return false;

#if defined(DEBUG_DISPLAY_PARAMS_CACHE)
    if (ComparePurpose::Merge == purpose)
        printf("Comparing:%s\n       to:%s\n", ToDebugString().c_str(), rhs.ToDebugString().c_str());
#endif

    TEST_LESS_THAN(GetType());
    TEST_LESS_THAN(IgnoresLighting());
    TEST_LESS_THAN(GetLineWidth());
    TEST_LESS_THAN(GetMaterial());
    TEST_LESS_THAN(GetLinePixels());
    TEST_LESS_THAN(GetFillFlags());
    TEST_LESS_THAN(HasRegionOutline());
    TEST_LESS_THAN(GetTextureMapping().GetTexture()); // ###TODO: Care about whether params match?

    if (ComparePurpose::Merge == purpose)
        {
        TEST_LESS_THAN(HasFillTransparency());
        TEST_LESS_THAN(HasLineTransparency());

        if (GetTextureMapping().IsValid())
            TEST_LESS_THAN(GetFillColor());     // Textures may use color so they can't be merged. (could test if texture actually uses color).

#if defined(DEBUG_DISPLAY_PARAMS_CACHE)
        printf ("Equal.\n");
#endif
        return false;
        }

    TEST_LESS_THAN(GetFillColor());
    TEST_LESS_THAN(GetLineColor());
    TEST_LESS_THAN(GetCategoryId().GetValueUnchecked());
    TEST_LESS_THAN(GetSubCategoryId().GetValueUnchecked());
    TEST_LESS_THAN(GetClass());

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool DisplayParams::IsEqualTo(DisplayParamsCR rhs, ComparePurpose purpose) const
    {
    if (&rhs == this)
        return true;

    TEST_EQUAL(GetType());
    TEST_EQUAL(IgnoresLighting());
    TEST_EQUAL(GetLineWidth());
    TEST_EQUAL(GetMaterial());
    TEST_EQUAL(GetLinePixels());
    TEST_EQUAL(GetFillFlags());
    TEST_EQUAL(HasRegionOutline());
    TEST_EQUAL(GetTextureMapping().GetTexture()); // ###TODO: Care about whether params match?

    if (ComparePurpose::Merge == purpose)
        {
        TEST_EQUAL(HasFillTransparency());
        TEST_EQUAL(HasLineTransparency());
        if (GetTextureMapping().IsValid())
            TEST_EQUAL(GetFillColor());     // Textures may use color so they can't be merged. (could test if texture actually uses color).

        return true;
        }

    TEST_EQUAL(GetFillColor());
    TEST_EQUAL(GetLineColor());
    TEST_EQUAL(GetCategoryId().GetValueUnchecked());
    TEST_EQUAL(GetSubCategoryId().GetValueUnchecked());
    TEST_EQUAL(GetClass());

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayParamsCR DisplayParamsCache::Get(DisplayParamsR toFind)
    {
    BeAssert(0 == toFind.GetRefCount()); // allocated on stack...
    toFind.AddRef();
    DisplayParamsCPtr pToFind(&toFind);
    auto iter = m_set.find(pToFind);
    if (m_set.end() == iter)
        {
        iter = m_set.insert(toFind.Clone()).first;
#if defined(DEBUG_DISPLAY_PARAMS_CACHE)
        printf("\nLooking for: %s\nCreated: %s\n", toFind.ToDebugString().c_str(), (*iter)->ToDebugString().c_str());
#endif
        }

    return **iter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DisplayParams::ToDebugString() const
    {
    Utf8CP types[3] = { "Mesh", "Linear", "Text" };
    Utf8PrintfString str("%s line:%x fill:%x width:%u pix:%u fillflags:%u class:%u, %s%s",
        types[static_cast<uint8_t>(m_type)],
        m_lineColor.GetValue(),
        m_fillColor.GetValue(),
        m_width,
        static_cast<uint32_t>(m_linePixels),
        static_cast<uint32_t>(m_fillFlags),
        static_cast<uint32_t>(m_class),
        m_ignoreLighting ? "ignoreLights " : "",
        m_hasRegionOutline ? "outlined" : "");

    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d Mesh::GetPoint(uint32_t index) const
    {
    return Points().Unquantize(index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d Mesh::GetTriangleRange(TriangleCR triangle) const
    {
    return DRange3d::From(GetPoint(triangle[0]),
                          GetPoint(triangle[1]),
                          GetPoint(triangle[2]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DVec3d Mesh::GetTriangleNormal(TriangleCR triangle) const
    {
    return DVec3d::FromNormalizedCrossProductToPoints(GetPoint(triangle[0]),
                                                      GetPoint(triangle[1]),
                                                      GetPoint(triangle[2]));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool Mesh::HasNonPlanarNormals() const
    {
    if (m_normals.empty())
        return false;

    OctEncodedNormal    normals[3];
    uint32_t const*     pIndex = m_triangles.Indices().data();
    uint32_t const*     pEnd = pIndex + m_triangles.Indices().size();

    for (pIndex = 0; pIndex < pEnd; pIndex += 3)
        {
        normals[0] = m_normals[pIndex[0]];
        normals[1] = m_normals[pIndex[1]];
        if (normals[0] != normals[1])
            return true;

        normals[2] = m_normals[pIndex[2]];
        if (normals[0] != normals[2] || normals[1] != normals[2])
            return true;
        }

    return false;
    }
     
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void Mesh::GetGraphics (bvector<Render::GraphicPtr>& graphics, Dgn::Render::SystemCR system, GetMeshGraphicsArgs& args, DgnDbR db) const
    {
    bool haveMesh = !Triangles().Empty();
    bool havePolyline = !haveMesh && !Polylines().empty();

    if (!haveMesh && !havePolyline)
        return;                                                                   

    Render::GraphicPtr thisGraphic;

    if (haveMesh)
        {
        if (args.m_meshArgs.Init(*this) &&
            (thisGraphic = system._CreateTriMesh(args.m_meshArgs, db)).IsValid())
            graphics.push_back (thisGraphic);

        MeshEdgesPtr    edges = GetEdges();

        if (args.m_visibleEdgesArgs.Init(*this) &&
            (thisGraphic = system._CreateVisibleEdges(args.m_visibleEdgesArgs, db)).IsValid())
            graphics.push_back(thisGraphic);

        if (args.m_invisibleEdgesArgs.Init(*this) &&
            (thisGraphic = system._CreateSilhouetteEdges(args.m_invisibleEdgesArgs, db)).IsValid())
            graphics.push_back(thisGraphic);

        if (args.m_polylineEdgesArgs.Init(*this) &&
            (thisGraphic = system._CreateIndexedPolylines(args.m_polylineEdgesArgs, db)).IsValid())
            graphics.push_back(thisGraphic);
        }
    else                           
        {
        if (args.m_polylineArgs.Init(*this) &&
            (thisGraphic = system._CreateIndexedPolylines(args.m_polylineArgs, db)).IsValid())
            graphics.push_back(thisGraphic);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T, typename U> static void insertVertexAttribute(bvector<uint16_t>& indices, T& table, U const& value, QPoint3dListCR vertices)
    {
    // Don't allocate the indices until we have non-uniform values
    if (table.empty())
        {
        table.GetIndex(value);
        BeAssert(table.IsUniform());
        BeAssert(0 == table.GetIndex(value));
        }
    else if (!table.IsUniform() || table.begin()->first != value)
        {
        if (indices.empty())
            {
            // back-fill uniform value for existing vertices...
            indices.resize(vertices.size() - 1);
            std::fill(indices.begin(), indices.end(), (uint16_t)0);
            }

        indices.push_back(table.GetIndex(value));
        BeAssert(!table.IsUniform());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t Mesh::AddVertex(QPoint3dCR vert, OctEncodedNormalCP normal, DPoint2dCP param, uint32_t fillColor, FeatureCR feature)
    {
    auto index = static_cast<uint32_t>(m_verts.size());

    m_verts.push_back(vert);
    m_features.Add(feature, m_verts.size());

    if (nullptr != normal)
        m_normals.push_back(*normal);
                                                                                                                 
    if (nullptr != param)
        m_uvParams.push_back(toFPoint2d(*param));

    insertVertexAttribute(m_colors, m_colorTable, fillColor, m_verts);
    return index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Mesh::Features::Add(FeatureCR feat, size_t numVerts)
    {
    if (nullptr == m_table)
        return;

    // Avoid allocating + populating big buffer of feature IDs unless necessary...
    uint32_t index = m_table->GetIndex(feat);
    if (!m_initialized)
        {
        // First feature - uniform.
        m_uniform = index;
        m_initialized = true;
        }
    else if (!m_indices.empty())
        {
        // Already non-uniform
        m_indices.push_back(index);
        }
    else if (m_uniform != index)
        {
        // Second feature - back-fill uniform for existing verts
        m_indices.resize(numVerts - 1);
        std::fill(m_indices.begin(), m_indices.end(), m_uniform);
        m_indices.push_back(index);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void Mesh::Features::ToFeatureIndex(FeatureIndex& index) const
    {
    if (!m_initialized)
        {
        index.m_type = FeatureIndex::Type::Empty;
        }
    else if (m_indices.empty())
        {
        index.m_type = FeatureIndex::Type::Uniform;
        index.m_featureID = m_uniform;
        }
    else
        {
        index.m_type = FeatureIndex::Type::NonUniform;
        index.m_featureIDs = m_indices.data();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Mesh::Features::SetIndices(bvector<uint32_t>&& indices)
    {
    if (indices.empty())
        {
        BeAssert(false);
        m_initialized = false;
        }
    else if (1 == indices.size())
        {
        m_uniform = indices.front();
        }
    else
        {
        m_indices = std::move(indices);
        }
    m_initialized = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d Mesh::ComputeRange() const
    {
    DRange3d range = DRange3d::NullRange();
    auto const& points = Points();
    size_t nPoints = points.size();
    for (size_t i = 0; i < nPoints; i++)
        range.Extend(points.Unquantize(i));

    return range;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d Mesh::ComputeUVRange() const
    {
    DRange3d range = DRange3d::NullRange();
    for (auto const& fpoint : m_uvParams)
        range.Extend(DPoint3d::FromXYZ(fpoint.x, fpoint.y, 0.0));

    return range;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TriangleKey::TriangleKey(TriangleCR triangle)
    {
    // Could just use std::sort - but this should be faster?
    if (triangle[0] < triangle[1])
        {
        if (triangle[0] < triangle[2])
            {
            m_sortedIndices[0] = triangle[0];
            if (triangle[1] < triangle[2])
                {
                m_sortedIndices[1] = triangle[1];
                m_sortedIndices[2] = triangle[2];
                }
            else
                {
                m_sortedIndices[1] = triangle[2];
                m_sortedIndices[2] = triangle[1];
                }
            }
        else
            {
            m_sortedIndices[0] = triangle[2];
            m_sortedIndices[1] = triangle[0];
            m_sortedIndices[2] = triangle[1];
            }
        }
    else
        {
        if (triangle[1] < triangle[2])
            {
            m_sortedIndices[0] = triangle[1];
            if (triangle[0] < triangle[2])
                {
                m_sortedIndices[1] = triangle[0];
                m_sortedIndices[2] = triangle[2];
                }
            else
                {
                m_sortedIndices[1] = triangle[2];
                m_sortedIndices[2] = triangle[0];
                }
            }
        else
            {
            m_sortedIndices[0] = triangle[2];
            m_sortedIndices[1] = triangle[1];
            m_sortedIndices[2] = triangle[0];
            }
        }

    BeAssert (m_sortedIndices[0] < m_sortedIndices[1]);
    BeAssert (m_sortedIndices[1] < m_sortedIndices[2]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TriangleKey::operator<(TriangleKeyCR rhs) const
    {
    COMPARE_VALUES (m_sortedIndices[0], rhs.m_sortedIndices[0]);
    COMPARE_VALUES (m_sortedIndices[1], rhs.m_sortedIndices[1]);
    COMPARE_VALUES (m_sortedIndices[2], rhs.m_sortedIndices[2]);

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool VertexKey::operator<(VertexKeyCR rhs) const
    {
    // This primarily exists because VertexKey comparisons are quite slow in non-optimized builds and app teams complained...
    static_assert(1 == sizeof(DgnGeometryClass), "unexpected size");
    static_assert(std::is_standard_layout<VertexKey>::value, "not standard layout");
    static_assert(0x00 == offsetof(VertexKey, m_param), "unexpected offset");
    static_assert(0x10 == offsetof(VertexKey, m_normalAndPos), "unexpected offset");
    static_assert(0x18 == offsetof(VertexKey, m_elemId), "unexpected offset");
    static_assert(0x20 == offsetof(VertexKey, m_subcatId), "unexpected offset");
    static_assert(0x28 == offsetof(VertexKey, m_fillColor), "unexpected offset");
    static_assert(0x2C == offsetof(VertexKey, m_class), "unexpected offset");
    static_assert(0x2D == offsetof(VertexKey, m_normalValid), "unexpected offset");

    size_t offset = offsetof(VertexKey, m_normalAndPos) + m_normalValid ? 0 : sizeof(uint16_t);
    size_t size = offsetof(VertexKey, m_normalValid) - offset;
    auto cmp = memcmp(reinterpret_cast<uint8_t const*>(this)+offset, reinterpret_cast<uint8_t const*>(&rhs)+offset, size);
    if (0 != cmp)
        return cmp < 0;

    if (m_paramValid)
        {
        constexpr double s_paramTolerance  = .1;
        COMPARE_VALUES_TOLERANCE (m_param.x, rhs.m_param.x, s_paramTolerance);
        COMPARE_VALUES_TOLERANCE (m_param.y, rhs.m_param.y, s_paramTolerance);
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilder::AddTriangle(TriangleCR triangle)
    {
    if (triangle.IsDegenerate())
        return;

    TriangleKey key(triangle);

    if (m_triangleSet.insert(key).second)
        m_mesh->AddTriangle(triangle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/017
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilder::AddFromPolyfaceVisitor(PolyfaceVisitorR visitor, TextureMappingCR mappedTexture, DgnDbR dgnDb, FeatureCR feature, bool doVertexCluster, bool includeParams, uint32_t fillColor, bool requireNormals)
    {
    auto const&     points = visitor.Point();
    bool const*     visitorVisibility = visitor.GetVisibleCP();
    size_t          nTriangles = points.size() - 2;
    
    // The face represented by this visitor should be convex (we request that in facet options) - so we do a simple fan triangulation.
    for (size_t iTriangle =0; iTriangle < nTriangles; iTriangle++)
        {
        if (doVertexCluster)
            {
            DVec3d      cross;

            cross.CrossProductToPoints (points.at(0), points.at(iTriangle+1), points.at(iTriangle+2));
            if (cross.MagnitudeSquared() < m_areaTolerance)
                return;
            }

        Triangle            newTriangle(!visitor.GetTwoSided());
        bvector<DPoint2d>   params = visitor.Param();
        bool                visibility[3];

        visibility[0] = (0 == iTriangle) ? visitorVisibility[0] : false;
        visibility[1] = visitorVisibility[iTriangle+1];
        visibility[2] = (iTriangle == nTriangles-1) ? visitorVisibility[iTriangle+2] : false;

        bool haveParams = includeParams && !params.empty();
        newTriangle.SetEdgeFlags(visibility);
        if (haveParams && mappedTexture.IsValid())
            {
            auto const&         textureMapParams = mappedTexture.GetParams();
            bvector<DPoint2d>   computedParams;

            BeAssert (m_mesh->Verts().empty() || !m_mesh->Params().empty());
            if (SUCCESS == textureMapParams.ComputeUVParams (computedParams, visitor))
                params = computedParams;
            }

        if (requireNormals && visitor.Normal().size() < points.size())
            continue; // TFS#790263: Degenerate triangle - no normals.

        for (size_t i = 0; i < 3; i++)
            {
            size_t      index = (0 == i) ? 0 : iTriangle + i; 
            VertexKey   vertex(points[index], feature, fillColor, m_mesh->Verts().GetParams(), requireNormals ? &visitor.Normal()[index] : nullptr, haveParams ? &params[index] : nullptr);

            newTriangle[i] = doVertexCluster ? AddClusteredVertex(vertex) : AddVertex(vertex);
            if (m_currentPolyface.IsValid())
                m_currentPolyface->m_vertexIndexMap.Insert(newTriangle[i], visitor.ClientPointIndex()[index]);
            }

        AddTriangle(newTriangle);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilder::AddPolyline (bvector<DPoint3d>const& points, FeatureCR feature, bool doVertexCluster, uint32_t fillColor, double startDistance, DPoint3dCR  rangeCenter)
    {
    MeshPolyline    newPolyline(startDistance, rangeCenter);

    for (auto& point : points)
        {
        VertexKey vertex(point, feature, fillColor, m_mesh->Verts().GetParams());
        newPolyline.GetIndices().push_back (doVertexCluster ? AddClusteredVertex(vertex) : AddVertex(vertex));
        }

    m_mesh->AddPolyline (newPolyline);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilder::AddPolyline(bvector<QPoint3d> const& points, FeatureCR feature, uint32_t fillColor, double startDistance, DPoint3dCR rangeCenter)
    {
    MeshPolyline newPolyline(startDistance, rangeCenter);
    for (auto const& point : points)
        {
        VertexKey key(point, feature, fillColor, nullptr, nullptr);
        newPolyline.GetIndices().push_back(AddVertex(key));
        }

    m_mesh->AddPolyline(newPolyline);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilder::AddPointString(bvector<DPoint3d> const& points, FeatureCR feature, uint32_t fillColor, double startDistance, DPoint3dCR rangeCenter)
    {
    // Assume no duplicate points in point strings (or, too few to matter).
    // Why? Because drawGridDots() potentially sends us tens of thousands of points (up to 83000 on my large monitor in top view), and wants to do so every frame.
    // The resultant map lookups/inserts/rebalancing kill performance in non-optimized builds.
    // NB: rangeCenter and startDistance unused...
    MeshPolyline polyline(startDistance, rangeCenter);
    for (auto const& point : points)
        {
        QPoint3d qpoint(point, m_mesh->Verts().GetParams());
        auto index = static_cast<uint32_t>(m_mesh->Verts().size());
        m_mesh->AddVertex(qpoint, nullptr, nullptr, fillColor, feature);
        polyline.GetIndices().push_back(index);
        }

    m_mesh->AddPolyline(polyline);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Mesh::AddPolyline(MeshPolylineCR polyline) 
    {
    BeAssert(PrimitiveType::Polyline == GetType() || PrimitiveType::Point == GetType()); 
    
    if (PrimitiveType::Polyline == GetType() && polyline.GetIndices().size() < 2)
        {
        // BeAssert(false);
        return;
        }
        

    m_polylines.push_back(polyline);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t MeshBuilder::AddVertex(VertexMap& verts, VertexKey const& vertex)
    {
    // Avoid doing lookup twice - once to find existing, again to add if not present
    auto index = static_cast<uint32_t>(m_mesh->Verts().size());
    auto insertPair = verts.Insert(vertex, index);
    if (insertPair.second)
        m_mesh->AddVertex(vertex.GetPosition(), vertex.GetNormal(), vertex.GetParam(), vertex.GetFillColor(), vertex.GetFeature());

    return insertPair.first->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
GeomPart::GeomPart(DRange3dCR range, GeometryList const& geometries) : m_range (range), m_facetCount(0), m_geometries(geometries)
    { 
    }                

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeomPart::IsCurved() const
    {
    return m_geometries.ContainsCurves();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceList GeomPart::GetPolyfaces(IFacetOptionsR facetOptions, GeometryCR instance, ViewContextR context)
    {
    return GetPolyfaces(facetOptions, &instance, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceList GeomPart::GetPolyfaces(IFacetOptionsR facetOptions, GeometryCP instance, ViewContextR context)
    {
    PolyfaceList polyfaces;
    for (auto& geometry : m_geometries) 
        {
        BeAssert(geometry->GetTransform().IsIdentity());
        PolyfaceList thisPolyfaces = geometry->GetPolyfaces (facetOptions, context);

        for (auto const& thisPolyface : thisPolyfaces)
            {
            Polyface polyface(*thisPolyface.m_displayParams, *thisPolyface.m_polyface->Clone());
            if (nullptr != instance)
                polyface.Transform(instance->GetTransform());

            polyfaces.push_back(polyface);
            }
        }

    return polyfaces;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeomPart::SetInCache(bool inCache)
    {
    for (auto& geometry : m_geometries)
        geometry->SetInCache(inCache);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
StrokesList GeomPart::GetStrokes (IFacetOptionsR facetOptions, GeometryCR instance, ViewContextR context)
    {
    return GetStrokes(facetOptions, &instance, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
StrokesList GeomPart::GetStrokes(IFacetOptionsR facetOptions, GeometryCP instance, ViewContextR context)
    {
    StrokesList strokes;

    for (auto& geometry : m_geometries) 
        {
        StrokesList   thisStrokes = geometry->GetStrokes(facetOptions, context);

        if (!thisStrokes.empty())
            strokes.insert (strokes.end(), thisStrokes.begin(), thisStrokes.end());
        }

    if (nullptr != instance)
        {
        for (auto& stroke : strokes)
            stroke.Transform(instance->GetTransform());
        }

    return strokes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t GeomPart::GetFacetCount(FacetCounter& counter, GeometryCR instance) const
    {
    if (0 == m_facetCount)
        for (auto& geometry : m_geometries) 
            m_facetCount += geometry->GetFacetCount(counter);
            
    return m_facetCount;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
Geometry::Geometry(TransformCR tf, DRange3dCR range, DgnElementId entityId, DisplayParamsCR params, bool isCurved, DgnDbR db)
    : m_params(&params), m_transform(tf), m_tileRange(range), m_entityId(entityId), m_isCurved(isCurved), m_facetCount(0), m_hasTexture(params.IsTextured())
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t Geometry::GetFacetCount(IFacetOptionsR options) const
    {
    if (0 != m_facetCount)
        return m_facetCount;
    
    FacetCounter counter(options);
    return (m_facetCount = _GetFacetCount(counter));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IFacetOptionsPtr Geometry::CreateFacetOptions(double chordTolerance)
    {
    static double       s_defaultAngleTolerance = msGeomConst_piOver2;
    IFacetOptionsPtr    opts = IFacetOptions::Create();

    opts->SetChordTolerance(chordTolerance);
    opts->SetAngleTolerance(s_defaultAngleTolerance);
    opts->SetMaxPerFace(100);
    opts->SetConvexFacetsRequired(true);             // Defer triangulation of facets to simple fans.
    opts->SetCurvedSurfaceMaxPerFace(3);
    opts->SetParamsRequired(true);
    opts->SetNormalsRequired(true);
    opts->SetEdgeChainsRequired(true);

    return opts;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
IFacetOptionsPtr Geometry::CreateFacetOptions(double chordTolerance, NormalMode normalMode) const
    {
    auto facetOptions = CreateFacetOptions(chordTolerance / m_transform.ColumnXMagnitude());
    bool normalsRequired = false;

    switch (normalMode)
        {
        case NormalMode::Always:    
            normalsRequired = true; 
            break;
        case NormalMode::CurvedSurfacesOnly:    
            normalsRequired = m_isCurved; 
            break;
        }

    facetOptions->SetNormalsRequired(normalsRequired);
    facetOptions->SetParamsRequired(HasTexture());

    return facetOptions;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceList Geometry::GetPolyfaces(double chordTolerance, NormalMode normalMode, ViewContextR context)
    {
    auto polyfaces = _GetPolyfaces(*CreateFacetOptions(chordTolerance, normalMode), context);
    if (!m_clip.IsValid() || polyfaces.empty())
        return polyfaces;

    GeometryClipper geomClipper(m_clip.get());

    PolyfaceList clippedPolyfaces;
    for (auto& polyface : polyfaces)
        {
        geomClipper.DoClipPolyface(clippedPolyfaces, polyface);
        }

    return clippedPolyfaces;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
StrokesList Geometry::GetStrokes (IFacetOptionsR facetOptions, ViewContextR context)
    {
    auto strokes = _GetStrokes(facetOptions, context);
    if (!m_clip.IsValid() || strokes.empty())
        return strokes;

    GeometryClipper geomClipper(m_clip.get());

    StrokesList clippedStrokes;
    for (auto& stroke : strokes)
        {
        geomClipper.DoClipStrokes(clippedStrokes, stroke);
        }

    return clippedStrokes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void Strokes::Transform(TransformCR transform)
    {
    for (auto& stroke : m_strokes)
        transform.Multiply (stroke.m_points, stroke.m_points);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr PrimitiveGeometry::FixPolyface(PolyfaceHeaderR geom, IFacetOptionsR facetOptions)
    {
    // Avoid IPolyfaceConstruction if possible...AddPolyface_matched() does a ton of expensive remapping which is unnecessary for our use case.
    // (Plus we can avoid cloning the input if caller owns it)
    PolyfaceHeaderPtr polyface(&geom);
    size_t maxPerFace;
    if (geom.GetNumFacet(maxPerFace) > 0 && (int)maxPerFace > facetOptions.GetMaxPerFace())
        {
        IPolyfaceConstructionPtr builder = PolyfaceConstruction::New(facetOptions);
        builder->AddPolyface(geom);
        polyface = &builder->GetClientMeshR();
        }
    else
        {
        bool addNormals = facetOptions.GetNormalsRequired() && 0 == geom.GetNormalCount(),
             addParams = facetOptions.GetParamsRequired() && 0 == geom.GetParamCount(),
             addFaceData = addParams && 0 == geom.GetFaceCount();

        if (addNormals)
            AddNormals(*polyface, facetOptions);

        if (addParams)
            AddParams(*polyface, facetOptions);

        if (addFaceData)
            polyface->BuildPerFaceFaceData();

        if (!geom.HasConvexFacets() && facetOptions.GetConvexFacetsRequired())
            polyface->Triangulate(3);

        // Not necessary to add edges chains -- edges will be generated from visibility flags later
        // and decimation will not handle edge chains correctly.
        }

    return polyface;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveGeometry::AddNormals(PolyfaceHeaderR polyface, IFacetOptionsR facetOptions)
    {
    static double s_defaultCreaseRadians = Angle::DegreesToRadians(45.0);
    static double s_defaultConeRadians = Angle::DegreesToRadians(90.0);
    polyface.BuildApproximateNormals(s_defaultCreaseRadians, s_defaultConeRadians, facetOptions.GetHideSmoothEdgesWhenGeneratingNormals());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveGeometry::AddParams(PolyfaceHeaderR polyface, IFacetOptionsR facetOptions)
    {
    LocalCoordinateSelect selector;
    switch (facetOptions.GetParamMode())
        {
        case FACET_PARAM_01BothAxes:
            selector = LOCAL_COORDINATE_SCALE_01RangeBothAxes;
            break;
        case FACET_PARAM_01LargerAxis:
            selector = LOCAL_COORDINATE_SCALE_01RangeLargerAxis;
            break;
        default:
            selector = LOCAL_COORDINATE_SCALE_UnitAxesAtLowerLeft;
            break;
        }

    polyface.BuildPerFaceParameters(selector);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceList PrimitiveGeometry::_GetPolyfaces(IFacetOptionsR facetOptions, ViewContextR context)
    {
    PolyfaceHeaderPtr polyface = m_geometry->GetAsPolyfaceHeader();
    
    if (polyface.IsValid())
        {
        if (m_inCache)
            polyface = polyface->Clone();

        if (!HasTexture())
            polyface->ClearParameters(false);

        // Make sure params, normals, etc present if needed. Note we wait until now so that the tolerance can be computed...
        polyface = FixPolyface(*polyface, facetOptions);

        BeAssertOnce(GetTransform().IsIdentity()); // Polyfaces are transformed during collection.
        return PolyfaceList (1, Polyface(GetDisplayParams(), *polyface));
        }

    CurveVectorPtr      curveVector = m_geometry->GetAsCurveVector();

    if (curveVector.IsValid() && !curveVector->IsAnyRegionType()) // Non region or unfilled planar regions....)
        return PolyfaceList();

    IPolyfaceConstructionPtr polyfaceBuilder = IPolyfaceConstruction::Create(facetOptions);

    ISolidPrimitivePtr  solidPrimitive = curveVector.IsNull() ? m_geometry->GetAsISolidPrimitive() : nullptr;
    MSBsplineSurfacePtr bsplineSurface = solidPrimitive.IsNull() && curveVector.IsNull() ? m_geometry->GetAsMSBsplineSurface() : nullptr;

    // According to Brien we should not even bother checking planar.
    // The planar flag exists to allow us to address z-fighting when shapes are sketched onto surfaces (e.g. for push-pull modeling)
    bool isPlanar = curveVector.IsValid();
    if (curveVector.IsValid())
        polyfaceBuilder->AddRegion(*curveVector);
    else if (solidPrimitive.IsValid())
        polyfaceBuilder->AddSolidPrimitive(*solidPrimitive);
    else if (bsplineSurface.IsValid())
        polyfaceBuilder->Add(*bsplineSurface);

    PolyfaceList    polyfaces;

    polyface = polyfaceBuilder->GetClientMeshPtr();
    if (polyface.IsValid())
        {
        if (!GetTransform().IsIdentity())
            polyface->Transform(GetTransform());

        // If there is a region outline, we will generate it separately as a polyline in _GetStrokes().
        // If there is no region outline, and never should be one, don't generate edges
        // See: text background (or anything else with blanking fill)
        DisplayParamsCR params = GetDisplayParams();
        bool wantEdges = curveVector.IsNull() || (!params.HasBlankingFill() && !params.HasRegionOutline());
        polyfaces.push_back(Polyface(GetDisplayParams(), *polyface, wantEdges, isPlanar));
        }

    return polyfaces;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
StrokesList PrimitiveGeometry::_GetStrokes (IFacetOptionsR facetOptions, ViewContextR context)
    {
    StrokesList             tileStrokes;
    CurveVectorPtr          curveVector = m_geometry->GetAsCurveVector();
    
    if (!curveVector.IsValid())
        return tileStrokes;

    Strokes::PointLists     strokePoints;

    if (! curveVector->IsAnyRegionType() || GetDisplayParams().HasRegionOutline())
        {
        strokePoints.clear();
        collectCurveStrokes(strokePoints, *curveVector, facetOptions, GetTransform());

        if (!strokePoints.empty())
            {
            bool isPlanar = curveVector->IsAnyRegionType();
            BeAssert(isPlanar == GetDisplayParams().HasRegionOutline());
            tileStrokes.push_back(Strokes(GetDisplayParams(), std::move(strokePoints), m_disjoint, isPlanar));
            }
        }

    return tileStrokes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SolidKernelGeometry::SolidKernelGeometry(IBRepEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsCR params, DgnDbR db)
        : Geometry(tf, range, elemId, params, BRepUtil::HasCurvedFaceOrEdge(solid), db), m_entity(&solid)
    {
#if defined (BENTLEYCONFIG_PARASOLID)    
    if (DgnDb::ThreadId::Client != DgnDb::GetThreadId())
        PK_BODY_change_partition(PSolidUtil::GetEntityTag (*m_entity), PSolidThreadUtil::GetThreadPartition());
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceList SolidKernelGeometry::_GetPolyfaces(IFacetOptionsR facetOptions, ViewContextR context)
    {
    PolyfaceList tilePolyfaces;

#if defined (BENTLEYCONFIG_PARASOLID)
    DRange3d entityRange = m_entity->GetEntityRange();
    if (entityRange.IsNull())
        return tilePolyfaces;                                                                                                                                                                                                 

    double              rangeDiagonal = entityRange.DiagonalDistance();
    static double       s_minRangeRelTol = 1.0e-4;
    double              minChordTolerance = rangeDiagonal * s_minRangeRelTol;
    IFacetOptionsPtr    pFacetOptions = facetOptions.Clone();
    
    if (facetOptions.GetChordTolerance() < minChordTolerance)
        pFacetOptions->SetChordTolerance (minChordTolerance);

    pFacetOptions->SetParamsRequired (true); // Can't rely on HasTexture due to face attached material that may have texture.

    if (nullptr != m_entity->GetFaceMaterialAttachments())
        {
        bvector<PolyfaceHeaderPtr>  polyfaces;
        bvector<FaceAttachment>     params;

        if (!BRepUtil::FacetEntity(*m_entity, polyfaces, params, *pFacetOptions))
            return tilePolyfaces;

        GeometryParams baseParams;

        // Require valid category/subcategory for sub-category appearance color/material...
        baseParams.SetCategoryId(GetDisplayParams().GetCategoryId());
        baseParams.SetSubCategoryId(GetDisplayParams().GetSubCategoryId());
        baseParams.SetGeometryClass(GetDisplayParams().GetClass());

        BeAssert(nullptr != context.GetRenderSystem());
        for (size_t i=0; i<polyfaces.size(); i++)
            {
            auto&   polyface = polyfaces[i];

            if (polyface->HasFacets())
                {
                GeometryParams faceParams;
                params[i].ToGeometryParams(faceParams, baseParams);
                
                GraphicParams gfParams;
                context.CookGeometryParams(faceParams, gfParams);

                auto displayParams = DisplayParams::CreateForMesh(gfParams, &faceParams, false, context.GetDgnDb(), *context.GetRenderSystem());
                tilePolyfaces.push_back (Polyface(*displayParams, *polyface));
                }
            }
        }
    else
        {
        auto polyface = BRepUtil::FacetEntity(*m_entity, *pFacetOptions);
    
        if (polyface.IsValid() && polyface->HasFacets())
            tilePolyfaces.push_back (Polyface(GetDisplayParams(), *polyface));
        }

    if (!GetTransform().IsIdentity())
        for (auto& tilePolyface : tilePolyfaces)
            tilePolyface.m_polyface->Transform (GetTransform());

    return tilePolyfaces;

#else
    return tilePolyfaces;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryPtr Geometry::Create(TextStringR textString, TransformCR transform, DRange3dCR range, DgnElementId entityId, DisplayParamsCR params, DgnDbR db, bool checkGlyphBoxes)
    {
    return TextStringGeometry::Create(textString, transform, range, entityId, params, db, checkGlyphBoxes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryPtr Geometry::Create(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId entityId, DisplayParamsCR params, bool isCurved, DgnDbR db, bool disjoint)
    {
    return PrimitiveGeometry::Create(geometry, tf, range, entityId, params, isCurved, db, disjoint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryPtr Geometry::Create(IBRepEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId entityId, DisplayParamsCR params, DgnDbR db)
    {
    return SolidKernelGeometry::Create(solid, tf, range, entityId, params, db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryPtr Geometry::Create(GeomPartR part, TransformCR transform, DRange3dCR range, DgnElementId entityId, DisplayParamsCR params, DgnDbR db)
    {
    return GeomPartInstanceGeometry::Create(part, transform, range, entityId, params, db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryAccumulator::AddGeometry(IGeometryR geom, bool isCurved, DisplayParamsCR displayParams, TransformCR transform, ClipVectorCP clip, bool disjoint)
    {
    DRange3d range;
    if (!geom.TryGetRange(range))
        return false;

    auto tf = m_haveTransform ? Transform::FromProduct(m_transform, transform) : transform;
    tf.Multiply(range, range);

    return AddGeometry(geom, isCurved, displayParams, tf, clip, range, disjoint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryAccumulator::AddGeometry(IGeometryR geom, bool isCurved, DisplayParamsCR displayParams, TransformCR transform, ClipVectorCP clip, DRange3dCR range, bool disjoint)
    {
    GeometryPtr geometry = Geometry::Create(geom, transform, range, GetElementId(), displayParams, isCurved, GetDgnDb(), disjoint);
    if (geometry.IsNull())
        return false;

    geometry->SetClipVector(clip);

    m_geometries.push_back(*geometry);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* NB: This is just for testing the performance impact of cloning geometry (which is
* unnecessary in most cases).
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> RefCountedPtr<T> cloneGeometry(T const& geom)
    {
#if defined(ETT_PROFILE_CLONE)
    return const_cast<T*>(&geom);
#else
    return geom.Clone();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/17
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr cloneGeometry(PolyfaceQueryCR query)
    {
#if defined(ETT_PROFILE_CLONE)
    auto header = dynamic_cast<PolyfaceHeaderCP>(&query);
    if (nullptr != header)
        return const_cast<PolyfaceHeaderP>(header);
    else
#endif
        return query.Clone();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryAccumulator::Add(CurveVectorR curves, bool filled, DisplayParamsCR displayParams, TransformCR transform, ClipVectorCP clip, bool disjoint)
    {
    if (m_surfacesOnly && !curves.IsAnyRegionType())
        return true;    // ignore...

    bool isCurved = curves.ContainsNonLinearPrimitive();
    IGeometryPtr geom = IGeometry::Create(CurveVectorPtr(&curves));
    return AddGeometry(*geom, isCurved, displayParams, transform, clip, disjoint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryAccumulator::Add(ISolidPrimitiveR primitive, DisplayParamsCR displayParams, TransformCR transform)
    {
    bool isCurved = primitive.HasCurvedFaceOrEdge();
    IGeometryPtr geom = IGeometry::Create(ISolidPrimitivePtr(&primitive));
    return AddGeometry(*geom, isCurved, displayParams, transform, nullptr, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryAccumulator::Add(RefCountedMSBsplineSurface& surface, DisplayParamsCR displayParams, TransformCR transform)
    {
    bool isCurved = (surface.GetUOrder() > 2 || surface.GetVOrder() > 2);
    IGeometryPtr geom = IGeometry::Create(MSBsplineSurfacePtr(&surface));
    return AddGeometry(*geom, isCurved, displayParams, transform, nullptr, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryAccumulator::Add(PolyfaceHeaderR polyface, bool filled, DisplayParamsCR displayParams, TransformCR transform)
    {
    if (m_haveTransform)
        polyface.Transform(Transform::FromProduct(m_transform, transform));
    else if (!transform.IsIdentity())
        polyface.Transform(transform);

    DRange3d range = polyface.PointRange();
    IGeometryPtr geom = IGeometry::Create(PolyfaceHeaderPtr(&polyface));
    AddGeometry(*geom, false, displayParams, Transform::FromIdentity(), nullptr, range, false);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryAccumulator::Add(IBRepEntityR body, DisplayParamsCR displayParams, TransformCR transform)
    {
    DRange3d range = body.GetEntityRange();
    Transform tf = m_haveTransform ? Transform::FromProduct(m_transform, transform) : transform;
    tf.Multiply(range, range);

    m_geometries.push_back(*Geometry::Create(body, tf, range, GetElementId(), displayParams, GetDgnDb()));
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryAccumulator::Add(TextStringR textString, DisplayParamsCR displayParams, TransformCR transform)
    {
    if (m_surfacesOnly)
        return true;

    Transform tf = m_haveTransform ? Transform::FromProduct(m_transform, transform) : transform;

    DRange2d range2d;
        {
        // ###TODO: Fonts are a freaking mess.
        BeMutexHolder lock(DgnFonts::GetMutex());
        range2d = textString.GetRange();
        }

    DRange3d range = DRange3d::From(range2d.low.x, range2d.low.y, 0.0, range2d.high.x, range2d.high.y, 0.0);
    Transform::FromProduct(tf, textString.ComputeTransform()).Multiply(range, range);

    m_geometries.push_back(*Geometry::Create(textString, tf, range, GetElementId(), displayParams, GetDgnDb(), m_checkGlyphBoxes));
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
MeshBuilderMap GeometryAccumulator::ToMeshBuilders(GeometryOptionsCR options, double tolerance, FeatureTableP featureTable, ViewContextR context) const
    {
    return ToMeshBuilderMap(options, tolerance, featureTable, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
MeshBuilderMap GeometryAccumulator::ToMeshBuilderMap(GeometryOptionsCR options, double tolerance, FeatureTableP featureTable, ViewContextR context) const
    {
    DRange3d range = m_geometries.ComputeRange();
    bool is2d = !range.IsNull() && range.IsAlmostZeroZ();

    MeshBuilderMap builderMap(tolerance, featureTable, range, is2d);
    if (m_geometries.empty())
        return builderMap;

    // This ensures the builder map is organized in the same order as the geometry list, and no meshes are merged.
    // This is required to make overlay decorations render correctly.
    uint16_t order = 0;
    for (auto const& geom : m_geometries)
        {
        auto polyfaces = geom->GetPolyfaces(tolerance, options.m_normalMode, context);
        for (auto const& tilePolyface : polyfaces)
            {
            PolyfaceHeaderPtr polyface = tilePolyface.m_polyface;
            if (polyface.IsNull() || 0 == polyface->GetPointCount())
                continue;

            DisplayParamsCPtr displayParams = tilePolyface.m_displayParams;
            bool hasTexture = displayParams.IsValid() && displayParams->IsTextured();

            MeshBuilderMap::Key key(*displayParams, nullptr != polyface->GetNormalIndexCP(), Mesh::PrimitiveType::Mesh, tilePolyface.m_isPlanar);
            if (options.WantPreserveOrder())
                key.SetOrder(order++);

            MeshBuilderR meshBuilder = builderMap[key];

            auto edgeOptions = (options.WantEdges() && tilePolyface.m_displayEdges) ? MeshEdgeCreationOptions::DefaultEdges : MeshEdgeCreationOptions::NoEdges;
            meshBuilder.BeginPolyface(*polyface, edgeOptions);

            uint32_t fillColor = displayParams->GetFillColor();
            for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*polyface); visitor->AdvanceToNextFace(); /**/)
                meshBuilder.AddFromPolyfaceVisitor(*visitor, displayParams->GetTextureMapping(), GetDgnDb(), geom->GetFeature(), false, hasTexture, fillColor, nullptr != polyface->GetNormalCP());

            meshBuilder.EndPolyface();
            }

        if (!options.WantSurfacesOnly())
            {
            auto tileStrokesArray = geom->GetStrokes(*geom->CreateFacetOptions(tolerance, NormalMode::Never), context);
            for (auto& tileStrokes : tileStrokesArray)
                {
                DisplayParamsCPtr displayParams = tileStrokes.m_displayParams;
                MeshBuilderMap::Key key(*displayParams, false, tileStrokes.m_disjoint ? Mesh::PrimitiveType::Point : Mesh::PrimitiveType::Polyline, tileStrokes.m_isPlanar);
                if (options.WantPreserveOrder())
                    key.SetOrder(order++);

                MeshBuilderR builder = builderMap[key];
                uint32_t fillColor = displayParams->GetLineColor();
                for (auto& strokePoints : tileStrokes.m_strokes)
                    {
                    if (tileStrokes.m_disjoint)
                        builder.AddPointString(strokePoints.m_points, geom->GetFeature(), fillColor, strokePoints.m_startDistance, strokePoints.m_rangeCenter);
                    else
                        builder.AddPolyline(strokePoints.m_points, geom->GetFeature(), false, fillColor, strokePoints.m_startDistance, strokePoints.m_rangeCenter);
                    }
                }
            }
        }

    return builderMap;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
MeshList GeometryAccumulator::ToMeshes(GeometryOptionsCR options, double tolerance, ViewContextR context, FeatureTableP featureTable) const
    {
    MeshList meshes;
    if (m_geometries.empty())
        return meshes;

    MeshBuilderMap builderMap = ToMeshBuilderMap(options, tolerance, featureTable, context);

    for (auto& builder : builderMap)
        {
        MeshP mesh = builder.second->GetMesh();
        if (!mesh->IsEmpty())
            meshes.push_back(mesh);
        }

    return meshes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryAccumulator::SaveToGraphicList(bvector<GraphicPtr>& graphics, GeometryOptionsCR options, double tolerance, ViewContextR context) const
    {
    MeshList                meshes = ToMeshes(options, tolerance, context);
    GetMeshGraphicsArgs     args;

    for (auto const& mesh : meshes)
        mesh->GetGraphics (graphics, GetSystem(), args, GetDgnDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
uint16_t ColorTable::GetIndex(uint32_t color)
    {
    BeAssert(!IsFull());
    BeAssert(empty() || (0 != ColorDef(color).GetAlpha()) == m_hasAlpha);

    auto iter = m_map.find(color);
    if (m_map.end() != iter)
        return iter->second;
    else if (IsFull())
        {
        BeAssert(false);
        return 0;
        }

    // The table should never contain a mix of opaque and translucent colors
    if (empty())
        m_hasAlpha = (0 != ColorDef(color).GetAlpha());

    uint16_t index = GetNumIndices();
    m_map[color] = index;
    return index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void ColorTable::ToColorIndex(ColorIndex& index, bvector<uint32_t>& colors, bvector<uint16_t> const& indices) const
    {
    index.Reset();
    if (empty())
        {
        BeAssert(false && "empty color table");
        }
    else if (IsUniform())
        {
        index.SetUniform(m_map.begin()->first);
        }
    else
        {
        BeAssert(!indices.empty());

        colors.resize(size());
        for (auto const& kvp : *this)
            colors[kvp.second] = kvp.first;

        index.SetNonUniform(GetNumIndices(), colors.data(), indices.data(), m_hasAlpha);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool TextStringGeometry::DoGlyphBoxes (IFacetOptionsR facetOptions)
    {
    if (!m_checkGlyphBoxes)
        return false;

    DRange2d            textRange = m_text->GetRange();
    double              minDimension = std::min (textRange.high.x - textRange.low.x, textRange.high.y - textRange.low.y) * GetTransform().ColumnXMagnitude();
    static const double s_minGlyphRatio = 1.0; 
    
    return minDimension < s_minGlyphRatio * facetOptions.GetChordTolerance();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceList TextStringGeometry::_GetPolyfaces(IFacetOptionsR facetOptionsIn, ViewContextR context)
    {
    return GlyphCache::GetPolyfaces(*this, facetOptionsIn.GetChordTolerance(), context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
StrokesList TextStringGeometry::_GetStrokes (IFacetOptionsR facetOptions, ViewContextR context)
    {
    return GlyphCache::GetStrokes(*this, facetOptions.GetChordTolerance(), context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t TextStringGeometry::_GetFacetCount(FacetCounter& counter) const
    {
#if defined(IF_COUNTING_FACETS)
    InitGlyphCurves();
    size_t              count = 0;

    for (auto& glyphCurve : m_glyphCurves)
        count += counter.GetFacetCount(*glyphCurve);

    return count;
#else
    return 0;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
IFacetOptionsPtr GlyphCache::CreateFacetOptions(double chordTolerance)
    {
    auto opts = Geometry::CreateFacetOptions(chordTolerance);
    opts->SetParamsRequired(false);
    opts->SetNormalsRequired(false);
    opts->SetEdgeChainsRequired(false);
    return opts;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  01/18
+---------------+---------------+---------------+---------------+---------------+------*/
void GlyphCache::Geom::InitRasterPolyface(CurveVectorCR curves, IFacetOptionsR facetOptionsIn)
    {
    DRange3d range;
    if (!curves.GetRange(range))
        {
        BeAssert(false);
        return;
        }

    DPoint3d ll = range.low;
    DPoint3d ul = DPoint3d::From(range.low.x, range.high.y);
    DPoint3d ur = range.high;
    DPoint3d lr = DPoint3d::From(range.high.x, range.low.y);

    bvector<DPoint3d> quadPts;
    quadPts.push_back(ll);
    quadPts.push_back(ul);
    quadPts.push_back(ur);
    quadPts.push_back(lr);

    IFacetOptionsPtr facetOptions = facetOptionsIn.Clone();
    facetOptions->SetParamsRequired(true);

    IPolyfaceConstructionPtr builder = IPolyfaceConstruction::Create(*facetOptions);
    //builder->AddPolygon(quadPts); // linker error???  unimplemented API!
    builder->AddTriangulation(quadPts); // use instead...
    m_rasterPolyface = builder->GetClientMeshPtr();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GlyphCache::GetGeometry(StrokesList* strokes, PolyfaceList* polyfaces, TextStringGeometry const& geom, double chordTolerance, ViewContextR context)
    {
    TextStringCR textString = geom.GetText();
    DgnGlyphCP const* glyphs = textString.GetGlyphs();
    if (nullptr == glyphs)
        return;

    DPoint3dCP glyphOrigins = textString.GetGlyphOrigins();
    BeAssert(nullptr != glyphOrigins);

    DVec3d xAxis, yAxis;
    textString.ComputeGlyphAxes(xAxis, yAxis);
    Transform rot = Transform::From(RotMatrix::From2Vectors(xAxis, yAxis));
    Transform textTransform = Transform::FromProduct(geom.GetTransform(), textString.ComputeTransform());

    auto facetOptions = CreateFacetOptions(chordTolerance);

    for (size_t i = 0; i < textString.GetNumGlyphs(); i++)
        {
        auto textGlyph = glyphs[i];
        Glyph* glyph = nullptr != textGlyph ? FindOrInsert(*textGlyph, textString.GetStyle().GetFont()) : nullptr;
        if (nullptr == glyph || !glyph->IsValid())
            continue;
        else if ((glyph->IsStrokes() && nullptr == strokes) || (!glyph->IsStrokes() && nullptr == polyfaces))
            continue;

        Transform glyphTransform = Transform::FromProduct(Transform::From(glyphOrigins[i]), rot);
        glyphTransform = Transform::FromProduct(textTransform, glyphTransform);

        if (glyph->IsStrokes())
            {
            Strokes::PointLists points = glyph->GetStrokes(*facetOptions);
            if (!points.empty())
                {
                for (auto& loop : points)
                    {
                    glyphTransform.Multiply(loop.m_points, loop.m_points);
                    glyphTransform.Multiply(loop.m_rangeCenter);
                    }

                strokes->push_back(Strokes(geom.GetDisplayParams(), std::move(points), false, true));
                }
            }
        else
            {
#define TEST_RASTER_GLYPHS
            DisplayParamsCPtr displayParams(&geom.GetDisplayParams());
#if !defined(TEST_RASTER_GLYPHS)
            PolyfaceHeaderPtr polyface = glyph->GetPolyface(*facetOptions);
#else
            Glyph::RasterPolyface raster = glyph->GetRasterPolyface(*facetOptions, *context.GetRenderSystem(), context.GetDgnDb());
            PolyfaceHeaderPtr polyface = raster.m_polyface;
            if (raster.IsValid())
                displayParams = displayParams->CloneForRasterText(*raster.m_texture);
#endif
            if (polyface.IsValid() && polyface->HasFacets())
                {
                polyface->Transform(glyphTransform);
                polyfaces->push_back(Polyface(*displayParams, *polyface, false, true));
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
GlyphCache::Glyph* GlyphCache::FindOrInsert(DgnGlyphCR glyph, DgnFontCR font)
    {
    auto iter = m_map.find(&glyph);
    if (m_map.end() == iter)
        iter = m_map.Insert(&glyph, Glyph(glyph, font)).first;

    return &iter->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T, typename U> void GlyphCache::Glyph::GetGeometry(IFacetOptionsR facetOptions, bool requestRaster, T createGeom, U acceptGeom)
    {
    BeAssert(IsValid());
    Tolerance tol = GetTolerance(facetOptions.GetChordTolerance());
    auto& geom = m_geom[tol];
    double chordTolerance = GetChordTolerance(tol, facetOptions.GetChordTolerance());
    if (kTolerance_Smallest == tol && chordTolerance < m_smallestTolerance)
        {
        if (requestRaster)
            geom.InvalidateRaster(); // this should never be necessary...tolerance does not affect textured quad.
        else
            geom.Invalidate();

        m_smallestTolerance = chordTolerance;
        }

    if ((!geom.IsValid() && !requestRaster) || (!geom.IsValidRaster() && requestRaster))
        createGeom(geom);

    if ((geom.IsValid() && !requestRaster) || (geom.IsValidRaster() && requestRaster))
        acceptGeom(geom);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool MeshArgs::Init(MeshCR mesh)
    {
    Clear();
    if (mesh.Triangles().Empty())
        return false;

    m_pointParams = mesh.Points().GetParams();

    Set(m_numIndices, m_vertIndex, mesh.Triangles().Indices());
    Set(m_numPoints, m_points, mesh.Points());
    Set(m_textureUV, mesh.Params());
    if (!mesh.GetDisplayParams().IgnoresLighting())    // ###TODO: Avoid generating normals in the first place if no lighting...
        Set(m_normals, mesh.Normals());

    m_texture = const_cast<TextureP>(mesh.GetDisplayParams().GetTextureMapping().GetTexture()); // ###TODO: constness...
    m_material = mesh.GetDisplayParams().GetMaterial();
    m_fillFlags = mesh.GetDisplayParams().GetFillFlags();
    m_isPlanar = mesh.IsPlanar();
    m_edgeWidth = mesh.GetDisplayParams().GetLineWidth();
    mesh.GetColorTable().ToColorIndex(m_colors, m_colorTable, mesh.Colors());

    mesh.ToFeatureIndex(m_features);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshArgs::Clear()
    {
    m_numIndices = 0;
    m_vertIndex = nullptr;
    m_numPoints = 0;
    m_points = nullptr;
    m_normals = nullptr;
    m_textureUV = nullptr;
    m_texture = nullptr;
    m_isPlanar = false;

    m_colors.Reset();
    m_colorTable.clear();
    m_features.Reset();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static void initLinearGraphicParams(T& args, MeshCR mesh)
    {
    args.m_linePixels = mesh.GetDisplayParams().GetLinePixels();
    args.m_width = mesh.GetDisplayParams().GetLineWidth();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool  ElementMeshEdgeArgs::Init(MeshCR mesh)
    {
    MeshEdgesPtr    meshEdges = mesh.GetEdges();
    m_pointParams = mesh.Points().GetParams();

    initLinearGraphicParams(*this, mesh);

    if (!meshEdges.IsValid() || meshEdges->m_visible.empty())
        return false;

    m_points    = mesh.Points().data();
    m_edges     = meshEdges->m_visible.data();
    m_numEdges  = meshEdges->m_visible.size();
    m_isPlanar  = mesh.IsPlanar();
    
    mesh.GetColorTable().ToColorIndex(m_colors, m_colorTable, mesh.Colors());
    mesh.ToFeatureIndex(m_features);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool  ElementSilhouetteEdgeArgs::Init(MeshCR mesh)
    {
    MeshEdgesPtr    meshEdges = mesh.GetEdges();
    m_pointParams = mesh.Points().GetParams();

    initLinearGraphicParams(*this, mesh);

    if (!meshEdges.IsValid() || meshEdges->m_silhouette.empty())
        return false;

    m_points    = mesh.Points().data();
    m_normals   = meshEdges->m_silhouetteNormals.data();
    m_edges     = meshEdges->m_silhouette.data();
    m_numEdges  = meshEdges->m_silhouette.size();

    mesh.GetColorTable().ToColorIndex(m_colors, m_colorTable, mesh.Colors());
    mesh.ToFeatureIndex(m_features);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool  ElementPolylineEdgeArgs::Init(MeshCR mesh)
    {
    Reset();
    MeshEdgesPtr    meshEdges = mesh.GetEdges();
    m_pointParams = mesh.Points().GetParams();

    initLinearGraphicParams(*this, mesh);

    if (!meshEdges.IsValid() || meshEdges->m_polylines.empty())
        return false;

    m_disjoint = false;
    m_isEdge = true;
    m_is2d = mesh.Is2d();
    m_isPlanar  = mesh.IsPlanar();

    m_polylines.reserve(meshEdges->m_polylines.size());

    for (auto& polyline : meshEdges->m_polylines)
        {
        IndexedPolyline indexedPolyline;

        if (indexedPolyline.Init(polyline.GetIndices(), polyline.GetStartDistance(), polyline.GetRangeCenter()))
            m_polylines.push_back(indexedPolyline);
        }
                                                
    FinishInit(mesh);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PolylineArgs::Reset()
    {
    m_disjoint = m_is2d = m_isPlanar = false;
    m_numPoints = m_numLines = 0;
    m_points = nullptr;
    m_lines = nullptr;
    m_polylines.clear();

    m_colors.Reset();
    m_colorTable.clear();
    m_features.Reset();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool PolylineArgs::Init(MeshCR mesh)
    {
    Reset();

    initLinearGraphicParams(*this, mesh);

    m_is2d = mesh.Is2d();
    m_isPlanar = mesh.IsPlanar();
    m_disjoint = Mesh::PrimitiveType::Point == mesh.GetType();
    m_isEdge = mesh.GetDisplayParams().HasRegionOutline();

    m_polylines.reserve(mesh.Polylines().size());

    for (auto const& polyline : mesh.Polylines())
        {
        IndexedPolyline indexedPolyline;
        if (indexedPolyline.Init(polyline))
            m_polylines.push_back(indexedPolyline);
        }
    if (!IsValid())
        return false;

    FinishInit(mesh);
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void PolylineArgs::FinishInit(MeshCR mesh)
    {
    m_pointParams = mesh.Points().GetParams();
    m_numPoints = static_cast<uint32_t>(mesh.Points().size());
    m_points = &mesh.Points()[0];

    mesh.GetColorTable().ToColorIndex(m_colors, m_colorTable, mesh.Colors());
    mesh.ToFeatureIndex(m_features);

    m_numLines = static_cast<uint32_t>(m_polylines.size());
    m_lines = &m_polylines[0];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_ActivateGraphicParams(GraphicParamsCR gfParams, GeometryParamsCP geomParams)
    {
    m_graphicParams = gfParams;
    m_geometryParamsValid = nullptr != geomParams;
    if (m_geometryParamsValid)
        m_geometryParams = *geomParams;
    else
        m_geometryParams = GeometryParams();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicPtr System::_CreateTile(TextureCR tile, GraphicBuilder::TileCorners const& corners, DgnDbR db, GraphicParamsCR params) const
    {
    TriMeshArgs rasterTile;

    // corners
    // [0] [1]
    // [2] [3]
    DPoint3dCP pts = corners.m_pts;
    rasterTile.m_pointParams = QPoint3d::Params(DRange3d::From(pts, 4));
    QPoint3d vertex[4];
    for (uint32_t i = 0; i < 4; ++i)
        vertex[i] = QPoint3d(pts[i], rasterTile.m_pointParams);

    rasterTile.m_points = vertex;
    rasterTile.m_numPoints = 4;

    static uint32_t indices[] = {0,1,2,2,1,3};
    rasterTile.m_numIndices = 6;
    rasterTile.m_vertIndex = indices;
    
    static FPoint2d textUV[] = 
        {
            {0.0f, 0.0f},
            {1.0f, 0.0f},
            {0.0f, 1.0f},
            {1.0f, 1.0f},
        };

    rasterTile.m_textureUV = textUV;
    rasterTile.m_texture = const_cast<Render::Texture*>(&tile);
    rasterTile.m_material = params.GetMaterial(); // not likely...
    rasterTile.m_isPlanar = true;

    return _CreateTriMesh(rasterTile, db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddShape(int numPoints, DPoint3dCP points, bool filled)
    {
    CurveVectorPtr curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer, ICurvePrimitive::CreateLineString(points, numPoints));
    m_accum.Add(*curve, filled, GetMeshDisplayParams(filled), GetLocalToWorldTransform(), GetCurrentClip(), false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddArc2d(DEllipse3dCR ellipse, bool isEllipse, bool filled, double priority)
    {
    double zDepth = Target::DepthFromDisplayPriority(static_cast<int32_t>(priority));
    if (0.0 == zDepth)
        {
        _AddArc(ellipse, isEllipse, filled);
        }
    else
        {
        auto ell = ellipse;
        ell.center.z = zDepth;
        _AddArc(ell, isEllipse, filled);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddArc(DEllipse3dCR ellipse, bool isEllipse, bool filled)
    {
    CurveVectorPtr curve = CurveVector::Create((isEllipse || filled) ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateArc(ellipse));
    if (filled && !isEllipse && !ellipse.IsFullEllipse())
        {
        DSegment3d segment;
        ICurvePrimitivePtr gapSegment;

        ellipse.EvaluateEndPoints(segment.point[1], segment.point[0]);
        gapSegment = ICurvePrimitive::CreateLine(segment);
        gapSegment->SetMarkerBit(ICurvePrimitive::CURVE_PRIMITIVE_BIT_GapCurve, true);
        curve->push_back(gapSegment);
        }

    m_accum.Add(*curve, filled, curve->IsAnyRegionType() ? GetMeshDisplayParams(filled) : GetLinearDisplayParams(), GetLocalToWorldTransform(), GetCurrentClip(), false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddLineString2d(int numPoints, DPoint2dCP points, double priority)
    {
    std::valarray<DPoint3d> pts3d(numPoints);
    copy2dTo3d(pts3d, points, priority);
    _AddLineString(numPoints, &pts3d[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddLineString(int numPoints, DPoint3dCP points)
    {
    CurveVectorPtr curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None, ICurvePrimitive::CreateLineString(points, numPoints));
    _AddCurveVectorR(*curve, false); // possibly a zero-length line segment (== disjoint)...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddPointString2d(int numPoints, DPoint2dCP points, double priority)
    {
    std::valarray<DPoint3d> pts3d(numPoints);
    copy2dTo3d(pts3d, points, priority);
    _AddPointString(numPoints, &pts3d[0]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddPointString(int numPoints, DPoint3dCP points)
    {
    CurveVectorPtr curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None, ICurvePrimitive::CreatePointString(points, numPoints));
    m_accum.Add(*curve, false, GetLinearDisplayParams(), GetLocalToWorldTransform(), GetCurrentClip(), true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddPolyface(PolyfaceQueryCR meshDataIn, bool filled)
    {
    _AddPolyfaceR(*meshDataIn.Clone(), filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddPolyfaceR(PolyfaceHeaderR mesh, bool filled)
    {
    Add(mesh, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddBody(IBRepEntityCR entity)
    {
    _AddBodyR(const_cast<IBRepEntityR>(entity));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddBodyR(IBRepEntityR entity)
    {
    m_accum.Add(entity, GetMeshDisplayParams(false), GetLocalToWorldTransform());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddShape2d(int numPoints, DPoint2dCP points, bool filled, double priority)
    {
    std::valarray<DPoint3d> pts3d(numPoints);
    copy2dTo3d(numPoints, &pts3d[0], points, priority);
    _AddShape(numPoints, &pts3d[0], filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddTextString(TextStringCR text)
    {
    // ###TODO_ELEMENT_TILE: May want to treat as box if too small...
    _AddTextStringR(*text.Clone());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddTextStringR(TextStringR text)
    {
    // ###TODO_ELEMENT_TILE: May want to treat as box if too small...
    m_accum.Add(text, GetTextDisplayParams(), GetLocalToWorldTransform());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddTextString2d(TextStringCR text, double priority)
    {
    double zDepth = Target::DepthFromDisplayPriority(static_cast<int32_t>(priority));
    if (0.0 == zDepth)
        {
        _AddTextString(text);
        }
    else
        {
        TextStringPtr ts = text.Clone();
        auto origin = ts->GetOrigin();
        origin.z = zDepth;
        ts->SetOrigin(origin);
        _AddTextStringR(*ts);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddTextString2dR(TextStringR text, double priority)
    {
    double zDepth = Target::DepthFromDisplayPriority(static_cast<int32_t>(priority));
    if (0.0 == zDepth)
        {
        _AddTextStringR(text);
        }
    else
        {
        TextStringPtr ts = text.Clone();
        auto origin = ts->GetOrigin();
        origin.z = zDepth;
        ts->SetOrigin(origin);
        _AddTextStringR(*ts);
        }
    }

PUSH_MSVC_IGNORE(6386) // Static analysis warning claims we overrun tmpPts...bogus.
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddTriStrip(int numPoints, DPoint3dCP points, AsThickenedLine usageFlags)
    {
    BeAssert(numPoints >= 3);

    if (AsThickenedLine::Yes == usageFlags) // represents thickened line...
        {
        int nPt = 0;
        auto tmpPts = reinterpret_cast<DPoint3dP>(_alloca((numPoints+1)*sizeof(DPoint3d)));
        
        for (int iPtS1 = 0; iPtS1 < numPoints; iPtS1 += 2)
            tmpPts[nPt++] = points[iPtS1];

        for (int iPtS2 = numPoints-1; iPtS2 > 0; iPtS2 -= 2)
            tmpPts[nPt++] = points[iPtS2];

        tmpPts[nPt] = tmpPts[0]; // Add closure point...simplifies drop of extrude thickness

        _AddShape(numPoints+1, tmpPts, true);
        }
    else
        {
        // spew triangles
        for (int iPt = 0; iPt < numPoints-2; iPt++)
            _AddShape(3, points+iPt, true);
        }
    }
POP_MSVC_IGNORE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddTriStrip2d(int numPoints, DPoint2dCP points, AsThickenedLine usageFlags, double priority)
    {
    std::valarray<DPoint3d> pts3d(numPoints);
    copy2dTo3d(pts3d, points, priority);
    _AddTriStrip(numPoints, &pts3d[0], usageFlags);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddSolidPrimitive(ISolidPrimitiveCR primitive)
    {
    _AddSolidPrimitiveR(*primitive.Clone());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddSolidPrimitiveR(ISolidPrimitiveR primitive)
    {
    m_accum.Add(primitive, GetMeshDisplayParams(false), GetLocalToWorldTransform());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddCurveVector(CurveVectorCR curves, bool isFilled)
    {
    _AddCurveVectorR(*curves.Clone(), isFilled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isDisjointCurvePrimitive(ICurvePrimitiveCR prim)
    {
    switch (prim.GetCurvePrimitiveType())
        {
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PointString:
            return true;
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            {
            DSegment3dCR segment = *prim.GetLineCP();
            return segment.IsAlmostSinglePoint();
            }
        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            {
            bvector<DPoint3d> const& points = *prim.GetLineStringCP();
            return 1 == points.size() || (2 == points.size() && points[0].AlmostEqual(points[1]));
            }
        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::AddCurveVector(CurveVectorR curves, bool isFilled, bool disjoint)
    {
    BeAssert(!isFilled || !disjoint);
    m_accum.Add(curves, isFilled, curves.IsAnyRegionType() ? GetMeshDisplayParams(isFilled) : GetLinearDisplayParams(), GetLocalToWorldTransform(), GetCurrentClip(), disjoint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddCurveVectorR(CurveVectorR curves, bool isFilled)
    {
    size_t numDisjoint = 0;
    bool haveContinuous = false;

    // NB: Somebody might stick a 'point' or point string into a curve vector with a boundary...
    // No idea what they expect us to do if it also contains continuous curves but it's dumb anyway.
    if (!isFilled && CurveVector::BOUNDARY_TYPE_None == curves.GetBoundaryType())
        {
        for (auto const& prim : curves)
            {
            if (isDisjointCurvePrimitive(*prim))
                ++numDisjoint;
            else
                haveContinuous = true;
            }
        }
    else
        {
        haveContinuous = true;
        }

    bool haveDisjoint = numDisjoint > 0;
    BeAssert(haveDisjoint || haveContinuous);
    if (haveDisjoint != haveContinuous)
        {
        // The typical case...
        AddCurveVector(curves, isFilled, haveDisjoint);
        return;
        }

    // Must split up disjoint and continuous into two separate curve vectors...
    // Note std::partition does not preserve relative order, but we don't care because boundary type NONE.
    std::partition(curves.begin(), curves.end(), [](ICurvePrimitivePtr const& arg) { return !isDisjointCurvePrimitive(*arg); });
    auto firstDisjoint = curves.begin() + (curves.size() - numDisjoint);

    CurveVectorPtr disjointCurves = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);
    disjointCurves->insert(disjointCurves->begin(), firstDisjoint, curves.end());
    curves.erase(firstDisjoint, curves.end());

    AddCurveVector(curves, false, false);
    AddCurveVector(*disjointCurves, false, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddCurveVector2d(CurveVectorCR curves, bool isFilled, double priority)
    {
    double zDepth = Target::DepthFromDisplayPriority(static_cast<int32_t>(priority));
    if (0.0 == zDepth)
        {
        _AddCurveVector(curves, isFilled);
        }
    else
        {
        Transform tf = Transform::From(DPoint3d::FromXYZ(0.0, 0.0, zDepth));
        auto cv = curves.Clone(tf);
        _AddCurveVectorR(*cv, isFilled);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddCurveVector2dR(CurveVectorR curves, bool isFilled, double priority)
    {
    double zDepth = Target::DepthFromDisplayPriority(static_cast<int32_t>(priority));
    if (0.0 == zDepth)
        {
        _AddCurveVectorR(*curves.Clone(), isFilled);
        }
    else
        {
        Transform tf = Transform::From(DPoint3d::FromXYZ(0.0, 0.0, zDepth));
        auto cv = curves.Clone(tf);
        _AddCurveVectorR(*cv, isFilled);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddBSplineCurve(MSBsplineCurveCR bcurve, bool filled)
    {
    CurveVectorPtr cv = CurveVector::Create(bcurve.params.closed ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateBsplineCurve(bcurve));
    m_accum.Add(*cv, filled, bcurve.params.closed ? GetMeshDisplayParams(filled) : GetLinearDisplayParams(), GetLocalToWorldTransform(), GetCurrentClip(), false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddBSplineCurveR(RefCountedMSBsplineCurveR bcurve, bool filled)
    {
    MSBsplineCurvePtr pBcurve(&bcurve);
    CurveVectorPtr cv = CurveVector::Create(bcurve.params.closed ? CurveVector::BOUNDARY_TYPE_Outer : CurveVector::BOUNDARY_TYPE_Open, ICurvePrimitive::CreateBsplineCurve(pBcurve));
    m_accum.Add(*cv, filled, bcurve.params.closed ? GetMeshDisplayParams(filled) : GetLinearDisplayParams(), GetLocalToWorldTransform(), GetCurrentClip(), false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddBSplineCurve2d(MSBsplineCurveCR bcurve, bool filled, double priority)
    {
    double zDepth = Target::DepthFromDisplayPriority(static_cast<int32_t>(priority));
    if (0.0 == zDepth)
        {
        _AddBSplineCurve(bcurve, filled);
        }
    else
        {
        MSBsplineCurvePtr bs = bcurve.CreateCopy();
        int nPoles = bs->GetNumPoles();
        DPoint3d* poles = bs->GetPoleP();
        for (int i = 0; i < nPoles; i++)
            poles[i].z = zDepth;

        _AddBSplineCurveR(*bs, filled);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddBSplineCurve2dR(RefCountedMSBsplineCurveR bcurve, bool filled, double priority)
    {
    double zDepth = Target::DepthFromDisplayPriority(static_cast<int32_t>(priority));
    if (0.0 == zDepth)
        {
        _AddBSplineCurveR(bcurve, filled);
        }
    else
        {
        int nPoles = bcurve.GetNumPoles();
        DPoint3d* poles = bcurve.GetPoleP();
        for (int i = 0; i < nPoles; i++)
            poles[i].z = zDepth;

        _AddBSplineCurveR(bcurve, filled);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/03
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddBSplineSurface(MSBsplineSurfaceCR surface)
    {
    _AddBSplineSurfaceR(*surface.Clone());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddBSplineSurfaceR(RefCountedMSBsplineSurfaceR surf)
    {
    m_accum.Add(surf, GetMeshDisplayParams(false), GetLocalToWorldTransform());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   07/04
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddDgnOle(DgnOleDraw* dgnOle)
    {
    BeAssert("TODO");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveBuilder::_AddSubGraphic(Graphic& gf, TransformCR subToGf, GraphicParamsCR gfParams, ClipVectorCP clip)
    {
    // ###TODO_ELEMENT_TILE: Overriding GraphicParams?
    // ###TODO_ELEMENT_TILE: Clip...
    Render::GraphicPtr graphic(&gf);
    if (nullptr != clip || !subToGf.IsIdentity())
        {
        GraphicBranch branch;
        branch.Add(gf);
        graphic = GetSystem()._CreateBranch(std::move(branch), GetDgnDb(), Transform::FromProduct(GetLocalToWorldTransform(), subToGf), clip);
        }

    if (graphic.IsValid())
        m_primitives.push_back(graphic);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicBuilderPtr PrimitiveBuilder::_CreateSubGraphic(TransformCR subToGf, ClipVectorCP clip) const
    {
    auto tf = subToGf.IsIdentity() ? GetLocalToWorldTransform() : Transform::FromIdentity();
    auto params = m_createParams.SubGraphic(tf);
    GraphicBuilderPtr subGf = GetSystem()._CreateGraphic(params);
    // ###TODO: Set clip on subgraphic?
    return subGf;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicPtr GeometryListBuilder::_Finish()
    {
    if (!IsOpen())
        {
        BeAssert(false);
        return nullptr;
        }

    GraphicPtr graphic = _FinishGraphic(m_accum);
    BeAssert(graphic.IsValid()); // callers of Finish() assume they're getting back a non-null graphic...

    m_isOpen = false;
    m_accum.Clear();
    return graphic;
    }

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   09/17
//=======================================================================================
struct PrimitiveBuilderContext : ViewContext
{
    System& m_system;

    explicit PrimitiveBuilderContext(PrimitiveBuilderR gf) : m_system(gf.GetSystem())
        {
        SetDgnDb(gf.GetDgnDb());
        Attach(gf.GetViewport(), DrawPurpose::NotSpecified);
        }

    SystemP _GetRenderSystem() const override { return &m_system; }
    GraphicBuilderPtr _CreateGraphic(GraphicBuilder::CreateParams const&) override { BeAssert(false); return nullptr; }
    GraphicPtr _CreateBranch(GraphicBranch&, DgnDbR, TransformCR, ClipVectorCP) override { BeAssert(false); return nullptr; }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicPtr PrimitiveBuilder::_FinishGraphic(GeometryAccumulatorR accum)
    {
    if (!accum.IsEmpty())
        {
        // Overlay decorations don't test Z. Tools like to layer multiple primitives on top of one another; they rely on the primitives rendering
        // in that same order to produce correct results (e.g., a thin line rendered atop a thick line of another color).
        // No point generating edges for graphics that are always rendered in smooth shade mode.
        GeometryOptions options(GetCreateParams());
        PrimitiveBuilderContext context(*this);
        double tolerance = ComputeTolerance(accum);
        accum.SaveToGraphicList(m_primitives, options, tolerance, context);
        }

    if (1 != m_primitives.size())
        return GetSystem()._CreateGraphicList(std::move(m_primitives), GetDgnDb());

    GraphicPtr graphic = *m_primitives.begin();
    m_primitives.clear();
    return graphic;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/17
+---------------+---------------+---------------+---------------+---------------+------*/
double PrimitiveBuilder::ComputeTolerance(GeometryAccumulatorR accum) const
    {
    constexpr double s_toleranceMult = 0.25;
    auto const& params = GetCreateParams();
    if (params.IsViewCoordinates())
        {
        return s_toleranceMult;
        }
    else if (nullptr == params.GetViewport())
        {
        BeAssert(!accum.GetGeometries().ContainsCurves() && "No viewport supplied to GraphicBuilder::CreateParams - falling back to default coarse tolerance");
        return 20.0;
        }
    else
        {
        BeAssert(!accum.IsEmpty());
        DRange3d range = accum.GetGeometries().ComputeRange(); // NB: Already multiplied by transform...

        // NB: Geometry::CreateFacetOptions() will apply any scale factors from transform...no need to do it here.
        DPoint3d pt = DPoint3d::FromInterpolate(range.low, 0.5, range.high);
        return params.GetViewport()->GetPixelSizeAtPoint(&pt) * s_toleranceMult;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::ReInitialize(TransformCR localToWorld, TransformCR accumTf, DgnElementId elemId)
    {
    m_accum.ReInitialize(accumTf, elemId);
    ActivateGraphicParams(GraphicParams(), nullptr);
    m_createParams.SetPlacement(localToWorld);
    m_isOpen = true;

    _Reset();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3d GeometryList::ComputeRange() const
    {
    DRange3d range = DRange3d::NullRange();
    for (auto const& geom : *this)
        range.Extend(geom->GetTileRange());

    return range;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/17
+---------------+---------------+---------------+---------------+---------------+------*/
GeometryList GeometryList::Slice(size_t startIdx, size_t endIdx) const
    {
    BeAssert(startIdx < endIdx && endIdx <= size());

    GeometryList list;
    list.m_complete = m_complete;
    list.m_curved = m_curved;
    list.m_list.assign(begin() + startIdx, begin() + endIdx);

    return list;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void TriangleList::AddTriangle(TriangleCR triangle)
    {
    uint8_t         flags = triangle.m_singleSided ? 1 : 0;

    for (size_t i=0; i<3; i++)
        {
        if (triangle.GetEdgeVisible(i))
            flags |= (0x0002 << i);
        
        m_indices.push_back(triangle.m_indices[i]);
        }

    m_flags.push_back(flags);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Triangle  TriangleList::GetTriangle(size_t index) const
    {
    if (index > m_flags.size())
        {
        BeAssert(false);
        return Triangle();
        }
    
    uint8_t         flags = m_flags.at(index);
    Triangle        triangle(0 != (flags & 0x0001));
    uint32_t const* pIndex = &m_indices.at(index*3);

    for (size_t i=0; i<3; i++)
        {
        triangle.m_indices[i] = pIndex[i];
        triangle.m_edgeFlags[i] = (0 == (flags & (0x0002 << i))) ? MeshEdge::Flags::Invisible : MeshEdge::Flags::Visible;
        }

    return triangle;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayParamsCPtr DisplayParams::Create(Type type, DgnCategoryId catId, DgnSubCategoryId subCatId, GradientSymbCP gradient, RenderMaterialId materialId, ColorDef lineColor, ColorDef fillColor, uint32_t width, LinePixels linePixels, FillFlags fillFlags, DgnGeometryClass geomClass, bool ignoreLights, DgnDbR dgnDb, System& renderSys, TextureMappingCR texMap)
    {
    switch (type)
        {
        case Type::Mesh:
            {
            MaterialPtr mat;
            TextureMapping tex;
            TextureMappingCP pTex = nullptr;
            if (materialId.IsValid())
                {
                // NB: For now, we will never have a persistent material with a non-persistent texture (so ignore texMap arg)
                mat = renderSys._GetMaterial(materialId, dgnDb);
                }
            else if (nullptr != gradient)
                {
                tex = TextureMapping(*renderSys._GetTexture(*gradient, dgnDb));
                pTex = &tex;
                }
            else if (texMap.IsValid())
                {
                pTex = &texMap;
                }

            return new DisplayParams(lineColor, fillColor, width, linePixels, mat.get(), gradient, pTex, fillFlags, catId, subCatId, geomClass);
            }
        case Type::Linear:
            {
            return new DisplayParams(lineColor, width, linePixels, catId, subCatId, geomClass);
            }
        case Type::Text:
            return new DisplayParams(lineColor, catId, subCatId, geomClass);
        default:
            BeAssert(false);
            return nullptr;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
MeshBuilderR MeshBuilderMap::operator[](Key const& key)
    {
    auto found = m_map.find(key);
    if (m_map.end() == found)
        {
        MeshBuilderPtr builder = MeshBuilder::Create(*key.m_params, m_vertexTolerance, m_facetAreaTolerance, m_featureTable, key.m_type, m_range, m_is2d, key.m_isPlanar);
        found = m_map.Insert(key, builder).first;
        }

    return *found->second;
    }


