/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/RenderPrimitives.h>
#if defined(BENTLEYCONFIG_PARASOLID)
#include <BRepCore/PSolidUtil.h>
#endif

USING_NAMESPACE_BENTLEY_RENDER
USING_NAMESPACE_BENTLEY_RENDER_PRIMITIVES

#define COMPARE_VALUES_TOLERANCE(val0, val1, tol) { if (val0 < val1 - tol) return true; if (val0 > val1 + tol) return false; }
#define COMPARE_VALUES(val0, val1) { if (val0 < val1) return true; if (val0 > val1) return false; }

// #define DEBUG_DISPLAY_PARAMS_CACHE

BEGIN_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/18
+---------------+---------------+---------------+---------------+---------------+------*/
static double depthFromDisplayPriority(double priority)
    {
    // zDepth is obtained from GeometryParams::GetNetDisplayPriority(), which returns an int32_t.
    // Coming from mstn, priorities tend to be in [-500..500]
    // Let's assume that mstn's range is the full range and clamp anything outside that.
    // Map them to [-s_half2dDepthRange, s_half2dDepthRange]
    static const double s_half2dDepthRange = 0.5 * ViewDefinition2d::Get2dFrustumDepth();
    static const double priorityRange = 500;
    static const double ratio = s_half2dDepthRange / priorityRange;

    auto depth = std::min(priority, priorityRange);
    depth = std::max(depth, -priorityRange);
    return depth * ratio;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
static void copy2dTo3d(int numPoints, DPoint3dP pts3d, DPoint2dCP pts2d, double priority)
    {
    double depth = depthFromDisplayPriority(priority);
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
    fpoint.x = static_cast<float>(dpoint.x);
    fpoint.y = static_cast<float>(dpoint.y);
    return fpoint;
    }

//=======================================================================================
// Largely lifted from GeomLibs/CurveVector.cpp.  Modified to track whether a resulting
// list of points is from a linestring.  If so, an output flag is set which signifies it
// is able to decimated at a later time.
// @bsistruct                                                   Mark.Schlosser  04/2019
//=======================================================================================
struct LinearGeometryCollectorForDecimation
{
size_t errors;
IFacetOptionsR m_facetOptions;

LinearGeometryCollectorForDecimation(IFacetOptionsR options)
    : m_facetOptions(options)
    {
    errors = 0;
    }

size_t NumErrors() {return errors;}

// Return false if anything other than Line or LineString
bool CollectPointsFromSinglePrimitive
(
ICurvePrimitiveCR prim,
bvector<DPoint3d> &path
)
    {
    size_t initialCount = path.size();
    prim.AddStrokes(path, m_facetOptions, true);
    PolylineOps::PackAlmostEqualAfter(path, initialCount);
    return true;
    }

// Return false if anything other than Line or LineString
bool CollectPointsFromSinglePrimitive
(
ICurvePrimitiveCR prim,
bvector<bvector<DPoint3d>> &paths,
bvector<bool> &canDecimateFlags
)
    {
    paths.push_back(bvector<DPoint3d>());
    canDecimateFlags.push_back(prim.GetCurvePrimitiveType() == ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_LineString);
    return CollectPointsFromSinglePrimitive(prim, paths.back());
    }

// Return false if anything other than Line or LineString
bool CollectPointsFromSinglePrimitive
(
ICurvePrimitiveCR prim,
bvector<bvector<bvector<DPoint3d>>> &paths,
bvector<bvector<bool>> &canDecimateFlags
)
    {
    paths.push_back(bvector<bvector<DPoint3d>>());
    canDecimateFlags.push_back(bvector<bool>());
    return CollectPointsFromSinglePrimitive(prim, paths.back(), canDecimateFlags.back());
    }

void CollectPointsFromLinearPrimitives
(
CurveVectorCR curves,
bvector<bvector<bvector<DPoint3d>>> &paths,
bvector<bvector<bool>> &canDecimateFlags
)
    {
    if (curves.IsUnionRegion())
        {
        // Each child will become a new top level region
        for (auto &prim : curves)
            {
            CollectPointsFromLinearPrimitives(*prim->GetChildCurveVectorP(), paths, canDecimateFlags);
            }
        }
    else if (curves.IsOpenPath() || curves.IsClosedPath())
        {
        paths.push_back(bvector<bvector<DPoint3d>>());
        auto &parityLoops = paths.back();
        parityLoops.push_back(bvector<DPoint3d>());
        auto &loop = parityLoops.back();
        canDecimateFlags.push_back(bvector<bool>());
        auto &decFlags = canDecimateFlags.back();
        bool canDecimate = true;
        for (auto &prim : curves)
            {
            // We're producing a single line string (path or loop) from multiple curve primitives.
            // The result is eligible for decimation if it contains no curved geometry.
            switch (prim->GetCurvePrimitiveType())
                {
                // Only line strings are worth decimating. However a path containing more than one Line primitive is in effect a line string.
                case ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_LineString:
                case ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_Line:
                    break;
                // Anything else is curved, therefore not eligible for decimation (already stroked to desired tolerance).
                default:
                    canDecimate = false;
                    break;
                }

            if (!CollectPointsFromSinglePrimitive(*prim, loop))
                errors++;
            }

        decFlags.push_back(canDecimate);

        if (curves.IsClosedPath())
            PolylineOps::EnforceClosure(loop);
        }
    else if (curves.GetBoundaryType() == CurveVector::BOUNDARY_TYPE_None)
        {
        // Anything (single prim, open, closed, parity, union) is possible . ..
        for (auto &prim : curves)
            {
            auto child = prim->GetChildCurveVectorCP();
            if (nullptr != child)
                CollectPointsFromLinearPrimitives(*child, paths, canDecimateFlags);
            else
                CollectPointsFromSinglePrimitive(*prim, paths, canDecimateFlags);
            }
        }
    else
        errors++;
    }
};

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mark.Schlosser   04/2019
+--------------------------------------------------------------------------------------*/
static bool collectLinearGeometry(CurveVectorCR curve, bvector<bvector<bvector<DPoint3d>>> &regionPoints, bvector<bvector<bool>> &canDecimateFlags, IFacetOptionsR strokeOptions)
    {
    LinearGeometryCollectorForDecimation collector(strokeOptions);
    regionPoints.clear ();
    collector.CollectPointsFromLinearPrimitives(curve, regionPoints, canDecimateFlags);
    return collector.NumErrors () == 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static bool collectCurveStrokes (Strokes::PointLists& strokes, CurveVectorCR curve, IFacetOptionsR facetOptions, TransformCP transform) // returns true if any can be decimated
    {
    bvector<bvector<bvector<DPoint3d>>> strokesArray;
    bvector<bvector<bool>> canDecimateArray;

    collectLinearGeometry(curve, strokesArray, canDecimateArray, facetOptions);

    bool canDecimateSomething = false;
    int i = 0;
    for (auto& loop : strokesArray)
        {
        int j = 0;
        auto& decFlags = canDecimateArray[i++];
        for (auto& loopStrokes : loop)
            {
            if (nullptr != transform)
                transform->Multiply(loopStrokes, loopStrokes);

            bool canDecimate = decFlags[j++];
            strokes.emplace_back(std::move(loopStrokes), canDecimate);
            canDecimateSomething = canDecimateSomething || canDecimate;
            }
        }

    return canDecimateSomething;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static bool collectCurveStrokes (Strokes::PointLists& strokes, CurveVectorCR curve, IFacetOptionsR facetOptions, TransformCR transform) // returns true if any can be decimated
    {
    return collectCurveStrokes(strokes, curve, facetOptions, &transform);
    }

//=======================================================================================
// This exists only to consolidate implementation of GetPolyfaces/Strokes used for
// everything except geometry part instances. Used to be used by all Geometry classes
// but TFS#813379 you can't create a single facet options for a geometry part's list of
// geometries - some may have/need normals, UV params, etc; others may not.
// @bsistruct                                                   Paul.Connelly   01/18
//=======================================================================================
struct SingularGeometry : Geometry
{
protected:
    using Geometry::Geometry;

    IFacetOptionsPtr CreateFacetOptions(double chordTolerance, NormalMode normalMode, ViewContextCR context) const;

    virtual PolyfaceList _GetPolyfaces(IFacetOptionsR, ViewContextR) = 0;
    virtual StrokesList _GetStrokes(IFacetOptionsR, ViewContextR) { return StrokesList(); }

    PolyfaceList _GetPolyfaces(double chordTolerance, NormalMode normalMode, ViewContextR context) override
        {
        return _GetPolyfaces(*CreateFacetOptions(chordTolerance, normalMode, context), context);
        }
    StrokesList _GetStrokes(double chordTolerance, ViewContextR context) override
        {
        return _GetStrokes(*CreateFacetOptions(chordTolerance, NormalMode::Never, context), context);
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct PrimitiveGeometry : SingularGeometry
{
private:
    IGeometryPtr        m_geometry;
    bool                m_inCache = false;
    bool                m_disjoint;

    PrimitiveGeometry(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsCR params, bool isCurved, DgnDbR db, bool disjoint)
        : SingularGeometry(tf, range, elemId, params, isCurved, db), m_geometry(&geometry), m_disjoint(disjoint) { }

    PolyfaceList _GetPolyfaces(IFacetOptionsR facetOptions, ViewContextR context) override;
    StrokesList _GetStrokes (IFacetOptionsR facetOptions, ViewContextR context) override;
    void _SetInCache(bool inCache) override { m_inCache = inCache; }

    static PolyfaceHeaderPtr FixPolyface(PolyfaceHeaderR, IFacetOptionsR);
    static void AddNormals(PolyfaceHeaderR, IFacetOptionsR);
    static void AddParams(PolyfaceHeaderR, IFacetOptionsR);
public:
    bool IsInstanceable() const override { return true; }

    static GeometryPtr Create(IGeometryR geometry, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsCR params, bool isCurved, DgnDbR db, bool disjoint)
        {
        BeAssert(!disjoint || geometry.GetAsCurveVector().IsValid());
        return new PrimitiveGeometry(geometry, tf, range, elemId, params, isCurved, db, disjoint);
        }
};

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   08/16
//=======================================================================================
struct SolidKernelGeometry : SingularGeometry
{
private:
    IBRepEntityPtr      m_entity;

    SolidKernelGeometry(IBRepEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsCR params, DgnDbR db);

    PolyfaceList _GetPolyfaces(IFacetOptionsR facetOptions, ViewContextR context) override;
public:
    bool IsInstanceable() const override { return true; }

    static GeometryPtr Create(IBRepEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsCR params, DgnDbR db)
        {
        return new SolidKernelGeometry(solid, tf, range, elemId, params, db);
        }
};

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     11/2016
//=======================================================================================
struct TextStringGeometry : SingularGeometry
{
private:
    TextStringPtr                   m_text;
    mutable bvector<CurveVectorPtr> m_glyphCurves;
    DgnDbR                          m_db;

    TextStringGeometry(TextStringR text, TransformCR transform, DRange3dCR range, DgnElementId elemId, DisplayParamsCR params, DgnDbR db, bool checkGlyphBoxes)
        : SingularGeometry(transform, range, elemId, params, true, db), m_text(&text), m_db(db)
        {
        // ###TODO remove dumb checkGlyphBoxes arg - it's not used?
        }

public:
    // Brien doesn't put text into geometry parts. Other bridges might. Raster text won't currently work properly due to the way the glyph atlases are produced.
    // No easy way to detect here if raster text will be produced. For now just don't instance text.
    bool IsInstanceable() const override { return false; }

    static GeometryPtr Create(TextStringR textString, TransformCR transform, DRange3dCR range, DgnElementId elemId, DisplayParamsCR params, DgnDbR db, bool checkGlyphBoxes)
        {
        return new TextStringGeometry(textString, transform, range, elemId, params, db, checkGlyphBoxes);
        }

    PolyfaceList _GetPolyfaces(IFacetOptionsR facetOptions, ViewContextR context) override;
    StrokesList _GetStrokes (IFacetOptionsR facetOptions, ViewContextR context) override;

    DgnDbR GetDb() const { return m_db; }
    TextStringCR GetText() const { return *m_text; }
};  // TextStringGeometry

//=======================================================================================
// @bsistruct                                                   Ray.Bentley     11/2016
//=======================================================================================
struct InstancedGeometry : Geometry
{
private:
    SharedGeomPtr   m_geom;

    InstancedGeometry(SharedGeomR geom, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsCR params, DgnDbR db)
        : Geometry(tf, range, elemId, params, geom.IsCurved(), db), m_geom(&geom) { }

public:
    static GeometryPtr Create(SharedGeomR geom, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsCR params, DgnDbR db)  { return new InstancedGeometry(geom, tf, range, elemId, params, db); }

    PolyfaceList _GetPolyfaces(double chordTolerance, NormalMode normalMode, ViewContextR context) override { return m_geom->GetPolyfaces(chordTolerance, normalMode, this, context); }
    StrokesList _GetStrokes (double chordTolerance, ViewContextR context) override { return m_geom->GetStrokes(chordTolerance, this, context); }
    SharedGeomCPtr _GetSharedGeom() const override { return m_geom; }
    bool IsInstanceable() const override { return false; }
};

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

        Strokes::PointLists GetStrokes() const
            {
            Strokes::PointLists clone;
            for (auto const& pointList : m_strokes)
                {
                clone.emplace_back(pointList.m_startDistance, pointList.m_canDecimate);
                clone.back().m_points = pointList.m_points;
                }

            return clone;
            }

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

            Raster() { } // for bmap...
            Raster(DgnGlyphCR glyph, DgnFontCR font)
                {
                m_name.Sprintf("%s-%u", font.GetName().c_str(), glyph.GetId());

                DgnGlyph::RasterStatus status = glyph.GetRaster(m_image);
                if (DgnGlyph::RasterStatus::Success != status)
                    {
                    DEBUG_PRINTF("Error %u retrieving raster", status);
                    return;
                    }

                DebugPrintImage();
                DebugSaveImage();
                }

            void DebugPrintImage() const;
            void DebugSaveImage() const;
            bool IsValid() const { return m_image.IsValid(); }
        };

    struct RasterPolyface
    {
        PolyfaceHeaderPtr   m_polyface;

        bool IsValid() const { return m_polyface.IsValid(); }
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
            if ((m_curves = glyph.GetCurveVector()).IsValid())
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
        Raster* GetRaster() { return m_raster.IsValid() ? &m_raster : nullptr; }
        bool GetRange(DRange3dR cRange) const { if (IsValid()) { return m_curves->GetRange(cRange); }  return false; }

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
                    [&](Geom& geom) { polyface.m_polyface = geom.GetRasterPolyface(); });
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

    using Map = std::map<DgnGlyphCP, Glyph>;

    Map m_map;

    GlyphCache() { }

    static GlyphCache& Get(DgnDbR db)
        {
        return *db.ObtainAppData(GetKey(), []() { return new GlyphCache(); });
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
* @bsimethod                                                    Mark.Schlosser  04/2019
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t Strokes::ComputePointCount() const
    {
    size_t sz = 0;
    for (auto const& stroke : m_strokes)
        sz += stroke.m_points.size();

    return static_cast<uint32_t>(sz);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/19
+---------------+---------------+---------------+---------------+---------------+------*/
SurfaceMaterial::SurfaceMaterial(MaterialR material, TextureMapping::Trans2x3 const* transform)
    {
    // Do not bother with a material that has no effect on rendering in iModel.js.
    if (!material.GetTextureMapping().IsValid() && MaterialComparison::MatchesDefaults(&material))
        return;

    m_material = &material;
    m_textureMapping = material.GetTextureMapping();
    if (m_textureMapping.IsValid() && nullptr != transform)
        m_textureMapping.SetTransform(*transform);
    }

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
    lineColor = AdjustTransparency(lineColor);
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
    lineColor = AdjustTransparency(lineColor);
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
void DisplayParams::InitMesh(ColorDef lineColor, ColorDef fillColor, uint32_t width, LinePixels pixels, SurfaceMaterialCR mat,
    FillFlags fillFlags, DgnCategoryId catId, DgnSubCategoryId subCatId, DgnGeometryClass geomClass)
    {
    InitGeomParams(catId, subCatId, geomClass);
    m_type = Type::Mesh;
    m_lineColor = AdjustTransparency(lineColor);
    m_fillColor = AdjustTransparency(fillColor);
    m_fillFlags = fillFlags;
    m_surfaceMaterial = mat;
    m_width = width;
    m_linePixels = pixels;

    BeAssert(nullptr == m_surfaceMaterial.GetGradient() || m_surfaceMaterial.GetTextureMapping().IsValid());
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

    SurfaceMaterial surfMat;
    auto grad = gf.GetGradientSymb();
    if (nullptr != grad)
        surfMat = SurfaceMaterial(TextureMapping(*sys._GetTexture(*grad, db)), grad);
    else if (nullptr != gf.GetMaterial())
        surfMat = SurfaceMaterial(*gf.GetMaterial(), gf.GetMaterialUVDetail().GetTransform());

    return DisplayParams(gf.GetLineColor(), gf.GetFillColor(), gf.GetWidth(), static_cast<LinePixels>(gf.GetLinePixels()), surfMat, fillFlags, catId, subCatId, geomClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayParams::RegionEdgeType DisplayParams::GetRegionEdgeType() const
    {
    if (HasBlankingFill())
        return RegionEdgeType::None;

    auto grad = GetSurfaceMaterial().GetGradient();
    if (nullptr != grad)
        {
        // Even if the gradient is not outlined, produce an outline to be displayed as the region's edges when fill ViewFlag is off.
        if (grad->GetIsOutlined() || FillFlags::None == (GetFillFlags() & FillFlags::Always))
            return RegionEdgeType::Outline;
        else
            return RegionEdgeType::None;
        }

    return m_fillColor != m_lineColor ? RegionEdgeType::Outline : RegionEdgeType::Default;
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
DisplayParamsCPtr DisplayParams::CloneForRasterText(TextureCR texture) const
    {
    BeAssert(Type::Text == GetType());

    TextureMapping::Trans2x3 tf(0.0, 1.0, 0.0, 1.0, 0.0, 0.0);
    TextureMapping::Params params;
    params.SetWeight(0.0);
    params.SetTransform(&tf);

    return CloneWithTextureOverride(TextureMapping(texture, params));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayParamsCPtr DisplayParams::CloneWithTextureOverride(TextureMappingCR textureMapping) const
    {
    auto clone = new DisplayParams(*this);
    clone->m_surfaceMaterial = SurfaceMaterial(textureMapping);

    return clone;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayParamsCPtr DisplayParams::CloneForMeshedLineString() const
    {
    BeAssert (Type::Linear == GetType());
    auto clone = new DisplayParams(*this);
    clone->m_type = Type::Mesh;
    clone->m_fillFlags = FillFlags::Always;

    return clone;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayParamsCPtr DisplayParams::Clone(ColorDef fillColor, ColorDef lineColor, uint32_t lineWidth, LinePixels linePixels) const
    {
    auto clone = new DisplayParams(*this);
    clone->m_fillColor = fillColor;
    clone->m_lineColor = lineColor;
    clone->m_width = lineWidth;
    clone->m_linePixels = linePixels;

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

/*---------------------------------------------------------------------------------**//**
* NB: We used to compare material and texture by address. When reading MeshBuilderMaps back from the tile cache data,
* we want to preserve order - so must compare by stable identifiers instead.
* (Materials/textures without stable identifiers are never serialized to cache, so safe to compare those by address).
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> int compareResources(T const* lhs, T const* rhs)
    {
    bool lhNull = nullptr == lhs, rhNull = nullptr == rhs;

    if (lhNull)
        return rhNull ? 0 : -1;
    else if (rhNull)
        return 1;

    auto const& lhKey = lhs->GetKey();
    auto const& rhKey = rhs->GetKey();

    bool lhValid = lhKey.IsValid(), rhValid = rhKey.IsValid();
    if (!lhValid)
        return rhValid ? -1 : 0;
    else if (!rhValid)
        return 1;

    BeAssert(lhValid && rhValid);

    if (lhKey < rhKey)
        return -1;
    else if (rhKey < lhKey)
        return 1;
    else
        return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
template<> int compareValues(MaterialP const& lhs, MaterialP const& rhs)
    {
    return compareResources(lhs, rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
template<> int compareValues(TextureCP const& lhs, TextureCP const& rhs)
    {
    return compareResources(lhs, rhs);
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

    TEST_LESS_THAN(GetType());
    TEST_LESS_THAN(IgnoresLighting());
    TEST_LESS_THAN(GetLineWidth());
    TEST_LESS_THAN(GetLinePixels());
    TEST_LESS_THAN(GetFillFlags());

    if (GetSurfaceMaterial().IsLessThan(rhs.GetSurfaceMaterial(), purpose))
        return true;

    // Region outline produces a polyline which is considered an edge of a region surface
    TEST_LESS_THAN(WantRegionOutline());

    if (ComparePurpose::Strict != purpose)
        {
        TEST_LESS_THAN(HasFillTransparency());
        TEST_LESS_THAN(HasLineTransparency());

        if (GetSurfaceMaterial().GetTextureMapping().IsValid())
            TEST_LESS_THAN(GetFillColor());     // Textures may use color so they can't be merged. (could test if texture actually uses color).

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
* @bsimethod                                                    Paul.Connelly   01/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool SurfaceMaterial::IsLessThan(SurfaceMaterialCR rhs, ComparePurpose purpose) const
    {
    TEST_LESS_THAN(GetTextureMapping().GetTexture());

    if (ComparePurpose::Merge == purpose)
        {
        // Batching uses material atlases. Atlases don't permit mixing opaque and translucent materials.
        auto lhCat = nullptr != GetMaterial() ? GetMaterial()->GetParams().m_transparency.Categorize() : Material::Transparency::None;
        auto rhCat = nullptr != rhs.GetMaterial() ? rhs.GetMaterial()->GetParams().m_transparency.Categorize() : Material::Transparency::None;
        if (lhCat != rhCat)
            return lhCat < rhCat;
        }
    else
        {
        TEST_LESS_THAN(IsValid());
        if (!IsValid())
            return false;

        if (ComparePurpose::Instance == purpose)
            {
            // Instancing requires the materials to have equivalent properties.
            auto compareMaterials = MaterialComparison::Compare(GetMaterial(), rhs.GetMaterial());
            if (CompareResult::Equal != compareMaterials)
                return CompareResult::Less == compareMaterials;
            }
        else
            {
            TEST_LESS_THAN(GetMaterial());
            }
        }


    // NB: ComparePurpose::Instance requires equal UV transform.
    if (ComparePurpose::Merge == purpose || !GetTextureMapping().IsValid())
        return false;

    auto tol = DoubleOps::SmallCoordinateRelTol();
    auto const& lmat = GetTextureMapping().GetParams().m_textureMat2x3;
    auto const& rmat = rhs.GetTextureMapping().GetParams().m_textureMat2x3;
    for (auto i = 0; i < 2; i++)
        {
        for (auto j = 0; j < 3; j++)
            {
            COMPARE_VALUES_TOLERANCE(lmat.m_val[i][j], rmat.m_val[i][j], tol);
            }
        }

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
    TEST_EQUAL(GetLinePixels());
    TEST_EQUAL(GetFillFlags());

    if (!GetSurfaceMaterial().IsEqualTo(rhs.GetSurfaceMaterial(), purpose))
        return false;

    // Region outline produces a polyline which is considered an edge of a region surface
    TEST_EQUAL(WantRegionOutline());

    if (ComparePurpose::Strict != purpose)
        {
        TEST_EQUAL(HasFillTransparency());
        TEST_EQUAL(HasLineTransparency());
        if (GetSurfaceMaterial().GetTextureMapping().IsValid())
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
* @bsimethod                                                    Paul.Connelly   01/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool SurfaceMaterial::IsEqualTo(SurfaceMaterialCR rhs, ComparePurpose purpose) const
    {
    TEST_EQUAL(IsValid());
    if (!IsValid())
        return true;

    TEST_EQUAL(GetTextureMapping().GetTexture());
    if (ComparePurpose::Merge == purpose)
        {
        // Batching uses material atlases. Atlases don't permit mixing opaque and translucent materials.
        auto lhCat = nullptr != GetMaterial() ? GetMaterial()->GetParams().m_transparency.Categorize() : Material::Transparency::None;
        auto rhCat = nullptr != rhs.GetMaterial() ? rhs.GetMaterial()->GetParams().m_transparency.Categorize() : Material::Transparency::None;
        if (lhCat != rhCat)
            return false;
        }
    else if (ComparePurpose::Instance == purpose)
        {
        if (!MaterialComparison::Equals(GetMaterial(), rhs.GetMaterial()))
            return false;
        }
    else
        {
        TEST_EQUAL(GetMaterial());
        }


    // NB: ComparePurpose::Instance requires equal UV transform
    if (ComparePurpose::Merge != purpose && GetTextureMapping().IsValid())
        return GetTextureMapping().GetParams().m_textureMat2x3.AlmostEqual(rhs.GetTextureMapping().GetParams().m_textureMat2x3);

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
    Utf8PrintfString str("%s line:%x fill:%x width:%u pix:%u fillflags:%u class:%u, %s",
        types[static_cast<uint8_t>(m_type)],
        m_lineColor.GetValue(),
        m_fillColor.GetValue(),
        m_width,
        static_cast<uint32_t>(m_linePixels),
        static_cast<uint32_t>(m_fillFlags),
        static_cast<uint32_t>(m_class),
        m_ignoreLighting ? "ignoreLights" : "");

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
* @bsimethod                                                    Ray.Bentley     09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void Mesh::CompressVertexQuantization()
    {
    QPoint3d        min(0xffff, 0xffff, 0xffff), max(0, 0, 0);// unused - , initialMin = min, initialMax = max;

    for (auto& vert : m_verts)
        {
        if (vert.x < min.x)
            min.x = vert.x;

        if (vert.x > max.x)
            max.x = vert.x;

        if (vert.y < min.y)
            min.y = vert.y;

        if (vert.y > max.y)
            max.y = vert.y;

        if (vert.z < min.z)
            min.z = vert.z;

        if (vert.z > max.z)
            max.z = vert.z;
        }

    if (min.x == 0 && min.y == 0 && min.z == 0 && max.x == 0xffff && max.y == 0xffff && max.z == 0xffff)
        return;     // No compression possible.

    DRange3d    newRange;

    newRange.low = min.Unquantize(m_verts.GetParams());
    newRange.high = max.Unquantize(m_verts.GetParams());

    m_verts.Requantize(QPoint3d::Params(newRange));
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
* @bsimethod                                                    Paul.Connelly   01/18
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicPtr Mesh::GetGraphics(MeshGraphicArgs& args, SystemCR system, DgnDbR db) const
    {
    GraphicPtr graphic;
    if (!Triangles().Empty())
        {
        if (args.m_meshArgs.Init(*this))
            graphic = system._CreateTriMesh(args.m_meshArgs, db);
        }
    else if (!Polylines().empty() && args.m_polylineArgs.Init(*this))
        {
        graphic = system._CreateIndexedPolylines(args.m_polylineArgs, db);
        }

    return graphic;
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
uint32_t Mesh::AddVertex(QPoint3dCR vert, OctEncodedNormalCP normal, DPoint2dCP param, uint32_t fillColor, FeatureCR feature, uint8_t materialIndex)
    {
    auto index = static_cast<uint32_t>(m_verts.size());

    m_verts.push_back(vert);
    m_features.Add(feature, m_verts.size());
    m_materials.Add(materialIndex, m_verts.size());

    if (nullptr != normal)
        m_normals.push_back(*normal);

    if (nullptr != param)
        m_uvParams.push_back(toFPoint2d(*param));

    insertVertexAttribute(m_colors, m_colorTable, fillColor, m_verts);

    return index;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
void    Mesh::AddAuxData(PolyfaceAuxDataCR auxData, size_t index)
    {
    if (m_auxChannels.empty())
        for (auto& channel: auxData.GetChannels())
            m_auxChannels.push_back(channel->CloneWithoutData());

    for (size_t i=0; i<m_auxChannels.size(); i++)
        m_auxChannels[i]->AppendDataByIndex(*auxData.GetChannels().at(i), index);
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
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
void Mesh::Materials::Add(uint8_t materialIndex, size_t numVerts)
    {
    if (!m_haveAtlas)
        return;

    if (!m_initialized)
        {
        m_uniform = materialIndex;
        m_initialized = true;
        }
    else if (!m_indices.empty())
        {
        m_indices.push_back(materialIndex);
        }
    else if (m_uniform != materialIndex)
        {
        m_indices.resize(numVerts - 1);
        std::fill(m_indices.begin(), m_indices.end(), m_uniform);
        m_indices.push_back(materialIndex);
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
    static_assert(0x2D == offsetof(VertexKey, m_materialIndex), "unexpected offset");
    static_assert(0x2E == offsetof(VertexKey, m_normalValid), "unexpected offset");

    // TFS#874608: This used to add 2 bytes to `offset` if m_normalValid=false to avoid unnecessarily comparing normals.
    // But invalid normals are initialized to 0 so they will not affect memcmp(), and optimizer was producing incorrect comparisons here.
    static constexpr size_t offset = offsetof(VertexKey, m_normalAndPos);
    static constexpr size_t size = offsetof(VertexKey, m_normalValid) - offset;
    auto cmp = memcmp(reinterpret_cast<uint8_t const*>(this)+offset, reinterpret_cast<uint8_t const*>(&rhs)+offset, size);
    if (0 != cmp)
        return cmp < 0;

    if (m_paramValid)
        {
        constexpr double s_paramTolerance  = .0001;
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
    // Prefer to avoid adding vertices originating from degenerate triangles before we get here...
    BeAssert(!triangle.IsDegenerate());

    TriangleKey key(triangle);

    if (m_triangleSet.insert(key).second)
        m_mesh->AddTriangle(triangle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/017
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilder::AddFromPolyfaceVisitor(PolyfaceVisitorR visitor, TextureMappingCR mappedTexture, DgnDbR dgnDb, FeatureCR feature, bool includeParams, uint32_t fillColor, bool requireNormals, uint8_t materialIndex)
    {
    if (visitor.Point().size() < 3)
        return;

    auto const&     points = visitor.Point();
    bool const*     visitorVisibility = visitor.GetVisibleCP();
    size_t          nTriangles = points.size() - 2;

    if (requireNormals && visitor.Normal().size() < points.size())
        return; // TFS#790263: Degenerate triangle - no normals.

    // The face represented by this visitor should be convex (we request that in facet options) - so we do a simple fan triangulation.
    for (size_t iTriangle =0; iTriangle < nTriangles; iTriangle++)
        {
        Triangle            newTriangle(!visitor.GetTwoSided());
        bvector<DPoint2d>   params = visitor.Param();
        bool                visibility[3];

        visibility[0] = (0 == iTriangle) ? visitorVisibility[0] : false;
        visibility[1] = visitorVisibility[iTriangle+1];
        visibility[2] = (iTriangle == nTriangles-1) ? visitorVisibility[iTriangle+2] : false;

        BeAssert(!includeParams || !params.empty());
        bool haveParams = includeParams && !params.empty();
        BeAssert(!haveParams || mappedTexture.IsValid());
        newTriangle.SetEdgeFlags(visibility);
        if (haveParams && mappedTexture.IsValid())
            {
            auto const&         textureMapParams = mappedTexture.GetParams();
            bvector<DPoint2d>   computedParams;

            BeAssert (m_mesh->Verts().empty() || !m_mesh->Params().empty());
            if (SUCCESS == textureMapParams.ComputeUVParams (computedParams, visitor))
                params = computedParams;
            else
                BeAssert(false && "ComputeUVParams() failed");
            }

        auto computeIndex   = [=](size_t i) { return (0 == i) ? 0 : iTriangle + i; };
        auto makeVertexKey  = [&](size_t index)
            {
            auto normal = requireNormals ? &visitor.Normal()[index] : nullptr;
            auto param = haveParams ? &params[index] : nullptr;
            return VertexKey(points[index], feature, fillColor, m_mesh->Verts().GetParams(), materialIndex, normal, param);
            };

        size_t indices[3]       = { computeIndex(0), computeIndex(1), computeIndex(2) };
        VertexKey vertices[3]   = { makeVertexKey(indices[0]), makeVertexKey(indices[1]), makeVertexKey(indices[2]) };

        // Previously we would add all 3 vertices to our map, then detect degenerate triangles in AddTriangle().
        // This led to unused vertex data, and caused mismatch in # of vertices when recreating the MeshBuilder from the data in the tile cache.
        // Detect beforehand instead.
        if (vertices[0].GetPosition() == vertices[1].GetPosition() || vertices[0].GetPosition() == vertices[2].GetPosition() || vertices[1].GetPosition() == vertices[2].GetPosition())
            continue;

        for (size_t i = 0; i < 3; i++)
            {
            size_t index = indices[i];
            VertexKey const& vertex = vertices[i];
            if (visitor.GetAuxDataCP().IsValid())
                {
                // No deduplication with auxData (for now...)
                newTriangle[i] = m_mesh->AddVertex(vertex.GetPosition(), vertex.GetNormal(), vertex.GetParam(), vertex.GetFillColor(), vertex.GetFeature(), vertex.GetMaterialIndex());
                m_mesh->AddAuxData(*visitor.GetAuxDataCP(), index);
                }
            else
                {
                newTriangle[i] = AddVertex(vertex);
                }

            if (m_currentPolyface.IsValid())
                m_currentPolyface->m_vertexIndexMap.Insert(newTriangle[i], visitor.ClientPointIndex()[index]);
            }

        AddTriangle(newTriangle);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilder::AddPolyline (bvector<DPoint3d>const& points, FeatureCR feature, uint32_t fillColor, double startDistance)
    {
    MeshPolyline    newPolyline(startDistance);

    for (auto& point : points)
        {
        VertexKey vertex(point, feature, fillColor, m_mesh->Verts().GetParams(), 0);
        newPolyline.GetIndices().push_back (AddVertex(vertex));
        }

    m_mesh->AddPolyline (newPolyline);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilder::AddPolyline(bvector<QPoint3d> const& points, FeatureCR feature, uint32_t fillColor, double startDistance)
    {
    MeshPolyline newPolyline(startDistance);
    for (auto const& point : points)
        {
        VertexKey key(point, feature, fillColor, nullptr, nullptr, 0);
        newPolyline.GetIndices().push_back(AddVertex(key));
        }

    m_mesh->AddPolyline(newPolyline);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/17
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilder::AddPointString(bvector<DPoint3d> const& points, FeatureCR feature, uint32_t fillColor, double startDistance)
    {
    // Assume no duplicate points in point strings (or, too few to matter).
    // Why? Because drawGridDots() potentially sends us tens of thousands of points (up to 83000 on my large monitor in top view), and wants to do so every frame.
    // The resultant map lookups/inserts/rebalancing kill performance in non-optimized builds.
    // NB: startDistance currently unused - Ray claims they will be used in future for non-cosmetic line styles? If not let's jettison them...
    MeshPolyline polyline(startDistance);
    for (auto const& point : points)
        {
        QPoint3d qpoint(point, m_mesh->Verts().GetParams());
        auto index = static_cast<uint32_t>(m_mesh->Verts().size());
        m_mesh->AddVertex(qpoint, nullptr, nullptr, fillColor, feature, 0);
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

    // Ignore single-point line strings...
    if (PrimitiveType::Polyline != GetType() || polyline.GetIndices().size() >= 2)
        m_polylines.push_back(polyline);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
uint8_t Mesh::GetMaterialIndex(MaterialP material) const
    {
    MaterialIndex index;
    if (m_materialAtlas.IsValid())
        {
        index = m_materialAtlas->Find(material);
        BeAssert(index.IsValid());
        }

    if (index.IsValid())
        return index.Unwrap();

    return 0;
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
        m_mesh->AddVertex(vertex.GetPosition(), vertex.GetNormal(), vertex.GetParam(), vertex.GetFillColor(), vertex.GetFeature(), vertex.GetMaterialIndex());

    return insertPair.first->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool PartGeom::Key::IsLessThan(PartGeom::KeyCR rhs, bool ignoreTolerance) const
    {
    TEST_LESS_THAN(m_partId.GetValueUnchecked());
    if (!ignoreTolerance && m_tolerance != rhs.m_tolerance)
        {
        // If two instances of a part have different scales, only treat them as incompatible for instancing if the
        // ratio of their scales meets or exceeds this threshold.
        static constexpr double s_maxPartToleranceRatio = 2.0;
        BeAssert(m_tolerance > 0.0 || rhs.m_tolerance > 0.0);
        double ratio = std::min(m_tolerance, rhs.m_tolerance) / std::max(m_tolerance, rhs.m_tolerance);
        if (ratio >= s_maxPartToleranceRatio)
            return m_tolerance < rhs.m_tolerance;
        }

    // NB: ComparePurpose::Instance because we can't instance the same mesh if each instance wants different UV params generated.
    return m_displayParams->IsLessThan(*rhs.m_displayParams, ComparePurpose::Instance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
PartGeom::PartGeom(PartGeom::KeyCR key, DRange3dCR range, GeometryList const& geometries, bool hasSymbologyChanges, DPoint3dCR translation)
    : SharedGeom(range, geometries, translation), m_key(key), m_hasSymbologyChanges(hasSymbologyChanges)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/19
+---------------+---------------+---------------+---------------+---------------+------*/
SharedGeom::SharedGeom(DRange3dCR range, GeometryList const& geometries, DPoint3dCR translation) : m_translation(translation), m_range(range), m_geometries(geometries)
    {
    for (auto const& geom : geometries)
        {
        if (!geom->IsInstanceable())
            {
            m_instanceable = false;
            break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/19
+---------------+---------------+---------------+---------------+---------------+------*/
SharedGeom::SharedGeom(DRange3dCR range, GeometryR geom, DPoint3dCR translation) : m_translation(translation), m_range(range), m_geometries(geom)
    {
    // We know it's a solid primitive, which is always instanceable.
    BeAssert(geom.IsInstanceable());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool SharedGeom::IsCurved() const
    {
    // ###TODO: Take into account polyfaces which should be decimated...
    return m_geometries.ContainsCurves();
    }

/*---------------------------------------------------------------------------------**//**
* Adjust tolerance to account for transform scale.
* @bsimethod                                                    Paul.Connelly   10/18
+---------------+---------------+---------------+---------------+---------------+------*/
double PartGeom::ComputeTolerance(TransformCR transform, double baseTolerance)
    {
    DPoint3d tempPt;
    Transform invTrans;

    tempPt.Init(baseTolerance, 0.0, 0.0);
    invTrans.InverseOf(transform);

    invTrans.MultiplyMatrixOnly(tempPt);
    return tempPt.Magnitude();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/18
+---------------+---------------+---------------+---------------+---------------+------*/
double PartGeom::ComputeTolerance(GeometryCP instance, double tolerance)
    {
    return nullptr != instance ? ComputeTolerance(instance->GetTransform(), tolerance) : tolerance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/19
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayParamsCR SharedGeom::GetInstanceDisplayParams(GeometryCP instance, DisplayParamsCR geomParams) const
    {
    return nullptr != instance ? instance->GetDisplayParams() : GetDisplayParams();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/19
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayParamsCR PartGeom::GetInstanceDisplayParams(GeometryCP instance, DisplayParamsCR geomParams) const
    {
    return HasSymbologyChanges() ? geomParams : T_Super::GetInstanceDisplayParams(instance, geomParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/19
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayParamsCPtr SharedGeom::CloneDisplayParamsForInstance(DisplayParamsCR baseParams, DisplayParamsCR instanceParams) const
    {
    auto ovrs = GetSymbologyOverrides();

    // NB: We must test IsEqualTo() in order to catch differences caused by face-attached materials.
    if (SymbologyOverrides::None == ovrs && baseParams.IsEqualTo(instanceParams, ComparePurpose::Merge))
        return &instanceParams;

    bool ovrRgb = SymbologyOverrides::None != (ovrs & SymbologyOverrides::Rgb);
    ColorDef fillColor = ovrRgb ? instanceParams.GetFillColorDef() : baseParams.GetFillColorDef();
    ColorDef lineColor = ovrRgb ? instanceParams.GetLineColorDef() : baseParams.GetLineColorDef();

    bool ovrAlpha = SymbologyOverrides::None != (ovrs & SymbologyOverrides::Alpha);
    fillColor.SetAlpha(ovrAlpha ? instanceParams.GetFillColorDef().GetAlpha() : baseParams.GetFillColorDef().GetAlpha());
    lineColor.SetAlpha(ovrAlpha ? instanceParams.GetLineColorDef().GetAlpha() : baseParams.GetLineColorDef().GetAlpha());

    uint32_t width = (SymbologyOverrides::None != (ovrs & SymbologyOverrides::LineWidth)) ? instanceParams.GetLineWidth() : baseParams.GetLineWidth();
    LinePixels pixels = (SymbologyOverrides::None != (ovrs & SymbologyOverrides::LinePixels)) ? instanceParams.GetLinePixels() : baseParams.GetLinePixels();

    return baseParams.Clone(fillColor, lineColor, width, pixels);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/19
+---------------+---------------+---------------+---------------+---------------+------*/
SolidPrimitiveGeom::Key::Key(ISolidPrimitiveR primitive, DisplayParamsCR displayParams, DRange3dCR range)
    : m_primitive(&primitive), m_displayParams(&displayParams)
    {
    double const* in = &range.low.x;
    for (size_t i = 0; i < 6; i++)
        m_range[i] = static_cast<int64_t>(in[i] / GetCompareTolerance());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool SolidPrimitiveGeom::Key::operator<(Key const& rhs) const
    {
    for (size_t i = 0; i < 6; i++)
        if (m_range[i] != rhs.m_range[i])
            return m_range[i] < rhs.m_range[i];

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool SolidPrimitiveGeom::Key::IsEquivalent(Key const& rhs) const
    {
    BeAssert(!(*this < rhs) && !(rhs < *this));

    return m_displayParams->IsEqualTo(*rhs.m_displayParams, ComparePurpose::Instance) && m_primitive->IsSameStructureAndGeometry(*rhs.m_primitive, GetCompareTolerance());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/19
+---------------+---------------+---------------+---------------+---------------+------*/
SolidPrimitiveGeomPtr SolidPrimitiveGeom::Create(KeyCR key, DRange3dCR range, DgnDbR db)
    {
    // ###TODO_INSTANCING: So far it appears solid primitives are sensibly defined at the origin.
    // If we discover cases in which they are not, we will need to add a translation.
    DPoint3d translation = DPoint3d::FromZero();
    bool isCurved = key.m_primitive->HasCurvedFaceOrEdge();
    IGeometryPtr igeom = IGeometry::Create(key.m_primitive);
    auto geom = Geometry::Create(*igeom, Transform::FromIdentity(), range, DgnElementId(), *key.m_displayParams, isCurved, db, false);
    return new SolidPrimitiveGeom(key, range, *geom, translation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceList SharedGeom::GetPolyfaces(double chordTolerance, NormalMode normalMode, GeometryCP instance, ViewContextR context)
    {
    chordTolerance = GetTolerance(chordTolerance);
    PolyfaceList polyfaces;
    for (auto& geometry : m_geometries)
        {
        PolyfaceList thisPolyfaces = geometry->GetPolyfaces (chordTolerance, normalMode, context);

        for (auto const& thisPolyface : thisPolyfaces)
            {
            // ###TODO_INSTANCING: Do we expect to encounter multiple geometries with differing symbologies?
            // Polyface polyface(*thisPolyface.m_displayParams, *thisPolyface.m_polyface->Clone());
            // auto const& displayParams = nullptr != instance ? instance->GetDisplayParams() : *m_key.m_displayParams;
            auto const& displayParams = GetInstanceDisplayParams(instance, *thisPolyface.m_displayParams);
            Polyface polyface(displayParams, *thisPolyface.m_polyface->Clone());
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
void SharedGeom::SetInCache(bool inCache)
    {
    for (auto& geometry : m_geometries)
        geometry->SetInCache(inCache);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/17
+---------------+---------------+---------------+---------------+---------------+------*/
StrokesList SharedGeom::GetStrokes(double chordTolerance, GeometryCP instance, ViewContextR context)
    {
    chordTolerance = GetTolerance(chordTolerance);
    StrokesList strokes;

    for (auto& geometry : m_geometries)
        {
        StrokesList   thisStrokes = geometry->GetStrokes(chordTolerance, context);
        for (auto& thisStroke : thisStrokes)
            strokes.emplace_back(std::move(thisStroke));
        }

    if (nullptr != instance)
        {
        for (auto& stroke : strokes)
            {
            // stroke.m_displayParams = &instance->GetDisplayParams();
            stroke.m_displayParams = &GetInstanceDisplayParams(instance, *stroke.m_displayParams);
            stroke.Transform(instance->GetTransform());
            }
        }

    return strokes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/19
+---------------+---------------+---------------+---------------+---------------+------*/
void SharedGeom::AddInstance(TransformCR tf, DisplayParamsCR params, DgnElementId elemId)
    {
    m_instances.push_back(Instance(tf, params, elemId));
    auto const& myParams = GetDisplayParams();
    auto myColor = myParams.GetFillColorDef(),
         dpColor = params.GetFillColorDef();

    if (myColor.GetValueNoAlpha() != dpColor.GetValueNoAlpha())
        m_symbology |= SymbologyOverrides::Rgb;

    if (myColor.GetAlpha() != dpColor.GetAlpha())
        m_symbology |= SymbologyOverrides::Alpha;

    if (myParams.GetLineWidth() != params.GetLineWidth())
        m_symbology |= SymbologyOverrides::LineWidth;

    if (myParams.GetLinePixels() != params.GetLinePixels())
        m_symbology |= SymbologyOverrides::LinePixels;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool SharedGeom::IsWorthInstancing() const
    {
    // Unless we've only got a single instance, we need to compare the relative impact of batching vs instancing. We can't do that here.
    return GetInstanceCount() > 1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/16
+---------------+---------------+---------------+---------------+---------------+------*/
Geometry::Geometry(TransformCR tf, DRange3dCR range, DgnElementId entityId, DisplayParamsCR params, bool isCurved, DgnDbR db)
    : m_params(&params), m_transform(tf), m_tileRange(range), m_entityId(entityId), m_isCurved(isCurved), m_hasTexture(params.IsTextured())
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool Geometry::WantPolylineEdges(uint32_t lineWeight, bool is3d)
    {
    // Polyline edges are much more expensive than simple edges, especially when they include extra triangles at joints to produce rounded corners.
    // Simple segment edges are cheaper, but do not have rounded corners and do not exhibit continuous line patterns across segments.
    // In 3d, only generate polyline edges if the edges are sufficiently wide.
    constexpr uint32_t widthThreshold3d = 5;
    return !is3d || lineWeight >= widthThreshold3d;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IFacetOptionsPtr Geometry::CreateFacetOptions(double chordTolerance, bool wantEdgeChains)
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
    opts->SetEdgeChainsRequired(wantEdgeChains);

    // Avoid Parasolid concurrency bottlenecks.
    opts->SetIgnoreHiddenBRepEntities(true);
    opts->SetOmitBRepEdgeChainIds(true);

    return opts;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/16
+---------------+---------------+---------------+---------------+---------------+------*/
IFacetOptionsPtr SingularGeometry::CreateFacetOptions(double chordTolerance, NormalMode normalMode, ViewContextCR context) const
    {
    bool wantEdgeChains = Geometry::WantPolylineEdges(GetDisplayParams().GetLineWidth(), context.Is3dView());
    auto facetOptions = Geometry::CreateFacetOptions(chordTolerance / GetTransform().ColumnXMagnitude(), wantEdgeChains);
    bool normalsRequired = false;

    switch (normalMode)
        {
        case NormalMode::Always:
            normalsRequired = true;
            break;
        case NormalMode::CurvedSurfacesOnly:
            normalsRequired = IsCurved();
            break;
        }

    facetOptions->SetNormalsRequired(normalsRequired);
    facetOptions->SetParamsRequired(HasTexture());

    if (DisplayParams::RegionEdgeType::Default != GetDisplayParams().GetRegionEdgeType())
        facetOptions->SetEdgeChainsRequired(false);

    return facetOptions;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mark.Schlosser  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceList Geometry::GetPolyfaces(double chordTolerance, NormalMode normalMode, ViewContextR context)
    {
    auto polyfaces = _GetPolyfaces(chordTolerance, normalMode, context);
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
StrokesList Geometry::GetStrokes (double chordTolerance, ViewContextR context)
    {
    auto strokes = _GetStrokes(chordTolerance, context);
    if (!m_clip.IsValid() || strokes.empty())
        return strokes;

    GeometryClipper geomClipper(m_clip.get());

    StrokesList clippedStrokes;
    for (auto& stroke : strokes)
        geomClipper.DoClipStrokes(clippedStrokes, std::move(stroke));

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

// #define REGENERATE_NORMALS
#if defined(REGENERATE_NORMALS)
    if (0 != geom.GetNormalCount() && facetOptions.GetNormalsRequired())
        geom.ClearNormals(true);
#endif

    PolyfaceHeaderPtr polyface(&geom);
    size_t maxPerFace;
    if (geom.GetNumFacet(maxPerFace) > 0 && (int)maxPerFace > facetOptions.GetMaxPerFace())
        {
        // Make sure we don't generate edge chains - decimation doesn't handle them correctly.
        bool wantedEdgeChains = facetOptions.GetEdgeChainsRequired();
        facetOptions.SetEdgeChainsRequired(false);
        IPolyfaceConstructionPtr builder = PolyfaceConstruction::New(facetOptions);
        builder->AddPolyface(geom);
        polyface = &builder->GetClientMeshR();
        facetOptions.SetEdgeChainsRequired(wantedEdgeChains);
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

        if (maxPerFace > 3 && !geom.HasConvexFacets() && facetOptions.GetConvexFacetsRequired())
            polyface->Triangulate(3);

        // Not necessary to add edges chains -- edges will be generated from visibility flags later
        // and decimation will not handle edge chains correctly.
        // Edge chains are never serialized to flat buffer so we know they won't exist.
        }

    return polyface;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveGeometry::AddNormals(PolyfaceHeaderR polyface, IFacetOptionsR facetOptions)
    {
    static double   s_defaultCreaseRadians = Angle::DegreesToRadians(45.0);
    static double   s_sharedNormalSizeRatio = 5.0;      // Omit expensive shared normal if below this ratio of tolerance.

    polyface.BuildNormalsFast(s_defaultCreaseRadians, s_sharedNormalSizeRatio * facetOptions.GetChordTolerance());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PrimitiveGeometry::AddParams(PolyfaceHeaderR polyface, IFacetOptionsR facetOptions)
    {
    polyface.BuildPerFaceParameters(LOCAL_COORDINATE_SCALE_01RangeBothAxes);
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

        return PolyfaceList(1, Polyface(GetDisplayParams(), *polyface, true, false, nullptr, facetOptions.GetChordTolerance()));
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
        bool wantEdges = curveVector.IsNull() || DisplayParams::RegionEdgeType::Default == params.GetRegionEdgeType();
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

    if (!curveVector->IsAnyRegionType() || GetDisplayParams().WantRegionOutline())
        {
        strokePoints.clear();
        bool canDecimateSomething = collectCurveStrokes(strokePoints, *curveVector, facetOptions, GetTransform());

        if (!strokePoints.empty())
            {
            bool isPlanar = curveVector->IsAnyRegionType();
            BeAssert(isPlanar == GetDisplayParams().WantRegionOutline());
            auto decimationTolerance = canDecimateSomething ? facetOptions.GetChordTolerance() : 0.0;
            tileStrokes.emplace_back(Strokes(GetDisplayParams(), std::move(strokePoints), m_disjoint, isPlanar, decimationTolerance));
            }
        }

    return tileStrokes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SolidKernelGeometry::SolidKernelGeometry(IBRepEntityR solid, TransformCR tf, DRange3dCR range, DgnElementId elemId, DisplayParamsCR params, DgnDbR db)
        : SingularGeometry(tf, range, elemId, params, solid.HasCurvedFaceOrEdge(), db), m_entity(&solid)
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
    pFacetOptions->SetBRepConcurrentFacetting(false); // Concurrent facetting is actually slower...
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
                FaceAttachmentUtil::ToGeometryParams(params[i], faceParams, baseParams);

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
GeometryPtr Geometry::Create(SharedGeomR geom, TransformCR transform, DRange3dCR range, DgnElementId entityId, DisplayParamsCR params, DgnDbR db)
    {
    return InstancedGeometry::Create(geom, transform, range, entityId, params, db);
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

    // NB: If we're stroking a styled curve vector, we have set m_addingCurved based on whether the input curve vector was curved - the
    // stroked components may not be.
    bool isCurved = m_addingCurved || curves.ContainsNonLinearPrimitive();
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
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
MeshBuilderSet GeometryAccumulator::ToMeshBuilders(GeometryOptionsCR options, double tolerance, FeatureTableP featureTable, ViewContextR context) const
    {
    DRange3d range = m_geometries.ComputeRange();
    bool is2d = !range.IsNull() && range.IsAlmostZeroZ();

    MeshBuilderSet builders(tolerance, featureTable, range, is2d);

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

            MeshBuilderKey key(*displayParams, nullptr != polyface->GetNormalIndexCP(), Mesh::PrimitiveType::Mesh, tilePolyface.m_isPlanar);
            MeshBuilderR meshBuilder = builders[key];

            auto edgeOptions = (options.WantEdges() && tilePolyface.m_displayEdges) ? MeshEdgeCreationOptions::DefaultEdges : MeshEdgeCreationOptions::NoEdges;
            meshBuilder.BeginPolyface(*polyface, edgeOptions);

            uint32_t fillColor = displayParams->GetFillColor();
            uint8_t materialIndex = meshBuilder.GetMaterialIndex(displayParams->GetSurfaceMaterial().GetMaterial());
            auto feature = geom->GetFeature();
            auto hasNormals = nullptr != polyface->GetNormalCP();
            auto const& texMap = displayParams->GetSurfaceMaterial().GetTextureMapping();
            for (PolyfaceVisitorPtr visitor = PolyfaceVisitor::Attach(*polyface); visitor->AdvanceToNextFace(); /**/)
                meshBuilder.AddFromPolyfaceVisitor(*visitor, texMap, GetDgnDb(), feature, hasTexture, fillColor, hasNormals, materialIndex);

            meshBuilder.EndPolyface();
            }

        if (!options.WantSurfacesOnly())
            {
            auto tileStrokesArray = geom->GetStrokes(tolerance, context);
            for (auto& tileStrokes : tileStrokesArray)
                {
                DisplayParamsCPtr displayParams = tileStrokes.m_displayParams;
                auto type = tileStrokes.m_disjoint ? Mesh::PrimitiveType::Point : Mesh::PrimitiveType::Polyline;
                MeshBuilderKey key(*displayParams, false, type, tileStrokes.m_isPlanar);
                MeshBuilderR builder = builders[key];
                uint32_t fillColor = displayParams->GetLineColor();
                for (auto& strokePoints : tileStrokes.m_strokes)
                    {
                    if (tileStrokes.m_disjoint)
                        builder.AddPointString(strokePoints.m_points, geom->GetFeature(), fillColor, strokePoints.m_startDistance);
                    else
                        builder.AddPolyline(strokePoints.m_points, geom->GetFeature(), fillColor, strokePoints.m_startDistance);
                    }
                }
            }
        }

    return builders;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/17
+---------------+---------------+---------------+---------------+---------------+------*/
MeshList GeometryAccumulator::ToMeshes(GeometryOptionsCR options, double tolerance, ViewContextR context, FeatureTableP featureTable) const
    {
    MeshList meshes;
    if (m_geometries.empty())
        return meshes;

    auto builders = ToMeshBuilders(options, tolerance, featureTable, context);
    builders.GetMeshes(meshes);
    return meshes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryAccumulator::SaveToGraphicList(bvector<GraphicPtr>& graphics, GeometryOptionsCR options, double tolerance, ViewContextR context, bool isWorldCoords) const
    {
    MeshList            meshes = ToMeshes(options, tolerance, context);
    MeshGraphicArgs     args;

    // Not really a point in translating for view coords, though shouldn't actually matter except #stupidviewcliptools assign nonsense Z to view overlays. (TFS#930451)
    if (!isWorldCoords)
        {
        for (auto const& mesh : meshes)
            {
            auto graphic = mesh->GetGraphics(args, GetSystem(), GetDgnDb());
            if (graphic.IsValid())
                graphics.push_back(graphic);
            }
        }
    else
        {
        // All of the meshes are quantized to the same range.
        // If that range is small relative to the distance from the origin, quantization errors can produce display artifacts.
        // Remove the translation from the quantization parameters and add it to the transform.
        GraphicBranch branch;
        DPoint3d qorigin;
        QPoint3d::Params qparams;
        for (auto const& mesh : meshes)
            {
            auto& verts = mesh->VertsR();
            if (branch.m_entries.empty())
                {
                qparams = verts.GetParams();
                qorigin = qparams.origin;
                qparams.origin.Zero();
                }
            else
                {
                BeAssert(verts.GetParams().origin.AlmostEqual(qorigin));
                BeAssert(verts.GetParams().scale.AlmostEqual(qparams.scale));
                }

            verts.SetParams(qparams);

            auto graphic = mesh->GetGraphics(args, GetSystem(), GetDgnDb());
            if (graphic.IsValid())
                branch.Add(*graphic);
            }

        if (!branch.m_entries.empty())
            graphics.push_back(GetSystem()._CreateBranch(std::move(branch), GetDgnDb(), Transform::From(qorigin), nullptr));
        }
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
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool ColorTable::FindByIndex(ColorDef& color, uint16_t index) const
    {
    for (auto const& kvp : *this)
        {
        if (kvp.second == index)
            {
            color = ColorDef(kvp.first);
            return true;
            }
        }

    return false;
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
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
IFacetOptionsPtr GlyphCache::CreateFacetOptions(double chordTolerance)
    {
    auto opts = Geometry::CreateFacetOptions(chordTolerance, false);
    opts->SetParamsRequired(false);
    opts->SetNormalsRequired(false);
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

    DRange2d textRange2d = textString.GetRange();
    DRange3d textRange = DRange3d::From(textRange2d.low.x, textRange2d.low.y, 0.0, textRange2d.high.x, textRange2d.high.y, 0.0);
    textTransform.Multiply(textRange, textRange);

    auto facetOptions = CreateFacetOptions(chordTolerance);

    DPoint3d textRangeCenter = DPoint3d::FromInterpolate(textRange.low, 0.5, textRange.high);
    double pixelSize = context.GetPixelSizeAtPoint(&textRangeCenter);
    double meterSize = 0.0 != pixelSize ? 1.0 / pixelSize : 0.0;

    // NB: Need scaled 2d range - ignore 3d transform...
    double minAxis = std::min(textRange2d.high.x - textRange2d.low.x, textRange2d.high.y - textRange2d.low.y) * geom.GetTransform().ColumnXMagnitude();
    double textSize = minAxis * meterSize;
    constexpr double s_minToleranceRatioMultiplier = 2.0; // from TileTree.cpp; used to multiply the s_minToleranceRatio
    constexpr double s_texSizeThreshold = 64.0;
    bool doTextAsRasterIfPossible = textSize / s_minToleranceRatioMultiplier <= s_texSizeThreshold;

    IPolyfaceConstructionPtr glyphBoxBuilder;
    bool doGlyphBoxes = context.WantGlyphBoxes(textSize);
    bvector<DPoint3d> glyphBox(5);
    if (doGlyphBoxes)
        {
        if (nullptr == polyfaces)
            return;

        glyphBoxBuilder = IPolyfaceConstruction::Create(*facetOptions);
        }

    for (size_t i = 0; i < textString.GetNumGlyphs(); i++)
        {
        auto textGlyph = glyphs[i];

        Glyph* glyph = nullptr != textGlyph ? FindOrInsert(*textGlyph, textString.GetStyle().GetFont()) : nullptr;
        if (nullptr == glyph || !glyph->IsValid())
            continue;

        if (!doGlyphBoxes)
            {
            if ((glyph->IsStrokes() && nullptr == strokes) || (!glyph->IsStrokes() && nullptr == polyfaces))
                continue;
            }

        Transform glyphTransform = Transform::FromProduct(Transform::From(glyphOrigins[i]), rot);
        glyphTransform = Transform::FromProduct(textTransform, glyphTransform);

        if (doGlyphBoxes)
            {
            DRange2d glyphRange = textGlyph->GetExactRange();

            glyphBox[0].x = glyphBox[3].x = glyphBox[4].x = glyphRange.low.x;
            glyphBox[1].x = glyphBox[2].x = glyphRange.high.x;

            glyphBox[0].y = glyphBox[1].y = glyphBox[4].y = glyphRange.low.y;
            glyphBox[2].y = glyphBox[3].y = glyphRange.high.y;

            for (auto& pt : glyphBox)
                pt.z = 0.0;

            glyphTransform.Multiply(glyphBox, glyphBox);
            glyphBoxBuilder->AddTriangulation(glyphBox);
            }
        else if (glyph->IsStrokes())
            {
            Strokes::PointLists points = glyph->GetStrokes(*facetOptions);
            if (!points.empty())
                {
                for (auto& loop : points)
                    glyphTransform.Multiply(loop.m_points, loop.m_points);

                strokes->emplace_back(Strokes(geom.GetDisplayParams(), std::move(points), false, true, 0.0));
                }
            }
        else
            {
            DisplayParamsCPtr displayParams(&geom.GetDisplayParams());
            PolyfaceHeaderPtr polyface;

            // NB: Raster text only supported for truetype fonts; and even then it's possible for raster generation to fail to produce a valid image.
            // Only produce raster text if the image is valid - otherwise the glyph renders as an untextured box.
            Image* rasterImage = nullptr;
            auto glyphRaster = doTextAsRasterIfPossible ? glyph->GetRaster() : nullptr;
            if (nullptr != glyphRaster)
                {
                rasterImage = &glyphRaster->m_image; // save the raster image to put into texture atlas
                Glyph::RasterPolyface raster = glyph->GetRasterPolyface(*facetOptions, *context.GetRenderSystem(), context.GetDgnDb());
                polyface = raster.m_polyface->Clone();
                }
            else
                {
                polyface = glyph->GetPolyface(*facetOptions);
                }

            if (polyface.IsValid() && polyface->HasFacets())
                {
                polyface->Transform(glyphTransform);
                polyfaces->push_back(Polyface(*displayParams, *polyface, false, true, rasterImage));
                }
            }
        }

    if (doGlyphBoxes)
        {
        auto polyface = glyphBoxBuilder->GetClientMeshPtr();
        if (polyface.IsValid() && polyface->HasFacets())
            polyfaces->push_back(Polyface(geom.GetDisplayParams(), *polyface, false, true));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
GlyphCache::Glyph* GlyphCache::FindOrInsert(DgnGlyphCR glyph, DgnFontCR font)
    {
    auto iter = m_map.find(&glyph);
    if (m_map.end() == iter)
        //iter = m_map.Insert(&glyph, Glyph(glyph, font)).first;
        iter = m_map.insert(std::make_pair(&glyph, Glyph(glyph, font))).first;

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
* @bsimethod                                                    Mark.Schlosser  02/18
+---------------+---------------+---------------+---------------+---------------+------*/
void GlyphCache::Glyph::Raster::DebugPrintImage() const
    {
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/18
+---------------+---------------+---------------+---------------+---------------+------*/
void GlyphCache::Glyph::Raster::DebugSaveImage() const
    {
//#define DEBUG_GLYPH_IMAGE_DIR L"d:\\cr\\tmp\\glyphs"
#if defined(DEBUG_GLYPH_IMAGE_DIR)
    if (!m_image.IsValid())
        return;

    // Image is all-white; only alpha channel differs...convert back to greyscale
    Image image = m_image;
    auto& bytes = image.GetByteStreamR();
    for (size_t i = 0; i < bytes.size(); i += 4)
        {
        uint8_t& alpha = bytes[i+3];
        bytes[i] = bytes[i+1] = bytes[i+2] = alpha;
        alpha = 0xff;
        }

    BeFileName::CreateNewDirectory(DEBUG_GLYPH_IMAGE_DIR);
    BeFileName filename(DEBUG_GLYPH_IMAGE_DIR);

    WPrintfString pngName(L"%hs_%u_%u.png", m_name.c_str(), image.GetWidth(), image.GetHeight());
    filename.AppendToPath(pngName.c_str());

    BeFile file;
    if (BeFileStatus::Success != file.Create(filename.c_str(), true))
        return;

    ImageSource src(image, ImageSource::Format::Png, 100, Image::BottomUp::Yes);
    file.Write(nullptr, src.GetByteStream().GetData(), src.GetByteStream().GetSize());
#endif
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

    m_texture = const_cast<TextureP>(mesh.GetDisplayParams().GetSurfaceMaterial().GetTextureMapping().GetTexture()); // ###TODO: constness...

    // NB: The atlas may contain unused materials. Only write it out if at least 2 are used.
    MaterialAtlasP atlas = mesh.GetMaterialAtlas();
    if (nullptr != atlas && mesh.HasMultipleMaterials())
        {
        m_materialIndices = mesh.GetMaterialIndices();
        m_material.m_atlas = atlas;
        m_material.m_indices = &m_materialIndices.front();
        }
    else
        {
        m_material.m_material = mesh.GetDisplayParams().GetSurfaceMaterial().GetMaterial();
        }

    m_fillFlags = mesh.GetDisplayParams().GetFillFlags();
    m_isPlanar = mesh.IsPlanar();
    m_is2d = mesh.Is2d();

    mesh.GetColorTable().ToColorIndex(m_colors, m_colorTable, mesh.Colors());
    mesh.ToFeatureIndex(m_features);

    m_edges.m_width = mesh.GetDisplayParams().GetLineWidth();
    m_edges.m_linePixels = mesh.GetDisplayParams().GetLinePixels();

    MeshEdgesCP meshEdges = mesh.GetEdges().get();
    if (nullptr == meshEdges)
        return true;

    m_edges.m_edges.Init(*meshEdges);
    m_edges.m_silhouettes.Init(*meshEdges);

    m_polylineEdges.reserve(meshEdges->m_polylines.size());
    for (auto const& meshPolyline : meshEdges->m_polylines)
        {
        PolylineEdgeArgs::Polyline polyline;
        if (polyline.Init(meshPolyline))
            m_polylineEdges.push_back(polyline);
        }

    m_edges.m_polylines.Init(m_polylineEdges);
    m_auxChannels = mesh.GetAuxChannels();

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
    m_is2d = false;

    m_colors.Reset();
    m_colorTable.clear();
    m_features.Reset();

    m_polylineEdges.clear();
    m_edges.Clear();

    m_materialIndices.clear();
    m_material.m_atlas = nullptr;
    m_material.m_indices = nullptr;
    m_material.m_material = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/17
+---------------+---------------+---------------+---------------+---------------+------*/
void PolylineArgs::Reset()
    {
    m_flags = PolylineFlags();
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

    auto const& displayParams = mesh.GetDisplayParams();
    m_width = displayParams.GetLineWidth();
    m_linePixels = displayParams.GetLinePixels();

    if (mesh.Is2d())
        m_flags.SetIs2d();

    if (mesh.IsPlanar())
        m_flags.SetIsPlanar();

    if (Mesh::PrimitiveType::Point == mesh.GetType())
        m_flags.SetIsDisjoint();

    if (displayParams.WantRegionOutline())
        {
        // This polyline is behaving as the edges of a region surface.
        if (nullptr == displayParams.GetSurfaceMaterial().GetGradient() || displayParams.GetSurfaceMaterial().GetGradient()->GetIsOutlined())
            m_flags.SetIsNormalEdge();
        else
            m_flags.SetIsOutlineEdge(); // edges only displayed if fill undisplayed...
        }

    m_polylines.reserve(mesh.Polylines().size());

    for (auto const& polyline : mesh.Polylines())
        {
        IndexedPolylineArgs::Polyline indexedPolyline;
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
    double zDepth = depthFromDisplayPriority(priority);
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
    double zDepth = depthFromDisplayPriority(priority);
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
    double zDepth = depthFromDisplayPriority(priority);
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
    double zDepth = depthFromDisplayPriority(priority);
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
    double zDepth = depthFromDisplayPriority(priority);
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
    double zDepth = depthFromDisplayPriority(priority);
    if (0.0 == zDepth)
        {
        _AddBSplineCurve(bcurve, filled);
        }
    else
        {
        MSBsplineCurvePtr bs = bcurve.CreateCopy();
        size_t nPoles = bs->GetNumPoles();
        DPoint3d* poles = bs->GetPoleP();
        for (size_t i = 0; i < nPoles; i++)
            poles[i].z = zDepth;

        _AddBSplineCurveR(*bs, filled);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/17
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometryListBuilder::_AddBSplineCurve2dR(RefCountedMSBsplineCurveR bcurve, bool filled, double priority)
    {
    double zDepth = depthFromDisplayPriority(priority);
    if (0.0 == zDepth)
        {
        _AddBSplineCurveR(bcurve, filled);
        }
    else
        {
        size_t nPoles = bcurve.GetNumPoles();
        DPoint3d* poles = bcurve.GetPoleP();
        for (size_t i = 0; i < nPoles; i++)
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
        // Attach(gf.GetViewport(), DrawPurpose::NotSpecified);
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
        accum.SaveToGraphicList(m_primitives, options, tolerance, context, !GetCreateParams().IsViewCoordinates());
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
    else // ###TODO...if (nullptr == params.GetViewport())
        {
        BeAssert(!accum.GetGeometries().ContainsCurves() && "No viewport supplied to GraphicBuilder::CreateParams - falling back to default coarse tolerance");
        return 20.0;
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
            SurfaceMaterial surfMat;
            if (materialId.IsValid())
                {
                // NB: For now, we will never have a persistent material with a non-persistent texture (so ignore texMap arg)
                MaterialPtr mat = renderSys._GetMaterial(materialId, dgnDb);
                if (mat.IsValid())
                    surfMat = SurfaceMaterial(*mat);
                }
            else if (nullptr != gradient)
                {
                surfMat = SurfaceMaterial(TextureMapping(*renderSys._GetTexture(*gradient, dgnDb)), gradient);
                }
            else if (texMap.IsValid())
                {
                surfMat = SurfaceMaterial(texMap);
                }

            return new DisplayParams(lineColor, fillColor, width, linePixels, surfMat, fillFlags, catId, subCatId, geomClass);
            }
        case Type::Linear:
            {
            return new DisplayParams(lineColor, width, linePixels, catId, subCatId, geomClass);
            }
        case Type::Text:
            {
            auto params = new DisplayParams(lineColor, catId, subCatId, geomClass);
            params->m_surfaceMaterial = SurfaceMaterial(texMap);
            return params;
            }
        default:
            BeAssert(false);
            return nullptr;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
uint8_t DisplayParams::GetMinTransparency()
    {
    // Threshold below which we consider a color fully opaque.
    return 15;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef DisplayParams::AdjustTransparency(ColorDef color)
    {
    if (color.GetAlpha() < GetMinTransparency())
        color.SetAlpha(0);

    return color;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
MeshBuilderList::MeshBuilderList(MeshBuilderKeyCR key, MeshBuilderSetCR set) : m_key(key), m_set(set)
    {
    m_head = CreateMeshBuilder(m_key.GetDisplayParams());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
MeshBuilderPtr MeshBuilderList::CreateMeshBuilder(DisplayParamsCR params) const
    {
    MaterialAtlasPtr atlas;
    if (Mesh::PrimitiveType::Mesh == m_key.GetPrimitiveType() && !m_key.GetDisplayParams().IgnoresLighting())
        {
        auto const& mat = params.GetSurfaceMaterial();
        if (!mat.GetTextureMapping().IsValid() && nullptr == mat.GetGradient())
            atlas = MaterialAtlas::Create(mat.GetMaterial(), m_set.GetMaxMaterialsPerMesh());
        }

    auto vertTol = m_set.GetVertexTolerance();
    auto areaTol = m_set.GetFacetAreaTolerance();
    auto featureTable = m_set.GetFeatureTable();
    auto type = m_key.GetPrimitiveType();
    auto isPlanar = m_key.IsPlanar();
    auto nodeIndex = m_key.GetNodeIndex();
    return MeshBuilder::Create(params, vertTol, areaTol, featureTable, type, m_set.GetRange(), m_set.Is2d(), isPlanar, nodeIndex, atlas.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
MeshBuilderR MeshBuilderList::GetMeshBuilder(DisplayParamsCR params)
    {
    // Find a builder whose material atlas is not full, or whose atlas already contains a compatible material, or who doesn't support material atlas.
    // (The latter will be true for: point strings, polylines, any surface with a texture/gradient or which ignores lighting).
    MeshBuilderPtr builder = m_head;
    MeshBuilderPtr tail = builder;
    auto material = params.GetSurfaceMaterial().GetMaterial();
    do {
        auto atlas = builder->GetMesh()->GetMaterialAtlas();
        if (nullptr == atlas || atlas->Insert(material).IsValid())
            return *builder;

        tail = builder;
        builder = builder->m_next;
    } while (builder.IsValid());

    // No room in any builder's atlas - must create a new one
    builder = CreateMeshBuilder(params);
    tail->m_next = builder;

    return *builder;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
size_t MeshBuilderList::GetVertexCount() const
    {
    size_t count = 0;
    ForEach([&](MeshBuilderCR builder, bool&) { count += builder.GetMesh()->Points().size(); });
    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilderList::GetMeshes(MeshListR meshes) const
    {
    ForEach([&](MeshBuilderCR builder, bool&)
        {
        auto mesh = builder.m_mesh;
        if (!mesh->IsEmpty())
            meshes.push_back(mesh);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilderList::GetMeshes(bvector<MeshPtr>& meshes, SharedGeomCR sharedGeom) const
    {
    ForEach([&](MeshBuilderCR builder, bool&)
        {
        auto mesh = builder.m_mesh;
        if (!mesh->IsEmpty())
            {
            mesh->SetSharedGeom(sharedGeom);
            meshes.push_back(mesh);
            }
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
MaterialIndex MaterialAtlas::Insert(MaterialP mat)
    {
    auto trans = nullptr != mat ? mat->GetParams().m_transparency.Categorize() : TransparencyCategory::None;
    if (trans != GetTransparency())
        return MaterialIndex();

    if (nullptr != mat && mat->GetTextureMapping().IsValid())
        return MaterialIndex();

    auto index = Find(mat);
    if (index.IsValid())
        return index;

    if (IsFull())
        return MaterialIndex();

    uint8_t newIndex = NumMaterials();
    m_materialToIndex[mat] = newIndex;
    return MaterialIndex(newIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<Material::CreateParams> MaterialAtlas::ToList() const
    {
    bvector<Material::CreateParams> list(m_materialToIndex.size());
    for (auto const& kvp : m_materialToIndex)
        {
        auto const& mat = kvp.first;
        list[kvp.second] = mat.IsValid() ? mat->GetParams() : Material::CreateParams();
        }

    return list;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
Material::CreateParams const& MaterialComparison::GetDefaults()
    {
    static Material::CreateParams s_defaults;
    return s_defaults;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
CompareResult MaterialComparison::Compare(MaterialP lhs, MaterialP rhs)
    {
    if (lhs == rhs)
        return CompareResult::Equal;

    auto const& lhParams = nullptr != lhs ? lhs->GetParams() : GetDefaults();
    auto const& rhParams = nullptr != rhs ? rhs->GetParams() : GetDefaults();

    return Compare(lhParams, rhParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool MaterialComparison::MatchesDefaults(MaterialP mat)
    {
    return nullptr == mat || MatchesDefaults(mat->GetParams());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool MaterialComparison::MatchesDefaults(Material::CreateParams const& params)
    {
    if (params.m_textureMapping.IsValid())
        return false;

    auto const& defaults = GetDefaults();
    return Equals(params, defaults);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
CompareResult MaterialComparison::Compare(Material::CreateParams const& lhs, Material::CreateParams const& rhs)
    {
    auto cmp = lhs.m_diffuseColor.Compare(rhs.m_diffuseColor);
    if (CompareResult::Equal == cmp)
        {
        cmp = lhs.m_specularColor.Compare(rhs.m_specularColor);
        if (CompareResult::Equal == cmp)
            cmp = lhs.m_transparency.Compare(rhs.m_transparency);
        }

    if (CompareResult::Equal != cmp)
        return cmp;

#define COMPARE_MAT(X) if (lhs.X != rhs.X) return lhs.X < rhs.X ? CompareResult::Less : CompareResult::Greater

    COMPARE_MAT(m_specularExponent);
    COMPARE_MAT(m_diffuse);
    COMPARE_MAT(m_specular);

    return CompareResult::Equal;

#undef COMPARE_MAT
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
MeshBuilderListPtr MeshBuilderSet::FindList(MeshBuilderKeyCR key) const
    {
    auto found = m_set.find(key);
    return m_set.end() != found ? *found : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
MeshBuilderR MeshBuilderSet::operator[](MeshBuilderKeyCR key)
    {
    auto found = m_set.find(key);
    if (m_set.end() == found)
        found = m_set.insert(MeshBuilderList::Create(key, *this)).first;

    // std::set iterators are const, but we know the members used by our comparator will not change.
    MeshBuilderListR list = const_cast<MeshBuilderListR>(**found);
    return list.GetMeshBuilder(key.GetDisplayParams());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilderSet::GetMeshes(MeshListR meshes) const
    {
    for (auto const& builder : m_set)
        builder->GetMeshes(meshes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/19
+---------------+---------------+---------------+---------------+---------------+------*/
void MeshBuilderSet::GetMeshes(bvector<MeshPtr>& meshes, SharedGeomCR sharedGeom) const
    {
    for (auto const& builder : m_set)
        builder->GetMeshes(meshes, sharedGeom);
    }

