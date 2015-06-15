/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/linestyle/LsSymbology.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <DgnPlatform/DgnCore/LsLocal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
double          LsPointComponent::_GetLength () const
    {
    return NULL == GetStrokeComponentCP() ? 0 : GetStrokeComponentCP()->_GetLength();
    }

/*---------------------------------------------------------------------------------**//**
* Initialize the line style modifiers to default values. All scale factors are set
* to 1, widths are set to zero, plane is set to identity etc.
* @bsimethod                                                    JimBartlett     11/99
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStyleSymb::Init (ILineStyleCP lStyle)
    {
    m_lStyle    = lStyle;

    memset (&m_options, 0, sizeof(m_options));
    m_nIterate  = 0;
    m_scale       = m_dashScale = m_gapScale = 1.0;
    m_orgWidth    = m_endWidth  = m_phaseShift  = m_autoPhase = 0.0;
    m_maxCompress = 0.3;
    m_planeByRows.InitIdentity ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      03/00
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStyleSymb::SetPlaneAsMatrixRows (RotMatrixCP pPlane)
    {
    if (NULL != pPlane)
        {
        m_planeByRows = *pPlane;
        m_options.plane = true;
        }
    else
        {
        m_planeByRows.InitIdentity ();
        m_options.plane = false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStyleSymb::SetNormalVec
(
DPoint3dCP normal
)
    {
    // BEIJING_WIP The previous implementation invoked
    //     InitFromVectorAndRotationAngle (*((DVec3d const *)normal), 0.0);
    //  With angle zero, that always retunred an identity matrix.
    //  (And with nonzero angle, the bvector is a description of the effect, and does not appear in
    //   the z column as was incorrectly expected.)
    DVec3d xVec, yVec, zVec;
    ((DVec3d const *)normal)->GetNormalizedTriad(xVec, yVec, zVec);
    RotMatrix matrix;
    matrix.InitFromColumnVectors (xVec, yVec, zVec);
    SetPlaneAsMatrixRows (&matrix);
    }

/*---------------------------------------------------------------------------------**//**
* Set the origin width override
* @param        width override origin width
* @bsimethod                                                    JimBartlett     04/99
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStyleSymb::SetOriginWidth
(
double          width
)
    {
    if (width >= 0)
        {
        m_orgWidth = width;
        m_options.orgWidth = true;
        }
    else
        {
        m_options.orgWidth = false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Set the end width override
* @param        width override end width
* @bsimethod                                                    JimBartlett     04/99
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStyleSymb::SetEndWidth
(
double          width
)
    {
    if (width >= 0)
        {
        m_endWidth = width;
        m_options.endWidth = true;
        }
    else
        {
        m_options.endWidth = false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Set both the origin and end width to the same width value
* @param        width override end width
* @bsimethod                                                    JimBartlett     11/99
+---------------+---------------+---------------+---------------+---------------+------*/
void    LineStyleSymb::SetWidth
(
double          width
)
    {
    SetOriginWidth (width);
    SetEndWidth (width);
    }

/*---------------------------------------------------------------------------------**//**
* Get the maximum width of the origin and end widths specified in the overrides.
* @bsimethod                                                    JimBartlett     04/99
+---------------+---------------+---------------+---------------+---------------+------*/
double          LineStyleSymb::GetMaxWidth () const
    {
    double  width = 0.0;

    if (m_options.orgWidth)
        width = m_orgWidth;

    if (m_options.endWidth && (m_endWidth > width))
        width = m_endWidth;

    return  width;
    }

/*---------------------------------------------------------------------------------**//**
* Set the override scale factor
* @param        scaleFactor New scale factor
* @bsimethod                                                    JimBartlett     04/99
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStyleSymb::SetScale
(
double          scaleFactor
)
    {
    if (scaleFactor <= 0.0 || scaleFactor == 1.0)
        {
        m_options.scale = false;
        m_scale = 1.0;
        }
    else
        {
        m_options.scale = true;
        m_scale = scaleFactor;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Get the modifier m_scale factor. If m_scale is not set it returns 1.0;
* @return       m_scale factor (1 if not specified in modifiers)
* @bsimethod                                                    JimBartlett     09/99
+---------------+---------------+---------------+---------------+---------------+------*/
double          LineStyleSymb::GetScale () const
    {
    return (IsScaled () && m_scale > 0) ? m_scale : 1.0;
    }

/*---------------------------------------------------------------------------------**//**
* Set the override dash scale factor
* @param        scaleFactor New scale factor
* @bsimethod                                                    JimBartlett     04/99
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStyleSymb::SetDashScale
(
double          scaleFactor
)
    {
    if (scaleFactor <= 0.0 || scaleFactor == 1.0)
        {
        m_options.dashScale = false;
        m_dashScale = 1.0;
        }
    else
        {
        m_options.dashScale = true;
        m_dashScale = scaleFactor;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Get the modifier dash scale factor. If dash scale is not set it returns 1.0;
* @return       m_scale factor (1 if not specified in modifiers)
* @bsimethod                                                    JimBartlett     09/99
+---------------+---------------+---------------+---------------+---------------+------*/
double          LineStyleSymb::GetDashScale () const
    {
    return (HasDashScale () && m_dashScale > 0.0) ? m_dashScale : 1.0;
    }

/*---------------------------------------------------------------------------------**//**
* Set the override gap scale factor
* @param        scaleFactor New scale factor
* @bsimethod                                                    JimBartlett     04/99
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStyleSymb::SetGapScale
(
double          scaleFactor
)
    {
    if (scaleFactor <= 0.0 || scaleFactor == 1.0)
        {
        m_options.gapScale = false;
        m_gapScale = 1.0;
        }
    else
        {
        m_options.gapScale = true;
        m_gapScale = scaleFactor;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Get the modifier gap scale factor. If dash scale is not set it returns 1.0;
* @return       m_scale factor (1 if not specified in modifiers)
* @bsimethod                                                    JimBartlett     09/99
+---------------+---------------+---------------+---------------+---------------+------*/
double          LineStyleSymb::GetGapScale () const
    {
    return (HasGapScale () && m_gapScale > 0.0) ? m_gapScale : 1.0;
    }

/*---------------------------------------------------------------------------------**//**
* Set the override automatic phase mode
* @param    isOn        true to enable automatic phase, false to disable it
* @param    fraction    fraction value for automatic phase
* @bsimethod                                                    JimBartlett     04/99
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStyleSymb::SetFractionalPhase
(
bool            isOn,
double          fraction
)
    {
    m_options.autoPhase = isOn;

    if (isOn)
        {
        m_options.phaseShift = false;
        m_autoPhase = fraction;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStyleSymb::SetCenterPhase
(
bool            isOn
)
    {
    m_options.centerPhase = isOn;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStyleSymb::SetTangents
(
DPoint3dCP  start,
DPoint3dCP  end
)
    {
    m_options.startTangentSet = false;
    m_options.endTangentSet   = false;

    if (NULL != start)
        {
        m_options.startTangentSet = true;
        m_startTangent = *start;

        }

    if (NULL != end)
        {
        m_options.endTangentSet = true;
        m_endTangent = *end;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Set the override phase shift
* @param    isOn        true to enable phase shift, false to disable it
* @param    fraction    distance value for phase shift
* @bsimethod                                                    JimBartlett     04/99
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStyleSymb::SetPhaseShift
(
bool            isOn,
double          distance
)
    {
    if (isOn)
        {
        m_options.phaseShift = true;
        m_options.autoPhase = false;
        m_phaseShift = distance;
        }
    else
        {
        m_options.phaseShift = false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Set the segment mode
* @bsimethod                                                    RayBentley      02/00
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStyleSymb::SetTreatAsSingleSegment
(
bool        segmentMode
)
    {
    m_options.treatAsSingleSegment = segmentMode;
    }


/*---------------------------------------------------------------------------------**//**
* Set the cosmetic flag
* @param    cosmetic true for cosmetic style, false for geometric style
* @bsimethod                                                    JimBartlett     09/99
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStyleSymb::SetCosmetic
(
bool        cosmetic
)
    {
    m_options.cosmetic = cosmetic;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/03
+---------------+---------------+---------------+---------------+---------------+------*/
LineStyleSymb::LineStyleSymb ()
    {
    memset (&m_lStyle, 0, offsetof (LineStyleSymb, m_planeByRows) + sizeof (m_planeByRows) - offsetof (LineStyleSymb, m_lStyle));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/08
+---------------+---------------+---------------+---------------+---------------+------*/
void    LineStyleSymb::CheckContinuationData ()
    {
    if (m_options.xElemPhaseSet)
        {
        m_options.phaseShift = true;
        m_options.autoPhase = false;
        m_options.continuationXElems = true;
        m_phaseShift = m_xElemPhase;
        m_options.xElemPhaseSet = false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* see whether this element should be drawn with a custom linestyle.
* @return the hardware linestyle to be used.
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
int             LineStyleSymb::FromResolvedStyle
(
LineStyleInfoCP     styleInfo,
ViewContextR        context,        // Used to resolve pixel-based line styles
DPoint3dCP          startTangent,
DPoint3dCP          endTangent
)
    {
#if defined (WIP_LINESTYLES)
    // 0 - 7 use hardware linestyles.
    if (IS_LINECODE (styleNo))
        return styleNo;

    DgnDbR     dgnProject = context.GetDgnDb ();
    LsDefinitionP   nameRec = NULL; // only look in the system table for positive ids

    if ((styleNo < 0) || (NULL == (nameRec = LsSystemMap::GetSystemMapP (true)->Find (styleNo))))
        {
        LsMap*      lsMap;

        if (NULL == (lsMap = LsMap::GetMapPtr (dgnProject, true)))
            return 0; // This fails during a DWG cache load and when validating range often the elemhandle comes through with a NULL modelref.

        nameRec = lsMap->Find (styleNo);
        }

    if (NULL == nameRec)
        return 0;

    // Make this call before IsContinuous() to force the components to load.  Loading the components
    // will make some linestyles into "continuous" because early DWG styles did not set this bit correctly,
    // so there are a lot of unlabeled continuous styles out there.
    LsComponentCP    lStyle = nameRec->GetComponentCP (nullptr);

    // If the line style is continuous and has no width, leave now.
    if (nameRec->IsContinuous () && (!lStyleParams || (0 == (lStyleParams->modifiers & (STYLEMOD_SWIDTH | STYLEMOD_EWIDTH | STYLEMOD_TRUE_WIDTH)))))
        return 0;

    LineStyleParams tmpLSParams;
    
    if (lStyleParams)
        tmpLSParams = *lStyleParams;
    else
        tmpLSParams.Init ();

    // If the line style is mapped to a hardware line code return the hardware line code number.
    if (nameRec->IsHardware())
        return  (int) nameRec->GetHardwareStyle();

    // If the line style definition can be loaded correctly we set it up as the current line style.
    if (NULL == lStyle)
        return  0;

    bool        xElemPhaseSet = m_options.xElemPhaseSet;

    Init (nameRec);

    SetTangents (startTangent, endTangent);

    if ((NULL != startTangent) && xElemPhaseSet)
        {
        // if there's a start tangent, then that means we're continuing from a previous call.
        // the phase shift value should be valid too.
        m_options.phaseShift = true;
        m_options.autoPhase = false;
        m_options.continuationXElems = true;
        m_phaseShift = m_xElemPhase;
        }

    if (!m_options.phaseShift)
        {
        if (tmpLSParams.modifiers & STYLEMOD_DISTPHASE)
            SetPhaseShift (true, tmpLSParams.distPhase);
        else if (tmpLSParams.modifiers & STYLEMOD_FRACTPHASE)
            SetFractionalPhase (true, tmpLSParams.fractPhase);
        else if (tmpLSParams.modifiers & STYLEMOD_CENTERPHASE)
            SetCenterPhase (true);
        }

    if (tmpLSParams.modifiers & STYLEMOD_NORMAL)
        SetNormalVec (&tmpLSParams.normal);

    if (tmpLSParams.modifiers & STYLEMOD_RMATRIX)
        SetPlaneAsMatrixRows (&tmpLSParams.rMatrix);

    if (tmpLSParams.modifiers & STYLEMOD_DSCALE)
        SetDashScale (tmpLSParams.dashScale);

    if (tmpLSParams.modifiers & STYLEMOD_GSCALE)
        SetGapScale (tmpLSParams.gapScale);

    SetTreatAsSingleSegment ((tmpLSParams.modifiers & STYLEMOD_NOSEGMODE) && !(tmpLSParams.modifiers & STYLEMOD_SEGMODE));

#ifdef DGNV10FORMAT_CHANGES_WIP
    //  When this is a query model we get "GetLineStyleScale->GetModelInfo->....LoadFromDb" with an invalid model ID.
    double scale = nameRec->IsPhysical () ? 1.0 : dgnModel->GetLineStyleScale ();
#else
    double scale = 1.0;
#endif

    if (tmpLSParams.modifiers & STYLEMOD_SCALE && 0.0 != tmpLSParams.scale)
        scale *= tmpLSParams.scale;

    // now adjust scale for units in linestyle definition
    double unitDef = nameRec->GetUnitsDefinition();
    if (unitDef < mgds_fc_epsilon)
        unitDef = 1.0;

    // Update unitDef to convert to UORs
    if (nameRec->IsUnitsMaster ())
        {
        unitDef *= 1000;
        }
    else if (nameRec->IsUnitsDevice ())
        {
        if (nameRec->GetUnitsDefinition() > mgds_fc_epsilon)
            unitDef *= context.GetPixelSizeAtPoint (NULL);
        }
    else if (nameRec->IsUnitsUOR ())
        {
        // Get True Scale factor
        unitDef *= nameRec->GetTrueScale (nullptr);

        // Historically distance shifts are stored in master units.  This used to match the line styles.  Now
        // with imported styles, we need to convert the shift to UORs.
        if (tmpLSParams.modifiers & STYLEMOD_DISTPHASE)
            {
            double uorPhase;
#if defined (NEEDS_WORK_ELEMENT_REFACTOR)
#endif
            uorPhase = tmpLSParams.distPhase * 1000;
            SetPhaseShift (true, uorPhase);
            }
        }

    scale *= unitDef;

    double  startWidth = tmpLSParams.startWidth;
    double  endWidth   = tmpLSParams.endWidth;

    // if the start/end are "true width", then the scale factor is not applied
    if (!(tmpLSParams.modifiers & STYLEMOD_TRUE_WIDTH))  // TRUE_WIDTH means both don't scale, and use UORs.
        {
        startWidth *= scale;
        endWidth   *= scale;
        }

    if (tmpLSParams.modifiers & STYLEMOD_SWIDTH)
        SetOriginWidth (startWidth);

    if (tmpLSParams.modifiers & STYLEMOD_EWIDTH)
        SetEndWidth (endWidth);

    if (nameRec->IsSCScaleIndependent()) // linestyles that are independent of sharedcell's scale.
        {
        Transform       localToFrustum;

        // get the scale from the current localToFrustum to the current modelRef's scale, and back that out of the linestyle scale.
        if (SUCCESS == context.GetCurrLocalToFrustumTrans (localToFrustum))
            {
            DVec3d  xCol;
            localToFrustum.GetMatrixColumn (xCol, 0);

            scale /= xCol.Magnitude ();
            }
        }

    SetScale (scale);
#endif

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* see whether this element should be drawn with a custom linestyle.
* @return the hardware linestyle to be used.
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
int             LineStyleSymb::FromResolvedElemDisplayParams
(
ElemDisplayParamsCR elParams,
ViewContextR        context,        // Used to resolve pixel-based line styles
DPoint3dCP          startTangent,
DPoint3dCP          endTangent
)
    {
    BeAssert (NULL == GetILineStyle());
    return FromResolvedStyle (elParams.GetLineStyle (), context, startTangent, endTangent);
    }

/*---------------------------------------------------------------------------------**//**
* see whether this element should be drawn with a custom linestyle.
* @return the hardware linestyle to be used.
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
int             LineStyleSymb::FromNaturalElemDisplayParams
(
ElemDisplayParamsR  elParams,
ViewContextR        context,        // Used to resolve pixel-based line styles
DPoint3dCP          startTangent,
DPoint3dCP          endTangent
)
    {
    elParams.Resolve (context);

    return FromResolvedElemDisplayParams (elParams, context, startTangent, endTangent);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
double          LineStyleSymb::GetEndWidth () const
    {
    return m_options.endWidth ? m_endWidth : m_orgWidth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
double          LineStyleSymb::GetOriginWidth () const
    {
    return m_options.orgWidth ? m_orgWidth : m_endWidth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  01/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStyleSymb::ClearContinuationData ()
    {
    m_options.xElemPhaseSet      = 0;
    m_options.startTangentSet    = 0;
    m_options.endTangentSet      = 0;
    m_options.continuationXElems = 0;
    }

ILineStyleCP        LineStyleSymb::GetILineStyle()    const {return m_lStyle;}
void                LineStyleSymb::GetPlaneAsMatrixRows (RotMatrixR matrix) const {matrix = m_planeByRows;}
double              LineStyleSymb::GetPhaseShift()    const {return m_phaseShift;}
double              LineStyleSymb::GetFractionalPhase()     const {return m_autoPhase;}
double              LineStyleSymb::GetMaxCompress()   const {return m_maxCompress;}
int                 LineStyleSymb::GetNumIterations() const {return m_nIterate;}
double              LineStyleSymb::GetTotalLength()   const {return m_totalLength;}
DPoint3dCP          LineStyleSymb::GetStartTangent()  const {return &m_startTangent;}
DPoint3dCP          LineStyleSymb::GetEndTangent()    const {return &m_endTangent;}
bool                LineStyleSymb::IsCenterPhase()    const {return m_options.centerPhase;}
bool                LineStyleSymb::IsCosmetic ()      const {return m_options.cosmetic;}
bool                LineStyleSymb::IsScaled ()        const {return m_options.scale;}
bool                LineStyleSymb::IsTreatAsSingleSegment () const {return m_options.treatAsSingleSegment;}
bool                LineStyleSymb::IsAutoPhase()      const {return m_options.autoPhase;}
bool                LineStyleSymb::IsElementClosed()  const {return m_options.elementIsClosed; }
bool                LineStyleSymb::IsCurve()          const {return m_options.isCurve; }
bool                LineStyleSymb::HasDashScale()     const {return m_options.dashScale;}
bool                LineStyleSymb::HasGapScale()      const {return m_options.gapScale;}
bool                LineStyleSymb::HasOrgWidth()      const {return m_options.orgWidth;}
bool                LineStyleSymb::HasEndWidth()      const {return m_options.endWidth;}
bool                LineStyleSymb::HasPhaseShift()    const {return m_options.phaseShift;}
bool                LineStyleSymb::HasIterationLimit()const {return m_options.iterationLimit;}
bool                LineStyleSymb::HasPlane()         const {return m_options.plane;}
bool                LineStyleSymb::HasStartTangent()  const {return m_options.startTangentSet;}
bool                LineStyleSymb::HasEndTangent()    const {return m_options.endTangentSet;}
void                LineStyleSymb::SetTotalLength (double length) {m_totalLength = length;}
void                LineStyleSymb::SetLineStyle (ILineStyleCP lstyle) {m_lStyle = lstyle;}

/*---------------------------------------------------------------------------------**//**
* calulate the number of repetitions of this linestyle necessary to cover this element.
* @bsimethod                                                    Keith.Bentley   04/03
+---------------+---------------+---------------+---------------+---------------+------*/
double          LsComponent::_CalcRepetitions (LineStyleSymbCP lsSymb) const
    {
    double patLen = _GetLength() * lsSymb->GetScale();

    if (0.0 == patLen)
        return  1;

    return  lsSymb->GetTotalLength() / patLen;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LsCompoundComponent::_IsBySegment () const
    {
    if (2 != GetNumComponents())
        return  false;

    return GetComponentCP (0)->_IsBySegment() || GetComponentCP (1)->_IsBySegment();
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    ChuckKirschman  06/02
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LsStrokePatternComponent::_IsAffectedByWidth (bool currentStatusOnly) const
    {
    // This is needed because the range check wants it to be true so that it applies if
    // the style is changed.  However, most other uses want to know only the current
    // status.
    return (currentStatusOnly ? m_options.affectedByWidth : true);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    ChuckKirschman  06/02
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LsCompoundComponent::_IsAffectedByWidth (bool currentStatusOnly) const
    {
    for (size_t i=0; i<GetNumComponents(); i++)
        {
        if (GetComponentCP(i)->_IsAffectedByWidth(currentStatusOnly))
            return  true;
        }

    return  false;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    ChuckKirschman  06/02
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LsInternalComponent::_IsAffectedByWidth (bool currentStatusOnly) const
    {
    // only solid linecode can have width
    return (0 == m_hardwareLineCode);
    }

bool            LsInternalComponent::IsHardwareStyle ()  const { return 0 != m_hardwareLineCode ? true : false; }
uint32_t        LsInternalComponent::GetHardwareStyle () const { return m_hardwareLineCode; }

//  The cast is okay here because for internal components the IdentKey is simply a built-in line code.
uint32_t        LsInternalComponent::GetLineCode () const { return (uint32_t)GetLocation ()->GetIdentKey (); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  02/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LineStyleParams::operator==(LineStyleParamsCR rhs) const
    {
    if (this == &rhs)
        return true;

    if (rhs.modifiers != modifiers)
        return false;

    if (0 == rhs.modifiers && 0 == modifiers)
        return true; // No need to compare further if both inactive...

    if (rhs.reserved   != reserved   ||
        rhs.scale      != scale      ||
        rhs.dashScale  != dashScale  ||
        rhs.gapScale   != gapScale   ||
        rhs.startWidth != startWidth ||
        rhs.endWidth   != endWidth   ||
        rhs.distPhase  != distPhase  ||
        rhs.fractPhase != fractPhase ||
        rhs.lineMask   != lineMask   ||
        rhs.mlineFlags != mlineFlags)
        return false;

    if (!rhs.normal.IsEqual (normal))
        return false;

    if (!rhs.rMatrix.IsEqual (rMatrix))
        return false;

    return true;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  03/15
+---------------+---------------+---------------+---------------+---------------+------*/
LineStyleInfo::LineStyleInfo(DgnStyleId styleId, LineStyleParamsCP params) {m_styleId = styleId; if (params) m_styleParams = *params; else m_styleParams.Init();}
LineStyleInfoPtr LineStyleInfo::Create (DgnStyleId styleId, LineStyleParamsCP params) {return new LineStyleInfo (styleId, params);}

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnStyleId LineStyleInfo::GetStyleId () const {return m_styleId;}
LineStyleParamsCP LineStyleInfo::GetStyleParams () const {return 0 != m_styleParams.modifiers ? &m_styleParams : nullptr;}

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  03/15
+---------------+---------------+---------------+---------------+---------------+------*/
void LineStyleInfo::CopyFrom (LineStyleInfoCR other)
    {
    m_styleId = other.m_styleId;
    m_styleParams = other.m_styleParams;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod                                                    Brien.Bastings  03/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool LineStyleInfo::operator==(LineStyleInfoCR rhs) const
    {
    if (this == &rhs)
        return true;

    if (rhs.m_styleId != m_styleId)
        return false;

    if (!(rhs.m_styleParams == m_styleParams))
        return false;

    return true;
    }

