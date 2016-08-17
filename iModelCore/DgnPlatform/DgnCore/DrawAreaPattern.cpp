/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DrawAreaPattern.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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

    return ((mag > 1.0e-5) ? mag : 1.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void PatternParams::ApplyTransform(TransformCR transform)
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

        if (!DoubleOps::WithinTolerance(1.0, scale, 1.0e-5))
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

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
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
        context.GetIDrawGeom().AddPolyface(*polyface, true);

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
        context.GetIDrawGeom().AddPolyface(*polyface, true);
        }

    return true;
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

            cellTrans.TranslateInLocalCoordinates(orgTrans, patOrg.x/scale, patOrg.y/scale, context.GetCurrentGeometryParams().GetNetDisplayPriority());
            cellTrans.Multiply(tileCorners, cellCorners, 8);

            if (ClipPlaneContainment_StronglyOutside == context.GetTransformClipStack().ClassifyPoints(tileCorners, 8))
                continue;

            if (drawFiltered)
                {
                DPoint3d    tmpPt;

                cellTrans.GetTranslation(tmpPt);
                context.GetCurrentGraphicR().AddPointString(1, &tmpPt, NULL);
                }
            else
                {
                context.DrawSymbol(&symbCell, &cellTrans, NULL);
                }

            wasAborted = context.WasAborted();
            }
        }

    return wasAborted;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void ProcessAreaPattern(ViewContextR context, Render::GraphicBuilderR graphic, Render::GeometryParamsCR params, CurveVectorCR boundary)
    {
#if defined (NEEDSWORK_REVISIT_PATTERN_SYMBOLS_SCDEF)
    PatternParamsCR pattern = *params.GetPatternParams();
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
    symbCell.ApplyGeometryParams(*context.GetCurrentGeometryParams());

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
static int DrawHatchGPA(ViewContextR context, Render::GraphicBuilderR graphic, Render::GeometryParamsCR params, GPArrayP pGPA)
    {
    size_t        nGot, sourceCount = pGPA->GetGraphicsPointCount();
    DPoint3d      localPoints[MAX_GPA_STROKES];
    bool          is3d = context.Is3dView();
    double        priority = params.GetNetDisplayPriority();
    GraphicsPoint gp;

    for (size_t i=0; i < sourceCount;)
        {
        if (0 == (i % 100) && context.CheckStop())
            return ERROR;

        nGot = 0;
        
        while (pGPA->GetGraphicsPoint(i, gp))
            {
            gp.point.GetProjectedXYZ(localPoints[nGot++]);

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
            graphic.AddLineString((int) nGot, localPoints);
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

            graphic.AddLineString2d((int) nGot, &localPoints2dBuf[0], priority);
            }
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
static GPArrayP GetBoundaryGPA(CurveVectorCR boundary, RotMatrixCR rMatrix, DPoint3dCR origin, bool useRange)
    {
    GPArrayP    gpa = GPArray::Grab();

    if (!useRange)
        {
        gpa->Add(boundary);

        return gpa;
        }

    DPoint3d    shapePts[5];

    GetBoundaryShapePts(shapePts, boundary, rMatrix, origin);

    gpa->Add(shapePts, 5);
    gpa->MarkBreak();
    gpa->MarkMajorBreak();

    return gpa;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void GetHatchLineLimitTransform(TransformR scaledTransform, TransformCR hatchTransform, GPArrayP boundGpa)
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
static void AddHatchLinesToGPA(GPArrayP hatchGpa, GPArrayP boundGpa, TransformR scaledTransform)
    {
    jmdlGraphicsPointArray_addTransformedCrossHatchClipped(hatchGpa, boundGpa, &scaledTransform, nullptr, nullptr, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void ProcessHatchBoundary(ViewContextR context, Render::GraphicBuilderR graphic, Render::GeometryParamsCR params, GPArrayP boundGpa, TransformR baseTransform, TransformR hatchTransform, double angle, double space)
    {
    DVec3d      xVec,yVec, zVec;
    DPoint3d    origin;

    hatchTransform.GetOriginAndVectors(origin, xVec, yVec, zVec);
    xVec.Init(cos(angle), sin(angle), 0.0);
    zVec.CrossProduct(xVec, yVec);
    zVec.Scale(space);
    hatchTransform.InitFromOriginAndVectors(origin, xVec, yVec, zVec);

    Transform       scaledTransform;
    GPArraySmartP   hatchGpa;

    GetHatchLineLimitTransform(scaledTransform, hatchTransform, boundGpa);
    AddHatchLinesToGPA(hatchGpa, boundGpa, scaledTransform);

    hatchGpa->Transform(&baseTransform);

    PatternHelper::DrawHatchGPA(context, graphic, params, hatchGpa);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void ProcessHatchPattern(ViewContextR context, Render::GraphicBuilderR graphic, Render::GeometryParamsCR params, CurveVectorCR boundary)
    {
    PatternParamsCR pattern = *params.GetPatternParams();
    GPArraySmartP   boundGpa(PatternHelper::GetBoundaryGPA(boundary, pattern.GetOrientation(), pattern.GetOrigin(), false));

    if (nullptr == boundGpa || 0 == boundGpa->GetCount())
        return;

    Transform baseTransform, invBaseTransform;
    Transform hatchTransform;

    hatchTransform.InitFromRowValues(1,0,0, 0, 0,0,0, 0, 0,-1,0, 0);
    baseTransform.InitFrom(pattern.GetOrientation(), pattern.GetOrigin());
    invBaseTransform.InverseOf(baseTransform);

    boundGpa->Transform(&invBaseTransform);

    PatternHelper::ProcessHatchBoundary(context, graphic, params, boundGpa, baseTransform, hatchTransform, pattern.GetPrimaryAngle(), pattern.GetPrimarySpacing());

    if (0.0 != pattern.GetSecondarySpacing())
        PatternHelper::ProcessHatchBoundary(context, graphic, params, boundGpa, baseTransform, hatchTransform, pattern.GetSecondaryAngle(), pattern.GetSecondarySpacing());
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
static void ProcessDWGHatchBoundary(ViewContextR context, Render::GraphicBuilderR graphic, Render::GeometryParamsCR params, GPArrayP boundGpa, TransformR baseTransform, TransformR hatchTransform, DwgHatchDefLine const* hatchLines, int nDefLines)
    {
    DRange3d    boundRange;

    bsiDRange3d_init(&boundRange);
    jmdlDRange3d_extendByGraphicsPointArray(&boundRange, boundGpa);

    if (!context.Is3dView())
        boundRange.low.z = boundRange.high.z = 0.0;

    StatusInt       status = SUCCESS;
    double          rangeDiagonal = boundRange.low.Distance(boundRange.high);
    GPArraySmartP   hatchGpa, dashGpa;

    // NOTE: In ACAD hatch definitions both the base angle and scale have already been applied to the definitions and MUST not be applied again!
    for (DwgHatchDefLine const* lineP = hatchLines; SUCCESS == status && lineP < &hatchLines[nDefLines]; lineP++)
        {
        if (!IsValidPatternDefLine(lineP, rangeDiagonal))
            continue;

        DVec3d      xVec, yVec, zVec;
        DPoint3d    origin;

        hatchTransform.GetOriginAndVectors(origin, xVec, yVec, zVec);
        xVec.Init(cos(lineP->m_angle), sin(lineP->m_angle), 0.0);
        zVec.Init(lineP->m_offset.x, lineP->m_offset.y, 0.0);
        origin.Init(lineP->m_through.x, lineP->m_through.y, 0.0);
        hatchTransform.InitFromOriginAndVectors(origin, xVec, yVec, zVec);

        Transform   scaledTransform;

        GetHatchLineLimitTransform(scaledTransform, hatchTransform, boundGpa);
        AddHatchLinesToGPA(hatchGpa, boundGpa, scaledTransform);

        if (0 != lineP->m_nDashes)
            {
            // NOTE: Copy of jmdlGraphicsPointArray_expandDashPattern to avoid s_maxCollectorPoint limit...
            int         i1;
            DPoint4d    point0, point1;
            int         curveType;
            double      dashPeriod = jmdlGPA_computeDashPeriod(lineP->m_dashes, lineP->m_nDashes);

            for (int i0 = 0; SUCCESS == status && jmdlGraphicsPointArray_parseFragment(hatchGpa, &i1, &point0, &point1, &curveType, i0); i0 = i1 + 1)
                {
                if (0 == curveType)
                    {
                    GraphicsPoint   gp0, gp1;

                    jmdlGraphicsPointArray_getGraphicsPoint(hatchGpa, &gp0, i0);

                    for (int i = i0 + 1; SUCCESS == status && i <= i1; i++, gp0 = gp1)
                        {
                        jmdlGraphicsPointArray_getGraphicsPoint(hatchGpa, &gp1, i);
                        jmdlGPA_expandSingleLineDashPattern(dashGpa, &gp0, &gp1, lineP->m_dashes, lineP->m_nDashes, dashPeriod, MAX_LINE_DASHES);

                        dashGpa->Transform(&baseTransform);
                        status = PatternHelper::DrawHatchGPA(context, graphic, params, dashGpa);
                        dashGpa->Empty();
                        }
                    }
                else
                    {
                    jmdlGraphicsPointArray_appendFragment(dashGpa, hatchGpa, i0, i1, 0);
                    jmdlGraphicsPointArray_markBreak(dashGpa);

                    dashGpa->Transform(&baseTransform);
                    status = PatternHelper::DrawHatchGPA(context, graphic, params, dashGpa);
                    dashGpa->Empty();
                    }
                }
            }
        else
            {
            hatchGpa->Transform(&baseTransform);
            status = PatternHelper::DrawHatchGPA(context, graphic, params, hatchGpa);
            }

        hatchGpa->Empty();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static void ProcessDWGHatchPattern(ViewContextR context, Render::GraphicBuilderR graphic, Render::GeometryParamsCR params, CurveVectorCR boundary)
    {
    PatternParamsCR pattern = *params.GetPatternParams();
    GPArraySmartP   boundGpa(PatternHelper::GetBoundaryGPA(boundary, pattern.GetOrientation(), pattern.GetOrigin(), false));

    if (nullptr == boundGpa || 0 == boundGpa->GetCount())
        return;

    Transform baseTransform, invBaseTransform;
    Transform hatchTransform;

    hatchTransform.InitFromRowValues(1,0,0, 0, 0,0,0, 0, 0,-1,0, 0);
    baseTransform.InitFrom(pattern.GetOrientation(), pattern.GetOrigin());
    invBaseTransform.InverseOf(baseTransform);
    boundGpa->Transform(&invBaseTransform);

    PatternHelper::ProcessDWGHatchBoundary(context, graphic, params, boundGpa, baseTransform, hatchTransform, &pattern.GetDwgHatchDef().front(), (int) pattern.GetDwgHatchDef().size());
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
    return GetViewFlags().m_patterns;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawAreaPattern(Render::GraphicBuilderR graphic, CurveVectorCR boundary, Render::GeometryParamsR params)
    {
    // NEEDSWORK_PATTERNS: This is all just temporary to see if the PatternParams is being converted from V8 ok...
    //                     We'll want display patterns as geometry maps all the time (at least in 3d), but I'm waiting
    //                     to see what mesh-tiles/multi-threading QvElem creation does to drawing, ex. removal of ViewContext, etc.
    //                     For now the "dropped" pattern geometry is just being added directly to the graphic.
    if (_CheckStop() || !_WantAreaPatterns() || !boundary.IsAnyRegionType())
        return;

    PatternParamsCP pattern;

    if (nullptr == (pattern = params.GetPatternParams()))
        return;

    // Can greatly speed up fit calculation by just drawing boundary...
    if (DrawPurpose::FitView == GetDrawPurpose())
        {
        graphic.AddCurveVector(boundary, false);
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

    Render::GeometryParams cookedParams(params);

    bool changed = PatternHelper::Cook(*this, graphic, cookedParams);

    if (Is3dView())
        PatternHelper::AdjustOrigin(*cookedParams.GetPatternParamsP(), boundary);

    if (pattern->GetSymbolId().IsValid())
        PatternHelper::ProcessAreaPattern(*this, graphic, cookedParams, boundary);
    else if (0 != pattern->GetDwgHatchDef().size())
        PatternHelper::ProcessDWGHatchPattern(*this, graphic, cookedParams, boundary);
    else
        PatternHelper::ProcessHatchPattern(*this, graphic, cookedParams, boundary);

    if (changed)
        CookGeometryParams(params, graphic); // Restore original symbology...

    if (nullptr != detail)
        detail->SetNonSnappable(!wasSnappable);
    }

