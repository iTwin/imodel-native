/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#define     MAX_GPA_STROKES         1000
#define     MAX_GPA_HATCH_LINES     10000
#define     MAX_LINE_DASHES         100000  // NOTE: This used to be the limit for all the dashes, not just the dashes per line so I don't expect to hit it...
#define     MAX_HATCH_ITERATIONS    50000
#define     MAX_AREA_PATTERN_TILES  1E6     // pre-Athens...which can really slow things down; create warning uses 10000...  Bmped back up to 1E6 with introduction of geometry map patterning - RayB 7/2013.

#define     TOLERANCE_ChoordAngle   .4
#define     TOLERANCE_ChoordLen     1000

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     02/93
+---------------+---------------+---------------+---------------+---------------+------*/
static double transformPatternSpace(double oldSpace, RotMatrixCR patRot, double angle, TransformCR transform)
    {
    DVec3d      yDir;
    RotMatrix   tmpRot, angRot;

    if (0.0 != angle)
        {
        angRot.InitFromPrincipleAxisRotations(RotMatrix::FromIdentity(), 0.0, 0.0, angle);
        tmpRot.InitProduct(patRot, angRot);
        }
    else
        {
        tmpRot = patRot;
        }

    tmpRot.GetColumn(yDir, 1);
    yDir.Scale(yDir, oldSpace);
    transform.MultiplyMatrixOnly(yDir, yDir);

    return yDir.Magnitude();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     02/93
+---------------+---------------+---------------+---------------+---------------+------*/
static double getTransformPatternScale(TransformCR transform)
    {
    DVec3d xDir;

    transform.GetMatrixColumn(xDir, 0);

    double mag = xDir.Magnitude ();

    return ((mag > 1.0e-10) ? mag : 1.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void PatternParams::ApplyTransform(TransformCR transform, uint32_t options)
    {
    if (m_symbolId.IsValid())
        {
        m_space1 = transformPatternSpace(m_space1, m_rMatrix, m_angle1, transform);
        m_space2 = transformPatternSpace(m_space2, m_rMatrix, m_angle2, transform);
        m_scale *= getTransformPatternScale(transform);
        }
    else if (0 != m_hatchLines.size())
        {
        double scale = getTransformPatternScale(transform);

        if (!DoubleOps::WithinTolerance(1.0, scale, 1.0e-10))
            {
            m_scale *= scale;

            for (DwgHatchDefLine& line : m_hatchLines)
                {
                line.m_through.x *= scale;
                line.m_through.y *= scale;

                line.m_offset.x *= scale;
                line.m_offset.y *= scale;

                for (short iDash = 0; iDash < line.m_nDashes; iDash++)
                    line.m_dashes[iDash] *= scale;
                }
            }
        }
    else
        {
        m_space1 = transformPatternSpace(m_space1, m_rMatrix, m_angle1, transform);

        if (0.0 != m_space2)
            m_space2 = transformPatternSpace(m_space2, m_rMatrix, m_angle2, transform);
        }

    transform.Multiply(m_origin);
    m_rMatrix.InitProduct(transform, m_rMatrix);
    m_rMatrix.SquareAndNormalizeColumns(m_rMatrix, 0, 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     BrienBastings   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus PatternParams::FromJson(Json::Value const& json)
    {
    if (!json["origin"].isNull())
        {
        DPoint3d origin;

        JsonUtils::DPoint3dFromJson(origin, json["origin"]);
        SetOrigin(origin);
        }

    if (!json["rotation"].isNull())
        {
        YawPitchRollAngles angles = JsonUtils::YawPitchRollFromJson(json["rotation"]);
        RotMatrix rMatrix = angles.ToRotMatrix();

        SetOrientation(rMatrix);
        }

    if (!json["space1"].isNull())
        SetPrimarySpacing(json["space1"].asDouble());

    if (!json["space2"].isNull())
        SetSecondarySpacing(json["space2"].asDouble());

    if (!json["angle1"].isNull())
        SetPrimaryAngle(JsonUtils::ToAngle(json["angle1"]).Radians());

    if (!json["angle2"].isNull())
        SetSecondaryAngle(JsonUtils::ToAngle(json["angle2"]).Radians());

    if (!json["scale"].isNull())
        SetScale(json["scale"].asDouble());

    if (!json["color"].isNull())
        SetColor(ColorDef(json["color"].asUInt()));

    if (!json["weight"].isNull())
        SetWeight(json["weight"].asUInt());

    SetInvisibleBoundary(json["invisibleBoundary"].asBool());
    SetSnappable(json["snappable"].asBool());

    if (!json["symbolId"].isNull())
        {
        DgnGeometryPartId symbolId;
        symbolId.FromJson(json["symbolId"]);
        if (symbolId.IsValid())
            SetSymbolId(symbolId);
        }

    if (!json["defLines"].isNull() && json["defLines"].isArray())
        {
        bvector<DwgHatchDefLine> defLines;
        uint32_t nDefLines = (uint32_t) json["defLines"].size();

        for (uint32_t i=0; i < nDefLines; i++)
            {
            DwgHatchDefLine line;

            line.m_angle = JsonUtils::ToAngle(json["defLines"][i]["angle"]).Radians();
            JsonUtils::DPoint2dFromJson(line.m_through, json["defLines"][i]["through"]);
            JsonUtils::DPoint2dFromJson(line.m_offset, json["defLines"][i]["offset"]);
            line.m_nDashes = std::min((short) json["defLines"][i]["dashes"].size(), (short) MAX_DWG_HATCH_LINE_DASHES);

            for (short iDash=0; iDash < line.m_nDashes; iDash++)
                line.m_dashes[iDash] = json["defLines"][i]["dashes"][iDash].asDouble();

            defLines.push_back(line);
            }

        SetDwgHatchDef(defLines);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                     BrienBastings   04/18
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value PatternParams::ToJson() const
    {
    Json::Value value;

    if (!m_origin.IsEqual(DPoint3d::FromZero()))
        JsonUtils::DPoint3dToJson(value["origin"], m_origin);

    if (!m_rMatrix.IsIdentity())
        {
        YawPitchRollAngles angles;
        YawPitchRollAngles::TryFromRotMatrix(angles, m_rMatrix); // Is this ok?
        value["rotation"] = JsonUtils::YawPitchRollToJson(angles);
        }

    if (0.0 != m_space1)
        value["space1"] = Json::Value(m_space1);

    if (0.0 != m_space2)
        value["space2"] = Json::Value(m_space2);

    if (0.0 != m_angle1)
        value["angle1"] = JsonUtils::FromAngle(Angle::FromRadians(m_angle1));

    if (0.0 != m_angle2)
        value["angle2"] = JsonUtils::FromAngle(Angle::FromRadians(m_angle2));

    if (0.0 != m_scale && 1.0 != m_scale)
        value["scale"] = Json::Value(m_scale);

    if (m_useColor)
        value["color"] = Json::Value(m_color.GetValue());

    if (m_useWeight)
        value["weight"] = Json::Value(m_weight);

    if (m_invisibleBoundary)
        value["invisibleBoundary"] = Json::Value(m_invisibleBoundary);

    if (m_snappable)
        value["snappable"] = Json::Value(m_snappable);

    if (m_symbolId.IsValid())
        value["symbolId"] = m_symbolId.ToHexStr();

    if (!m_hatchLines.empty())
        {
        for (auto const& line : m_hatchLines)
            {
            Json::Value defLine;

            if (0.0 != line.m_angle)
                defLine["angle"] = JsonUtils::FromAngle(Angle::FromRadians(line.m_angle));

            if (!line.m_through.IsEqual(DPoint2d::FromZero()))
                JsonUtils::DPoint2dToJson(defLine["through"], line.m_through);

            if (!line.m_offset.IsEqual(DPoint2d::FromZero()))
                JsonUtils::DPoint2dToJson(defLine["offset"], line.m_offset);

            for (int iDash=0; iDash < line.m_nDashes; ++iDash)
                defLine["dashes"].append(Json::Value(line.m_dashes[iDash]));

            value["defLines"].append(defLine);
            }
        }

    return value;
    }

/*=================================================================================**//**
* @bsiclass                                                     BrienBastings   11/07
+===============+===============+===============+===============+===============+======*/
struct PatternHelper
{
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void AdjustOrigin(PatternParamsR pattern, CurveVectorCR boundary)
    {
    DVec3d      planeNormal;
    DPoint3d    planePt;

    if (!boundary.GetStartPoint(planePt))
        return;

    double      t;
    DVec3d      diff, rtmp;
    DPoint3d    origin = pattern.GetOrigin();

    pattern.GetOrientation().GetColumn(planeNormal, 2);
    diff.DifferenceOf(planePt, origin);
    t = diff.DotProduct(planeNormal);
    rtmp.Scale(planeNormal, t);
    origin.SumOf(rtmp, origin);
    pattern.SetOrigin(origin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static bool GetSymbolTileInfo
(
CurveVectorCR       boundary,
DgnGeometryPartCR   symbol,
DRange3dR           symbolRange,
DPoint2dR           symbolOrg,
DPoint2dR           spacing,
DPoint2dR           low,
DPoint2dR           high,
bool&               isPlanar,
DPoint3dCR          origin,
RotMatrixCR         rMatrix,
double&             scale,
double              rowSpacing,
double              columnSpacing
)
    {
    symbolRange = symbol.GetBoundingBox();

    if (symbolRange.IsNull())
        return false;

    if (columnSpacing < 0.0)
        spacing.x = -columnSpacing;
    else
        spacing.x = columnSpacing + scale * (symbolRange.high.x - symbolRange.low.x);

    if (rowSpacing < 0.0)
        spacing.y = -rowSpacing;
    else
        spacing.y = rowSpacing + scale * (symbolRange.high.y - symbolRange.low.y);

    if (spacing.x < 1.0e-5 || spacing.y < 1.0e-5)
        return false;

    RotMatrix   invMatrix;
    Transform   transform;

    invMatrix.InverseOf(rMatrix);
    transform.InitFrom(invMatrix);
    transform.TranslateInLocalCoordinates(transform, -origin.x, -origin.y, -origin.z);

    DRange3d    localRange;

    boundary.GetRange(localRange, transform);

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

    symbolOrg.x = symbolRange.low.x * scale;
    symbolOrg.y = symbolRange.low.y * scale;

    low.x -= symbolOrg.x;
    low.y -= symbolOrg.y;

    high.x -= symbolOrg.x;
    high.y -= symbolOrg.y;

    double  numTiles = ((high.x - low.x) / spacing.x) * ((high.y - low.y) / spacing.y);

    if (numTiles > MAX_AREA_PATTERN_TILES)
        {
        double  factor = (numTiles / MAX_AREA_PATTERN_TILES) * 0.5;

        // NOTE: Increase pattern scale and display *something* instead of useless message center alert...
        spacing.Scale(spacing, factor);
        scale *= factor;
        }

    // Can't use a stencil for a non-planar pattern cell....
    isPlanar = ((symbolRange.high.z - symbolRange.low.z) < 1.0e-5);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void GetSymbolOrientationAndScale
(
RotMatrixR      rMatrix,
double&         scale,
RotMatrixCR     patternRMatrix,
double          patternAngle,
double          patternScale
)
    {
    RotMatrix   angleRot;
    
    angleRot.InitFromAxisAndRotationAngle(2, patternAngle);
    rMatrix.InitProduct(patternRMatrix, angleRot);
    scale = patternScale ? patternScale : 1.0;
    }

#if defined (NEEDSWORK_PATTERN_GEOMETRY_MAP)
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
    symbCell.ApplyGeometryParams(*context.GetCurrentGeometryParams());

    DRange2d range;
    DisplayHandler::GetDPRange(&range.low, &eh.GetElementCP ()->hdr.dhdr.range);
    range.high.x = range.low.x + spacing.x;
    range.high.y = range.low.y + spacing.y;

    context.GetIViewDraw().DefineQVGeometryMap(*pMaterial, symbCell.GetElemHandle(), range, !symbCell.IsPointCellSymbol(), context, true);

    return pMaterial;
    }

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
    static      DgnElementAppData::Key s_appDataKey;

    if (DrawPurpose::Plot == context.GetDrawPurpose())      // Opt for slower, higher quality when plotting.
        return false;

#if !defined (BENTLEYCONFIG_GRAPHICS_OPENGLES)  //  We always want to use geometry map with OpenGL ES because the our OpenGL implementation of PushClipStencil does not work
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
    if (symbCell.IsPointCellSymbol() && PatternParamsModifierFlags::None != (params->modifiers & PatternParamsModifierFlags::Color) && context.GetCurrentGeometryParams()->m_symbology.color != params->color)
        {
        // NOTE: Don't need to worry about overrides, context overrides CAN NOT look at m_currDisplayParams, so changing line color doesn't affect them...
        context.GetCurrentGeometryParams()->SetLineColor(params->color);
        context.CookGeometryParams();
        }

    OvrGraphicParamsP  ovrMatSymb = context.GetOverrideGraphicParams();

    ovrMatSymb->SetFillTransparency(0xff);
    ovrMatSymb->SetMaterial(appData->GetMaterial());
    context.GetIDrawGeom().ActivateOverrideGraphicParams(ovrMatSymb);

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
        context.GetIDrawGeom().AddPolyfaceR(*polyface, true);

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
        context.GetIDrawGeom().AddPolyfaceR(*polyface, true);
        }

    return true;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static bool DrawCellTiles(ViewContextR context, Render::GraphicBuilderR graphic, Render::GeometryParamsR params, CurveVectorCR boundary, DgnGeometryPartCR symbol, DPoint2dCR low, DPoint2dCR high, DPoint2dCR spacing, double scale, TransformCR orgTrans, DPoint3dCP symbolCorners)
    {
    auto tol = context.GetAreaPatternTolerance(boundary);
    ClipVectorPtr clip = ClipVector::CreateFromCurveVector(boundary, tol.GetChordTolerance(), tol.GetAngleTolerance());

    if (!clip.IsValid())
        return false;

    DgnViewportP     vp = context.GetViewport();
    bool             wasAborted = false;
    DPoint2d         patOrg;
    DPoint3d         tileCorners[8];
    Transform        symbolTrans;
    GraphicParams    graphicParams;
    PatternParamsPtr pattern = params.GetPatternParamsP();

    // NOTE: Need to cook GeometryParams to get GraphicParams, but we don't want to activate and bake into our QvElem...
    params.SetPatternParams(nullptr); // Need to clear so that we don't attempt to apply pattern to shapes in pattern symbol...
    context.CookGeometryParams(params, graphicParams);

    for (patOrg.x = low.x; patOrg.x < high.x && !wasAborted; patOrg.x += spacing.x)
        {
        for (patOrg.y = low.y; patOrg.y < high.y && !wasAborted; patOrg.y += spacing.y)
            {
            symbolTrans.TranslateInLocalCoordinates(orgTrans, patOrg.x/scale, patOrg.y/scale, 0.0); // NOTE: Don't supply net display priority, will be supplied by add calls...
            symbolTrans.Multiply(tileCorners, symbolCorners, 8);

            if (vp)
                {
                Transform symbolToWorld = Transform::FromProduct(graphic.GetLocalToWorldTransform(), symbolTrans);
                ElementAlignedBox3d range = symbol.GetBoundingBox();

                symbolToWorld.Multiply(range, range);

                if (!context.IsRangeVisible(range))
                    continue; // Symbol range doesn't overlap pick...
                }

            ClipPlaneContainment containment = clip->ClassifyPointContainment(tileCorners, 8); 

            if (ClipPlaneContainment_StronglyOutside == containment)
                continue;

            GeometryStreamIO::Collection collection(symbol.GetGeometryStream().GetData(), symbol.GetGeometryStream().GetSize());
            GraphicBuilderPtr partBuilder = graphic.CreateSubGraphic(symbolTrans, containment == ClipPlaneContainment_StronglyInside ? nullptr : clip.get());

            collection.Draw(*partBuilder, context, params, false, &symbol);

            if (wasAborted = context.WasAborted())
                break;

            graphic.AddSubGraphic(*partBuilder->Finish(), symbolTrans, graphicParams, containment == ClipPlaneContainment_StronglyInside ? nullptr : clip.get());
            }
        }

    params.SetPatternParams(pattern.get());

    return wasAborted;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void ProcessAreaPattern(ViewContextR context, Render::GraphicBuilderR graphic, Render::GeometryParamsR params, CurveVectorCR boundary)
    {
    PatternParamsCR pattern = *params.GetPatternParams();
    DgnGeometryPartCPtr symbolGeometry = context.GetDgnDb().Elements().Get<DgnGeometryPart>(pattern.GetSymbolId());

    if (!symbolGeometry.IsValid())
        return;

    double      scale;
    DPoint3d    origin = pattern.GetOrigin();
    RotMatrix   rMatrix;

    PatternHelper::GetSymbolOrientationAndScale(rMatrix, scale, pattern.GetOrientation(), pattern.GetPrimaryAngle(), pattern.GetScale());

    bool        isPlanar;
    DPoint2d    symbolOrg, spacing, low, high;
    DRange3d    symbolRange;

    if (!PatternHelper::GetSymbolTileInfo(boundary, *symbolGeometry, symbolRange, symbolOrg, spacing, low, high, isPlanar, origin, rMatrix, scale, pattern.GetPrimarySpacing(), pattern.GetSecondarySpacing()))
        return;

#if defined (NEEDSWORK_PATTERN_GEOMETRY_MAP)
    bool        isQVOutput = context.GetIViewDraw().IsOutputQuickVision();
    bool        useStencil = isQVOutput && isPlanar && !context.CheckICachedDraw(); // Can't use stencil if creating QvElem...dimension terminators want patterns!
    QvElem*     qvElem = NULL;

    if (useStencil && PatternHelper::ProcessAreaPatternAsGeometryMap(context, boundary, symbCell, params, rMatrix, origin, spacing, scale))
        return;

    if (useStencil && !PatternHelper::PushBoundaryClipStencil(context, boundary, qvElem))
        return;
#endif

    Transform   orgTrans;
    DPoint3d    symbolCorners[8];

    // Setup initial pattern instance transform
    LegacyMath::TMatrix::ComposeOrientationOriginScaleXYShear(&orgTrans, NULL, &rMatrix, &origin, scale, scale, 0.0);
    symbolRange.Get8Corners(symbolCorners);

    // NOTE: Union regions aren't valid clip boundaries, need to push separate clip for each solid area...
    if (boundary.IsUnionRegion())
        {
        for (ICurvePrimitivePtr curve : boundary)
            {
            if (curve.IsNull() || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType())
                continue;

            // NOTE: Cell tile exclusion check makes sending all tiles for each solid area less offensive (union regions also fairly rare)...
            if (DrawCellTiles(context, graphic, params, *curve->GetChildCurveVectorCP(), *symbolGeometry, low, high, spacing, scale, orgTrans, symbolCorners))
                break; // Was aborted...
            }

        return;
        }

    DrawCellTiles(context, graphic, params, boundary, *symbolGeometry, low, high, spacing, scale, orgTrans, symbolCorners);
    }

static void DrawSegment (bool is3d, Render::GraphicBuilderR graphic, double priority, DSegment3dCR segment)
    {
    if (is3d)
        {
        graphic.AddLineString (2, segment.point);
        }
    else
        {
        // To ensure display priority is properly honored in non-rasterized plots, it is necessary to call QuickVision 2D draw methods. TR 180390.
        int nGot = 2;
        DPoint2d localPoints[2];

        for (size_t iPoint = 0; iPoint < nGot; iPoint++)
            {
            localPoints[iPoint].x = segment.point[iPoint].x;
            localPoints[iPoint].y = segment.point[iPoint].y;
            }

        graphic.AddLineString2d (2, localPoints, priority);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static int DrawHatch(
ViewContextR context,
Render::GraphicBuilderR graphic,
Render::GeometryParamsCR params,
TransformCR transform,
bvector<DSegment3d> const &hatch,
size_t &numCheck)
    {
    bool          is3d = context.Is3dView();
    double        priority = params.GetNetDisplayPriority();
    DSegment3d    segmentB;
    for (auto &segmentA : hatch)
        {
        if (0 == (numCheck++ % 100) && context.CheckStop())
            return ERROR;
        transform.Multiply (segmentB.point, segmentA.point, 2);
        DrawSegment (is3d, graphic, priority, segmentB);
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/13
+---------------+---------------+---------------+---------------+---------------+------*/
static void GetBoundaryShapePts(DPoint3dP pts, CurveVectorCR boundary, RotMatrixCR rMatrix, DPoint3dCR origin)
    {
    RotMatrix   invMatrix;
    Transform   transform;

    invMatrix.InverseOf(rMatrix);
    transform.InitFrom(invMatrix);

    DRange3d    localRange;

    boundary.GetRange(localRange, transform);

    pts[0].x = pts[3].x = localRange.low.x;
    pts[1].x = pts[2].x = localRange.high.x;

    pts[0].y = pts[1].y = localRange.low.y;
    pts[2].y = pts[3].y = localRange.high.y;

    DPoint3d    shapeOrigin;

    transform.Multiply(shapeOrigin, origin);

    pts[0].z = pts[1].z = pts[2].z = pts[3].z = shapeOrigin.z;
    pts[4] = pts[0];

    transform.MultiplyTranspose(pts, pts, 5);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void GetHatchLineLimitTransform(TransformR scaledTransform, TransformCR hatchTransform, CurveVectorCR boundary)
    {
    scaledTransform = hatchTransform;
    DRange3d worldRange;
    if (boundary.GetRange (worldRange))
        {
        auto zScale = CurveVector::ComputeHatchDensityScale (hatchTransform, worldRange, MAX_GPA_HATCH_LINES);
        if (zScale.IsValid ())
            {
            RotMatrix   scaleMatrix = RotMatrix::FromScaleFactors (1.0, 1.0, zScale.Value ());
            scaledTransform.InitProduct(hatchTransform, scaleMatrix);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void ProcessHatchBoundary(ViewContextR context, Render::GraphicBuilderR graphic, Render::GeometryParamsCR params, CurveVectorCR boundary, TransformR baseTransform, TransformR hatchTransform, double angle, double space)
    {
    DVec3d      xVec,yVec, zVec;
    DPoint3d    origin;

    hatchTransform.GetOriginAndVectors(origin, xVec, yVec, zVec);
    xVec.Init(cos(angle), sin(angle), 0.0);
    zVec.CrossProduct(xVec, yVec);
    zVec.Scale(space);
    hatchTransform.InitFromOriginAndVectors(origin, xVec, yVec, zVec);

    Transform       scaledTransform;

    GetHatchLineLimitTransform(scaledTransform, hatchTransform, boundary);
    bvector<DSegment3d> hatch;
    CurveVector::CreateHatch (hatch, nullptr, boundary, scaledTransform);
    
    size_t numCheck = 0;
    PatternHelper::DrawHatch (context, graphic, params, baseTransform, hatch, numCheck);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void ProcessHatchPattern(ViewContextR context, Render::GraphicBuilderR graphic, Render::GeometryParamsR params, CurveVectorCR boundary)
    {
    PatternParamsCR pattern = *params.GetPatternParams();

    Transform baseTransform, invBaseTransform;
    Transform hatchTransform;

    hatchTransform.InitFromRowValues(1,0,0, 0, 0,0,0, 0, 0,-1,0, 0);
    baseTransform.InitFrom(pattern.GetOrientation(), pattern.GetOrigin());
    invBaseTransform.InverseOf(baseTransform);

    auto xzBoundary = boundary.Clone (invBaseTransform);

    if (xzBoundary.IsValid ())
        {
        PatternHelper::ProcessHatchBoundary(context, graphic, params, *xzBoundary, baseTransform, hatchTransform, pattern.GetPrimaryAngle(), pattern.GetPrimarySpacing());

        if (0.0 != pattern.GetSecondarySpacing())
            PatternHelper::ProcessHatchBoundary(context, graphic, params, *xzBoundary, baseTransform, hatchTransform, pattern.GetSecondaryAngle(), pattern.GetSecondarySpacing());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/01
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsValidPatternDefLine(DwgHatchDefLine const* lineP, double rangeDiagonal)
    {
    double      offsetMagnitude = lineP->m_offset.Magnitude();

    if (0.0 == offsetMagnitude || rangeDiagonal / offsetMagnitude > MAX_HATCH_ITERATIONS)
        return false;

    if (0 == lineP->m_nDashes)
        return true;

    double      totalDashLength = 0.0;

    for (int i=0; i < lineP->m_nDashes; i++)
        totalDashLength += fabs(lineP->m_dashes[i]);

    return (!(0.0 == totalDashLength || rangeDiagonal / totalDashLength > MAX_HATCH_ITERATIONS));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void ProcessDWGHatchBoundary(ViewContextR context, Render::GraphicBuilderR graphic, Render::GeometryParamsCR params, CurveVectorCR boundary, TransformR baseTransform, TransformR hatchTransform, DwgHatchDefLine const* hatchLines, int nDefLines)
    {
    DRange3d    boundRange;
    boundary.GetRange (boundRange);

    if (!context.Is3dView())
        boundRange.low.z = boundRange.high.z = 0.0;

    StatusInt       status = SUCCESS;
    double          rangeDiagonal = boundRange.low.Distance(boundRange.high);
    bvector<DSegment3d> dashSegments;
    size_t numCheck = 0;
    DashData dashLengths;
    bvector<DSegment3d> hatch;
    bvector<HatchSegmentPosition> positions;
    // NOTE: In ACAD hatch definitions both the base angle and scale have already been applied to the definitions and MUST not be applied again!
    for (DwgHatchDefLine const* lineP = hatchLines; SUCCESS == status && lineP < &hatchLines[nDefLines]; lineP++)
        {
        if (!IsValidPatternDefLine(lineP, rangeDiagonal))
            continue;
        dashLengths.SetDashLengths (lineP->m_dashes, lineP->m_nDashes);

        DVec3d      xVec, yVec, zVec;
        DPoint3d    origin;

        hatchTransform.GetOriginAndVectors(origin, xVec, yVec, zVec);
        xVec.Init(cos(lineP->m_angle), sin(lineP->m_angle), 0.0);
        zVec.Init(lineP->m_offset.x, lineP->m_offset.y, 0.0);
        origin.Init(lineP->m_through.x, lineP->m_through.y, 0.0);
        hatchTransform.InitFromOriginAndVectors(origin, xVec, yVec, zVec);

        Transform   scaledTransform;

        GetHatchLineLimitTransform(scaledTransform, hatchTransform, boundary);

        if (0 != lineP->m_nDashes)
            {
            CurveVector::CreateHatch (hatch, &positions, boundary, scaledTransform);
            dashLengths.AppendDashes (hatch, positions, dashSegments);
            status = PatternHelper::DrawHatch (context, graphic, params, baseTransform, dashSegments, numCheck);
            }
        else
            {
            CurveVector::CreateHatch (hatch, nullptr, boundary, scaledTransform);
            status = PatternHelper::DrawHatch (context, graphic, params, baseTransform, hatch, numCheck);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void ProcessDWGHatchPattern(ViewContextR context, Render::GraphicBuilderR graphic, Render::GeometryParamsR params, CurveVectorCR boundary)
    {
    PatternParamsCR pattern = *params.GetPatternParams();

    Transform baseTransform, invBaseTransform;
    Transform hatchTransform;

    hatchTransform.InitFromRowValues(1,0,0, 0, 0,0,0, 0, 0,-1,0, 0);
    baseTransform.InitFrom(pattern.GetOrientation(), pattern.GetOrigin());
    invBaseTransform.InverseOf(baseTransform);
    auto xzBoundary = boundary.Clone(invBaseTransform);

    PatternHelper::ProcessDWGHatchBoundary(context, graphic, params, *xzBoundary, baseTransform, hatchTransform, &pattern.GetDwgHatchDef().front(), (int) pattern.GetDwgHatchDef().size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sunand.Sandurkar 04/13
+---------------+---------------+---------------+---------------+---------------+------*/
static double GetAnnotationScale(PatternParamsCR params, ViewContextR context) {return 1.0;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sunand.Sandurkar 04/13
+---------------+---------------+---------------+---------------+---------------+------*/
static bool Cook(ViewContextR context, Render::GraphicBuilderR graphic, Render::GeometryParamsR params)
    {
    PatternParamsR pattern = *params.GetPatternParamsP();
    double annotationScale = PatternHelper::GetAnnotationScale(pattern, context);

    if (!DoubleOps::WithinTolerance(1.0, annotationScale, 1.0e-5))
        {
        pattern.SetScale(pattern.GetScale() * annotationScale);
        pattern.SetPrimarySpacing(pattern.GetPrimarySpacing() * annotationScale);
        pattern.SetSecondarySpacing(pattern.GetSecondarySpacing() * annotationScale);

        for (DwgHatchDefLine& line : pattern.GetDwgHatchDefR())
            {
            line.m_through.x *= annotationScale;
            line.m_through.y *= annotationScale;

            line.m_offset.x *= annotationScale;
            line.m_offset.y *= annotationScale;

            for (short iDash = 0; iDash < line.m_nDashes; iDash++)
                line.m_dashes[iDash] *= annotationScale;
            }
        }

    if (!pattern.GetUseColor() && !pattern.GetUseWeight())
        return false;

    if (pattern.GetUseColor())
        params.SetLineColor(pattern.GetColor());

    if (pattern.GetUseWeight())
        params.SetWeight(pattern.GetWeight());

    context.CookGeometryParams(params, graphic);

    return true;
    }

}; // PatternHelper

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewContext::_WantAreaPatterns()
    {
    return GetViewFlags().ShowPatterns();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawAreaPattern(Render::GraphicBuilderR builder, CurveVectorCR boundary, Render::GeometryParamsR params, bool doCook)
    {
    // NEEDSWORK_PATTERN_GEOMETRY_MAP - This is all just temporary to see if the PatternParams is being converted from V8 ok...
    // We'll want display patterns as geometry maps all the time (at least in 3d), but I'm waiting
    // to see what mesh-tiles/multi-threading QvElem creation does to drawing, ex. removal of ViewContext, etc.
    // For now the "dropped" pattern geometry is just being added directly to the graphic.
    // NOTE: It is left up to the caller if they want to call WantAreaPatterns, some decorator might want to display a pattern regardless of the ViewFlags.
    if (_CheckStop() || !boundary.IsAnyRegionType())
        return;

    PatternParamsCP pattern;

    if (nullptr == (pattern = params.GetPatternParams()))
        return;

    if (!builder.WantStrokePattern(*pattern))
        return;

    // Can greatly speed up fit calculation by just drawing boundary...
    if (DrawPurpose::FitView == GetDrawPurpose())
        {
        builder.AddCurveVector(boundary, false);
        return;
        }

    bool        wasSnappable = true;
    IPickGeomP  pickGeom = GetIPickGeom();
    GeomDetailP detail = pickGeom ? &pickGeom->_GetGeomDetail() : nullptr;

    if (nullptr != detail)
        {
        wasSnappable = detail->IsSnappable();
        detail->SetNonSnappable(!pattern->GetSnappable());
        }

    if (doCook)
        CookGeometryParams(params, builder);

    Render::GeometryParams cookedParams(params);

    bool changed = PatternHelper::Cook(*this, builder, cookedParams);

    if (Is3dView())
        PatternHelper::AdjustOrigin(*cookedParams.GetPatternParamsP(), boundary);

    if (pattern->GetSymbolId().IsValid())
        PatternHelper::ProcessAreaPattern(*this, builder, cookedParams, boundary);
    else if (0 != pattern->GetDwgHatchDef().size())
        PatternHelper::ProcessDWGHatchPattern(*this, builder, cookedParams, boundary);
    else
        PatternHelper::ProcessHatchPattern(*this, builder, cookedParams, boundary);

    if (changed)
        CookGeometryParams(params, builder); // Restore original symbology...

    if (nullptr != detail)
        detail->SetNonSnappable(!wasSnappable);
    }

