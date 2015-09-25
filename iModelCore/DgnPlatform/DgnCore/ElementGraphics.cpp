/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ElementGraphics.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  06/09
+===============+===============+===============+===============+===============+======*/
struct ElementGraphicsDrawGeom : public SimplifyViewDrawGeom
{
    DEFINE_T_SUPER(SimplifyViewDrawGeom)
private:

IElementGraphicsProcessor*  m_dropObj;

protected:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _DoClipping() const override {return m_dropObj->_WantClipping();}
virtual bool _DoTextGeometry() const override {return true;}
virtual bool _DoSymbolGeometry() const override {return true;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     06/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual IFacetOptionsP _GetFacetOptions() override
    {
    IFacetOptionsP dropOptions = m_dropObj->_GetFacetOptionsP();

    if (NULL == dropOptions)
        dropOptions = T_Super::_GetFacetOptions();

    return dropOptions;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _ProcessAsFacets(bool isPolyface) const override {return m_dropObj->_ProcessAsFacets(isPolyface);}
virtual bool _ProcessAsBody(bool isCurved) const override {return m_dropObj->_ProcessAsBody(isCurved);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void AnnounceCurrentState()
    {
    ElemMatSymb currentMatSymb;

    GetCurrentMatSymb(currentMatSymb);

    m_dropObj->_AnnounceTransform(m_context->GetCurrLocalToWorldTransformCP());
    m_dropObj->_AnnounceElemMatSymb(currentMatSymb);
    m_dropObj->_AnnounceElemDisplayParams(m_context->GetCurrentDisplayParams());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _ProcessCurvePrimitive(ICurvePrimitiveCR curve, bool isClosed, bool isFilled) override
    {
    AnnounceCurrentState();

    return m_dropObj->_ProcessCurvePrimitive(curve, isClosed, isFilled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _ProcessCurveVector(CurveVectorCR curves, bool isFilled) override
    {
    AnnounceCurrentState();

    return m_dropObj->_ProcessCurveVector(curves, isFilled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _ProcessSolidPrimitive(ISolidPrimitiveCR primitive) override
    {
    AnnounceCurrentState();

    return m_dropObj->_ProcessSolidPrimitive(primitive);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _ProcessSurface(MSBsplineSurfaceCR surface) override
    {
    AnnounceCurrentState();

    return m_dropObj->_ProcessSurface(surface);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _ProcessFacetSet(PolyfaceQueryCR facets, bool filled) override
    {
    AnnounceCurrentState();

    return m_dropObj->_ProcessFacets(facets, filled);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual StatusInt _ProcessBody(ISolidKernelEntityCR entity) override
    {
    AnnounceCurrentState();

    return m_dropObj->_ProcessBody(entity);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawTextString(TextStringCR text, double* zDepth) override
    {
    AnnounceCurrentState();

    if (SUCCESS == m_dropObj->_ProcessTextString(text))
        return;

    // NOTE: Now that we know we are dropping the text, we also want the adornments...
    T_Super::_DrawTextString(text, zDepth);
    text.DrawTextAdornments(*m_context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void ModifyDrawViewFlags(ViewFlagsR flags)
    {
    // Prefer "higher level" geometry from legacy types...
    flags.SetRenderMode(DgnRenderMode::SmoothShade);

    // Make sure linestyles drawn for drop...esp. when dropping linestyles!
    flags.styles = true;

    // Make sure to display fill so that fill/gradient can be added to output...
    flags.fill = true;

    // Make sure to display patterns so that patterns can be added to output...
    flags.patterns = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _SetDrawViewFlags(ViewFlags flags) override
    {
    T_Super::_SetDrawViewFlags(flags);
    ModifyDrawViewFlags(m_viewFlags);
    }

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void Init(ViewContextP context, IElementGraphicsProcessor* dropObj)
    {
    SetViewContext(context);
    m_dropObj = dropObj;
    ModifyDrawViewFlags(m_viewFlags);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
IElementGraphicsProcessor* GetIElementGraphicsProcessor() {return m_dropObj;}

}; // ElementGraphicsDrawGeom

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  06/09
+===============+===============+===============+===============+===============+======*/
struct ElementGraphicsContext : public NullContext
{
    DEFINE_T_SUPER(NullContext)
protected:

ElementGraphicsDrawGeom&    m_output;

virtual void _SetupOutputs() override {SetIViewDraw(m_output);}

public:

ElementGraphicsContext(IElementGraphicsProcessor* dropObj, ElementGraphicsDrawGeom& output) : m_output(output)
    {
    m_purpose = dropObj->_GetDrawPurpose();
    m_wantMaterials = true; // Setup material in ElemDisplayParams in case IElementGraphicsProcessor needs it...

    SetBlockAsynchs(true);
    m_output.Init(this, dropObj);
    _SetupOutputs();

    dropObj->_AnnounceContext(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/2015
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawTextString(TextStringCR text) override
    {
    // NOTE: When IElementGraphicsProcessor handles TextString we don't want to spew adornment geometry!
    text.GetGlyphSymbology(GetCurrentDisplayParams());
    CookDisplayParams();

    double zDepth = GetCurrentDisplayParams().GetNetDisplayPriority();
    GetIDrawGeom().DrawTextString(text, Is3dView() ? nullptr : &zDepth);                
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/12
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawSymbol(IDisplaySymbol* symbolDef, TransformCP trans, ClipPlaneSetP clipPlanes, bool ignoreColor, bool ignoreWeight) override
    {
    // Pass along any symbol that is drawn from _ExpandPatterns/_ExpandLineStyles, etc.
    m_output.ClipAndProcessSymbol(symbolDef, trans, clipPlanes, ignoreColor, ignoreWeight);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _DrawAreaPattern(ClipStencil& boundary) override
    {
    if (!m_output.GetIElementGraphicsProcessor()->_ExpandPatterns())
        return;

    T_Super::_DrawAreaPattern(boundary);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ILineStyleCP _GetCurrLineStyle(LineStyleSymbP* symb) override
    {
    ILineStyleCP  currStyle = T_Super::_GetCurrLineStyle(symb);

    if (!m_output.GetIElementGraphicsProcessor()->_ExpandLineStyles(currStyle))
        return NULL;

    return currStyle;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _CookDisplayParams(ElemDisplayParamsR elParams, ElemMatSymbR elMatSymb) override
    {
    // Apply ignores, resolve effective, and cook ElemMatSymb...
    elParams.Resolve(*this);
    elMatSymb.FromResolvedElemDisplayParams(elParams, *this, m_startTangent, m_endTangent);
    }

}; // ElementGraphicsContext

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/09
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGraphicsOutput::Process(IElementGraphicsProcessorR dropObj, GeometricElementCR element)
    {
    ElementGraphicsDrawGeom output;
    ElementGraphicsContext  context(&dropObj, output);

    context.SetDgnDb(element.GetDgnDb());
    
    if (nullptr != element.ToElement3d())
        {
        // NOTE: The processor only wants curves/edges. Don't output brep polyface when Parasolid isn't
        //       available, output the exact pre-computed wireframe geometry instead. Also better to avoid
        //       creating a ISolidKernelEntity un-neccesarily even when Parasolid is available.
        if (!dropObj._ProcessAsBody(true) && !dropObj._ProcessAsFacets(false))
            {
            if (context.ElementIsUndisplayed(element))
                return;

            context.SetCurrentElement(&element);

            ElementGeometryCollection collection(element);

            collection.SetBRepOutput(ElementGeometryCollection::BRepOutput::Edges | ElementGeometryCollection::BRepOutput::FaceIso);

            for (ElementGeometryPtr elemGeom : collection)
                {
                context.SetGeomStreamEntryId(collection.GetGeomStreamEntryId());

                context.GetCurrentDisplayParams() = collection.GetElemDisplayParams();
                context.CookDisplayParams();

                context.PushTransform(collection.GetGeometryToWorld());
                elemGeom->Draw(context);
                context.PopTransformClip();
                }

            context.SetCurrentElement(nullptr);
            return;
            }
        }
    
    context.VisitElement(element);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  12/11
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementGraphicsOutput::Process(IElementGraphicsProcessorR dropObj, DgnDbR dgnDb)
    {
    ElementGraphicsDrawGeom output;
    ElementGraphicsContext  context(&dropObj, output);

    context.GetCurrentDisplayParams() = ElemDisplayParams();
    context.SetDgnDb(dgnDb);

    dropObj._OutputGraphics(context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static double wireframe_getTolerance(CurveVectorCR curves)
    {
    static double s_minRuleLineTolerance = 1.0e-8;
   
    return  curves.ResolveTolerance(s_minRuleLineTolerance);      // TFS# 24423 - Length calculation can be very slow for B-Curves.  s_defaultLengthRelTol * curves.Length ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void wireframe_addRulePoints(bvector<DPoint3d>& pts, bvector<bool>& interior, DPoint3dCR pt, bool isVertex, double ruleTolerance, bool checkDistance)
    {
    if (checkDistance && 0 != pts.size())
        {
        if (pt.Distance(pts.back()) <= ruleTolerance)
            return; // Don't duplicate previous point...
        else if (pt.Distance(pts.front()) <= ruleTolerance)
            return; // Don't duplicate first point...
        }

    pts.push_back(pt);
    interior.push_back(!isVertex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool wireframe_collectPoints(CurveVectorCR curves, bvector<DPoint3d>& pts, bvector<bool>& interior, double ruleTolerance, bool checkDistance, ViewContextP context)
    {
    if (1 > curves.size())
        return false;

    if (curves.IsUnionRegion() || curves.IsParityRegion())
        {
        for (ICurvePrimitivePtr curve: curves)
            {
            if (curve.IsNull() || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType())
                continue;

            if (context && context->CheckStop())
                return true;

            wireframe_collectPoints(*curve->GetChildCurveVectorCP(), pts, interior, ruleTolerance, checkDistance, context);
            }
        }
    else
        {
        for (ICurvePrimitivePtr curve: curves)
            {
            if (!curve.IsValid())
                continue;

            if (context && context->CheckStop())
                return true;

            switch (curve->GetCurvePrimitiveType())
                {
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                    {
                    DSegment3dCP  segment = curve->GetLineCP();

                    wireframe_addRulePoints(pts, interior, segment->point[0], true, ruleTolerance, checkDistance);
                    wireframe_addRulePoints(pts, interior, segment->point[1], true, ruleTolerance, checkDistance);
                    break;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                    {
                    bvector<DPoint3d> const* points = curve->GetLineStringCP();

                    for (size_t iPt = 0; iPt < points->size(); ++iPt)
                        wireframe_addRulePoints(pts, interior, points->at(iPt), true, ruleTolerance, checkDistance);
                    break;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                    {
                    DEllipse3dCP  ellipse = curve->GetArcCP();

                    if (ellipse->IsFullEllipse())
                        {
                        for (size_t iRule = 0; iRule < 4; ++iRule)
                            {
                            double    theta = iRule * Angle::PiOver2();
                            DPoint3d  point;

                            ellipse->Evaluate(point, theta);
                            wireframe_addRulePoints(pts, interior, point, false, ruleTolerance, checkDistance);
                            }
                        break;
                        }

                    double      start, sweep;
                    DPoint3d    startPt, endPt;

                    ellipse->GetSweep(start, sweep);
                    ellipse->EvaluateEndPoints(startPt, endPt);

                    int  interiorPts = (int) (fabs(sweep) / (0.5 * Angle::Pi()));

                    wireframe_addRulePoints(pts, interior, startPt, true, ruleTolerance, checkDistance);

                    if (interiorPts > 0)
                        {
                        int     i;
                        double  theta, delta = sweep / (double) (interiorPts);

                        for (i=0, theta = start + delta; i < interiorPts; i++, theta += delta)
                            {
                            DPoint3d  point;

                            ellipse->Evaluate(point, theta);
                            wireframe_addRulePoints(pts, interior, point, DoubleOps::AlmostEqual(theta, start + sweep), ruleTolerance, checkDistance);
                            }
                        }

                    wireframe_addRulePoints(pts, interior, endPt, true, ruleTolerance, checkDistance);
                    break;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
                    {
                    bvector<DPoint3d> const* points = curve->GetAkimaCurveCP();

                    for (size_t iPt = 2; iPt < points->size()-2; ++iPt)
                        wireframe_addRulePoints(pts, interior, points->at(iPt), 2 == iPt || points->size()-2 == iPt+1, ruleTolerance, checkDistance);
                    break;
                    }

                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
                    {
                    MSBsplineCurveCP bcurve = curve->GetProxyBsplineCurveCP();
                    int              iPt, numPoints = bcurve->params.numPoles;
                    double           r, delta = 1.0 / (bcurve->params.closed ? bcurve->params.numPoles : bcurve->params.numPoles-1);

                    for (iPt = 0, r = 0.0; iPt < numPoints; r += delta, iPt++)
                        {
                        DPoint3d    point;

                        bcurve->FractionToPoint(point, r);
                        wireframe_addRulePoints(pts, interior, point, bcurve->params.closed ? false : (0 == iPt || numPoints == iPt+1), ruleTolerance, checkDistance);
                        }
                    break;
                    }

                default:
                    break;
                }
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool wireframe_collectArcMidPoints(CurveVectorCR curves, bvector<DPoint3d>& pts, bvector<bool>& interior, double ruleTolerance, bool checkDistance, ViewContextP context)
    {
    if (1 > curves.size())
        return false;

    if (curves.IsUnionRegion() || curves.IsParityRegion())
        {
        for (ICurvePrimitivePtr curve: curves)
            {
            if (curve.IsNull() || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType())
                continue;

            if (context && context->CheckStop())
                return true;

            wireframe_collectArcMidPoints(*curve->GetChildCurveVectorCP(), pts, interior, ruleTolerance, checkDistance, context);
            }
        }
    else
        {
        for (ICurvePrimitivePtr curve: curves)
            {
            if (!curve.IsValid() || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc != curve->GetCurvePrimitiveType())
                continue;

            if (context && context->CheckStop())
                return true;

            DPoint3d    point;

            curve->FractionToPoint(0.5, point);
            wireframe_addRulePoints(pts, interior, point, true, ruleTolerance, checkDistance);
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool wireframe_computeArc(DEllipse3dR ellipse, DPoint3dCR startPt, DPoint3dCR originPt, double sweepAngle, TransformCR transform, RotMatrixCR axes, RotMatrixCR invAxes, double ruleTolerance)
    {
    DPoint3d    endPt, centerPt, tmpPt;

    transform.Multiply(&endPt, &startPt, 1);
    centerPt = originPt;

    tmpPt = startPt;
    axes.Multiply(tmpPt);
    axes.Multiply(centerPt);
    centerPt.z = tmpPt.z;

    DVec3d      xVec, yVec, zVec;
    RotMatrix   rMatrix;

    zVec.Init(0.0, 0.0, 1.0);
    xVec.NormalizedDifference(tmpPt, centerPt);
    yVec.CrossProduct(zVec, xVec);
    rMatrix.InitFromColumnVectors(xVec, yVec, zVec);
    rMatrix.InitProduct(invAxes, rMatrix);
    axes.MultiplyTranspose(centerPt);

    double  radius = centerPt.Distance(startPt);

    if (radius < ruleTolerance)
        return false;

    ellipse.InitFromScaledRotMatrix(centerPt, rMatrix, radius, radius, 0.0, sweepAngle);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct StrokeSurfaceCurvesInfo
    {
    ViewContextR        m_context;
    MSBsplineSurfaceCR  m_surface;
    bool                m_includeEdges;
    bool                m_includeFaceIso;

    StrokeSurfaceCurvesInfo(ViewContextR context, MSBsplineSurfaceCR surface, bool includeEdges, bool includeFaceIso) : m_context(context), m_surface(surface), m_includeEdges(includeEdges), m_includeFaceIso(includeFaceIso) {}
    };

#define MAX_CLIPBATCH   200

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  06/05
+---------------+---------------+---------------+---------------+---------------+------*/
static int wireframe_drawSurfaceCurveCallback(void* userArg, MSBsplineCurveP bcurve, double u0, double u1, double v0, double v1)
    {
    StrokeSurfaceCurvesInfo* info = (StrokeSurfaceCurvesInfo*) userArg;

    // Special case when full boundary isn't displayed...process as strokes...
    if (info->m_includeEdges && !info->m_surface.holeOrigin && (u0 == u1 && u0 == v0 && u0 == v1))
        {
        BsurfBoundary  *boundP = &info->m_surface.boundaries[abs((int) u0)];
        int            lastEdge, thisEdge, numStrokes;
        DPoint2d       *bP, *endP;
        DPoint3d       strokeBuffer[MAX_CLIPBATCH+1];
        DSegment3d     chord;
 
        numStrokes = 0;
        bP = endP = boundP->points;
        info->m_surface.EvaluatePoint(chord.point[1], bP->x, bP->y);
        thisEdge = bsputil_edgeCode(bP, 0.0);
 
        for (endP += boundP->numPoints, bP++; bP < endP; bP++)
            {
            chord.point[0] = chord.point[1];
            lastEdge = thisEdge;
            info->m_surface.EvaluatePoint(chord.point[1], bP->x, bP->y);
            thisEdge = bsputil_edgeCode(bP, 0.0);
 
            // If both points are on the same edge then do not stroke this segment...
            if (!(lastEdge & thisEdge))
                {
                /* Leave this test in as it supports discontinuity in a B-spline (which arises from the conversion of group holes). */
                if (numStrokes && !LegacyMath::RpntEqual(&chord.point[0], strokeBuffer + numStrokes - 1))
                    {
                    info->m_context.GetIDrawGeom().DrawLineString3d(numStrokes, strokeBuffer, NULL);
                    numStrokes = 0;
                    }
 
                /* If the buffer is empty ... */
                if (!numStrokes)
                    {
                    strokeBuffer[0] = chord.point[0];
                    numStrokes = 1;
                    }
 
                strokeBuffer[numStrokes] = chord.point[1];
                numStrokes += 1;
 
                if (numStrokes >= MAX_CLIPBATCH-1)
                    {
                    info->m_context.GetIDrawGeom().DrawLineString3d(numStrokes, strokeBuffer, NULL);
                    strokeBuffer[0] = strokeBuffer[numStrokes - 1];
                    numStrokes = 1;
                    }
                }
            }
 
        info->m_context.GetIDrawGeom().DrawLineString3d(numStrokes, strokeBuffer, NULL);
 
        return (info->m_context.CheckStop() ? ERROR : SUCCESS);
        }

    bool  showThisRule = true;

    if (!info->m_includeEdges || !info->m_includeFaceIso)
        {
        if ((DoubleOps::AlmostEqual(u0, u1) && (DoubleOps::AlmostEqual(u0, 0.0) || DoubleOps::AlmostEqual(u0, 1.0))) ||
            (DoubleOps::AlmostEqual(v0, v1) && (DoubleOps::AlmostEqual(v0, 0.0) || DoubleOps::AlmostEqual(v0, 1.0))))
            showThisRule = info->m_includeEdges;
        else
            showThisRule = info->m_includeFaceIso;            
        }

    if (showThisRule)
        info->m_context.GetIDrawGeom().DrawBSplineCurve(*bcurve, false);

    return (info->m_context.CheckStop() ? ERROR : SUCCESS);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool wireframe_collectRules(DgnExtrusionDetailR detail, bvector<DSegment3d>& rules, bvector<bool>& interior, ViewContextP context)
    {
    bvector<DPoint3d> pts;

    if (wireframe_collectPoints(*detail.m_baseCurve, pts, interior, wireframe_getTolerance(*detail.m_baseCurve), true, context))
        return true;

    for (size_t iPt = 0; iPt < pts.size(); ++iPt)
        {
        DSegment3d  segment = DSegment3d::FromOriginAndDirection(pts[iPt], detail.m_extrusionVector);

        rules.push_back(segment);
        }

    return false;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool wireframe_collectRules(DgnRotationalSweepDetailR detail, bvector<DEllipse3d>& rules, bvector<bool>& interior, ViewContextP context)
    {
    double ruleTolerance = wireframe_getTolerance(*detail.m_baseCurve);
    bvector<bool> tmpInterior;
    bvector<DPoint3d> pts;

    if (wireframe_collectPoints(*detail.m_baseCurve, pts, tmpInterior, ruleTolerance, true, context))
        return true;

    RotMatrix   axes, invAxes, tmpRMatrix;
    Transform   transform;

    invAxes.InitFrom1Vector(detail.m_axisOfRotation.direction, 2, true);
    axes.TransposeOf(invAxes);

    tmpRMatrix.InitFromPrincipleAxisRotations(axes, 0.0, 0.0, detail.m_sweepAngle);
    tmpRMatrix.InitProduct(invAxes, tmpRMatrix);
    transform.From(tmpRMatrix, detail.m_axisOfRotation.origin);

    for (size_t iRule = 0; iRule < pts.size(); ++iRule)
        {
        DEllipse3d  ellipse;

        if (!wireframe_computeArc(ellipse, pts.at(iRule), detail.m_axisOfRotation.origin, detail.m_sweepAngle, transform, axes, invAxes, ruleTolerance))
            continue;

        rules.push_back(ellipse);
        interior.push_back(tmpInterior.at(iRule));
        }

    if (0 == rules.size()) // TR#115152 - Problem generating rule arc from arc profile with end point on axis of revolution and small sweep...
        {    
        pts.clear();
        tmpInterior.clear();

        if (wireframe_collectArcMidPoints(*detail.m_baseCurve, pts, tmpInterior, ruleTolerance, true, context))
            return true;

        for (size_t iRule = 0; iRule < pts.size(); ++iRule)
            {
            DEllipse3d  ellipse;

            if (!wireframe_computeArc(ellipse, pts.at(iRule), detail.m_axisOfRotation.origin, detail.m_sweepAngle, transform, axes, invAxes, ruleTolerance))
                continue;

            rules.push_back(ellipse);
            interior.push_back(tmpInterior.at(iRule));
            }
        }

    return false;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool wireframe_collectRules(DgnRuledSweepDetailR detail, bvector<DSegment3d>& rules, bvector<bool>& interior, ViewContextP context)
    {
    for (size_t iProfile = 0; iProfile < detail.m_sectionCurves.size()-1; ++iProfile)
        {
        bvector<bool> interior1;
        bvector<bool> interior2;
        bvector<DPoint3d> rulePts1;
        bvector<DPoint3d> rulePts2;

        if (wireframe_collectPoints(*detail.m_sectionCurves.at(iProfile), rulePts1, interior1, wireframe_getTolerance(*detail.m_sectionCurves.at(iProfile)), true, context) ||
            wireframe_collectPoints(*detail.m_sectionCurves.at(iProfile+1), rulePts2, interior2, wireframe_getTolerance(*detail.m_sectionCurves.at(iProfile+1)), true, context))
            return true;

        if (rulePts1.size() != rulePts2.size())
            {
            if (1 == rulePts2.size())
                {
                // Special case to handle zero scale in both XY...
                rulePts2.insert(rulePts2.end(), rulePts1.size()-1, rulePts2.front());
                interior2.insert(interior2.end(), interior1.size()-1, interior2.front());
                }
            else
                {
                rulePts1.clear(); rulePts2.clear(); interior1.clear(); interior2.clear();

                // In case of zero scale in only X or Y...we have no choice but to re-collect without excluding any points...
                if (wireframe_collectPoints(*detail.m_sectionCurves.at(iProfile), rulePts1, interior1, 0.0, false, context) ||
                    wireframe_collectPoints(*detail.m_sectionCurves.at(iProfile+1), rulePts2, interior2, 0.0, false, context))
                    return true;

                if (rulePts1.size() != rulePts2.size())
                    return true;
                }
            }

        for (size_t iRule = 0; iRule < rulePts1.size(); ++iRule)
            {
            DSegment3d  segment;

            segment.Init(rulePts1.at(iRule), rulePts2.at(iRule));
            rules.push_back(segment);
            interior.push_back(interior1.at(iRule));
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/13
+---------------+---------------+---------------+---------------+---------------+------*/
static void clearCurveVectorIds(CurveVectorCR curveVector)
    {
    for (ICurvePrimitivePtr curve: curveVector)
        {
        curve->SetId(NULL);

        if (curve->GetChildCurveVectorP ().IsValid())
            clearCurveVectorIds(*curve->GetChildCurveVectorP ());
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Ray.Bentley     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void drawSolidPrimitiveCurveVector(CurveVectorCR curveVector, ViewContextR context, TransformCP transform, CurvePrimitiveId::Type type, CurveTopologyIdCR id, CompoundDrawStateP cds)
    {
    bool    ignoreCurveIds = context.GetIViewDraw().IsOutputQuickVision() || context.CheckICachedDraw(); // Don't need when called from QvOutput/QvCachedOutput...

    if (!ignoreCurveIds) 
        {
        // Don't need ids when called from QvOutput...
        clearCurveVectorIds(curveVector);
        CurveTopologyId::AddCurveVectorIds(curveVector, type, id, cds);
        }

    if (transform)
        context.PushTransform(*transform);

    // Always output as open profile...
    WireframeGeomUtil::DrawOutline(curveVector, context.GetIDrawGeom());

    if (!ignoreCurveIds)
        {
        // Best not to leave our curve ids on the curve primitives...
        clearCurveVectorIds(curveVector);
        }

    if (transform)
        context.PopTransformClip();
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Ray.Bentley     10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void drawSolidPrimitiveCurve(ICurvePrimitivePtr primitive, ViewContextR context, CurveTopologyIdCR topologyId, CompoundDrawStateP cds)
    {
    bool    ignoreCurveIds = context.GetIViewDraw().IsOutputQuickVision() || context.CheckICachedDraw(); // Don't need when called from QvOutput/QvCachedOutput...

    if (!ignoreCurveIds)
        {
        // Don't need ids when called from QvOutput...
        CurvePrimitiveIdPtr newId = CurvePrimitiveId::Create(CurvePrimitiveId::Type_SolidPrimitive, topologyId, cds);

        primitive->SetId(newId.get());
        }

    context.GetIDrawGeom().DrawCurveVector(*CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, primitive), false);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void WireframeGeomUtil::Draw(ISolidPrimitiveCR primitive, ViewContextR context, bool includeEdges, bool includeFaceIso)
    {
    switch (primitive.GetSolidPrimitiveType())
        {
        case SolidPrimitiveType_DgnTorusPipe:
            {
            DgnTorusPipeDetail  detail;

            if (!primitive.TryGetDgnTorusPipeDetail(detail))
                return;

            int     maxURules = 4;
            bool    showCap0 = (Angle::IsFullCircle(detail.m_sweepAngle) ? includeFaceIso : includeEdges);
            bool    showCap1 = (Angle::IsFullCircle(detail.m_sweepAngle) ? false : includeEdges);

            if (showCap0)
                drawSolidPrimitiveCurve(ICurvePrimitive::CreateArc(detail.VFractionToUSectionDEllipse3d(0.0)), context, CurveTopologyId::FromSweepProfile(0), nullptr);

            if (showCap1)
                drawSolidPrimitiveCurve(ICurvePrimitive::CreateArc(detail.VFractionToUSectionDEllipse3d(1.0)), context, CurveTopologyId::FromSweepProfile(1), nullptr);

            if (!includeFaceIso)
                return;

            size_t  numVRules = DgnRotationalSweepDetail::ComputeVRuleCount(detail.m_sweepAngle);

            for (size_t vRule = 1; vRule < numVRules; ++vRule)
                {
                double  vFraction = (1.0 / numVRules) * vRule;

                drawSolidPrimitiveCurve(ICurvePrimitive::CreateArc(detail.VFractionToUSectionDEllipse3d(vFraction)), context, CurveTopologyId::FromSweepProfile(vRule + 1), nullptr);
                }

            for (int uRule = 0; uRule < maxURules; ++uRule)
                {
                double  uFraction = (1.0 / maxURules) * uRule;

                drawSolidPrimitiveCurve(ICurvePrimitive::CreateArc(detail.UFractionToVSectionDEllipse3d(uFraction)), context, CurveTopologyId::FromSweepLateral(uRule), nullptr);
                }
            return;
            }

        case SolidPrimitiveType_DgnCone:
            {
            DgnConeDetail  detail;

            if (!primitive.TryGetDgnConeDetail(detail))
                return;

            if (detail.m_radiusA > 0.0 && includeEdges)
                {
                DEllipse3d  ellipse;

                ellipse.InitFromDGNFields3d(detail.m_centerA, detail.m_vector0, detail.m_vector90, detail.m_radiusA, detail.m_radiusA, 0.0, msGeomConst_2pi);
                drawSolidPrimitiveCurve(ICurvePrimitive::CreateArc(ellipse), context, CurveTopologyId::FromSweepProfile(0), nullptr);
                }

            if (detail.m_radiusB > 0.0 && includeEdges)
                {
                DEllipse3d  ellipse;
    
                ellipse.InitFromDGNFields3d(detail.m_centerB, detail.m_vector0, detail.m_vector90, detail.m_radiusB, detail.m_radiusB, 0.0, msGeomConst_2pi);
                drawSolidPrimitiveCurve(ICurvePrimitive::CreateArc(ellipse), context, CurveTopologyId::FromSweepProfile(1), nullptr);
                }

            bool    ignoreSilhouettes = context.GetIViewDraw().IsOutputQuickVision() || context.CheckICachedDraw(); // Don't need when called from QvOutput/QvCachedOutput...

            if (!includeFaceIso || ignoreSilhouettes)
                return; // QVis handles cone silhouette display...

            DSegment3d  silhouettes[2];

            if (!detail.GetSilhouettes(silhouettes[0], silhouettes[1], context.GetViewToLocal()))
                return; // NOTE: This is expected to fail for TopologyCurveGenerator::CurveByIdCollector, don't allow associations to silhouettes!!!

            drawSolidPrimitiveCurve(ICurvePrimitive::CreateLine(silhouettes[0]), context, CurveTopologyId::FromSweepSilhouette(0), nullptr);
            drawSolidPrimitiveCurve(ICurvePrimitive::CreateLine(silhouettes[1]), context, CurveTopologyId::FromSweepSilhouette(1), nullptr);
            return;
            }

        case SolidPrimitiveType_DgnBox:
            {
            if (!includeEdges)
                return; // No face iso to display...

            DgnBoxDetail  detail;

            if (!primitive.TryGetDgnBoxDetail(detail))
                return;

            bvector<DPoint3d> corners;

            detail.GetCorners(corners);

            DPoint3d  baseRectangle[5];

            baseRectangle[0] = corners[0];
            baseRectangle[1] = corners[1];
            baseRectangle[2] = corners[3];
            baseRectangle[3] = corners[2];
            baseRectangle[4] = corners[0];

            DPoint3d  topRectangle[5];

            topRectangle[0] = corners[4];
            topRectangle[1] = corners[5];
            topRectangle[2] = corners[7];
            topRectangle[3] = corners[6];
            topRectangle[4] = corners[4];

            drawSolidPrimitiveCurve(ICurvePrimitive::CreateLineString(baseRectangle, 5), context, CurveTopologyId::FromSweepProfile(0), nullptr);
            drawSolidPrimitiveCurve(ICurvePrimitive::CreateLineString(topRectangle,  5), context, CurveTopologyId::FromSweepProfile(1), nullptr);

            for (uint32_t iRule = 0; iRule < 4; ++iRule)
                drawSolidPrimitiveCurve(ICurvePrimitive::CreateLine(DSegment3d::From(baseRectangle[iRule], topRectangle[iRule])), context, CurveTopologyId::FromSweepLateral(iRule), nullptr);
            return;
            }

        case SolidPrimitiveType_DgnSphere:
            {
            DgnSphereDetail  detail;

            if (!primitive.TryGetDgnSphereDetail(detail))
                return;

            if (includeFaceIso)
                {
                int     maxURules = 4;
            
                for (int uRule = 0; uRule < maxURules; ++uRule)
                    {
                    double  uFraction = (1.0 / maxURules) * uRule;

                    drawSolidPrimitiveCurve(ICurvePrimitive::CreateArc(detail.UFractionToVSectionDEllipse3d(uFraction)), context, CurveTopologyId::FromSweepLateral(uRule), nullptr);
                    }
                }

            if (!includeEdges)
                return;

            // Draw merdian if latitude sweep > 90 and includes 0.0 sweep latitude...
            if (fabs(detail.m_latitudeSweep) < Angle::PiOver2())
                return;

            double  vFraction = detail.LatitudeToVFraction(0.0);

            if (vFraction <= 0.0 || vFraction >= 1.0)
                return;

            drawSolidPrimitiveCurve(ICurvePrimitive::CreateArc(detail.VFractionToUSectionDEllipse3d(vFraction)), context, CurveTopologyId::FromSweepProfile(0), nullptr);
            return;
            }

        case SolidPrimitiveType_DgnExtrusion:
            {
            DgnExtrusionDetail  detail;

            if (!primitive.TryGetDgnExtrusionDetail(detail))
                return;

            if (includeEdges)
                {
                drawSolidPrimitiveCurveVector(*detail.m_baseCurve, context, NULL, CurvePrimitiveId::Type_SolidPrimitive, CurveTopologyId::FromSweepProfile(0), nullptr);

                if (context.CheckStop())
                    return;

                Transform  transform = Transform::From(detail.m_extrusionVector);

                drawSolidPrimitiveCurveVector(*detail.m_baseCurve, context, &transform, CurvePrimitiveId::Type_SolidPrimitive, CurveTopologyId::FromSweepProfile(1), nullptr);

                if (context.CheckStop())
                    return;
                }

            bvector<bool> interior;
            bvector<DSegment3d> rules;

            if (wireframe_collectRules(detail, rules, interior, &context))
                return;

            for (uint32_t iRule = 0; iRule < rules.size(); ++iRule)
                {
                if (!(interior.at(iRule) ? includeFaceIso : includeEdges))
                    continue;

                drawSolidPrimitiveCurve(ICurvePrimitive::CreateLine(rules.at(iRule)), context, CurveTopologyId::FromSweepLateral(iRule), nullptr);
                }
            return;
            }

        case SolidPrimitiveType_DgnRotationalSweep:
            {
            DgnRotationalSweepDetail  detail;

            if (!primitive.TryGetDgnRotationalSweepDetail(detail))
                return;

            bool    showCap0 = (Angle::IsFullCircle(detail.m_sweepAngle) ? includeFaceIso : includeEdges);
            bool    showCap1 = (Angle::IsFullCircle(detail.m_sweepAngle) ? false : includeEdges);

            if (showCap0)
                {
                drawSolidPrimitiveCurveVector(*detail.m_baseCurve, context, NULL, CurvePrimitiveId::Type_SolidPrimitive, CurveTopologyId::FromSweepProfile(0), nullptr);

                if (context.CheckStop())
                    return;
                }

            if (showCap1)
                {
                DPoint3d    axisPoint;
                Transform   transform;

                axisPoint.SumOf(detail.m_axisOfRotation.origin, detail.m_axisOfRotation.direction);
                transform.InitFromLineAndRotationAngle(detail.m_axisOfRotation.origin, axisPoint, detail.m_sweepAngle);

                drawSolidPrimitiveCurveVector(*detail.m_baseCurve, context, &transform, CurvePrimitiveId::Type_SolidPrimitive, CurveTopologyId::FromSweepProfile(1), nullptr);

                if (context.CheckStop())
                    return;
                }

            if (includeFaceIso)
                {
                // Draw V rules (if any needed) in addition to end caps...
                size_t  numVRules = detail.GetVRuleCount();

                for (size_t vRule = 1; vRule < numVRules; ++vRule)
                    {
                    double      vFraction = (1.0 / numVRules) * vRule;
                    Transform   transform, derivativeTransform;

                    if (!detail.GetVFractionTransform(vFraction, transform, derivativeTransform))
                        continue;

                    drawSolidPrimitiveCurveVector(*detail.m_baseCurve, context, &transform, CurvePrimitiveId::Type_SolidPrimitive, CurveTopologyId::FromSweepProfile(vRule + 1), nullptr);

                    if (context.CheckStop())
                        return;
                    }
                }

            // Draw U rule arcs based on profile geometry...
            bvector<bool> interior;
            bvector<DEllipse3d> rules;

            if (wireframe_collectRules(detail, rules, interior, &context))
                return;

            for (uint32_t uRule = 0; uRule < rules.size(); ++uRule)
                {
                if (!(interior.at(uRule) ? includeFaceIso : includeEdges))
                    continue;

                drawSolidPrimitiveCurve(ICurvePrimitive::CreateArc(rules.at(uRule)), context, CurveTopologyId::FromSweepLateral(uRule), nullptr);
                }
            return;
            }

        case SolidPrimitiveType_DgnRuledSweep:
            {
            DgnRuledSweepDetail detail;
    
            if (!primitive.TryGetDgnRuledSweepDetail(detail))
                return;

            if (includeEdges)
                {
                uint32_t curveIndex = 0;

                for (CurveVectorPtr curves: detail.m_sectionCurves)
                    {
                    drawSolidPrimitiveCurveVector(*curves, context, NULL, CurvePrimitiveId::Type_SolidPrimitive, CurveTopologyId::FromSweepProfile(curveIndex++), nullptr);

                    if (context.CheckStop())
                        return;
                    }
                }

            bvector<bool> interior;
            bvector<DSegment3d> rules;

            if (wireframe_collectRules(detail, rules, interior, &context))
                return;

            for (size_t uRule = 0; uRule < rules.size(); ++uRule)
                {
                if (!(interior.at(uRule) ? includeFaceIso : includeEdges))
                    continue;

                drawSolidPrimitiveCurve(ICurvePrimitive::CreateLine(rules.at(uRule)), context, CurveTopologyId::FromSweepLateral(uRule), nullptr);
                }
            return;
            }

        default:
            return;
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void WireframeGeomUtil::Draw(MSBsplineSurfaceCR surface, ViewContextR context, bool includeEdges, bool includeFaceIso)
    {
    if (includeEdges)
        {
        CurveVectorPtr  curves = surface.GetUnstructuredBoundaryCurves(0.0, true, true);

        if (curves.IsValid())
            {
            // NOTE: This should be BOUNDARY_TYPE_None with bcurve primitives. Output each curve separately so callers don't have to deal with nesting...
            for (ICurvePrimitivePtr curve : *curves)
                {
                if (!curve.IsValid())
                    continue;

                context.GetIDrawGeom().DrawCurveVector(*CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, curve), false);
                }
            }
        }

    if (includeFaceIso)
        {
        StrokeSurfaceCurvesInfo info(context, surface, false, true);

        bspproc_surfaceWireframeByCurves(&surface, wireframe_drawSurfaceCurveCallback, &info, false);
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  04/12
+---------------+---------------+---------------+---------------+---------------+------*/
void WireframeGeomUtil::Draw(ISolidKernelEntityCR entity, ViewContextR context, bool includeEdges, bool includeFaceIso)
    {
    T_HOST.GetSolidsKernelAdmin()._OutputBodyAsWireframe(entity, context, includeEdges, includeFaceIso);
    }

BEGIN_UNNAMED_NAMESPACE

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct RuleCollector : IElementGraphicsProcessor
{
protected:

Transform               m_currentTransform;

MSBsplineSurfaceCP      m_surface;
ISolidPrimitiveCP       m_primitive;
ISolidKernelEntityCP    m_entity;

bool                    m_includeEdges;
bool                    m_includeFaceIso;

CurveVectorPtr          m_curves;

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
explicit RuleCollector(bool includeEdges, bool includeFaceIso)
    {
    m_surface   = NULL;
    m_primitive = NULL;
    m_entity    = NULL;

    m_includeEdges   = includeEdges;
    m_includeFaceIso = includeFaceIso;
    }

virtual ~RuleCollector() {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _WantClipping() const override {return false;}
virtual bool _ProcessAsBody(bool isCurved) const override {return false;}
virtual bool _ProcessAsFacets(bool isPolyface) const override {return false;}
virtual void _AnnounceTransform(TransformCP trans) override {if (trans) m_currentTransform = *trans; else m_currentTransform.InitIdentity();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _ProcessCurveVector(CurveVectorCR curves, bool isFilled) override
    {
    if (m_curves.IsNull())
        m_curves = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);

    CurveVectorPtr  childCurve = curves.Clone();

    if (!m_currentTransform.IsIdentity())
        childCurve->TransformInPlace(m_currentTransform);

    m_curves->Add(childCurve);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _OutputGraphics(ViewContextR context) override
    {
    if (m_surface)
        WireframeGeomUtil::Draw(*m_surface, context, m_includeEdges, m_includeFaceIso);
    else if (m_primitive)
        WireframeGeomUtil::Draw(*m_primitive, context, m_includeEdges, m_includeFaceIso);
    else if (m_entity)
        WireframeGeomUtil::Draw(*m_entity, context, m_includeEdges, m_includeFaceIso);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
void SetBsplineSurface(MSBsplineSurfaceCR surface) {m_surface = &surface;}
void SetSolidPrimitive(ISolidPrimitiveCR primitive) {m_primitive = &primitive;}
void SetSolidEntity(ISolidKernelEntityCR entity) {m_entity = &entity;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr GetCurveVector() {return m_curves;}

}; // RuleCollector

END_UNNAMED_NAMESPACE

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  03/14
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr WireframeGeomUtil::CollectCurves(ISolidPrimitiveCR primitive, DgnDbR dgnDb, bool includeEdges, bool includeFaceIso)
    {
    RuleCollector   rules(includeEdges, includeFaceIso);

    rules.SetSolidPrimitive(primitive);
    ElementGraphicsOutput::Process(rules, dgnDb);

    return rules.GetCurveVector();
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  03/14
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr WireframeGeomUtil::CollectCurves(MSBsplineSurfaceCR surface, DgnDbR dgnDb, bool includeEdges, bool includeFaceIso)
    {
    RuleCollector   rules(includeEdges, includeFaceIso);

    rules.SetBsplineSurface(surface);
    ElementGraphicsOutput::Process(rules, dgnDb);

    return rules.GetCurveVector();
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  03/14
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr WireframeGeomUtil::CollectCurves(ISolidKernelEntityCR entity, DgnDbR dgnDb, bool includeEdges, bool includeFaceIso)
    {
    RuleCollector   rules(includeEdges, includeFaceIso);

    rules.SetSolidEntity(entity);
    ElementGraphicsOutput::Process(rules, dgnDb);

    return rules.GetCurveVector();
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct FaceAttachmentRuleCollector : IElementGraphicsProcessor
{
protected:

Transform                           m_currentTransform;
ElemDisplayParams                   m_currentDisplayParams;

ISolidKernelEntityCR                m_entity;
bool                                m_includeEdges;
bool                                m_includeFaceIso;
bmap<FaceAttachment, CurveVectorP>  m_uniqueAttachments;

bvector<CurveVectorPtr>&            m_curves;
bvector<ElemDisplayParams>&         m_params;

public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
explicit FaceAttachmentRuleCollector(ISolidKernelEntityCR entity, bvector<CurveVectorPtr>& curves, bvector<ElemDisplayParams>& params, bool includeEdges, bool includeFaceIso) : m_entity(entity), m_curves(curves), m_params(params)
    {
    m_includeEdges   = includeEdges;
    m_includeFaceIso = includeFaceIso;
    }

virtual ~FaceAttachmentRuleCollector() {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool _WantClipping() const override {return false;}
virtual bool _ProcessAsBody(bool isCurved) const override {return false;}
virtual bool _ProcessAsFacets(bool isPolyface) const override {return false;}
virtual void _AnnounceTransform(TransformCP trans) override {if (trans) m_currentTransform = *trans; else m_currentTransform.InitIdentity();}
virtual void _AnnounceElemDisplayParams(ElemDisplayParams const& params) override {m_currentDisplayParams = params;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
virtual BentleyStatus _ProcessCurveVector(CurveVectorCR curves, bool isFilled) override
    {
    bmap<FaceAttachment, CurveVectorP>::iterator found = m_uniqueAttachments.find(m_currentDisplayParams);

    if (found == m_uniqueAttachments.end())
        return SUCCESS;

    CurveVectorPtr  childCurve = curves.Clone();

    if (!m_currentTransform.IsIdentity())
        childCurve->TransformInPlace(m_currentTransform);

    found->second->Add(childCurve);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _OutputGraphics(ViewContextR context) override
    {
    T_FaceAttachmentsVec const& faceAttachmentsVec = m_entity.GetFaceMaterialAttachments()->_GetFaceAttachmentsVec();

    for (FaceAttachment attachment : faceAttachmentsVec)
        {
        CurveVectorPtr    curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);
        ElemDisplayParams faceParams;

        attachment.ToElemDisplayParams(faceParams);
        m_params.push_back(faceParams);
        m_curves.push_back(curve);
        m_uniqueAttachments[attachment] = curve.get();
        }

    WireframeGeomUtil::Draw(m_entity, context, m_includeEdges, m_includeFaceIso);
    }

}; // FaceAttachmentRuleCollector

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void WireframeGeomUtil::CollectCurves(ISolidKernelEntityCR entity, DgnDbR dgnDb, bvector<CurveVectorPtr>& curves, bvector<ElemDisplayParams>& params, bool includeEdges, bool includeFaceIso)
    {
    if (nullptr == entity.GetFaceMaterialAttachments())
        return; // No reason to call this method when there aren't attachments...

    FaceAttachmentRuleCollector rules(entity, curves, params, includeEdges, includeFaceIso);

    ElementGraphicsOutput::Process(rules, dgnDb);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/15
+---------------+---------------+---------------+---------------+---------------+------*/
PolyfaceHeaderPtr WireframeGeomUtil::CollectPolyface(ISolidKernelEntityCR entity, DgnDbR dgnDb, IFacetOptionsR options)
    {
    IFacetTopologyTablePtr facetsPtr;

    if (SUCCESS != DgnPlatformLib::QueryHost()->GetSolidsKernelAdmin()._FacetBody(facetsPtr, entity, options))
        return nullptr;

    PolyfaceHeaderPtr polyface = PolyfaceHeader::New();

    if (SUCCESS != IFacetTopologyTable::ConvertToPolyface(*polyface, *facetsPtr, options))
        return nullptr;

    polyface->SetTwoSided(ISolidKernelEntity::EntityType_Solid != entity.GetEntityType());
    polyface->Transform(entity.GetEntityTransform());

    return polyface;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void WireframeGeomUtil::CollectPolyfaces(ISolidKernelEntityCR entity, DgnDbR dgnDb, bvector<PolyfaceHeaderPtr>& polyfaces, bvector<ElemDisplayParams>& params, IFacetOptionsR options)
    {
    if (nullptr == entity.GetFaceMaterialAttachments())
        return; // No reason to call this method when there aren't attachments...

    IFacetTopologyTablePtr facetsPtr;

    if (SUCCESS != DgnPlatformLib::QueryHost()->GetSolidsKernelAdmin()._FacetBody(facetsPtr, entity, options))
        return;

    T_FaceToSubElemIdMap const& faceToSubElemIdMap = entity.GetFaceMaterialAttachments()->_GetFaceToSubElemIdMap();
    T_FaceAttachmentsVec const& faceAttachmentsVec = entity.GetFaceMaterialAttachments()->_GetFaceAttachmentsVec();
    bmap<int, PolyfaceHeaderCP> faceToPolyfaces;
    bmap<FaceAttachment, PolyfaceHeaderCP> uniqueFaceAttachments;

    for (T_FaceToSubElemIdMap::const_iterator curr = faceToSubElemIdMap.begin(); curr != faceToSubElemIdMap.end(); ++curr)
        {
        FaceAttachment faceAttachment = faceAttachmentsVec.at(curr->second.second);
        bmap<FaceAttachment, PolyfaceHeaderCP>::iterator found = uniqueFaceAttachments.find(faceAttachment);

        if (found == uniqueFaceAttachments.end())
            {
            PolyfaceHeaderPtr polyface = PolyfaceHeader::New();
            ElemDisplayParams faceParams;

            faceAttachment.ToElemDisplayParams(faceParams);
            params.push_back(faceParams);
            polyfaces.push_back(polyface);
            faceToPolyfaces[curr->first] = uniqueFaceAttachments[faceAttachment] = polyface.get();
            }
        else
            {
            faceToPolyfaces[curr->first] = found->second;
            }
        }

    if (SUCCESS != IFacetTopologyTable::ConvertToPolyfaces(polyfaces, faceToPolyfaces, *facetsPtr, options))
        return;

    for (size_t i=0; i<polyfaces.size(); i++)
        {
        polyfaces[i]->SetTwoSided(ISolidKernelEntity::EntityType_Solid != entity.GetEntityType());
        polyfaces[i]->Transform(entity.GetEntityTransform());
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void WireframeGeomUtil::DrawOutline(CurveVectorCR curves, GeomDrawR drawGeom)
    {
    if (1 > curves.size())
        return;

    if (curves.IsUnionRegion() || curves.IsParityRegion())
        {
        for (ICurvePrimitivePtr curve: curves)
            {
            if (curve.IsNull() || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType())
                continue;

            DrawOutline(*curve->GetChildCurveVectorCP(), drawGeom);
            }
        }
    else if (curves.IsClosedPath())
        {
        CurveVector::BoundaryType  saveType = curves.GetBoundaryType();

        const_cast <CurveVectorR> (curves).SetBoundaryType(CurveVector::BOUNDARY_TYPE_Open);
        drawGeom.DrawCurveVector(curves, false);
        const_cast <CurveVectorR> (curves).SetBoundaryType(saveType);
        }
    else
        {
        // Open and none path types ok...
        drawGeom.DrawCurveVector(curves, false);
        }
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void WireframeGeomUtil::DrawOutline2d(CurveVectorCR curves, GeomDrawR drawGeom, double zDepth)
    {
    if (1 > curves.size())
        return;

    if (curves.IsUnionRegion() || curves.IsParityRegion())
        {
        for (ICurvePrimitivePtr curve: curves)
            {
            if (curve.IsNull() || ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector != curve->GetCurvePrimitiveType())
                continue;

            DrawOutline2d(*curve->GetChildCurveVectorCP(), drawGeom, zDepth);
            }
        }
    else if (curves.IsClosedPath())
        {
        CurveVector::BoundaryType  saveType = curves.GetBoundaryType();

        const_cast <CurveVectorR> (curves).SetBoundaryType(CurveVector::BOUNDARY_TYPE_Open);
        drawGeom.DrawCurveVector2d(curves, false, zDepth);
        const_cast <CurveVectorR> (curves).SetBoundaryType(saveType);
        }
    else
        {
        // Open and none path types ok...
        drawGeom.DrawCurveVector2d(curves, false, zDepth);
        }
    }

#if defined (WIP_NEEDSWORK_ELEMENT)
static const double TOLERANCE_ChainMiterCosLimit = .707;

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  02/12
+===============+===============+===============+===============+===============+======*/
struct CurveVectorStroker : GraphicStroker
{
CurveVectorCR   m_curves;

CurveVectorStroker(CurveVectorCR curves) : m_curves(curves) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawCurveVector(ElementHandleCR eh, ViewContextR context, bool isFilled)
    {
    // NOTE: Always send outline to QVis as open profiles, avoids expensive QV topological analysis and problems with non-planar geometry...
    if (!isFilled && m_curves.IsAnyRegionType() && context.GetIViewDraw().IsOutputQuickVision())
        {
        if (eh.GetDgnModelP ()->Is3d())
            WireframeGeomUtil::DrawOutline(m_curves, context.GetIDrawGeom());
        else
            WireframeGeomUtil::DrawOutline2d(m_curves, context.GetIDrawGeom(), context.GetDisplayPriority());
        return;
        }

    if (eh.GetDgnModelP ()->Is3d())
        context.GetIDrawGeom().DrawCurveVector(m_curves, isFilled);
    else
        context.GetIDrawGeom().DrawCurveVector2d(m_curves, isFilled, context.GetDisplayPriority());
    }

}; // CurveVectorStroker

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  02/12
+===============+===============+===============+===============+===============+======*/
struct CurveVectorFillStroker : CurveVectorStroker
{
CurveVectorFillStroker(CurveVectorCR curves) : CurveVectorStroker(curves) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _StrokeForCache(CachedDrawHandleCR dh, ViewContextR context, double pixelSize) override
    {
    ElementHandleCR eh = *dh.GetElementHandleCP();

    DrawCurveVector(eh, context, true);
    }

}; // CurveVectorFillStroker

/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  02/12
+===============+===============+===============+===============+===============+======*/
struct CurveVectorOutlineStroker : CurveVectorStroker
{
/*----------------------------------------------------------------------------------*//**
* @bsiclass                                                     Keith.Bentley   04/03
+---------------+---------------+---------------+---------------+---------------+------*/
struct ChainTangentInfo
    {
    private:

    DPoint3d    m_point;
    DVec3d      m_tangent;
    bool        m_isValid;

    public:

    ChainTangentInfo() {m_isValid = false;}

    bool        IsValid() {return m_isValid;}
    DPoint3dCR  GetPoint() {return m_point;}
    DVec3dCR    GetTangent() {return m_tangent;}
    void        SetValid(bool yesNo) {m_isValid = yesNo;}
    void        Init(DPoint3dCR point, DVec3dCR tangent) {m_point = point; m_tangent = tangent; m_isValid = true;}

    }; // ChainTangentInfo

CurveVectorOutlineStroker(CurveVectorCR curves) : CurveVectorStroker(curves) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void _StrokeForCache(CachedDrawHandleCR dh, ViewContextR context, double pixelSize) override
    {
    ElementHandleCR eh = *dh.GetElementHandleCP();

    DrawCurveVector(eh, context, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   04/03
+---------------+---------------+---------------+---------------+---------------+------*/
static void GetChainTangents(ChainTangentInfo* startInfo, ChainTangentInfo* endInfo, ICurvePrimitiveCR curvePrimitive)
    {
    bool        isPoint = (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line == curvePrimitive.GetCurvePrimitiveType() && 0.0 == curvePrimitive.GetLineCP()->Length());
    DVec3d      tangents[2]; 
    DPoint3d    points[2];

    if (isPoint || !curvePrimitive.GetStartEnd(points[0], points[1], tangents[0], tangents[1]))
        {
        if (startInfo)
            startInfo->SetValid(false);

        if (endInfo)
            endInfo->SetValid(false);

        return;
        }

    tangents[0].Negate();

    if (startInfo)
        startInfo->Init(points[0], tangents[0]);

    if (endInfo)
        endInfo->Init(points[1], tangents[1]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
static void DrawStyled(ViewContextR context, CurveVectorCR curves, bool is3d, double zDepth)
    {
    if (1 > curves.size())
        return;

    bool              isClosed  = curves.IsClosedPath();
    bool              isComplex = (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Invalid == curves.HasSingleCurvePrimitive());
    ChainTangentInfo  currEnd, prevEnd, nextEnd, currStart, nextStart, chainStart;

    if (isComplex) // Support start/end tangents for linestyle w/thickness...
        {
        GetChainTangents(&currStart, &currEnd, *curves.front());

        if (isClosed)
            GetChainTangents(NULL, &prevEnd, *curves.back());

        chainStart = currStart;
        }

    DPoint3d    startTangent, endTangent;
    DPoint3d    *pStartTangent, *pEndTangent;
    size_t      nCmpns = curves.size();

    for (size_t iCmpn = 0; iCmpn < nCmpns; ++iCmpn)
        {
        ICurvePrimitivePtr curve = curves.at(iCmpn);

        if (!curve.IsValid())
            continue;

        if (isComplex) // Support start/end tangents for linestyle w/thickness...
            {
            ICurvePrimitivePtr nextCurve = iCmpn < nCmpns-1 ? curves.at(iCmpn+1) : NULL;

            if (!nextCurve.IsValid())
                {
                if (isClosed)
                    {
                    nextStart = chainStart;
                    }
                else
                    {
                    nextStart.SetValid(false);
                    nextEnd.SetValid(false);
                    }
                }
            else
                {
                GetChainTangents(&nextStart, &nextEnd, *nextCurve);
                }
            }

        pStartTangent = pEndTangent = NULL;

        if (prevEnd.IsValid() && currStart.IsValid())
            {
            if (currStart.GetTangent().DotProduct(prevEnd.GetTangent()) < TOLERANCE_ChainMiterCosLimit)
                {
                startTangent.DifferenceOf(currStart.GetTangent(), prevEnd.GetTangent());

                if (0.0 != startTangent.Normalize())
                    pStartTangent = &startTangent;
                }
            }

        if (currEnd.IsValid() && nextStart.IsValid())
            {
            if (currEnd.GetTangent().DotProduct(nextStart.GetTangent()) < TOLERANCE_ChainMiterCosLimit)
                {
                endTangent.DifferenceOf(currEnd.GetTangent(), nextStart.GetTangent());

                if (0.0 != endTangent.Normalize())
                    pEndTangent = &endTangent;
                }
            }

        context.SetLinestyleTangents(pStartTangent, pEndTangent); // NOTE: This needs to happen before CookElemDisplayParams to setup modifiers!

        if (isComplex)
            context.CookDisplayParams(); // Set/Clear linestyle start/end tangent modifiers. (needed for constant width change...)

        switch (curve->GetCurvePrimitiveType())
            {
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                {
                DSegment3d  segment = *curve->GetLineCP();

                if (is3d)
                    {
                    context.DrawStyledLineString3d(2, segment.point, NULL);
                    break;
                    }

                DPoint2d    points[2];

                points[0].Init(segment.point[0]);
                points[1].Init(segment.point[1]);
                    
                context.DrawStyledLineString2d(2, points, zDepth, NULL);
                break;
                }

            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                {
                bvector<DPoint3d> const* points = curve->GetLineStringCP();

                if (is3d)
                    {
                    context.DrawStyledLineString3d((int) points->size(), &points->front(), NULL, !isComplex && isClosed);
                    break;
                    }

                int                      nPts = (int) points->size();
                std::valarray<DPoint2d>  localPoints2dBuf(nPts);

                for (int iPt = 0; iPt < nPts; ++iPt)
                    {
                    DPoint3dCP  tmpPt = &points->front()+iPt;

                    localPoints2dBuf[iPt].x = tmpPt->x;
                    localPoints2dBuf[iPt].y = tmpPt->y;
                    }

                context.DrawStyledLineString2d(nPts, &localPoints2dBuf[0], zDepth, NULL, !isComplex && isClosed);
                break;
                }

            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                {
                DEllipse3d  ellipse = *curve->GetArcCP();

                if (is3d)
                    {
                    context.DrawStyledArc3d(ellipse, !isComplex && isClosed, NULL);
                    break;
                    }

                context.DrawStyledArc2d(ellipse, !isComplex && isClosed, zDepth, NULL);
                break;
                }

            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_AkimaCurve:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral:
                {
                MSBsplineCurveCP bcurve = curve->GetProxyBsplineCurveCP();
        
                if (is3d)
                    {
                    context.DrawStyledBSplineCurve3d(*bcurve);
                    break;
                    }

                context.DrawStyledBSplineCurve2d(*bcurve, zDepth);
                break;
                }

            default:
                {
                BeAssert(false && "Unexpected entry in CurveVector.");
                break;
                }
            }

        prevEnd   = currEnd;
        currStart = nextStart;
        currEnd   = nextEnd;
        }

    context.SetLinestyleTangents(NULL, NULL); // Make sure we clear linestyle tangents...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
static void DrawStyledCurveVector3d(ViewContextR context, CurveVectorCR curve)
    {
    if (NULL == context.GetCurrLineStyle(NULL))
        {
        if (context.GetIViewDraw().IsOutputQuickVision())
            WireframeGeomUtil::DrawOutline(curve, context.GetIDrawGeom());
        else
            context.GetIDrawGeom().DrawCurveVector(curve, false);
            
        return;
        }

    DrawStyled(context, curve, true, 0.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
static void DrawStyledCurveVector2d(ViewContextR context, CurveVectorCR curve, double zDepth)
    {
    if (NULL == context.GetCurrLineStyle(NULL))
        {
        if (context.GetIViewDraw().IsOutputQuickVision())
            WireframeGeomUtil::DrawOutline2d(curve, context.GetIDrawGeom(), zDepth);
        else
            context.GetIDrawGeom().DrawCurveVector2d(curve, false, zDepth);
            
        return;
        }

    DrawStyled(context, curve, false, zDepth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DrawStyledCurveVector(ElementHandleCR eh, ViewContextR context)
    {
    // Only open/closed paths are valid for linestyle display. (ex. exclude point strings)
    if (CurveVector::BOUNDARY_TYPE_None == m_curves.GetBoundaryType())
        {
        DrawCurveVector(eh, context, false);
        return;
        }

    bool    is3d = eh.GetDgnModelP ()->Is3d();

    DrawStyled(context, m_curves, is3d, is3d ? 0.0 : context.GetDisplayPriority());
    }

}; // CurveVectorOutlineStroker

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::_DrawCurveVector(ElementHandleCR eh, CurveVectorCR curves, GeomRepresentations info, bool allowCachedOutline)
    {
    if (0 != (info & DISPLAY_INFO_Thickness))
        {
        CurveVectorThicknessStroker  stroker(curves);

        DrawWithThickness(eh, stroker, 2);
        }

    if (0 != (info & (DISPLAY_INFO_Fill | DISPLAY_INFO_Surface)))
        {
        CurveVectorFillStroker  stroker(curves);

        DrawCached(eh, stroker, 1);
        }

    if (0 != (info & DISPLAY_INFO_Edge))
        {
        CurveVectorOutlineStroker  stroker(curves);

        if (allowCachedOutline && NULL == GetCurrLineStyle(NULL))
            {
            if (allowCachedOutline)
                DrawCached(eh, stroker, 0);
            else
                stroker.DrawCurveVector(eh, *this, false);
            }
        else
            {
            stroker.DrawStyledCurveVector(eh, *this);
            }
        }

    if (0 != (info & DISPLAY_INFO_Pattern))
        {
        CurveVectorFillStroker           stroker(curves);
        ViewContext::ClipStencil         clipStencil(stroker, 1);
        ViewContext::PatternParamSource  patParamSrc;
        
        DrawAreaPattern(eh, clipStencil, patParamSrc); // NOTE: Changes current matsymb...should do this last...
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::DrawStyledCurveVector3d(CurveVectorCR curve)
    {
#if defined (WIP_NEEDSWORK_ELEMENT)
    CurveVectorOutlineStroker::DrawStyledCurveVector3d(*this, curve);
#else
    GetIDrawGeom().DrawCurveVector(curve, false);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewContext::DrawStyledCurveVector2d(CurveVectorCR curve, double zDepth)
    {
#if defined (WIP_NEEDSWORK_ELEMENT)
    CurveVectorOutlineStroker::DrawStyledCurveVector2d(*this, curve, zDepth);
#else
    GetIDrawGeom().DrawCurveVector2d(curve, false, zDepth);
#endif
    }

