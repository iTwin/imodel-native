/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DrawAreaPattern.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#define     VCClipStencil           ViewContext::ClipStencil

#define     MAX_GPA_STROKES         1000
#define     MAX_GPA_HATCH_LINES     10000
#define     MAX_LINE_DASHES         100000  // NOTE: This used to be the limit for all the dashes, not just the dashes per line so I don't expect to hit it...
#define     MAX_HATCH_ITERATIONS    50000
#define     MAX_AREA_PATTERN_TILES  1E6     // pre-Athens...which can really slow things down; create warning uses 10000...  Bmped back up to 1E6 with introduction of geometry map patterning - RayB 7/2013.

#define     TOLERANCE_ChoordAngle   .4
#define     TOLERANCE_ChoordLen     1000

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
void PatternParams::Init()
    {
    rMatrix.InitIdentity();
    offset.Zero();
    space1 = 0.0;
    angle1 = 0.0;
    space2 = 0.0;
    angle2 = 0.0;
    scale = 1.0;
    tolerance = 0.0;
    annotationscale = 1.0;
    memset(cellName, 0, sizeof (cellName));
    cellId = 0;
    modifiers = PatternParamsModifierFlags::None;
    minLine = -1;
    maxLine = -1;
    color = ColorDef::Black();
    weight = 0;
    style = 0;
    holeStyle = static_cast<int16_t>(PatternParamsHoleStyleType::Normal);
    dwgHatchDef.pixelSize = 0.0;
    dwgHatchDef.islandStyle = 0;
    origin.Zero();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
PatternParams::PatternParams() {Init();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  07/12
+---------------+---------------+---------------+---------------+---------------+------*/
PatternParamsPtr PatternParams::Create() {return new PatternParams();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
PatternParamsPtr PatternParams::CreateFromExisting(PatternParamsCR params) 
    {
    PatternParamsP newParams = new PatternParams();
    newParams->Copy(params);
    return newParams;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void PatternParams::Copy(PatternParamsCR params) 
    {
    rMatrix = params.rMatrix;
    offset = params.offset;
    space1 = params.space1;
    angle1 = params.angle1;
    space2 = params.space2;
    angle2 = params.angle2;
    scale = params.scale;
    tolerance = params.tolerance;
    annotationscale = params.annotationscale;
    memcpy(cellName, params.cellName, sizeof (cellName));
    cellId = params.cellId;
    modifiers = params.modifiers;
    minLine = params.minLine;
    maxLine = params.maxLine;
    color = params.color;
    weight = params.weight;
    style = params.style;
    holeStyle = params.holeStyle;
    dwgHatchDef.pixelSize = params.dwgHatchDef.pixelSize;
    dwgHatchDef.islandStyle = params.dwgHatchDef.islandStyle;
    dwgHatchDef.hatchLines = params.dwgHatchDef.hatchLines;
    origin = params.origin;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool PatternParams::IsEqual(PatternParamsCR params, PatternParamsCompareFlags compareFlags) 
    {
    if (compareFlags & PATTERNPARAMSCOMPAREFLAGS_RMatrix)
        {
        if ((params.modifiers & PatternParamsModifierFlags::RotMatrix) != (modifiers & PatternParamsModifierFlags::RotMatrix))
            return false;

        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::RotMatrix))
            {
            if (!rMatrix.IsEqual(*(&params.rMatrix)))
                return false;
            }
        }

    if (compareFlags & PATTERNPARAMSCOMPAREFLAGS_Offset)
        {
        if ((params.modifiers & PatternParamsModifierFlags::Offset) != (modifiers & PatternParamsModifierFlags::Offset))
            return false;

        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Offset))
            {
            if (!offset.IsEqual(params.offset))
                return false;
            }
        }

    if (compareFlags & PATTERNPARAMSCOMPAREFLAGS_Default)
        {
        if ((params.modifiers & PatternParamsModifierFlags::Space1) != (modifiers & PatternParamsModifierFlags::Space1))
            return false;

        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Space1))
            {
            if (space1 != params.space1)
                return false;
            }

        if ((params.modifiers & PatternParamsModifierFlags::Space2) != (modifiers & PatternParamsModifierFlags::Space2))
            return false;

        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Space2))
            {
            if (space2 != params.space2)
                return false;
            }

        if ((params.modifiers & PatternParamsModifierFlags::Angle1) != (modifiers & PatternParamsModifierFlags::Angle1))
            return false;

        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Angle1))
            {
            if (angle1 != params.angle1)
                return false;
            }

        if ((params.modifiers & PatternParamsModifierFlags::Angle2) != (modifiers & PatternParamsModifierFlags::Angle2))
            return false;

        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Angle2))
            {
            if (angle2 != params.angle2)
                return false;
            }

        if ((params.modifiers & PatternParamsModifierFlags::Scale) != (modifiers & PatternParamsModifierFlags::Scale))
            return false;

        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Scale))
            {
            if (scale != params.scale)
                return false;
            }

        if ((params.modifiers & PatternParamsModifierFlags::Cell) != (modifiers & PatternParamsModifierFlags::Cell))
            return false;

        if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::Cell))
            {
            if (0 != wcscmp(cellName, params.cellName))
                return false;

            if (cellId != params.cellId)
                return false;
            }
        }

    if (compareFlags & PATTERNPARAMSCOMPAREFLAGS_Symbology)
        {
        if ((params.modifiers & PatternParamsModifierFlags::Color) != (modifiers & PatternParamsModifierFlags::Color))
            return false;

        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Color))
            {
            if (color != params.color)
                return false;
            }

        if ((params.modifiers & PatternParamsModifierFlags::Weight) != (modifiers & PatternParamsModifierFlags::Weight))
            return false;

        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Weight))
            {
            if (weight != params.weight)
                return false;
            }

        if ((params.modifiers & PatternParamsModifierFlags::Style) != (modifiers & PatternParamsModifierFlags::Style))
            return false;

        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Style))
            {
            if (style != params.style)
                return false;
            }
        }
  
    if (compareFlags & PATTERNPARAMSCOMPAREFLAGS_Mline)
        {
        if ((params.modifiers & PatternParamsModifierFlags::Multiline) != (modifiers & PatternParamsModifierFlags::Multiline))
            return false;

        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Multiline))
            {
            if (minLine != params.minLine)
                return false;

            if (maxLine != params.maxLine)
                return false;
            }
        }

    if (compareFlags & PATTERNPARAMSCOMPAREFLAGS_Tolerance)
        {
        if ((params.modifiers & PatternParamsModifierFlags::Tolerance) != (modifiers & PatternParamsModifierFlags::Tolerance))
            return false;
        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Tolerance))
            {
            if (tolerance != params.tolerance)
                return false;
            }
        }

    if (compareFlags & PATTERNPARAMSCOMPAREFLAGS_AnnotationScale)
        {
        if ((params.modifiers & PatternParamsModifierFlags::AnnotationScale) != (modifiers & PatternParamsModifierFlags::AnnotationScale))
            return false;
        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::AnnotationScale))
            {
            if (annotationscale != params.annotationscale)
                return false;
            }
        }

    if (compareFlags & PATTERNPARAMSCOMPAREFLAGS_HoleStyle)
        {
        if ((params.modifiers & PatternParamsModifierFlags::HoleStyle) != (modifiers & PatternParamsModifierFlags::HoleStyle))
            return false;

        if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::HoleStyle))
            {
            if (holeStyle != params.holeStyle)
                return false;
            }
        }

    if (compareFlags & PATTERNPARAMSCOMPAREFLAGS_DwgHatch)
        {
        if ((params.modifiers & PatternParamsModifierFlags::DwgHatchDef) != (modifiers & PatternParamsModifierFlags::DwgHatchDef))
            return false;

        if (PatternParamsModifierFlags::None != (params.modifiers  & PatternParamsModifierFlags::DwgHatchDef))
            {
            if (dwgHatchDef.pixelSize != params.dwgHatchDef.pixelSize)
                return false;

            if (dwgHatchDef.islandStyle != params.dwgHatchDef.islandStyle)
                return false;

            if (dwgHatchDef.hatchLines.size() != params.dwgHatchDef.hatchLines.size())
                return false;

            size_t  nHatchLines = dwgHatchDef.hatchLines.size();

            for (size_t i=0; i<nHatchLines; ++i)
                {
                DwgHatchDefLine const* h1 = &dwgHatchDef.hatchLines.at(i);
                DwgHatchDefLine const* h2 = &params.dwgHatchDef.hatchLines.at(i);

                if (0 != memcmp(h1, h2, sizeof (*h1)))
                    return false;
                }
            }
        }

    if (compareFlags & PATTERNPARAMSCOMPAREFLAGS_Origin)
        {
        if ((params.modifiers & PatternParamsModifierFlags::Origin) != (modifiers & PatternParamsModifierFlags::Origin))
            return false;

        if (PatternParamsModifierFlags::None != (modifiers & PatternParamsModifierFlags::Origin))
            {
            if (!origin.IsEqual(params.origin))
                return false;
            }
        }

    return true;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct PatternBoundaryCollector : IElementGraphicsProcessor
{
private:

IStrokeForCache&    m_stroker;
CurveVectorPtr      m_boundary;
ViewContextP        m_context;
Transform           m_currentTransform;

protected:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/13
+---------------+---------------+---------------+---------------+---------------+------*/
explicit PatternBoundaryCollector(IStrokeForCache& stroker) : m_stroker(stroker) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _ProcessAsFacets(bool isPolyface) const override {return false;}
virtual bool _ProcessAsBody(bool isCurved) const override {return false;}
virtual void _AnnounceContext(ViewContextR context) override {m_context = &context;}
virtual void _AnnounceTransform(TransformCP trans) override {if (trans) m_currentTransform = *trans; else m_currentTransform.InitIdentity();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _ProcessCurveVector(CurveVectorCR curves, bool isFilled) override
    {
    if (!curves.IsAnyRegionType() || m_boundary.IsValid())
        {
        BeAssert(false); // A valid boundary must be a closed, parity, or union region...

        return SUCCESS;
        }

    m_boundary = curves.Clone();

    if (NULL != m_context->GetCurrLocalToWorldTransformCP ())
        m_boundary->TransformInPlace(*m_context->GetCurrLocalToWorldTransformCP ());

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _OutputGraphics(ViewContextR context) override
    {
    m_stroker._StrokeForCache(context);
    }

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/13
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr GetBoundary() {return m_boundary;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/13
+---------------+---------------+---------------+---------------+---------------+------*/
static CurveVectorPtr Process(IStrokeForCache& stroker)
    {
    PatternBoundaryCollector  processor(stroker);

    ElementGraphicsOutput::Process(processor, stroker._GetDgnDb());

    return processor.GetBoundary();
    }

}; // PatternBoundaryCollector

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
ViewContext::ClipStencil::ClipStencil(IStrokeForCache& stroker) : m_stroker(stroker) {m_tmpQvElem = nullptr;}
ViewContext::ClipStencil::~ClipStencil() {if (m_tmpQvElem) T_HOST.GetGraphicsAdmin()._DeleteQvElem(m_tmpQvElem);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
QvElem* ViewContext::ClipStencil::GetQvElem(ViewContextR context)
    {
    if (nullptr != m_tmpQvElem)
        return m_tmpQvElem;

    bool    deleteQvElem;
    QvElem* qvElem = context.GetCachedGeometry(m_stroker, deleteQvElem);

    if (deleteQvElem)
        m_tmpQvElem = qvElem;

    return qvElem;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr ViewContext::ClipStencil::GetCurveVector()
    {
    if (m_curveVector.IsNull())
        m_curveVector = PatternBoundaryCollector::Process(m_stroker);

    return m_curveVector;
    }

/*=================================================================================**//**
* @bsiclass                                                     BrienBastings   11/07
+===============+===============+===============+===============+===============+======*/
struct PatternSymbol : IDisplaySymbol
{
private:

DgnGeomPartId       m_partId;
mutable DRange3d    m_range;

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _Draw(ViewContextR context) override
    {
#if defined (NEEDSWORK_REVISIT_PATTERN_SYMBOLS_SCDEF)
    // Pattern geometry will be a geom part...not an element...
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  08/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _GetRange(DRange3dR range) const override
    {
    range = m_range;
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
PatternSymbol(DgnGeomPartId partId, DgnDbR project)
    {
#if defined (NEEDSWORK_REVISIT_PATTERN_SYMBOLS_SCDEF)
    DgnElementPtr edP;

    // If pattern cell doesn't already exist in project...it should fail...
    m_elementRef = project.Elements().FindElement(cellId);
    if (NULL == m_elementRef)
        return;

    edP = m_elementRef->Duplicate();
    if (!edP.IsValid())
        return;

    BeAssert(edP.IsValid());
    m_eeh.SetDgnElement(edP.get());

    // Match dimension of modelRef; otherwise pattern in 3D reference to 2D ends up at z=0, which may be outside of the range of the reference file.
    if (modelRef->Is3d() && !DisplayHandler::Is3dElem(m_eeh.GetGraphicsCP ()))
        {
        m_eeh.GetHandler().ConvertTo3d(m_eeh, 0.0);
        }
    else if (!modelRef->Is3d() && DisplayHandler::Is3dElem(m_eeh.GetGraphicsCP ()))
        {
        DVec3d      flattenDir;
        Transform   flattenTrans;

        flattenDir.init(0.0, 0.0, 1.0);
        flattenTrans.InitIdentity();              
        flattenTrans.form3d[2][2] = 0.0;

        m_eeh.GetHandler().ConvertTo2d(m_eeh, flattenTrans, flattenDir);
        }

    m_range = m_eeh.GetGeometricElement()->_CalculateRange3d();
#else
    m_partId = partId;
    m_range  = DRange3d::NullRange();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool Is3dCellSymbol()
    {
#if defined (NEEDSWORK_REVISIT_PATTERN_SYMBOLS_SCDEF)
    return m_eeh.GetDgnModelP ()->Is3d();
#else
    return true;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
void ApplyElemDisplayParams(ElemDisplayParamsCR elParams)
    {
#if defined (NEEDSWORK_REVISIT_PATTERN_SYMBOLS_SCDEF)
    ElementPropertiesSetter remapper;

    remapper.SetCategory(elParams.GetCategoryId());
    remapper.SetTransparency(elParams.GetTransparency());

    if (IsPointCellSymbol())
        {
        remapper.SetColor(elParams.GetLineColor());
        remapper.SetWeight(elParams.GetWeight());
        remapper.SetLinestyle(elParams.GetLineStyle(), elParams.GetLineStyleParams());
        }

    remapper.Apply(m_eeh);
#endif
    }

}; // PatternSymbol

/*=================================================================================**//**
* @bsiclass                                                     BrienBastings   11/07
+===============+===============+===============+===============+===============+======*/
struct PatternHelper
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void CookPatternSymbology(PatternParamsCR params, ViewContextR context)
    {
    ElemDisplayParamsR elParams = context.GetCurrentDisplayParams();

    if (PatternParamsModifierFlags::None != ((PatternParamsModifierFlags::Color | PatternParamsModifierFlags::Weight | PatternParamsModifierFlags::Style) & params.modifiers))
        {
        if (PatternParamsModifierFlags::None != (params.modifiers & PatternParamsModifierFlags::Color))
            elParams.SetLineColor(params.color);

        if (PatternParamsModifierFlags::None != (params.modifiers & PatternParamsModifierFlags::Weight))
            elParams.SetWeight(params.weight);

#if defined (NEEDSWORK_WIP_LINESTYLE)
        if (PatternParamsModifierFlags::None != (params.modifiers & PatternParamsModifierFlags::Style))
            elParams->SetLineStyle(params.style, elParams->GetLineStyleParams());
#endif
        }

    // NOTE: Don't need to worry about overrides, context overrides CAN NOT look at m_currDisplayParams, so changing it doesn't affect them...
    context.CookDisplayParams();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static bool PushBoundaryClipStencil(ViewContextR context, VCClipStencil& boundary, QvElem*& qvElem)
    {
    if (NULL == (qvElem = boundary.GetQvElem(context)))
        return false;

    context.GetIViewDraw().PushClipStencil(qvElem);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void PopBoundaryClipStencil(ViewContextR context, QvElem* qvElem)
    {
    if (NULL == qvElem)
        return;

    context.GetIViewDraw().PopClipStencil();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static bool GetCellTileInfo
(
VCClipStencil&  boundary,
PatternSymbol&  symbCell,
DRange3dR       cellRange,
DPoint2dR       cellOrg,
DPoint2dR       spacing,
DPoint2dR       low,
DPoint2dR       high,
bool&           isPlanar,
DPoint3dCR      origin,
RotMatrixCR     rMatrix,
double&         scale,
double          rowSpacing,
double          columnSpacing
)
    {
    if (SUCCESS != symbCell._GetRange(cellRange))
        return false;

    if (columnSpacing < 0.0)
        spacing.x = -columnSpacing;
    else
        spacing.x = columnSpacing + scale * (cellRange.high.x - cellRange.low.x);

    if (rowSpacing < 0.0)
        spacing.y = -rowSpacing;
    else
        spacing.y = rowSpacing + scale * (cellRange.high.y - cellRange.low.y);

    if (spacing.x < 0.5 || spacing.y < 0.5)
        return false;

    RotMatrix   invMatrix;
    Transform   transform;

    invMatrix.InverseOf(rMatrix);
    transform.InitFrom(invMatrix);
    transform.TranslateInLocalCoordinates(transform, -origin.x, -origin.y, -origin.z);

    CurveVectorPtr  boundaryCurve = boundary.GetCurveVector();

    if (!boundaryCurve.IsValid())
        return false;

    DRange3d  localRange;

    boundaryCurve->GetRange(localRange, transform);

    low.x = localRange.low.x;
    low.y = localRange.low.y;

    high.x = localRange.high.x;
    high.y = localRange.high.y;

    if (low.x < 0.0)
        low.x -= spacing.x + fmod(low.x, spacing.x);
    else
        low.x -= fmod(low.x, spacing.x);

    if (low.y < 0.0)
        low.y -= spacing.y + fmod(low.y, spacing.y);
    else
        low.y -= fmod(low.y, spacing.y);

    cellOrg.x = cellRange.low.x * scale;
    cellOrg.y = cellRange.low.y * scale;

    low.x -= cellOrg.x;
    low.y -= cellOrg.y;

    high.x -= cellOrg.x;
    high.y -= cellOrg.y;

    double  numTiles = ((high.x - low.x) / spacing.x) * ((high.y - low.y) / spacing.y);

    if (numTiles > MAX_AREA_PATTERN_TILES)
        {
        double  factor = (numTiles / MAX_AREA_PATTERN_TILES) * 0.5;

        // NOTE: Increase pattern scale and display *something* instead of useless message center alert...
        spacing.Scale(spacing, factor);
        scale *= factor;
        }

    // Can't use a stencil for a non-planar pattern cell....
    isPlanar = !symbCell.Is3dCellSymbol() || ((cellRange.high.z - cellRange.low.z) < 2.0);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void GetCellOrientationAndScale
(
RotMatrixR      rMatrix,
double&         scale,
RotMatrixR      patternRMatrix,
double          patternAngle,
double          patternScale
)
    {
    RotMatrix   angleRot;
    
    angleRot.InitFromAxisAndRotationAngle(2, patternAngle);
    rMatrix.InitProduct(patternRMatrix, angleRot);
    scale = patternScale ? patternScale : 1.0;
    }

#if defined (NEEDS_WORK_MATERIAL)
/*=================================================================================**//**
* @bsiclass                                                     RayBentley      05/2007
+===============+===============+===============+===============+===============+======*/
struct GeometryMapPatternAppData : DgnElement::AppData
{
MaterialPtr     m_material;

GeometryMapPatternAppData(MaterialPtr material) : m_material(material) {}
MaterialCP GetMaterial() {return m_material.get();}
};
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
static MaterialPtr CreateGeometryMapMaterial(ViewContextR context, PatternSymbol& symbCell, PatternParamsP params, DPoint2dCR spacing)
    {
    MaterialPtr         pMaterial = Material::Create(symbCell.GetElemHandle().GetDgnModelP ()->GetDgnDb());
    MaterialSettingsR   settings = pMaterial->GetSettingsR ();
    MaterialMapP        map = settings.GetMapsR().AddMap(MaterialMap::MAPTYPE_Geometry);
    MaterialMapLayerR   layer = map->GetLayersR().GetTopLayerR();

    map->SetValue(1.0);
    layer.SetMode(MapMode::Parametric);
    layer.SetScale(1.0, 1.0, 1.0);
    layer.SetIsBackgroundTransparent(true);

    // NOTE: Need to setup pattern symbology on cell element and hide 0 length lines used as pattern cell extent markers, etc.
    PatternHelper::CookPatternSymbology(*params, context);
    symbCell.ApplyElemDisplayParams(*context.GetCurrentDisplayParams());

    context.GetIViewDraw().DefineQVGeometryMap(*pMaterial, symbCell.GetElemHandle(), &spacing, !symbCell.IsPointCellSymbol(), context, true);

    return pMaterial;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
static void AddPatternParametersToPolyface(PolyfaceHeaderPtr& polyface, RotMatrixCR rMatrix, DPoint3dCR origin, DPoint2dCR spacing)
    {
    Transform   cellToWorld = Transform::From(rMatrix, origin), worldToCell;

    cellToWorld.ScaleMatrixColumns(spacing.x, spacing.y, 1.0);
    worldToCell.InverseOf(cellToWorld);

    BlockedVectorDPoint2dR  params = polyface->Param();

    params.SetActive(true);

    for (DPoint3dCR point: polyface->Point())
        {
        DPoint3d    paramPoint;

        worldToCell.Multiply(paramPoint, point);
        params.push_back(DPoint2d::From(paramPoint.x, paramPoint.y));
        }
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
static size_t GetVertexCount(CurveVectorCR curveVector)
    {
    size_t  loopCount = 0;

    for (ICurvePrimitivePtr curve: curveVector)
        {
        if (!curve.IsValid())
            continue;

        switch (curve->GetCurvePrimitiveType())
            {
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                loopCount += curve->GetLineStringCP ()->size();
                break;

            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector:
                loopCount += GetVertexCount(*curve->GetChildCurveVectorCP ());
                break;

            default:
                loopCount += 2;
                break;
            }
        }

    return loopCount;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
static bool ProcessAreaPatternAsGeometryMap  
(
ViewContextR    context,
VCClipStencil&  boundary,
PatternSymbol&  symbCell,
PatternParamsP  params,
RotMatrixCR     rMatrix,
DPoint3dCR      origin, 
DPoint2dCR      spacing,
double          scale
)
    {
#if defined (NEEDSWORK_REVISIT_PATTERN_SYMBOLS_SCDEF)
    static      DgnElementAppData::Key s_appDataKey;

    if (DrawPurpose::Plot == context.GetDrawPurpose())      // Opt for slower, higher quality when plotting.
        return false;

#if 1 || !defined (BENTLEYCONFIG_GRAPHICS_OPENGLES)  //  We always want to use geometry map with OpenGL ES because the our OpenGL implementation of PushClipStencil does not work
    double          pixelSize = context.GetPixelSizeAtPoint(NULL);
    double          tilePixels = MAX (spacing.x, spacing.y) / pixelSize;
    static double   s_maxGeometryMapTile = 32.0;

    if (tilePixels > s_maxGeometryMapTile)
        return false;                                       // The pattern is too big... Draw it.
#endif

    DgnElementCP   cellElementRef;
    if (NULL == (cellElementRef = symbCell.GetElementRef()))
        return false;                                       // No place to persist map...

    GeometryMapPatternAppData*  appData;
    DPoint2d                    cellSpacing = spacing;

    cellSpacing.Scale(1.0 / scale);

    if (NULL == (appData = reinterpret_cast <GeometryMapPatternAppData*> (cellElementRef->FindAppData(s_appDataKey))))
        {
        MaterialPtr pMaterial = CreateGeometryMapMaterial(context, symbCell, params, cellSpacing);

        if (!pMaterial.IsValid())
            return false;

        cellElementRef->AddAppData(s_appDataKey, appData = new GeometryMapPatternAppData(pMaterial));
        }

    // NOTE: Colors aren't stored in geometry map for point cells, setup active matsymb color from pattern if different than element color...
    if (symbCell.IsPointCellSymbol() && PatternParamsModifierFlags::None != (params->modifiers & PatternParamsModifierFlags::Color) && context.GetCurrentDisplayParams()->GetLineColor() != params->color)
        {
        // NOTE: Don't need to worry about overrides, context overrides CAN NOT look at m_currDisplayParams, so changing line color doesn't affect them...
        context.GetCurrentDisplayParams()->SetLineColor(params->color);
        context.CookDisplayParams();
        }

    OvrMatSymbP  ovrMatSymb = context.GetOverrideMatSymb();

    ovrMatSymb->SetFillTransparency(0xff);
    ovrMatSymb->SetMaterial(appData->GetMaterial());
    context.GetIDrawGeom().ActivateOverrideMatSymb(ovrMatSymb);

    CurveVectorPtr  boundaryCurve = boundary.GetCurveVector();

    // NOTE: Use stencils for curved or complex boundaries as these can take significant time to create polyface...
#if !defined (BENTLEYCONFIG_GRAPHICS_OPENGLES)  //  Don't allow this with OpenGL ES since PushBoundaryClipStencil does not work for geometry map.
    static size_t   s_facetBoundaryMax = 1000;
    if (boundaryCurve->ContainsNonLinearPrimitive() || GetVertexCount(*boundaryCurve) > s_facetBoundaryMax)
        {
        QvElem* qvElem = NULL;

        if (!PatternHelper::PushBoundaryClipStencil(context, boundary, qvElem))
            return false;

        PolyfaceHeaderPtr       polyface = PolyfaceHeader::CreateVariableSizeIndexed();
        BlockedVectorDPoint3dR  points = polyface->Point();
        BlockedVectorIntR       pointIndices = polyface->PointIndex();

        points.SetActive(true);
        pointIndices.SetActive(true);
        points.resize(4);
        
        DPoint3d    shapePts[5];

        GetBoundaryShapePts(shapePts, *boundaryCurve, rMatrix, origin);

        for (int i=0; i<4; i++)
            {
            pointIndices.push_back(i+1);
            points[i] = shapePts[i];
            }
        
        pointIndices.push_back(0);
        AddPatternParametersToPolyface(polyface, rMatrix, origin, spacing);
        context.GetIDrawGeom().DrawPolyface(*polyface, true);

        PatternHelper::PopBoundaryClipStencil(context, qvElem);
        }
    else
#endif
        {
        IFacetOptionsPtr  options = IFacetOptions::CreateForCurves();

        options->SetConvexFacetsRequired(true);
        options->SetMaxPerFace(5000);

        IPolyfaceConstructionPtr  builder = IPolyfaceConstruction::New(*options);

        builder->AddRegion(*boundaryCurve);

        PolyfaceHeaderPtr  polyface = builder->GetClientMeshPtr();

        if (!polyface.IsValid())
            {
            BeAssert(false);
            return false;
            }

        AddPatternParametersToPolyface(polyface, rMatrix, origin, spacing);
        context.GetIDrawGeom().DrawPolyface(*polyface, true);
        }

    return true;
#else
    return false;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static bool DrawCellTiles(ViewContextR context, PatternSymbol& symbCell, DPoint2dCR low, DPoint2dCR high, DPoint2dCR spacing, double scale, TransformCR orgTrans, DPoint3dCP cellCorners, bool drawFiltered, CurveVectorCP boundaryToPush = NULL)
    {
    if (NULL != boundaryToPush)
        {
        ClipVectorPtr   clip = ClipVector::CreateFromCurveVector(*boundaryToPush, 0.0, TOLERANCE_ChoordAngle);

        if (!clip.IsValid())
            return false;

        clip->ParseClipPlanes();
        context.PushClip(*clip); // NOTE: Pop handled by context mark...
        }

    bool        wasAborted = false;
    DPoint2d    patOrg;
    DPoint3d    tileCorners[8];
    Transform   cellTrans;

    for (patOrg.x = low.x; patOrg.x < high.x && !wasAborted; patOrg.x += spacing.x)
        {
        for (patOrg.y = low.y; patOrg.y < high.y && !wasAborted; patOrg.y += spacing.y)
            {
            if (context.CheckStop())
                {
                wasAborted = true;
                break;
                }

            cellTrans.TranslateInLocalCoordinates(orgTrans, patOrg.x/scale, patOrg.y/scale, context.GetCurrentDisplayParams().GetNetDisplayPriority());
            cellTrans.Multiply(tileCorners, cellCorners, 8);

            if (ClipPlaneContainment_StronglyOutside == context.GetTransformClipStack().ClassifyPoints(tileCorners, 8))
                continue;

            if (drawFiltered)
                {
                DPoint3d    tmpPt;

                cellTrans.GetTranslation(tmpPt);
                context.GetIDrawGeom().DrawPointString3d(1, &tmpPt, NULL);
                }
            else
                {
                context.DrawSymbol(&symbCell, &cellTrans, NULL, false, false);
                }

            wasAborted = context.WasAborted();
            }
        }

    return wasAborted;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void ProcessAreaPattern
(
ViewContextR    context,
VCClipStencil&  boundary,
PatternParamsP  params,
DPoint3dR       origin,
double          contextScale
)
    {
#if defined (NEEDSWORK_REVISIT_PATTERN_SYMBOLS_SCDEF)
    PatternSymbol symbCell(DgnElementId(params->cellId), context.GetDgnDb());

    if (!symbCell.GetElemHandle().IsValid())
        return;

    double      scale;
    RotMatrix   rMatrix;

    PatternHelper::GetCellOrientationAndScale(rMatrix, scale, params->rMatrix, params->angle1, params->scale);

    bool        isPlanar;
    DPoint2d    cellOrg, spacing, low, high;
    DRange3d    cellRange;

    // The contextScale value allows PlotContext to adjust the pattern scale to reduce
    // the size of the QV display list and resulting plot output file.
    scale *= contextScale;

    if (!PatternHelper::GetCellTileInfo(boundary, symbCell, cellRange, cellOrg, spacing, low, high, isPlanar, origin, rMatrix, scale, params->space1, params->space2))
        return;

    bool        isQVOutput = context.GetIViewDraw().IsOutputQuickVision();
    bool        useStencil = isQVOutput && isPlanar && !context.CheckICachedDraw(); // Can't use stencil if creating QvElem...dimension terminators want patterns!
    QvElem*     qvElem = NULL;

    if (useStencil && PatternHelper::ProcessAreaPatternAsGeometryMap(context, boundary, symbCell, params, rMatrix, origin, spacing, scale))
        return;

    if (useStencil && !PatternHelper::PushBoundaryClipStencil(context, boundary, qvElem))
        return;

    // NOTE: Setup symbology AFTER visit to compute stencil/clip since that may change current display params!
    PatternHelper::CookPatternSymbology(*params, context);
    symbCell.ApplyElemDisplayParams(*context.GetCurrentDisplayParams());

    bool        drawFiltered = false;
    Transform   orgTrans;
    DPoint3d    cellCorners[8];

    // Setup initial pattern instance transform
    LegacyMath::TMatrix::ComposeOrientationOriginScaleXYShear(&orgTrans, NULL, &rMatrix, &origin, scale, scale, 0.0);
    cellRange.Get8Corners(cellCorners);

    if (isQVOutput)
        {
        int     cellCmpns = symbCell.GetElemHandle().GetGraphicsCP ()->GetComplexComponentCount();
        double  numTiles = ((high.x - low.x) / spacing.x) * ((high.y - low.y) / spacing.y);

        if (numTiles * cellCmpns > 5000)
            {
            DPoint3d    viewPts[8];
            DRange2d    viewRange;

            orgTrans.Multiply(viewPts, cellCorners, 8);
            context.LocalToView(viewPts, viewPts, 8);
            viewRange.InitFrom(*viewPts, *8);

            if (symbCell.IsPointCellSymbol())
                drawFiltered = (viewRange.extentSquared() < context.GetMinLOD ());
            }
        }

    if (useStencil)
        {
        DrawCellTiles(context, symbCell, low, high, spacing, scale, orgTrans, cellCorners, drawFiltered);
        }
    else
        {
        CurveVectorPtr  boundaryCurve = boundary.GetCurveVector();

        // NOTE: Union regions aren't valid clip boundaries, need to push separate clip for each solid area...
        if (boundaryCurve->IsUnionRegion())
            {
            for (ICurvePrimitivePtr curve: *boundaryCurve)
                {
                if (curve.IsNull() || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType())
                    continue;

                ViewContext::ContextMark mark(&context);

                // NOTE: Cell tile exclusion check makes sending all tiles for each solid area less offensive (union regions also fairly rare)...
                if (DrawCellTiles(context, symbCell, low, high, spacing, scale, orgTrans, cellCorners, drawFiltered, curve->GetChildCurveVectorCP ()))
                    break; // Was aborted...
                }
            }
        else
            {
            DrawCellTiles(context, symbCell, low, high, spacing, scale, orgTrans, cellCorners, drawFiltered, boundaryCurve.get());
            }
        }

    PatternHelper::PopBoundaryClipStencil(context, qvElem);

    context.DeleteSymbol(&symbCell); // Only needed if DrawSymbol has been called...i.e. early returns are ok!
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static int DrawHatchGPA
(
GPArrayP        pGPA,
ViewContextR    context
)
    {
    size_t        nGot, sourceCount = pGPA->GetGraphicsPointCount();
    DPoint3d      localPoints[MAX_GPA_STROKES];
    bool          is3d = context.Is3dView();
    double        priority = context.GetCurrentDisplayParams().GetNetDisplayPriority();
    GraphicsPoint gp;

    for (size_t i=0; i < sourceCount;)
        {
        if (0 == (i % 100) && context.CheckStop())
            return ERROR;

        nGot = 0;
        
        while (pGPA->GetGraphicsPoint(i, gp))
            {
            gp.point.GetProjectedXYZ (localPoints[nGot++]);

            if (gp.IsCurveBreak())
                {
                i++;
                break;
                }
            else if (nGot >= MAX_GPA_STROKES)
                {
                // Leave i where it is to resume from non-break !!!
                break;
                }
            else
                {
                i++;
                }
            }

        if (nGot <= 1)
            continue;

        if (is3d)
            {
            context.DrawStyledLineString3d((int) nGot, localPoints, NULL, false);
            }
        else
            {
            // To ensure display priority is properly honored in non-rasterized plots, it is necessary to call QuickVision 2D draw methods. TR 180390.
            std::valarray<DPoint2d> localPoints2dBuf(nGot);

            for (size_t iPoint = 0; iPoint < nGot; iPoint++)
                {
                localPoints2dBuf[iPoint].x = localPoints[iPoint].x;
                localPoints2dBuf[iPoint].y = localPoints[iPoint].y;
                }

            context.DrawStyledLineString2d((int) nGot, &localPoints2dBuf[0], priority, NULL, false);
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/13
+---------------+---------------+---------------+---------------+---------------+------*/
static void GetBoundaryShapePts
(
DPoint3dP       pts,
CurveVectorCR   boundaryCurve,
RotMatrixCR     rMatrix,
DPoint3dCR      origin
)
    {
    RotMatrix   invMatrix;
    Transform   transform;

    invMatrix.InverseOf(rMatrix);
    transform.InitFrom(invMatrix);

    DRange3d    localRange;

    boundaryCurve.GetRange(localRange, transform);

    pts[0].x = pts[3].x = localRange.low.x;
    pts[1].x = pts[2].x = localRange.high.x;

    pts[0].y = pts[1].y = localRange.low.y;
    pts[2].y = pts[3].y = localRange.high.y;

    DPoint3d    stencilOrigin;

    transform.Multiply(stencilOrigin, origin);

    pts[0].z = pts[1].z = pts[2].z = pts[3].z = stencilOrigin.z;
    pts[4] = pts[0];

    transform.MultiplyTranspose(pts, pts, 5);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static GPArrayP GetBoundaryGPA
(
VCClipStencil&  boundary,
RotMatrixCR     rMatrix,
DPoint3dCR      origin,
bool            useElmRange
)
    {
    CurveVectorPtr boundaryCurve = boundary.GetCurveVector();

    if (!boundaryCurve.IsValid())
        return NULL;

    GPArrayP    gpa = GPArray::Grab();

    if (!useElmRange)
        {
        gpa->Add(*boundaryCurve);

        return gpa;
        }

    DPoint3d    shapePts[5];

    GetBoundaryShapePts(shapePts, *boundaryCurve, rMatrix, origin);

    gpa->Add(shapePts, 5);
    gpa->MarkBreak();
    gpa->MarkMajorBreak();

    return gpa;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void GetHatchLineLimitTransform
(
TransformR      scaledTransform,
TransformCR     hatchTransform,
GPArrayP        boundGpa
)
    {
    double      zScale;

    // modify the transform to limit the number of hatch lines.
    if ((zScale = jmdlGPA_hatchDensityScale(&hatchTransform, boundGpa, NULL, MAX_GPA_HATCH_LINES)) > 1.0)
        {
        RotMatrix   scaleMatrix;

        bsiRotMatrix_initFromScaleFactors(&scaleMatrix, 1.0, 1.0, zScale);
        scaledTransform.InitProduct(hatchTransform, scaleMatrix);
        }
    else
        {
        scaledTransform = hatchTransform;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void AddHatchLinesToGPA
(
GPArrayP        hatchGpa,
GPArrayP        boundGpa,
TransformR      scaledTransform
)
    {
    jmdlGraphicsPointArray_addTransformedCrossHatchClipped(hatchGpa, boundGpa, &scaledTransform, NULL, NULL, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void ProcessHatchBoundary
(
GPArrayP        boundGpa,
ViewContextR    context,
TransformR      baseTransform,
TransformR      hatchTransform,
double          angle,
double          space
)
    {
    DVec3d      xVec,yVec, zVec;
    DPoint3d    origin;

    hatchTransform.GetOriginAndVectors(origin, xVec, yVec, zVec);
    xVec.Init( cos(angle), sin(angle), 0.0);
    zVec.CrossProduct(xVec, yVec);
    zVec.Scale(space);
    hatchTransform.InitFromOriginAndVectors(origin, xVec, yVec, zVec);

    Transform       scaledTransform;
    GPArraySmartP   hatchGpa;

    GetHatchLineLimitTransform(scaledTransform, hatchTransform, boundGpa);
    AddHatchLinesToGPA (hatchGpa, boundGpa, scaledTransform);

    hatchGpa->Transform(&baseTransform);

    PatternHelper::DrawHatchGPA (hatchGpa, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void ProcessHatchPattern
(
ViewContextR    context,
VCClipStencil&  boundary,
PatternParams*  params,
DPoint3dR       origin
)
    {
#if !defined (BENTLEYCONFIG_GRAPHICS_OPENGLES)  //  We always want to use geometry map with OpenGL ES because the our OpenGL implementation of PushClipStencil does not work
    bool            useStencil = context.GetIViewDraw().IsOutputQuickVision() && !context.CheckICachedDraw(); // Can't use stencil if creating QvElem...dimension terminators want patterns!
#else
    bool            useStencil = false;
#endif
    useStencil = false;
    GPArraySmartP   boundGpa(PatternHelper::GetBoundaryGPA (boundary, params->rMatrix, origin, useStencil));

    if (NULL == boundGpa || 0 == boundGpa->GetCount())
        return;

    QvElem*         qvElem = NULL;

    if (useStencil && !PatternHelper::PushBoundaryClipStencil(context, boundary, qvElem))
        return;

    // NOTE: Setup symbology AFTER visit to compute stencil/clip since that may change current display params!
    PatternHelper::CookPatternSymbology(*params, context);

    Transform       baseTransform, invBaseTransform;
    Transform       hatchTransform;

    hatchTransform.InitFromRowValues(1,0,0, 0, 0,0,0, 0, 0,-1,0, 0);
    baseTransform.InitFrom(params->rMatrix, origin);
    invBaseTransform.InverseOf(baseTransform);

    boundGpa->Transform(&invBaseTransform);

    PatternHelper::ProcessHatchBoundary(boundGpa, context, baseTransform, hatchTransform, params->angle1, params->space1);

    if (PatternParamsModifierFlags::None != (params->modifiers & PatternParamsModifierFlags::Space2))
        PatternHelper::ProcessHatchBoundary(boundGpa, context, baseTransform, hatchTransform, params->angle2, params->space2);

    PatternHelper::PopBoundaryClipStencil(context, qvElem);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/01
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsValidPatternDefLine
(
DwgHatchDefLineP    lineP,
double              rangeDiagonal
)
    {
    double      offsetMagnitude = lineP->offset.Magnitude();

    if (0.0 == offsetMagnitude || rangeDiagonal / offsetMagnitude > MAX_HATCH_ITERATIONS)
        return false;

    if (0 == lineP->nDashes)
        return true;

    double      totalDashLength = 0.0;

    for (int i=0; i < lineP->nDashes; i++)
        totalDashLength += fabs(lineP->dashes[i]);

    return (!(0.0 == totalDashLength || rangeDiagonal / totalDashLength > MAX_HATCH_ITERATIONS));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void ProcessDWGHatchBoundary
(
GPArrayP            boundGpa,
ViewContextR        context,
TransformR          baseTransform,
TransformR          hatchTransform,
double              annotationScale,
DwgHatchDefLineP    hatchLines,
int                 nDefLines,
bool                is3d
)
    {
    DRange3d    boundRange;

    bsiDRange3d_init(&boundRange);
    jmdlDRange3d_extendByGraphicsPointArray(&boundRange, boundGpa);

    if (!is3d)
        boundRange.low.z = boundRange.high.z = 0.0;

    StatusInt       status = SUCCESS;
    double          rangeDiagonal = boundRange.low.Distance(boundRange.high);
    GPArraySmartP   hatchGpa, dashGpa;

    // NOTE: In ACAD hatch definitions both the base angle and scale have already been applied to the definitions and MUST not be applied again!
    for (DwgHatchDefLineP lineP = hatchLines; SUCCESS == status && lineP < &hatchLines[nDefLines]; lineP++)
        {
        if (!IsValidPatternDefLine(lineP, rangeDiagonal))
            continue;

        DVec3d      xVec, yVec, zVec;
        DPoint3d    origin;

        // NOTE: Annotation scale isn't handled by CookPatternParams since we ignore scale for draw, need to apply to hatch lines directly...
        hatchTransform.GetOriginAndVectors(origin, xVec, yVec, zVec);
        xVec.Init( cos(lineP->angle), sin(lineP->angle), 0.0);
        zVec.Init( lineP->offset.x * annotationScale, lineP->offset.y * annotationScale, 0.0);
        origin.Init( lineP->through.x * annotationScale, lineP->through.y * annotationScale, 0.0);
        hatchTransform.InitFromOriginAndVectors(origin, xVec, yVec, zVec);

        Transform    scaledTransform;

        GetHatchLineLimitTransform(scaledTransform, hatchTransform, boundGpa);
        AddHatchLinesToGPA (hatchGpa, boundGpa, scaledTransform);

        if (0 != lineP->nDashes)
            {
            double*         dashesOut = lineP->dashes;
            bvector<double> localDashes;

            if (1.0 != annotationScale)
                {
                localDashes.insert(localDashes.begin(), dashesOut, dashesOut + lineP->nDashes);

                for (double& dashLen: localDashes)
                    dashLen *= annotationScale;

                dashesOut = &localDashes.front();
                }

            // NOTE: Copy of jmdlGraphicsPointArray_expandDashPattern to avoid s_maxCollectorPoint limit...
            int         i1;
            DPoint4d    point0, point1;
            int         curveType;
            double      dashPeriod = jmdlGPA_computeDashPeriod(dashesOut, lineP->nDashes);

            for (int i0 = 0; SUCCESS == status && jmdlGraphicsPointArray_parseFragment(hatchGpa, &i1, &point0, &point1, &curveType, i0); i0 = i1 + 1)
                {
                if (0 == curveType)
                    {
                    GraphicsPoint   gp0, gp1;

                    jmdlGraphicsPointArray_getGraphicsPoint(hatchGpa, &gp0, i0);

                    for (int i = i0 + 1; SUCCESS == status && i <= i1; i++, gp0 = gp1)
                        {
                        jmdlGraphicsPointArray_getGraphicsPoint(hatchGpa, &gp1, i);
                        jmdlGPA_expandSingleLineDashPattern(dashGpa, &gp0, &gp1, dashesOut, lineP->nDashes, dashPeriod, MAX_LINE_DASHES);

                        dashGpa->Transform(&baseTransform);

                        status = PatternHelper::DrawHatchGPA (dashGpa, context);

                        dashGpa->Empty();
                        }
                    }
                else
                    {
                    jmdlGraphicsPointArray_appendFragment(dashGpa, hatchGpa, i0, i1, 0);
                    jmdlGraphicsPointArray_markBreak(dashGpa);

                    dashGpa->Transform(&baseTransform);

                    status = PatternHelper::DrawHatchGPA (dashGpa, context);

                    dashGpa->Empty();
                    }
                }
            }
        else
            {
            hatchGpa->Transform(&baseTransform);

            status = PatternHelper::DrawHatchGPA (hatchGpa, context);
            }

        hatchGpa->Empty();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void ProcessDWGHatchPattern
(
ViewContextR        context,
VCClipStencil&      boundary,
PatternParamsP      params,
DPoint3dR           origin
)
    {
    // NOTE: Never use stencil for DWG patterns, boundary can be open elements...
    GPArraySmartP   boundGpa(PatternHelper::GetBoundaryGPA (boundary, params->rMatrix, origin, false));

    if (NULL == boundGpa || 0 == boundGpa->GetCount())
        return;

    Transform   baseTransform, invBaseTransform;
    Transform   hatchTransform;

    // NOTE: Setup symbology AFTER visit to compute stencil/clip since that may change current display params!
    PatternHelper::CookPatternSymbology(*params, context);

    hatchTransform.InitFromRowValues(1,0,0, 0, 0,0,0, 0, 0,-1,0, 0);

    DPoint3d    hatchOrigin = origin;

    if (PatternParamsModifierFlags::None == (params->modifiers & PatternParamsModifierFlags::DwgHatchOrigin))
        {
        // Old style DWG Hatch Definitions are implicitly about (0,0)
        params->rMatrix.MultiplyTranspose(hatchOrigin);
        hatchOrigin.x = hatchOrigin.y = 0.0;
        params->rMatrix.Multiply(hatchOrigin);
        }

    baseTransform.InitFrom(params->rMatrix, hatchOrigin);
    invBaseTransform.InverseOf(baseTransform);

    boundGpa->Transform(&invBaseTransform);

    double  annotationScale = PatternHelper::GetAnnotationScale(*params, context);

    PatternHelper::ProcessDWGHatchBoundary(boundGpa, context, baseTransform, hatchTransform, annotationScale, &params->dwgHatchDef.hatchLines.front(), (int) params->dwgHatchDef.hatchLines.size(), context.Is3dView());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sunand.Sandurkar 04/13
+---------------+---------------+---------------+---------------+---------------+------*/
static double GetAnnotationScale(PatternParamsCR params, ViewContextR context)
    {
    if (PatternParamsModifierFlags::None == (PatternParamsModifierFlags::AnnotationScale & params.GetModifiers()))
        return 1.0;

    double  annotationScale = params.GetAnnotationScale();

    return (0.0 == annotationScale ? 1.0 : annotationScale);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sunand.Sandurkar 04/13
+---------------+---------------+---------------+---------------+---------------+------*/
static PatternParamsPtr CookPatternParams(PatternParamsCR params, ViewContextR context)
    {
    PatternParamsPtr  cookedParams = PatternParams::CreateFromExisting(params);
    double            annotationScale = PatternHelper::GetAnnotationScale(*cookedParams, context);

    cookedParams->scale  *= annotationScale;
    cookedParams->space1 *= annotationScale;
    cookedParams->space2 *= annotationScale;

    return cookedParams;
    }

}; // PatternHelper

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::_WantAreaPatterns()
    {
    ViewFlagsCP viewFlags = GetViewFlags();

    if (!viewFlags || !viewFlags->patterns)
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/14
+---------------+---------------+---------------+---------------+---------------+------*/
static void correctPatternOffsetAndRotation(PatternParamsR params, DPoint3dR origin, ViewContext::ClipStencil& boundary)
    {
    if (PatternParamsModifierFlags::None != (params.modifiers & PatternParamsModifierFlags::Origin) ||
        PatternParamsModifierFlags::None != (params.modifiers & PatternParamsModifierFlags::DwgHatchOrigin))
        return; // Explicit origin supplied, assume PatternParam information is good (ex. cut section). Normal patterns specify an offset, not an origin...

    CurveVectorPtr  curves = boundary.GetCurveVector();

    if (!curves.IsValid())
        return;

    // TR#261083 - We didn't store good pattern offsets for complex shapes, grouped holes, and assoc regions. Offset was from the
    //             lower left corner of range and might not lie in the plane of the geometry. Getting the CurveVector now to check
    //             the origin is ok (since it's cached) and even though we don't have the legacy complex element issue anymore, the
    //             PatternParams could still be from an OvrMatSymb where the offset/rotation aren't explicitly set per-boundary.
    DVec3d      planeNormal;
    DPoint3d    planePt;

    if (!curves->GetStartPoint(planePt))
        return;

    if (PatternParamsModifierFlags::None == (params.modifiers & PatternParamsModifierFlags::RotMatrix))
        {
        Transform   localToWorld;

        if (curves->GetAnyFrenetFrame(localToWorld))
            {
            localToWorld.GetMatrix(params.rMatrix);
            params.rMatrix.SquareAndNormalizeColumns(params.rMatrix, 2, 0);
            }
        }

    double      t;
    DVec3d      diff, rtmp;

    params.rMatrix.GetColumn(planeNormal, 2);
    diff.DifferenceOf(planePt, origin);
    t = diff.DotProduct(planeNormal);
    rtmp.Scale(planeNormal, t);
    origin.SumOf(rtmp, origin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawAreaPattern(ClipStencil& boundary)
    {
    if (_CheckStop())
        return;

    if (!_WantAreaPatterns())
        return;

    PatternParamsCP params = m_ovrMatSymb.GetPatternParams();

    if (nullptr == params)
        params = m_elemMatSymb.GetPatternParams();

    if (nullptr == params)
        return;

    DPoint3d    origin = params->origin;

    // Can greatly speed up fit calculation by just drawing boundary...
    if (DrawPurpose::FitView == GetDrawPurpose())
        return boundary.GetStroker()._StrokeForCache(*this);

    IPickGeom*  pickGeom = GetIPickGeom();
    GeomDetailP detail = pickGeom ? &pickGeom->_GetGeomDetail() : NULL;
    bool        wasSnappable = true;

    if (NULL != detail)
        {
        wasSnappable = detail->IsSnappable();
        detail->SetNonSnappable(PatternParamsModifierFlags::None == (params->modifiers & PatternParamsModifierFlags::Snap));
        }

    PatternParamsPtr cookedParams = PatternHelper::CookPatternParams(*params, *this);

    if (Is3dView())
        correctPatternOffsetAndRotation(*cookedParams, origin, boundary);

    // Use mark to keep track of number of CoordSys to pop...
    ViewContext::ContextMark mark(this);

    /* NOTE: If parity isn't used then pattern won't match fill display. This means we can't use
             a stencil. QV allows for parity or 0 winding rule, neither of which matches old pattern
             display, which in gpa case made use of booleans to subtract holes.
             useParity = (0 != (cookedParams->modifiers & PatternParamsModifierFlags::HoleStyle) && PatternParamsHoleStyleType::Parity == cookedParams->holeStyle) */

    if (PatternParamsModifierFlags::None != (cookedParams->modifiers & PatternParamsModifierFlags::Cell))
        PatternHelper::ProcessAreaPattern(*this, boundary, cookedParams.get(), origin, m_patternScale);
    else if (PatternParamsModifierFlags::None != (cookedParams->modifiers & PatternParamsModifierFlags::DwgHatchDef))
        PatternHelper::ProcessDWGHatchPattern(*this, boundary, cookedParams.get(), origin);
    else
        PatternHelper::ProcessHatchPattern(*this, boundary, cookedParams.get(), origin);

    if (NULL != detail)
        detail->SetNonSnappable(!wasSnappable);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::WantAreaPatterns() {return _WantAreaPatterns();}
void ViewContext::DrawAreaPattern(ClipStencil& boundary) {_DrawAreaPattern(boundary);}
