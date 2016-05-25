/*----------------------------------------------------------------------+
|
|     $Source: DgnCore/linestyle/LineStyleApi.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/VecMath.h>

USING_NAMESPACE_BENTLEY_RENDER

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   04/03
+---------------+---------------+---------------+---------------+---------------+------*/
static double  getLinearLength (DPoint3dCP pts, int nPts, int& disconnectPt)
    {
    DPoint3dCP point = pts + 1;
    DPoint3dCP last  = pts + (nPts-1);

    double  length  = 0.0;

    while (point <= last)
        {
        // Ignore disconnects.
        if (point->x == DISCONNECT)
            {
            disconnectPt = static_cast<int>(point-pts);
            return 0.0;
            }

        length += (point-1)->Distance (*point);
        point++;
        }

    return  length;
    }
    enum {DEFAULT_MINUMUM_LOD   = 50,};       // extent squared

/*---------------------------------------------------------------------------------**//**
* Check to see whether a single repeitition of this linestyle for this element is discernible in
* this context. If not, we just draw a solid line.
* @bsimethod                                                    Keith.Bentley   04/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool LsComponent::IsWidthDiscernible (ViewContextP context, Render::LineStyleSymbCP lsSymb, DPoint3dCR pt) const
    {
    // Not attached...is discernable...
    if (NULL == context->GetViewport())
        return true;

    // Line codes are always discernible.  This catches line codes in a compound.
    if (_HasLineCodes())
        return true;

    DPoint3d    max;

    max.Init (_GetLength(), _GetMaxWidth(nullptr) * 1.5, 0);
    max.Scale (lsSymb->GetScale());

    // see if there's a width modifier on the element, and if the linestyle is affected by it.
    if (_IsAffectedByWidth(true))
        {
        double  maxSymbWidth = lsSymb->GetMaxWidth() * 1.5;

        if (maxSymbWidth > max.y)
            max.y = maxSymbWidth;
        }

    if (_IsContinuousOrSingleDash()) // Ignore length for stuff that is continuous
        max.x = max.y;

    DPoint3d    vec[2];

    vec[0] = pt;
    vec[1].SumOf (*vec,max);

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    context->LocalToView (vec, vec, 2);
#else
    context->WorldToView (vec, vec, 2);
#endif


    double minLODSize = DEFAULT_MINUMUM_LOD * 0.25;

    return (vec[0].DistanceSquaredXY (vec[1]) > minLODSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/05
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LsComponent::IsSingleRepDiscernible
(
ViewContextP   context,
LineStyleSymbCP lsSymb,
DPoint3dCR      pt
) const
    {
    // Not attached...is discernable...
    if (NULL == context->GetViewport ())
        return true;

    if (_IsContinuous())
        return true;

    double      length = _GetLength();
    DPoint3d    max;

    max.Init (length, length, 0);
    max.Scale (lsSymb->GetScale());

    DPoint3d    vec[2];

    vec[0] = pt;
    vec[1].SumOf (*vec,max);

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    context->LocalToView (vec, vec, 2);
#else
    context->WorldToView (vec, vec, 2);
#endif

    double      minLODSize = DEFAULT_MINUMUM_LOD * 0.25;

    return (vec[0].DistanceSquaredXY (vec[1]) > minLODSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   04/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LsComponent::_StrokeLineString (Render::GraphicR graphic, LineStyleContextR lsContext, LineStyleSymbP lsSymb, DPoint3dCP pts, int nPts, bool isClosed) const
    {
    ViewContextP viewContext = lsContext.GetViewContext();
    double      totalLength;
    int         disconnect=0;

    // totalLength of 0.0 can be a disconnect.
    if (!IsWidthDiscernible (viewContext, lsSymb, *pts) || 0.0 == (totalLength = getLinearLength (pts, nPts, disconnect)))
        {
        if (disconnect > 1) // if we have a disconnect, recursively draw the part before and after
            {
            _StrokeLineString (graphic, lsContext, lsSymb, pts, disconnect, false);

            return _StrokeLineString (graphic, lsContext, lsSymb, pts+disconnect+1, nPts-disconnect-1, false);
            }

        graphic.AddLineString (nPts, pts);

        return SUCCESS;
        }

    lsSymb->SetElementClosed (isClosed);
    lsSymb->SetTotalLength (totalLength);

    double iterationLength = _GetLength() * lsSymb->GetScale();
    if (nullptr == viewContext || iterationLength == 0 || totalLength/iterationLength < 1000)
        return _DoStroke (lsContext, pts, nPts, lsSymb);

    //  We should never encounter this if drawing line styles with textures.  
    BeAssert (false && L"Trying to clip line string");
 #if defined (NEEDS_WORK_CONTINUOUS_RENDER)
    //  If there are more than a thousand iterations we assume it is not essential 
    //  to have all of the intermediate iterations start in the correct place. Therefore,
    //  we can skip drawing some of the segments.  In some cases, this is absolutely 
    //  essential for performance -- especially on tablets.
    context->ValidateScanRange();

    TransformClipStackR clipStack = context->GetTransformClipStack();

    int startPoint = 0;
    int accepted = -1;
    for (int i = 0; i < nPts-1; ++i)
        {
        if (clipStack.ClassifyPoints(pts + i, 2) == ClipPlaneContainment_StronglyOutside)  //  rejected -- draw whatever we have accepted
            {
            //  Current point and next point are both excluded by the same plane.  This segment is excluded
            if (accepted >= 0)
                {
                _DoStroke (context, pts+accepted, i-accepted+1, lsSymb);
                }

            accepted = -1;
            startPoint = i+1;
            continue;
            }

        if (-1 == accepted)
            //  Nothing previously accepted; current point and next are not rejected by the same plane.
            accepted = startPoint;
        }
    if (accepted >= 0)
        {
        _DoStroke (context, pts+accepted, nPts-accepted, lsSymb);
        }
#endif

    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   04/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LsComponent::_StrokeLineString2d (Render::GraphicR graphic, LineStyleContextR lsContext, LineStyleSymbP lsSymb, DPoint2dCP pts, int nPts, double zDepth, bool isClosed) const
    {
    if (nPts < 2)
        return ERROR;

    // allocate a local 3d buffer for the points
    ScopedArray<DPoint3d, 50>   pointsArray((unsigned)nPts);
    DPoint3dP   pts3d = pointsArray.GetData();

    // copy points, set zDepth
    for (int i=0; i<nPts; i++, pts++)
        {
        pts3d[i].x = pts->x;
        pts3d[i].y = pts->y;
        pts3d[i].z = zDepth;
        }

    // output points in 3d
    return _StrokeLineString (graphic, lsContext, lsSymb, pts3d, nPts, isClosed);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
static inline bool biggerThanPixel (double val, double pixelSize)
    {
    return  fabs (val) > pixelSize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       LsComponent::StrokeContinuousArc (LineStyleContextR context, LineStyleSymbCP lsSymb, DPoint3dCP origin, RotMatrixCP rMatrix,
                                        double r0, double r1, double const* inStart, double const* inSweep, DPoint3dCP range) const
    {
    bool filled = false;        // if the linestyle on the arc is indiscernible, then just draw an unfilled arc.

    bool isWidthDiscernible = IsWidthDiscernible (context.GetViewContext(), lsSymb, *origin);

    // if the linestyle is too small to recognize in this view, just draw the arc with no style.
    if (isWidthDiscernible)
        {
        /*
           Sometimes we see arcs with a truewidth that is exactly equal to the radius*2. They are intended to represent a filled half-circle (or
           quarter circle). This comes from DWG, because apparently AutoCAD is too feeble to have a filled circle (so we see two of them in a row).
           Unfortunately this causes problems for QV since the math to stroke the arc and create the offsets degenerates to a bunch of points
           all nearly on top of each other. QV sometimes decides that the shape crosses on itself and then doesn't fill it. All of the below
           is to test for:

            1) continuous linestyle
            2) not during picking
            3) r0==r1 (circular)
            4) no tapertrokepattern
            5) width = 2*radius

           if all of that is true, then use the QV filled arc (or complex shape) rather than sending the arc through the normal linestyle code.
        */
        if (!_IsContinuous() 
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
        || (NULL != context->GetIPickGeom())
#endif
        )
            return  ERROR;

        //  NEEDS_WORK_CONTINUOUS_RENDER is not appropriate for generating a texture.
        double pixelWidth = (NULL == context.GetViewContext()->GetViewport()) ? 1.0 : context.GetViewContext()->GetPixelSizeAtPoint (origin);
        double startWidth = lsSymb->GetOriginWidth();
        double endWidth   = lsSymb->GetEndWidth();

        if (biggerThanPixel (r0-r1, pixelWidth) || biggerThanPixel(startWidth-endWidth, pixelWidth) || biggerThanPixel (startWidth-(r0*2),pixelWidth))
            return  ERROR;

        filled = true;
        r0 = r1 = startWidth;
        }

    DVec3d      xCol, yCol, zCol;

    rMatrix->GetColumns (xCol, yCol, zCol);

    DEllipse3d  ellipse;
    auto& graphic = context.GetGraphicR();

    ellipse.InitFromDGNFields3d (*origin, xCol, yCol, r0, r1, inStart ? *inStart : 0.0, inSweep ? *inSweep : msGeomConst_pi);

    if (isWidthDiscernible)
        {
        // NOTE: QVis filled arc can handle 180 case...need complex shape for anything else...
        if (filled && !ellipse.IsFullEllipse () && inSweep && (fabs (double (*inSweep-msGeomConst_pi)) > .001))
            {
            CurveVectorPtr  curve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
            DPoint3d        pts[3];
    
            ellipse.EvaluateEndPoints (pts[2], pts[0]);
            pts[1] = ellipse.center;

            curve->push_back (ICurvePrimitive::CreateArc (ellipse));
            curve->push_back (ICurvePrimitive::CreateLineString (pts, 3));

            graphic.AddCurveVector (*curve, filled);
            }
        else
            {
            graphic.AddArc (ellipse, NULL == inSweep, filled);
            }
        }
    else
        {
#if defined (NEEDS_WORK_DGNITEM)
        GeometryParamsP elParams = context->GetCurrentGeometryParams();
        if (0 == elParams->GetWeight())
            {
            context->GetCurrentGraphic().AddArc (ellipse, NULL == inSweep, filled, range);
            }
        else
            {
            // True width is not discernable; need to ignore non-zero integer weight. Otherwise, discepancies
            // may arise due to level of detail differences between view display and printing. [TFS 8535]
            GraphicParams saveMatSymb;
            saveMatSymb = *context->GetGraphicParams();
            GeometryParamsStateSaver saveState (*context->GetCurrentGeometryParams(), false, false, false, true, false);
            elParams->SetWeight (0);
            context->CookGeometryParams();
            context->GetCurrentGraphic().ActivateGraphicParams (context->GetGraphicParams());
            context->GetCurrentGraphic().AddArc (ellipse, NULL == inSweep, filled, range);
            context->GetCurrentGraphic().ActivateGraphicParams (&saveMatSymb);
            }
#else
        graphic.AddArc (ellipse, NULL == inSweep, filled);
#endif
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            genArc3d (DPoint3dP outPts, double xPos, double yPos, double sine, double cosine, int nVerts)
    {
    for (int i=0; i<nVerts; i++)
        {
        outPts[i].x = xPos;
        outPts[i].y = yPos;
        outPts[i].z = 0.0;

        xPos = (xPos*cosine) - (yPos*sine);
        yPos = (yPos*cosine) + (outPts[i].x*sine);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   04/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LsComponent::_StrokeArc (Render::GraphicR graphic, LineStyleContextR lsContext, LineStyleSymbP lsSymb, DPoint3dCP origin, RotMatrixCP rMatrix,
                                        double r0, double r1, double const* inStart, double const* inSweep, DPoint3dCP range) const
    {
    if (SUCCESS == StrokeContinuousArc (lsContext, lsSymb, origin, rMatrix, r0, r1, inStart, inSweep, range))
        return SUCCESS;

    ViewContextP viewContext = lsContext.GetViewContext();
    double      start = inStart ? *inStart : 0.0;
    double      sweep = inSweep ? *inSweep : msGeomConst_2pi;
    DPoint3d    vec[2];

    vec[0]   = *origin;
    vec[1].x = origin->x + r0;
    vec[1].y = origin->y + r1;
    vec[1].z = origin->z;

    // there's no real science to this calculation. It is just an emperical formula to try to get about the right
    // number of vectors in an arc taking into consideration the size (in screen coords), a tolerance, and the sweep.
    // When we don't have a viewport...just use 100.
    double   numVerts = 100;

    if (NULL != viewContext->GetViewport ())
        {
#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
        viewContext->LocalToView (vec, vec, 2);
#else
        viewContext->WorldToView (vec, vec, 2);
#endif

        double  dist = vec[0].Distance (vec[1]);
        double  arcTolerance = .01;

        numVerts = fabs (dist * sweep * (.02 / arcTolerance));
        }

    int   nPts = (numVerts > 200) ? 200 : (int) numVerts;

    if (nPts < 5)
        nPts = 5;

    double xpos      = cos (start);
    double ypos      = sin (start);
    double stepAngle = sweep / (double) (nPts - 1);
    double sinAngle  = sin (stepAngle);
    double cosAngle  = cos (stepAngle);

    ScopedArray<DPoint3d, 50> scopedPts(nPts);
    DPoint3dP pts = scopedPts.GetData();
    genArc3d (pts, xpos, ypos, sinAngle, cosAngle, nPts);

    Transform   trans;
    LegacyMath::TMatrix::ComposeOrientationOriginScaleXYShear (&trans, NULL, rMatrix, origin, r0, r1, 0.0);
    trans.Multiply (pts, pts, nPts);

    bool hasStartTan = lsSymb->HasStartTangent();
    bool hasEndTan   = lsSymb->HasEndTangent();

    if (!hasStartTan || !hasEndTan)
        {
        DVec3d    startTang, endTang;

        startTang.Init (r0 * sin (start), -r1 * cos (start), 0.0);
        endTang.Init (-r0 * sin (start + sweep), r1 * cos (start + sweep), 0.0);

        rMatrix->Multiply(startTang);
        rMatrix->Multiply(endTang);

        lsSymb->SetTangents (hasStartTan ? lsSymb->GetStartTangent() : &startTang, hasEndTan ? lsSymb->GetEndTangent() : &endTang);
        }

    // NOTE: Save/Restore flags that aren't setup every time...
    bool    saveTreatAsSingle = lsSymb->IsTreatAsSingleSegment ();
    bool    saveIsCurve = lsSymb->IsCurve ();

    lsSymb->SetTreatAsSingleSegment (true);
    lsSymb->SetIsCurve (true);
    lsSymb->SetElementClosed (NULL == inSweep);
    
    int     disconnect;
    double  totalLength = getLinearLength (pts, nPts, disconnect);

    lsSymb->SetTotalLength (totalLength);

    StatusInt status = _DoStroke (lsContext, pts, nPts, lsSymb);

    lsSymb->SetTreatAsSingleSegment (saveTreatAsSingle);
    lsSymb->SetIsCurve (saveIsCurve);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LsComponent::_StrokeBSplineCurve (Render::GraphicR graphic, LineStyleContextR lsContext, LineStyleSymbP lsSymb, MSBsplineCurveCP curve, double const* optTolerance) const
    {
    DPoint3d    firstPt;
    curve->FractionToPoint (firstPt, 0.0);

    ViewContextP viewContext = lsContext.GetViewContext();
    // if the linestyle is too small to recognize in this view, just draw the bspline with no style.
    if (!IsWidthDiscernible (viewContext, lsSymb, firstPt))
        {
        graphic.AddBSplineCurve (*curve, false);

        return SUCCESS;
        }

    bvector<DPoint3d> points;
    //  NEEDS_WORK_CONTINUOUS_RENDER should be using GetPixelSizeAtPoint when generating a texture
    double      tolerance = (optTolerance ? *optTolerance : viewContext->GetPixelSizeAtPoint (&firstPt));

    curve->AddStrokes (points, tolerance);
    int nPoints = (int)points.size ();
    if (nPoints == 0)
        return ERROR;

    if (nPoints > 1)
        {
        // NOTE: LineJoint::FromVertices isn't happy with coincident points (perpvec set to x), should really fix this...de-dup here is safer change for Athens...
        // Accept point 0.
        // for each later point, compare to tail of packed and growing (block of) accepted points.
        bvector<DPoint3d>::iterator  lastAcceptedPoint = points.begin ();
        for (bvector<DPoint3d>::iterator  candidatePoint    = lastAcceptedPoint,
                                          endPoint          = points.end ();
            ++candidatePoint < endPoint;
            )
            {
            if (!lastAcceptedPoint->IsEqual (*candidatePoint, 1.0e-12))
                *(++lastAcceptedPoint) = *candidatePoint;
            }
        nPoints = (int)(std::distance (points.begin(), lastAcceptedPoint) + 1);
        }

    StatusInt   status = SUCCESS;

    if (nPoints > 1)
        {
        // NOTE: Save/Restore flags that aren't setup every time...
        bool    saveTreatAsSingle = lsSymb->IsTreatAsSingleSegment ();
        bool    saveIsCurve = lsSymb->IsCurve ();

        lsSymb->SetTreatAsSingleSegment (true);
        lsSymb->SetIsCurve (true);
        lsSymb->SetElementClosed (curve->IsClosed ());
    
        int     disconnect;
        double  totalLength = getLinearLength (&points[0], nPoints, disconnect);

        lsSymb->SetTotalLength (totalLength);

        status = _DoStroke (lsContext, &points[0], nPoints, lsSymb);

        lsSymb->SetTreatAsSingleSegment (saveTreatAsSingle);
        lsSymb->SetIsCurve (saveIsCurve);
        }


    return status;
    }

LsComponentId       LsComponent::GetId ()const {return m_location.GetComponentId ();}
LsComponentType     LsComponent::GetComponentType () const {return m_location.GetComponentType ();}
Utf8String          LsComponent::GetDescription () const {return m_descr;}
DgnDbP              LsComponent::GetDgnDbP() const {return m_location.GetDgnDb();}

bool                LsStroke::IsDash ()             TESTSTROKEMODE (STROKE_Dash)
bool                LsStroke::IsDashFirst ()        TESTSTROKEMODE (STROKE_DashFirst)
bool                LsStroke::IsDashLast ()         TESTSTROKEMODE (STROKE_DashLast)
bool                LsStroke::IsStretchable ()      TESTSTROKEMODE (STROKE_Stretchable)
bool                LsStroke::IsRigid ()            TESTSTROKEMODE (STROKE_Rigid)
void                LsStroke::SetIsDash             SETSTROKEMODE (STROKE_Dash)
void                LsStroke::SetIsDashFirst        SETSTROKEMODE (STROKE_DashFirst)
void                LsStroke::SetIsDashLast         SETSTROKEMODE (STROKE_DashLast)
void                LsStroke::SetIsStretchable      SETSTROKEMODE (STROKE_Stretchable)
void                LsStroke::SetIsRigid            SETSTROKEMODE (STROKE_Rigid)

double              LsStroke::GetLength ()        const    {return m_length;}
double              LsStroke::GetStartWidth ()    const    {return m_orgWidth;}
double              LsStroke::GetEndWidth ()      const    {return m_endWidth;}
LsCapMode           LsStroke::GetCapMode()        const    {return (LsCapMode)m_capMode;}
LsStroke::WidthMode LsStroke::GetWidthMode ()     const    {return (WidthMode)(m_widthMode & 0x03);}

size_t              LsStrokePatternComponent::GetNumberStrokes ()     const {return  m_nStrokes;}
LsStrokeCP          LsStrokePatternComponent::GetStrokeCP (size_t index) const {return  &m_strokes[index];}
LsStrokeP           LsStrokePatternComponent::GetStrokeP  (size_t index) {return  &m_strokes[index];}


LsSymbolReference::VertexMask  LsSymbolReference::GetVertexMask ()     const {return (LsSymbolReference::VertexMask)(m_mod1 & VERTEX_Any);}
LsSymbolReference::StrokeJustification LsSymbolReference::GetJustification()   const {return (LsSymbolReference::StrokeJustification)(m_mod1 & LCPOINT_ONSTROKE);}
bool                LsSymbolReference::GetNoPartial ()    const {return 0 != (m_mod1 & LCPOINT_NOPARTIAL);}
bool                LsSymbolReference::GetClipPartial()   const {return 0 == (m_mod1 & LCPOINT_NOCLIP);}
bool                LsSymbolReference::GetStretchable()   const {return 0 == (m_mod1 & LCPOINT_NOSCALE);}
bool                LsSymbolReference::GetDgnDb()       const {return 0 != (m_mod1 & LCPOINT_PROJECT);}
bool                LsSymbolReference::GetUseElementColor()  const {return 0 == (m_mod1 & LCPOINT_COLOR);}
bool                LsSymbolReference::GetUseElementWeight() const {return 0 == (m_mod1 & LCPOINT_WEIGHT);}
double              LsSymbolReference::GetXOffset()         const {return m_offset.x;}
double              LsSymbolReference::GetYOffset()         const {return m_offset.y;}
double              LsSymbolReference::GetAngle()           const {return m_angle;}
int                 LsSymbolReference::GetStrokeNumber()    const {return m_strokeNo;}
LsSymbolComponentP  LsSymbolReference::GetSymbolComponentP() const {return m_symbol.get ();}
LsSymbolComponentCP LsSymbolReference::GetSymbolComponentCP() const {return m_symbol.get ();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    john.gooding                    09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsSymbolReference::SetJustification(LsSymbolReference::StrokeJustification  justification)
    {
    m_mod1 = (m_mod1 & ~LCPOINT_ONSTROKE) | (justification & LCPOINT_ONSTROKE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    john.gooding                    10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsSymbolReference::SetRotationMode (LsSymbolReference::RotationMode value)
    {
    uint32_t    newValue = 0;
    if (ROTATE_Adjusted == value)
        newValue |= LCPOINT_ADJROT;
    if (ROTATE_Absolute == value)
        newValue |= LCPOINT_ABSROT;

    m_mod1 = (m_mod1 & ~(LCPOINT_ABSROT | LCPOINT_ADJROT)) | newValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    john.gooding                    09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsSymbolReference::SetVertexMask (LsSymbolReference::VertexMask mask)
    {
    m_mod1 = (m_mod1 & ~VERTEX_Any) | (mask & VERTEX_Any);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    john.gooding                    09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsSymbolReference::SetNoPartial (bool value)
    {
    uint32_t newValue = value ? LCPOINT_NOPARTIAL : 0;
    m_mod1 = (m_mod1 & ~LCPOINT_NOPARTIAL) | newValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    john.gooding                    09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsSymbolReference::SetClipPartial(bool value)
    {
    uint32_t newValue = !value ? LCPOINT_NOCLIP : 0;
    m_mod1 = (m_mod1 & ~LCPOINT_NOCLIP) | newValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    john.gooding                    09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsSymbolReference::SetStretchable(bool value)
    {
    uint32_t newValue = !value ? LCPOINT_NOSCALE : 0;
    m_mod1 = (m_mod1 & ~LCPOINT_NOSCALE) | newValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    john.gooding                    09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsSymbolReference::SetDgnDb(bool value)
    {
    uint32_t newValue = value ? LCPOINT_PROJECT : 0;
    m_mod1 = (m_mod1 & ~LCPOINT_PROJECT) | newValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    john.gooding                    09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsSymbolReference::SetUseElementColor(bool value)
    {
    uint32_t newValue = value ? 0 : LCPOINT_COLOR;
    m_mod1 = (m_mod1 & ~LCPOINT_COLOR) | newValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    john.gooding                    09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsSymbolReference::SetUseElementWeight(bool value)
    {
    uint32_t newValue = value ? 0 : LCPOINT_WEIGHT;
    m_mod1 = (m_mod1 & ~LCPOINT_WEIGHT) | newValue;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
Utf8CP LsCacheStyleEntry::GetStyleName () const 
    {
    return m_name;
    }

void                LsSymbolReference::SetXOffset(double value)     {m_offset.x = value;}
void                LsSymbolReference::SetYOffset(double value)     {m_offset.y = value;}
void                LsSymbolReference::SetAngle(double value)       {m_angle = value;}
void                LsSymbolReference::SetStrokeNumber(int value)   {m_strokeNo = value;}
void                LsSymbolReference::SetSymbolComponent(LsSymbolComponentR symbolComponent) { m_symbol = &symbolComponent; }

LsDefinitionP       LsCacheStyleEntry::GetLineStyleP ()            const { return m_nameRec; }
LsDefinitionCP      LsCacheStyleEntry::GetLineStyleCP ()           const { return m_nameRec; }

DgnStyleId          LsDefinition::GetStyleId()            const {return DgnStyleId (m_styleId); }
double              LsDefinition::GetUnitsDefinition()    const {return m_unitDef;}
LsUnit              LsDefinition::GetUnitsType()          const {return (LsUnit)(m_attributes & LSATTR_UNITMASK);}
double              LsDefinition::_GetMaxWidth()           const {return m_maxWidth;}
int                 LsDefinition::GetHardwareStyle ()     const {return (m_hardwareLineCode > 0 ? m_hardwareLineCode : 0);}

bool                LsDefinition::IsContinuous ()         const {return 0 != (m_attributes & LSATTR_CONTINUOUS);}
bool                LsDefinition::IsSnappable ()          const {return _IsSnappable (); }
bool                LsDefinition::IsNoWidth ()            const {return 0 != (m_attributes & LSATTR_NOWIDTH);}
bool                LsDefinition::IsPhysical ()           const {return 0 != (m_attributes & LSATTR_PHYSICAL);}
bool                LsDefinition::IsInternal ()           const {return m_location.GetComponentType() == LsComponentType::Internal;}
bool                LsDefinition::IsUnitsMeters()         const {return (LSATTR_UNITMETERS == GetUnits());}
bool                LsDefinition::IsUnitsDevice()         const {return (LSATTR_UNITDEV == GetUnits());}
bool                LsDefinition::IsHardware()            const {return (m_hardwareLineCode > 0 ? true : false);}

void                LsDefinition::SetUnitsDefinition(double newValue)      { m_unitDef = newValue;}
void                LsDefinition::SetUnitsType (LsUnit unitsType) { m_attributes = (m_attributes & ~LSATTR_UNITMASK)   |  static_cast<uint32_t>(unitsType);}
void                LsDefinition::SetIsContinuous (bool newValue)  { m_attributes = (m_attributes & ~LSATTR_CONTINUOUS) | (newValue ? LSATTR_CONTINUOUS : 0);}
void                LsDefinition::SetIsSnappable  (bool newValue)  { m_attributes = (m_attributes & ~LSATTR_NOSNAP)     | (!newValue ? LSATTR_NOSNAP : 0);}
void                LsDefinition::SetIsPhysical   (bool newValue)  { m_attributes = (m_attributes & ~LSATTR_PHYSICAL)   | (newValue ? LSATTR_PHYSICAL : 0);}

size_t              LsCompoundComponent::GetNumberComponents()  const   {return m_components.size ();}
LsComponentP        LsCompoundComponent::GetComponentP(size_t index) const   
    {
    if (index >= m_components.size ())
        return NULL;

    return m_components[index].m_subComponent.get ();
    }

LsComponentCP       LsCompoundComponent::GetComponentCP(size_t index) const   
    {
    return GetComponentP (index);
    }

double              LsCompoundComponent::GetOffsetToComponent(size_t index) const   
    {
    return (index < m_components.size ()) ? m_components[index].m_offset : 0.0;
    }


