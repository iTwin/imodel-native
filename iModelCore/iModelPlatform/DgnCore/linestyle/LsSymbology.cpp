/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static double getMaxStyleOffset(LineStyleSymbCR symb, LsComponentCR topComponent)
    {
    // NOTE: Because "maxWidth" is twice the maximum offset, it gets divided by two.
    double styleWidth = 0.0;

    switch (topComponent.GetComponentType())
        {
        case LsComponentType::LinePoint:
            {
            LsPointComponentCR pointComponent = (LsPointComponentCR)topComponent;

            // Only symbols affect width, stroke component is strictly for positioning symbols...
            for (size_t iSymb = 0; iSymb < pointComponent.GetNumberSymbols(); iSymb++)
                {
                LsSymbolReferenceCP symbolRef = pointComponent.GetSymbolCP(iSymb);
                if (nullptr == symbolRef)
                    continue;

                // Never affected by width, sometimes not affected by scale...
                double scale = (symbolRef->GetSymbolComponentCP()->IsNoScale() ? 1.0 : symb.GetScale());
                styleWidth = DoubleOps::Max(styleWidth, (symbolRef->_GetMaxWidth() * scale) / 2.0);
                }
            break;
            }

        case LsComponentType::Compound:
            {
            LsCompoundComponentCR compoundComponent = (LsCompoundComponentCR)topComponent;

            for (size_t iComp = 0; iComp < compoundComponent.GetNumComponents(); iComp++)
                {
                LsComponentCP component = compoundComponent.GetComponentCP(iComp);

                if (nullptr != component)
                    styleWidth = DoubleOps::Max(styleWidth, getMaxStyleOffset(symb, *component));
                }
            break;
            }

        default:
            {
            // A width modifier overrides the definition even if it's 0.0...
            if (topComponent._IsAffectedByWidth(false) && symb.HasTrueWidth())
                styleWidth = symb.GetMaxWidth() / 2.0;
            else
                styleWidth = ((topComponent._GetMaxWidth() * symb.GetScale()) / 2.0);
            break;
            }
        }

    return styleWidth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double LsPointComponent::_GetLength () const
    {
    return NULL == GetStrokeComponentCP() ? 0 : GetStrokeComponentCP()->_GetLength();
    }

/*---------------------------------------------------------------------------------**//**
* Initialize the line style modifiers to default values. All scale factors are set
* to 1, widths are set to zero, plane is set to identity etc.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LineStyleSymb::Init (ILineStyleCP lStyle)
    {
    m_lStyle = lStyle;
    memset(&m_options, 0, sizeof(m_options));
    m_nIterate = 0;
    m_scale = m_dashScale = m_gapScale = 1.0;
    m_orgWidth = m_endWidth = m_phaseShift = m_autoPhase = m_styleWidth = 0.0;
    m_maxCompress = 0.3;
    m_planeByRows.InitIdentity();
    m_useLinePixels = false;
    m_useStroker = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool LineStyleSymb::operator==(LineStyleSymbCR rhs) const
    {
    if (this == &rhs)
        return true;

    if (rhs.m_lStyle != m_lStyle)
        return false;

    if (NULL == rhs.m_lStyle && NULL == m_lStyle)
        return true; // No need to compare further if both inactive...

    if (0 != memcmp (&rhs.m_options, &m_options, sizeof (m_options)))
        return false;

    if (rhs.m_nIterate != m_nIterate)
        return false;

    if (rhs.m_scale != m_scale)
        return false;

    if (rhs.m_dashScale != m_dashScale)
        return false;

    if (rhs.m_gapScale != m_gapScale)
        return false;

    if (rhs.m_orgWidth != m_orgWidth)
        return false;

    if (rhs.m_endWidth != m_endWidth)
        return false;

    if (rhs.m_phaseShift != m_phaseShift)
        return false;

    if (rhs.m_autoPhase != m_autoPhase)
        return false;

    if (rhs.m_maxCompress != m_maxCompress)
        return false;

    if (rhs.m_totalLength != m_totalLength)
        return false;

    if (rhs.m_xElemPhase != m_xElemPhase)
        return false;

    if (!rhs.m_startTangent.IsEqual (m_startTangent))
        return false;

    if (!rhs.m_endTangent.IsEqual (m_endTangent))
        return false;

    if (!rhs.m_planeByRows.IsEqual (m_planeByRows))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LineStyleSymb::SetPlaneAsMatrixRows (RotMatrixCP pPlane)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LineStyleSymb::SetNormalVec (DPoint3dCP normal)
    {
    // The previous implementation invoked
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LineStyleSymb::SetOriginWidth (double width)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LineStyleSymb::SetEndWidth (double width)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LineStyleSymb::SetWidth (double width)
    {
    SetOriginWidth (width);
    SetEndWidth (width);
    }

/*---------------------------------------------------------------------------------**//**
* Get the maximum width of the origin and end widths specified in the overrides.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double LineStyleSymb::GetMaxWidth () const
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LineStyleSymb::SetScale (double scaleFactor)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double LineStyleSymb::GetScale () const
    {
    return (IsScaled () && m_scale > 0) ? m_scale : 1.0;
    }

/*---------------------------------------------------------------------------------**//**
* Set the override dash scale factor
* @param        scaleFactor New scale factor
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LineStyleSymb::SetDashScale (double scaleFactor)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double          LineStyleSymb::GetDashScale () const
    {
    return (HasDashScale () && m_dashScale > 0.0) ? m_dashScale : 1.0;
    }

/*---------------------------------------------------------------------------------**//**
* Set the override gap scale factor
* @param        scaleFactor New scale factor
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double          LineStyleSymb::GetGapScale () const
    {
    return (HasGapScale () && m_gapScale > 0.0) ? m_gapScale : 1.0;
    }

/*---------------------------------------------------------------------------------**//**
* Set the override automatic phase mode
* @param    isOn        true to enable automatic phase, false to disable it
* @param    fraction    fraction value for automatic phase
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LineStyleSymb::SetFractionalPhase (bool isOn, double fraction)
    {
    m_options.autoPhase = isOn;

    if (isOn)
        {
        m_options.phaseShift = false;
        m_autoPhase = fraction;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LineStyleSymb::SetCenterPhase (bool isOn)
    {
    m_options.centerPhase = isOn;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LineStyleSymb::SetTangents (DVec3dCP start, DVec3dCP end)
    {
    m_options.startTangentSet = false;
    m_options.endTangentSet   = false;

    if (nullptr != start)
        {
        m_options.startTangentSet = true;
        m_startTangent = *start;
        }

    if (nullptr != end)
        {
        m_options.endTangentSet = true;
        m_endTangent = *end;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Set the override phase shift
* @param    isOn        true to enable phase shift, false to disable it
* @param    fraction    distance value for phase shift
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LineStyleSymb::SetPhaseShift (bool isOn, double distance)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LineStyleSymb::SetTreatAsSingleSegment (bool segmentMode)
    {
    m_options.treatAsSingleSegment = segmentMode;
    }

/*---------------------------------------------------------------------------------**//**
* Set the cosmetic flag
* @param    cosmetic true for cosmetic style, false for geometric style
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LineStyleSymb::SetCosmetic (bool cosmetic)
    {
    m_options.cosmetic = cosmetic;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LineStyleSymb::LineStyleSymb()
    {
    memset (&m_lStyle, 0, offsetof (LineStyleSymb, m_planeByRows) + sizeof (m_planeByRows) - offsetof (LineStyleSymb, m_lStyle));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LineStyleSymb::CheckContinuationData ()
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LineStyleSymb::ClearContinuationData ()
    {
    m_options.xElemPhaseSet      = 0;
    m_options.startTangentSet    = 0;
    m_options.endTangentSet      = 0;
    m_options.continuationXElems = 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LineStyleSymb::Init(DgnStyleId styleId, LineStyleParamsCR styleParams, DgnDbR db)
    {
    Clear(); // In case of error make sure m_lStyle is nullptr so we callers know this LineStyleSymb isn't valid...

    if (!styleId.IsValid())
        return;

    LsCacheR lsCache = db.LineStyles().GetCache();
    LsDefinitionP nameRec = lsCache.GetLineStyleP(styleId);

    if (nullptr == nameRec)
        return;

    // Make this call before IsContinuous() to force the components to load.  Loading the components
    // will make some linestyles into "continuous" because early DWG styles did not set this bit correctly,
    // so there are a lot of unlabeled continuous styles out there.
    LsComponentCP topComponent = nameRec->GetComponentCP();

    // If the line style definition can be loaded correctly we set it up as the current line style.
    if (nullptr == topComponent)
        return;

    if (topComponent->GetComponentType() == LsComponentType::Internal)
        {
        LsInternalComponentCP internalComponent = (LsInternalComponentCP)topComponent;
        uint32_t lc = internalComponent->GetLineCode();
        BeAssert(lc <= MAX_LINECODE); // 0 is valid, it's the "internal default" identifier that is affected by width overrides...
        LinePixels lp;
        switch(lc)
            {
            case 0:
                lp = LinePixels::Solid;
                break;
            case 1:
                lp = LinePixels::Code1;
                break;
            case 2:
                lp = LinePixels::Code2;
                break;
            case 3:
                lp = LinePixels::Code3;
                break;
            case 4:
                lp = LinePixels::Code4;
                break;
            case 5:
                lp = LinePixels::Code5;
                break;
            case 6:
                lp = LinePixels::Code6;
                break;
            case 7:
                lp = LinePixels::Code7;
                break;
            default:
                return;
            }

        if (LinePixels::Solid != lp)
            {
            Init(nameRec);  //  This is required.  The logic that tests UseLinePixels is skipped unless the LineStyleSymb is initialized with the nameRec
            SetUseLinePixels((uint32_t)lp);
            return;
            }
        }

    // If the line style is continuous and has no width, leave now.
    if (nameRec->IsContinuous() && (0 == (styleParams.modifiers & (STYLEMOD_SWIDTH | STYLEMOD_EWIDTH | STYLEMOD_TRUE_WIDTH))) && 0.0 == topComponent->_GetMaxWidth())
        return;

    if (nameRec->IsHardware())
        {
        //  This should have been caught by the previous test for topComponent->GetComponentType() == LsComponentType::Internal
        BeAssert(!nameRec->IsHardware());
        return;
        }

    Init(nameRec);

    m_options.isContinuous = nameRec->IsContinuous();

    if (styleParams.modifiers & STYLEMOD_DISTPHASE)
        SetPhaseShift(true, styleParams.distPhase);
    else if (styleParams.modifiers & STYLEMOD_FRACTPHASE)
        SetFractionalPhase(true, styleParams.fractPhase);
    else if (styleParams.modifiers & STYLEMOD_CENTERPHASE)
        SetCenterPhase(true);

    //  It appears that display takes care of keeping the texture parallel to the view.
    if (styleParams.modifiers & STYLEMOD_NORMAL)
        SetNormalVec(&styleParams.normal);

    if (styleParams.modifiers & STYLEMOD_RMATRIX)
        SetPlaneAsMatrixRows(&styleParams.rMatrix);

    SetTreatAsSingleSegment((styleParams.modifiers & STYLEMOD_NOSEGMODE) && !(styleParams.modifiers & STYLEMOD_SEGMODE));

    //  I don't see a way to set these in the user interface so I am assuming these are not important.  Therefore, I will
    //  not try to figure out how to make the texture generator deal with these.
    if (styleParams.modifiers & STYLEMOD_DSCALE)
        SetDashScale(styleParams.dashScale);

    if (styleParams.modifiers & STYLEMOD_GSCALE)
        SetGapScale(styleParams.gapScale);

#ifdef DGNV10FORMAT_CHANGES_WIP
    //  DgnDb does not support physical units or model-based line style scale.
    double scale = nameRec->IsPhysical () ? 1.0 : dgnModel->GetLineStyleScale ();
#else
    double scaleWithoutUnits = 1.0;
#endif

    if (styleParams.modifiers & STYLEMOD_SCALE && 0.0 != styleParams.scale)
        scaleWithoutUnits *= styleParams.scale;

    // now adjust scale for units in linestyle definition
    double unitDef = nameRec->GetUnitsDefinition();
    if (unitDef == 0)
        unitDef = 1.0;

    // NOTE: Removed nameRec->IsUnitsDevice() check. Problematic if not drawing in immediate mode...and a bad idea (draws outside element range, i.e. not pickable, etc.)
    //       Removed nameRec->IsUnitsMeters() check. No additional scaling is needed.
    double scaleWithUnits = scaleWithoutUnits * unitDef;
    double startWidth = styleParams.startWidth;
    double endWidth = styleParams.endWidth;

    // if the start/end are "true width", then the scale factor is not applied
    if (!(styleParams.modifiers & STYLEMOD_TRUE_WIDTH))  // TRUE_WIDTH means both don't scale, and use UORs.
        {
        startWidth *= scaleWithoutUnits;
        endWidth *= scaleWithoutUnits;
        }

    if (styleParams.modifiers & STYLEMOD_SWIDTH)
        SetOriginWidth(startWidth);

    if (styleParams.modifiers & STYLEMOD_EWIDTH)
        SetEndWidth(endWidth);

    SetScale(scaleWithUnits);

    if (!topComponent->_HasRasterImageComponent())
        SetUseStroker(true);

    // Set the maximum offset for "discernable" checks and to pad element ranges as the poorly named m_styleWidth.
    m_styleWidth = getMaxStyleOffset(*this, *topComponent);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double LineStyleSymb::GetEndWidth () const
    {
    return m_options.endWidth ? m_endWidth : m_orgWidth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double LineStyleSymb::GetOriginWidth () const
    {
    return m_options.orgWidth ? m_orgWidth : m_endWidth;
    }

/*---------------------------------------------------------------------------------**//**
* calulate the number of repetitions of this linestyle necessary to cover this element.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double LsComponent::_CalcRepetitions (LineStyleSymbCP lsSymb) const
    {
    double patLen = _GetLength() * lsSymb->GetScale();

    if (0.0 == patLen)
        return  1;

    return  lsSymb->GetTotalLength() / patLen;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool LsCompoundComponent::_IsBySegment () const
    {
    if (2 != GetNumComponents())
        return  false;

    return GetComponentCP (0)->_IsBySegment() || GetComponentCP (1)->_IsBySegment();
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool LsStrokePatternComponent::_IsAffectedByWidth (bool currentStatusOnly) const
    {
    // This is needed because the range check wants it to be true so that it applies if
    // the style is changed.  However, most other uses want to know only the current
    // status.
    return (currentStatusOnly ? m_options.affectedByWidth : true);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool LsCompoundComponent::_IsAffectedByWidth (bool currentStatusOnly) const
    {
    for (size_t i=0; i<GetNumComponents(); i++)
        {
        if (GetComponentCP(i)->_IsAffectedByWidth(currentStatusOnly))
            return  true;
        }

    return  false;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool LsInternalComponent::_IsAffectedByWidth (bool currentStatusOnly) const
    {
    // only solid linecode can have width
    return (0 == m_hardwareLineCode);
    }

bool LsInternalComponent::IsHardwareStyle () const { return 0 != m_hardwareLineCode ? true : false; }
uint32_t LsInternalComponent::GetHardwareStyle () const { return m_hardwareLineCode; }

//  The cast is okay here because for internal components the IdentKey is simply a built-in line code.
uint32_t LsInternalComponent::GetLineCode () const { return GetLocation()->GetComponentId().GetValue(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool LineStyleParams::operator==(LineStyleParamsCR rhs) const
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
        rhs.fractPhase != fractPhase)
        return false;

    if (!rhs.normal.IsEqual (normal))
        return false;

    if (!rhs.rMatrix.IsEqual (rMatrix))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LineStyleParams::ApplyTransform(TransformCR transform, uint32_t options)
    {
    if (modifiers & STYLEMOD_NORMAL)
        {
        transform.MultiplyMatrixOnly(normal);
        normal.Normalize();
        }

    if (modifiers & STYLEMOD_RMATRIX)
        {
        RotMatrix rTmp;

        /*---------------------------------------------------------------
        The rotation matrix is stored (through some cosmic anomaly) in
        row format like shared cells and views so it needs to be
        transposed for the multiplication.
        ---------------------------------------------------------------*/
        rTmp.InverseOf(rMatrix);
        rTmp.InitProduct(transform, rTmp);
        rTmp.SquareAndNormalizeColumns(rTmp, 0, 1);

        rMatrix.InverseOf(rTmp);
        }
    else if (!(modifiers & STYLEMOD_NORMAL))
        {
        RotMatrix rTmp = RotMatrix::FromIdentity();

        rTmp.InitProduct(transform, rTmp);
        rTmp.SquareAndNormalizeColumns(rTmp, 0, 1);

        if (!rTmp.IsIdentity())
            {
            rMatrix.InverseOf(rTmp);
            modifiers |= STYLEMOD_RMATRIX;
            }
        }

    if (options & 0x01)
        return;

    double      scaleFactor = 1.0;
    DVec3d      scaleVector;
    RotMatrix   scaleMatrix;

    transform.GetMatrix(scaleMatrix);
    scaleMatrix.NormalizeRowsOf(scaleMatrix, scaleVector);

    // Check for flatten transform, dividing scaleVector by 3 gives wrong scaleFactor
    if (scaleVector.x != 0.0 && scaleVector.y != 0.0 && scaleVector.z != 0.0)
        scaleFactor = (scaleVector.x + scaleVector.y + scaleVector.z) / 3.0;
    else
        scaleFactor = (scaleVector.x + scaleVector.y + scaleVector.z) / 2.0;

    if (1.0 == scaleFactor)
        return;

    modifiers |= STYLEMOD_SCALE;
    scale *= scaleFactor;

    if (!(modifiers & STYLEMOD_TRUE_WIDTH))
        return;

    if (modifiers & STYLEMOD_SWIDTH)
        startWidth *= scaleFactor;

    if (modifiers & STYLEMOD_EWIDTH)
        endWidth *= scaleFactor;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LineStyleInfo::LineStyleInfo(DgnStyleId styleId, LineStyleParamsCP params)
    {
    m_styleId = styleId;

    if (params)
        m_styleParams = *params;
    else
        m_styleParams.Init();
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
LineStyleInfoPtr LineStyleInfo::Create(DgnStyleId styleId, LineStyleParamsCP params)
    {
    return new LineStyleInfo(styleId, params);
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LineStyleInfo::CopyFrom(LineStyleInfoCR other)
    {
    m_styleId = other.m_styleId;
    m_styleParams = other.m_styleParams;
    m_lStyleSymb = other.m_lStyleSymb;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool LineStyleInfo::operator==(LineStyleInfoCR rhs) const
    {
    if (this == &rhs)
        return true;

    if (rhs.m_styleId != m_styleId)
        return false;

    if (!(rhs.m_styleParams == m_styleParams))
        return false;

    if (!(rhs.m_lStyleSymb == m_lStyleSymb))
        return false;

    return true;
    }

/*----------------------------------------------------------------------------------*//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void LineStyleInfo::Resolve(DgnDbR db)
    {
    m_lStyleSymb.Init(m_styleId, m_styleParams, db);
    }
