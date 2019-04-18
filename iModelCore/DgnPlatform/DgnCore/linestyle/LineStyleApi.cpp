/*----------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+----------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY_RENDER

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   04/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt LsComponent::_StrokeLineString(LineStyleContextR lsContext, LineStyleSymbR lsSymb, DPoint3dCP pts, int nPts, bool isClosed) const
    {
    if (nPts < 2)
        return ERROR;

    lsSymb.SetIsCurve(false);
    lsSymb.SetElementClosed(isClosed);
    lsSymb.SetTotalLength(PolylineOps::Length(pts, nullptr, 1, nPts));

    if (0.0 == lsSymb.GetTotalLength())
        {
        lsContext.GetGraphicR().AddLineString(nPts, pts);

        return SUCCESS;
        }

    return _DoStroke(lsContext, pts, nPts, lsSymb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   04/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt LsComponent::_StrokeLineString2d(LineStyleContextR lsContext, LineStyleSymbR lsSymb, DPoint2dCP pts, int nPts, double zDepth, bool isClosed) const
    {
    if (nPts < 2)
        return ERROR;

    // allocate a local 3d buffer for the points
    ScopedArray<DPoint3d, 50> pointsArray((unsigned) nPts);
    DPoint3dP pts3d = pointsArray.GetData();

    // copy points, set zDepth
    for (int i=0; i<nPts; i++, pts++)
        {
        pts3d[i].x = pts->x;
        pts3d[i].y = pts->y;
        pts3d[i].z = zDepth;
        }

    // output points in 3d
    return _StrokeLineString(lsContext, lsSymb, pts3d, nPts, isClosed);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LsComponent::StrokeContinuousArc(LineStyleContextR context, LineStyleSymbCR lsSymb, DEllipse3dCR arc, bool isClosed) const
    {
    /* Sometimes we see arcs with a truewidth that is exactly equal to the radius*2. They are intended to represent a filled half-circle (or
       quarter circle). This comes from DWG, because apparently AutoCAD is too feeble to have a filled circle (so we see two of them in a row).
       Unfortunately this causes problems for QV since the math to stroke the arc and create the offsets degenerates to a bunch of points
       all nearly on top of each other. QV sometimes decides that the shape crosses on itself and then doesn't fill it. All of the below
       is to test for:

       1) continuous linestyle
       2) not during picking
       3) r0==r1 (circular)
       4) no taper stroke pattern
       5) width = 2*radius

       if all of that is true, then use a complex shape rather than sending the arc through the normal linestyle code.
    */
    if (!_IsContinuous())
        return ERROR;

    double radius;

    if (!arc.IsCircular(radius))
        return ERROR;

    double startWidth = lsSymb.GetOriginWidth();
    double endWidth = lsSymb.GetEndWidth();

    if (!DoubleOps::WithinTolerance(startWidth, endWidth, 1.0e-5))
        return ERROR;

    if (!DoubleOps::WithinTolerance(startWidth, radius*2.0, 1.0e-5))
        return ERROR;

    //Render::GraphicBuilderR graphic = context.GetGraphicR();
    DEllipse3d tmpArc = arc;

    tmpArc.vector0.ScaleToLength(startWidth);
    tmpArc.vector90.ScaleToLength(startWidth);

    if (tmpArc.IsFullEllipse())
        {
        context.GetGraphicR().AddArc(tmpArc, isClosed, true);
        }
    else
        {
        // NOTE: QVis filled arc can handle 180 case...but we'll always create a real closed shape to play nice with other render targets...
        CurveVectorPtr curve = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Outer);
        DPoint3d       pts[3];
    
        tmpArc.EvaluateEndPoints(pts[2], pts[0]);
        pts[1] = tmpArc.center;

        curve->push_back(ICurvePrimitive::CreateArc(tmpArc));
        curve->push_back(ICurvePrimitive::CreateLineString(pts, 3));

        context.GetGraphicR().AddCurveVectorR(*curve, true);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   04/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt LsComponent::_StrokeArc(LineStyleContextR lsContext, LineStyleSymbR lsSymb, DEllipse3dCR arc, bool is3d, double zDepth, bool isClosed) const
    {
    if (SUCCESS == StrokeContinuousArc(lsContext, lsSymb, arc, isClosed))
        return SUCCESS;

    bvector<DPoint3d> points;

    PolylineOps::AddStrokes(arc, points, lsContext.GetFacetOptions());

    if (points.size() < 2)
        return ERROR;

    if (!is3d)
        {
        for (DPoint3dR pt : points)
            pt.z = zDepth;
        }

    lsSymb.SetIsCurve(true);
    lsSymb.SetElementClosed(isClosed);
    lsSymb.SetTotalLength(PolylineOps::Length(points));
    lsSymb.SetTreatAsSingleSegment(true);

    bool hasStartTan = lsSymb.HasStartTangent();
    bool hasEndTan = lsSymb.HasEndTangent();

    if (!hasStartTan || !hasEndTan)
        {
        DPoint3d point;
        DVec3d startTan = DVec3d::UnitX(), endTan = DVec3d::UnitX();
        DVec3d startK = DVec3d::FromZero (), endK = DVec3d::FromZero ();
        if (!hasStartTan)
            arc.FractionParameterToDerivatives (point, startTan, startK, 0.0);

        if (!hasEndTan)
            arc.FractionParameterToDerivatives (point, endTan, endK, 1.0);

        lsSymb.SetTangents(hasStartTan ? lsSymb.GetStartTangent() : &startTan, hasEndTan ? lsSymb.GetEndTangent() : &endTan);
        }

    return _DoStroke(lsContext, &points.front(), (int) points.size(), lsSymb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt LsComponent::_StrokeBSplineCurve(LineStyleContextR lsContext, LineStyleSymbR lsSymb, MSBsplineCurveCR curve, bool is3d, double zDepth) const
    {
    bvector<DPoint3d> points;

    curve.AddStrokes(lsContext.GetFacetOptions(), points);

    if (points.size() < 2)
        return ERROR;

    if (!is3d)
        {
        for (DPoint3dR pt : points)
            pt.z = zDepth;
        }

    lsSymb.SetIsCurve(true);
    lsSymb.SetElementClosed(curve.IsClosed());
    lsSymb.SetTotalLength(PolylineOps::Length(points));
    lsSymb.SetTreatAsSingleSegment(true);

    // NOTE: For whatever reason, start/end tangents aren't setup as with _StrokeArc?!?
    return _DoStroke(lsContext, &points.front(), (int) points.size(), lsSymb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    john.gooding                    09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
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


