/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/linestyle/LsCache.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <DgnPlatform/DgnCore/LsLocal.h>

static LsComponent*  loadComponent (LsComponentReader* reader);


USING_NAMESPACE_BENTLEY_SQLITE

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2015
//---------------------------------------------------------------------------------------
bool const LsLocation::operator < (LsLocation const &r ) const
    {
    if (GetComponentType() != r.GetComponentType())
        return  GetComponentType() < r.GetComponentType() ? true : false;

    if (GetComponentId().GetValue() != r.GetComponentId().GetValue())
        return  GetComponentId().GetValue() < r.GetComponentId().GetValue() ? true : false;

    BeAssert (GetFileKey() == r.GetFileKey());
    return  false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
LsComponentReader::~LsComponentReader ()
    {
    FREE_AND_CLEAR (m_rsc);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsComponentReader*   LsComponentReader::GetRscReader (const LsLocation* source, DgnDbR project)
    {
    return  new LsComponentReader (source, project);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/92
+---------------+---------------+---------------+---------------+---------------+------*/
static LsComponent* cacheLoadComponent(LsLocationCR location)
    {
    LsComponentReader* reader = LsComponentReader::GetRscReader (&location, *location.GetDgnDb());

    if (NULL == reader)
        return  NULL;

    LsComponent* component = loadComponent (reader);
    delete  reader;

    return component;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
void LsComponent::GetNextComponentId (LsComponentId& id, DgnDbR project, BeSQLite::PropertySpec spec)
    {
    SqlPrintfString sql("SELECT  max(Id) from " BEDB_TABLE_Property " where Namespace='%s' and Name='%s'", spec.GetNamespace(), spec.GetName());
    Statement stmt;
    stmt.Prepare(project, sql.GetUtf8CP());
    DbResult result = stmt.Step();
    if (BE_SQLITE_DONE != result && BE_SQLITE_ROW != result)
        {
        id = LsComponentId(1);
        return;
        }

    //  NOTNOW -- unclear what will happen to the magic values.  For now, avoid any collision with them.
    id = LsComponentId(std::max(stmt.GetValueInt(0) + 1, MAX_LINECODE+1));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2012
//---------------------------------------------------------------------------------------
LineStyleStatus LsComponent::AddComponentAsProperty (LsComponentId& componentId, DgnDbR project, PropertySpec spec, void const*data, uint32_t dataSize)
    {
    GetNextComponentId (componentId, project, spec);

    if (project.SaveProperty (spec, data, dataSize, componentId.GetValue(), 0) != BE_SQLITE_OK)
        return LINESTYLE_STATUS_SQLITE_Error;

    return LINESTYLE_STATUS_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
LsComponent::LsComponent (DgnDbR project, LsComponentType componentType, LsComponentId componentId) : m_isDirty (false)
    {
    m_location.SetLocation (project, componentType, componentId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsComponent::LsComponent (LsLocation const* location) : m_isDirty (false)
    {
    m_location.SetFrom (location);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding      09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsComponent::CopyDescription (Utf8CP buffer)
    {
    m_descr.AssignOrClear(buffer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsStrokePatternComponent::SaveToResource (LineCodeRsc& resource)
    {
    memset (&resource, 0, sizeof (resource));

    CopyDescription (resource.descr);

    //  double        orgAngle;
    //  double        endAngle;

    BeAssert (m_nStrokes <= _countof (m_strokes));
    
    resource.orgAngle_unused = 90.0;
    resource.endAngle_unused = 90.0;

    resource.nStrokes = static_cast <uint32_t> (m_nStrokes > _countof (m_strokes) ? _countof (m_strokes) : m_nStrokes);
    for (uint32_t i = 0; i < resource.nStrokes; i++)
        {
        m_strokes [i].SaveToResource (resource.stroke [i]);
        }
        
    resource.options  = 0;
    resource.maxIterate = 0;
    if (HasIterationLimit ())
        {
        resource.maxIterate = GetIterationLimit ();
        resource.options |= LCOPT_ITERATION;
        }

    if (_IsBySegment ())
        resource.options |= LCOPT_SEGMENT;
        
    switch (GetPhaseMode ())
        {
        case PHASEMODE_Fixed:
            resource.phase = GetDistancePhase ();
            break;
        case PHASEMODE_Fraction:
            resource.phase = GetFractionalPhase ();
            resource.options |= LCOPT_AUTOPHASE;
            break;
        case PHASEMODE_Center:
            resource.phase = 0;
            resource.options |= LCOPT_CENTERSTRETCH;
            break;
        default:
            BeAssert ("Unknown phase mode" && false);
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/92
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       LsStrokePatternComponent::CreateFromRsrc (LineCodeRsc const* pRsc)
    {
    if (pRsc==NULL || pRsc->nStrokes <= 0)
        {
        // create a default solid stroke
        InsertStroke (fc_hugeVal, true);
        }
    else
        {
        m_nStrokes = pRsc->nStrokes;
        if (m_nStrokes > 32)
            m_nStrokes = 32;

        StrokeData const* pData   = pRsc->stroke;
        StrokeData const* pEnd    = pData + m_nStrokes;

        LsStroke*   pStroke = m_strokes;
        for (;pData < pEnd; pData++, pStroke++)
            {
            pStroke->Init (pData->length, pData->width, (pData->widthMode & LCWIDTH_TAPEREND) ? pData->endWidth : pData->width,
                                (LsStroke::WidthMode)pData->widthMode, (LsStroke::CapMode)pData->capMode);
            pStroke->SetIsDash (pData->strokeMode & LCSTROKE_DASH);
            pStroke->SetIsRigid (TO_BOOL (pData->strokeMode & LCSTROKE_RAY));
            pStroke->SetIsStretchable (TO_BOOL(pData->strokeMode & LCSTROKE_SCALE));
            pStroke->SetIsDashFirst (pStroke->IsDash() ^ TO_BOOL(pData->strokeMode & LCSTROKE_SINVERT));
            pStroke->SetIsDashLast  (pStroke->IsDash() ^ TO_BOOL(pData->strokeMode & LCSTROKE_EINVERT));
            }

        SetIterationLimit ((pRsc->options & LCOPT_ITERATION) ? pRsc->maxIterate : 0);
        SetIterationMode ((pRsc->options & LCOPT_ITERATION) ? true : false);
        SetSegmentMode ((pRsc->options & LCOPT_SEGMENT) ? true : false);

        if (!IsRigid())
            {
            if (0 == (pRsc->options & (LCOPT_AUTOPHASE | LCOPT_CENTERSTRETCH)) && 0.0 != pRsc->phase)
                {
                SetDistancePhase (pRsc->phase);
                }
            else if (pRsc->options & LCOPT_AUTOPHASE)
                {
                SetFractionalPhase (pRsc->phase);
                }
            else if (pRsc->options & LCOPT_CENTERSTRETCH)
                {
                SetCenterPhaseMode();
                }
            }
        }

    CalcPatternLength();
    PostCreate ();
    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ChuckKirschman     2/07
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       LsStrokePatternComponent::PostCreate ()
    {
    // Set up affectedByWidth bit
    m_options.affectedByWidth = 0;

    LsStrokeCP   stroke = GetConstStrokePtr(0);
    for (size_t iStroke=0; iStroke<GetStrokeCount(); iStroke++, stroke++)
        {
        if (stroke->IsDash() && stroke->GetWidthMode () != LsStroke::LCWIDTH_None)
            {
            m_options.affectedByWidth = 1;
            break;
            }
        }
    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsStrokePatternComponent::Init
(
LineCodeRsc const* lcRsc
)
    {
    if (NULL != lcRsc)
        SetDescription (Utf8String(lcRsc->descr));

    CreateFromRsrc (lcRsc);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/92
+---------------+---------------+---------------+---------------+---------------+------*/
LsStrokePatternComponentP LsStrokePatternComponent::LoadStrokePatternComponent
(
LsComponentReader*    reader
)
    {
    LineCodeRsc*        lcRsc = (LineCodeRsc*) reader->GetRsc();
    const LsLocation*   source = reader->GetSource();

    //  This used to handle LsInternalComponent of line code 0 setting resource type to LsComponentType::LineCode.
    
    LsStrokePatternComponent* strokeComp = new LsStrokePatternComponent (source);
    strokeComp->Init (lcRsc);

    return  strokeComp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsCompoundComponent::LsCompoundComponent (LsLocation const *pLocation) :
            LsComponent (pLocation), m_postProcessed (false)
    {
    m_postProcessed = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
size_t          LsCompoundComponent::GetResourceSize ()
    {
    //  This guards against introducing a negative value into unsigned arithmetic
    if (m_components.size () == 0)
        return sizeof(LineStyleRsc) - sizeof(ComponentInfo);

    return sizeof(LineStyleRsc) + (m_components.size () - 1) * sizeof(ComponentInfo);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsCompoundComponent::InitLineStyleResource (LineStyleRsc& lsRsc)
    {
    memset (&lsRsc, 0, GetResourceSize ());
    CopyDescription (lsRsc.descr);

    // lsRsc = auxType -- not sure this is needed

    lsRsc.nComp = (int32_t)m_components.size ();
    
    ComponentInfo*   componentInfo = lsRsc.component;
    for (T_ComponentsCollectionIter curr = m_components.begin (); curr < m_components.end (); curr++, componentInfo++)
        {
        LsLocationCP        loc = curr->m_subComponent->GetLocation ();

        componentInfo->type = (uint32_t)loc->GetComponentType ();
        componentInfo->id = loc->GetComponentId ().GetValue();
        componentInfo->offset = curr->m_offset;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsCompoundComponent::_PostProcessLoad (DgnModelP modelRef)
    {
    if (m_postProcessed)
        return;
        
    m_postProcessed = true;
    
    for (T_ComponentsCollectionIter start = m_components.begin (); start < m_components.end (); start++)
        start->m_subComponent->_PostProcessLoad (modelRef);

    CalculateSize(modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsCompoundComponent::_ClearPostProcess ()
    {
    m_postProcessed = false;
    
    for (T_ComponentsCollectionIter start = m_components.begin (); start < m_components.end (); start++)
        start->m_subComponent->_ClearPostProcess ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsCompoundComponent::~LsCompoundComponent ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/92
+---------------+---------------+---------------+---------------+---------------+------*/
static void    offsetLinePoints
(
DPoint3d*           outPts,
DPoint3d  const*    inPts,
LineJoint const*    joints,
int                 nPts,
double              offset
)
    {
    if (0.0 == offset)
        {
        memcpy (outPts, inPts, nPts * sizeof (DPoint3d));
        }
    else
        {
        DPoint3dCP  src = inPts;
        DPoint3dCP  end = src + nPts;

        for (;src < end; src++, outPts++, joints++)
            outPts->SumOf (*src,joints->m_dir, offset * joints->m_scale);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     11/98
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LsCompoundComponent::_DoStroke (ViewContextP context, DPoint3dCP inPoints, int nPoints, LineStyleSymbCP modifiers) const
    {
    DVec3d    normal;
    RotMatrix matrix;
    modifiers->GetPlaneAsMatrixRows(matrix);
    matrix.GetRow (normal, 2);

    ScopedArray<LineJoint, 50> scopedJoints(nPoints);
    LineJoint*  joints = scopedJoints.GetData();
    LineJoint::FromVertices (joints, inPoints, nPoints, &normal, NULL, NULL);

    ScopedArray<DPoint3d, 50> scopedOffsetPts(nPoints);
    DPoint3d*   offsetPts = scopedOffsetPts.GetData();
    double      scale = modifiers->GetScale();

    for (T_ComponentsCollectionConstIter curr = m_components.begin (); curr < m_components.end (); curr++)
        {
        offsetLinePoints (offsetPts, inPoints, joints, nPoints, scale * curr->m_offset);
        if (SUCCESS != curr->m_subComponent->_DoStroke (context, offsetPts, nPoints, modifiers))
            return  ERROR;
        }

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   04/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsCompoundComponent::CalculateSize(DgnModelP modelRef)
    {
    BeAssert (m_postProcessed);
    m_size.x  = 0.0;
    m_size.y  = 0.0;

    for (size_t compNum = 0; compNum < GetNumComponents(); compNum++)
        {
        // The length is the distance for a complete pattern, so it's the length of the
        // longest component.  However, if continuous or LTYPE_Internal components are used,
        // the length of those don't count.
        if (!GetComponentCP(compNum)->_IsContinuous())
            {
            if (GetComponentCP(compNum)->_GetLength() > m_size.x)
                m_size.x = GetComponentCP(compNum)->_GetLength();
            }

        double offset    = fabs (GetOffset (compNum));
        double compWidth = GetComponentCP(compNum)->_GetMaxWidth(modelRef);
        double thisWidth = (offset + (compWidth/2.0)) * 2.0;

        if (thisWidth > m_size.y)
            m_size.y = thisWidth;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    John.Gooding    10/09
+---------------+---------------+---------------+---------------+---------------+------*/
LineStyleStatus LsCompoundComponent::AppendComponent
(
LsComponentR    childComponent, 
double          offset
)
    {
    LsOffsetComponent   lsOffset (offset, &childComponent);
        
    m_components.push_back (lsOffset);

    return LINESTYLE_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/92
+---------------+---------------+---------------+---------------+---------------+------*/
LsCompoundComponentP  LsCompoundComponent::LoadCompoundComponent
(
LsComponentReader*    reader
)
    {
    LineStyleRsc*   lsRsc = reader->GetRsc();
    if (NULL == lsRsc)
        return  NULL;

    LsCompoundComponent*  compound = new LsCompoundComponent (reader->GetSource());
    compound->SetDescription (Utf8String(lsRsc->descr).c_str());

    LsLocation  tmpLocation;

    for (uint32_t i=0; i < lsRsc->nComp; i++)
        {
        tmpLocation.GetCompoundComponentLocation (reader, i);
        
        LsOffsetComponent offsetComp (lsRsc->component[i].offset, LsCache::GetLsComponent (&tmpLocation));
        
        if (offsetComp.m_subComponent.get () == compound)
            {
            // This is a recursive definition (as in RPM10 from TR# 180688).
            BeAssert (0 && "recursive component definition");
            continue;
            }

        if (offsetComp.m_subComponent.get () == NULL)
            continue;

        compound->m_components.push_back (offsetComp);
        }

    return  compound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsInternalComponent::LsInternalComponent (LsLocation const *pLocation) :
            LsStrokePatternComponent (pLocation)
    {
    uint32_t id = pLocation->GetComponentId().GetValue();

    m_hardwareLineCode = 0;
    // Linecode only if hardware bit is set and masked value is within range.
    if ( (0 != (id & LSID_HARDWARE)) && ((id & LSID_HWMASK) <= MAX_LINECODE))
        m_hardwareLineCode = id & LSID_HWMASK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LsInternalComponent::_DoStroke (ViewContextP context, DPoint3dCP inPoints, int nPoints, LineStyleSymbCP modifiers) const
    {
#if defined (WIP_LINESTYLE)
    if (GetLocation ()->IsInternalDefault ())
        return LsStrokePatternComponent::_DoStroke (context, inPoints, nPoints, modifiers);

    int32_t style = GetHardwareStyle ();

    ElemMatSymb saveMatSymb;
    saveMatSymb = *context->GetElemMatSymb (); // Copy current ElemMatSymb

#if defined (NEEDS_WORK_DGNITEM)
    // Keep ElemDisplayParams and ElemMatSymb in synch...operations like drop lstyle use ElemDisplayParams not ElemMatSymb.
    ElemDisplayParamsStateSaver saveState (*context->GetCurrentDisplayParams (), false, false, false, true, false);
#endif

    // It's important to set the style via ElemDisplayParams, not ElemMatSymb, for printing to work correctly.
    context->GetCurrentDisplayParams ()->SetLineStyle (style);
    context->CookDisplayParams ();
    context->GetIDrawGeom ().ActivateMatSymb (context->GetElemMatSymb ()); // Activate the new matsymb

    // Style override that caused this linestyle to be used needs to be cleared in order to use the correct raster pattern for the strokes. 
    OvrMatSymbP ovrMatSymb = context->GetOverrideMatSymb ();
    uint32_t    saveFlags = ovrMatSymb->GetFlags ();

    if (0 != (saveFlags & MATSYMB_OVERRIDE_Style))
        {
        ovrMatSymb->SetFlags (saveFlags & ~MATSYMB_OVERRIDE_Style);
        context->GetIDrawGeom ().ActivateOverrideMatSymb (ovrMatSymb);
        }

    context->GetIDrawGeom ().DrawLineString3d (nPoints, inPoints, NULL); // Draw the linestring

    // Restore ElemMatSymb to previous state, ElemDisplayParams will be restored in ElemDisplayParamsStateSaver destructor...
    context->GetIDrawGeom ().ActivateMatSymb (&saveMatSymb);

    if (0 != (saveFlags & MATSYMB_OVERRIDE_Style))
        {
        ovrMatSymb->SetFlags (saveFlags);
        context->GetIDrawGeom ().ActivateOverrideMatSymb (ovrMatSymb);
        }

    return SUCCESS;
#else
    return ERROR;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
LsInternalComponentPtr LsInternalComponent::CreateInternalComponent
(
LsLocation&     location
)
    {
    LsLocation  tmpLocation;
    tmpLocation.SetFrom (&location, LsComponentType::Internal);

    LsInternalComponent*  comp = new LsInternalComponent (&location);
    comp->m_isDirty = true;
    comp->Init (NULL);
    
    return comp;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
StatusInt LsDefinition::UpdateStyleTable () const
    {
    DgnDbP project = GetLocation()->GetDgnDb();
    project->Styles ().LineStyles().Update (DgnStyleId(m_styleId), _GetName(), GetLocation()->GetComponentId(),
                                            GetLocation()->GetComponentType(), GetAttributes(), m_unitDef);

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsStrokePatternComponentP  LsInternalComponent::LoadInternalComponent
(
LsComponentReader*    reader
)
    {
    const LsLocation* location = reader->GetSource();

    LsLocation  tmpLocation;
    tmpLocation.SetFrom (location, LsComponentType::Internal);
    LsInternalComponent*  comp = new LsInternalComponent (location);
    
    comp->Init (NULL);

    return  comp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman   08/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LsCompoundComponent::_HasLineCodes () const
    {
    for (size_t compNum = 0; compNum < GetNumComponents(); compNum++)
        {
        if (GetComponentCP (compNum)->_HasLineCodes ())
            return  true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LsStrokePatternComponent::_HasWidth () const
    {
    LsStrokeCP   stroke = GetConstStrokePtr (0);

    for (size_t i=0; i<GetStrokeCount(); i++, stroke++)
        {
        /* No strokes with widths */
        if (stroke->HasWidth())
            return true;
        }

    return  false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman   06/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LsStrokePatternComponent::_HasUniformFullWidth (double *pWidth) const
    {
    LsStrokeCP   stroke = GetConstStrokePtr (0);
    bool        widthFound = false;
    double      width = 0.0;

    if (pWidth)
        *pWidth = 0.0;

    for (size_t i=0; i<GetStrokeCount(); i++, stroke++)
        {
        if (!stroke->IsDash())
            continue;

        // If it has width, it must be uniform and full.
        if (stroke->HasWidth())
            {
            if (!stroke->_HasUniformFullWidth())   // Dash is not uniform
                return false;

            if (widthFound && stroke->GetStartWidth() != width) // Dash not the same as other dashes
                return  false;

            widthFound = true;
            width = stroke->GetStartWidth();
            }
        }

    if (pWidth)
        *pWidth = width;

    return  (0.0 == width ? false : true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LsCompoundComponent::_HasWidth () const
    {
    return  m_size.y > 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Chuck.Kirschman   02/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool            LsCompoundComponent::_HasUniformFullWidth (double *pWidth) const
    {
    *pWidth = 0.0;
    double  maxWidth = 0.0;
    double  curWidth = 0.0;
    for (size_t compNum = 0; compNum < GetNumComponents(); compNum++)
        {
        if (!GetComponentCP (compNum)->_HasUniformFullWidth (&curWidth))
            return  false;
        if (curWidth > maxWidth)
            maxWidth = curWidth;
        }
    *pWidth = maxWidth;
    return true;
    }

/*----------------------------------------------------------------------------------*//**
* Check line style for continuous.  Criteria include:
* - Line codes only
*   - no components, or
*   - 1 huge dash & all dashes & no widths
* @bsimethod                                                    ChuckKirschman  04/02
+---------------+---------------+---------------+---------------+---------------+------*/
void            LsDefinition::CheckForContinuous
(
LsStrokePatternComponentCP    strokeComp
)
    {
    if ((NULL == strokeComp) || IsContinuous())
        return;

    if (strokeComp->_IsContinuous())
        m_attributes |= LSATTR_CONTINUOUS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void LsDefinition::PostProcessComponentLoad (DgnModelP modelRef)
    {
    if (NULL == modelRef)
        return;    //  defer post-processing until there is a request that supplies a modelRef.

    if (m_componentLoadPostProcessed)
        return;
        
    m_componentLoadPostProcessed = true;
    
    m_lsComp->_PostProcessLoad (modelRef);
    
    /* Look for continuous lines; mark them as such in the LineStyle */
    CheckForContinuous (dynamic_cast<LsStrokePatternComponentCP> (m_lsComp.get ()));

    if (IsContinuous() && !m_lsComp->_HasWidth ())
        m_attributes |= LSATTR_NOWIDTH;

    m_maxWidth = m_lsComp->_GetMaxWidth(modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void LsDefinition::ClearPostProcess()
    {
    m_componentLoadPostProcessed = false;
    
    if (NULL != m_lsComp.get ())
        m_lsComp->_ClearPostProcess ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
LineStyleStatus LsDefinition::SetComponent(LsComponentP lsComp)
    {
    if (NULL != lsComp && NULL != m_lsComp.get ())
        {
        if (lsComp->GetLocation ()->GetFileKey () != GetLocation ()->GetFileKey ())
            return LINESTYLE_STATUS_NotSameFile;
        }

    MarkDirty ();
    m_lsComp = lsComp;
    
    return LINESTYLE_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
LsComponentCP LsDefinition::GetComponentCP(DgnModelP modelRef) const
    {
    return GetComponentP (modelRef);
    }

/*---------------------------------------------------------------------------------**//**
  Get a temporary pointer to a component in the cache.
  If the component is not loaded, it and any necessary
  sub-components will be loaded.
* @bsimethod                                                    JimBartlett     3/92
+---------------+---------------+---------------+---------------+---------------+------*/
LsComponentP    LsDefinition::GetComponentP(DgnModelP modelRef) const
    {
    if (m_componentLookupFailed)
        return NULL;

    LsDefinitionP   nonConstThis = const_cast <LsDefinitionP> (this);

    // see if we have it cached. If so, use it.
    if (NULL != m_lsComp.get ())
        {
        nonConstThis->PostProcessComponentLoad (modelRef);
        return  m_lsComp.get ();
        }

    nonConstThis->m_componentLoadPostProcessed = false;
    LsComponentP    component = LsCache::GetLsComponent (nonConstThis->m_location);
    if (nullptr == component)
        {
        nonConstThis->m_componentLookupFailed = true;
        return NULL;
        }

    nonConstThis->SetComponent (component);
    nonConstThis->PostProcessComponentLoad (modelRef);
    
    return  component;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/92
+---------------+---------------+---------------+---------------+---------------+------*/
bool LsCompoundComponent::_ContainsComponent (LsComponentP other) const
    {
    if (LsComponent::_ContainsComponent(other))
        return  true;

    for (size_t i=0; i<GetNumComponents(); i++)
        {
        if (GetComponentCP (i)->_ContainsComponent (other))
            return  true;
        }

    return  false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
BentleyStatus       LsComponentReader::_LoadDefinition ()
    {
    if (NULL != m_rsc)
        return SUCCESS;

    HighPriorityOperationBlock highPriority; // see comments in BeSQLite.h
    m_componentType = m_source->GetComponentType();

    switch ((LsComponentType)m_componentType)
        {
        case LsComponentType::LineCode:
            LsStrokePatternComponent::CreateRscFromDgnDb ((LineCodeRsc**)&m_rsc, m_dgndb, m_source->GetComponentId(), false);
            break;

        case LsComponentType::Compound:
            LsCompoundComponent::CreateRscFromDgnDb ((LineStyleRsc**)&m_rsc, m_dgndb, m_source->GetComponentId(), false);
            break;

        case LsComponentType::LinePoint:
            LsPointComponent::CreateRscFromDgnDb ((LinePointRsc**)&m_rsc, m_dgndb, m_source->GetComponentId(), false);
            break;

        case LsComponentType::PointSymbol:
        //  case LS_ELEMENT_POINTSYMBOLV7:
            LsSymbolComponent::CreateRscFromDgnDb ((PointSymRsc**)&m_rsc, m_dgndb, m_source->GetComponentId(), false);
            break;

        case LsComponentType::Internal:
            break;
        }

    return  (NULL == m_rsc) ? ERROR : SUCCESS;
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/03
+---------------+---------------+---------------+---------------+---------------+------*/
static LsComponent*  loadComponent (LsComponentReader* reader)
    {
    if (SUCCESS != reader->_LoadDefinition())
        {
        if (LsComponentType::Internal == (LsComponentType)reader->GetComponentType())
            return LsInternalComponent::LoadInternalComponent (reader);

        return NULL;
        }

    switch ((LsComponentType)reader->GetComponentType())
        {
        case LsComponentType::Compound:
            return LsCompoundComponent::LoadCompoundComponent (reader);

        case LsComponentType::LineCode:
            return LsStrokePatternComponent::LoadStrokePatternComponent (reader);
            break;

        case LsComponentType::LinePoint:
            return LsPointComponent::LoadLinePoint (reader);

        case LsComponentType::PointSymbol:
            return LsSymbolComponent::LoadPointSym (reader);
            break;
        }

    return  NULL;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2015
//---------------------------------------------------------------------------------------
LsCacheP LsLocation::GetCacheP () const
    {
    if (GetDgnDb() == nullptr)
        return nullptr;

    return LsCache::GetDgnDbCache(*GetDgnDb(), true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    06/2015
//---------------------------------------------------------------------------------------
LsComponent* LsCache::GetLsComponent(LsLocationCR location)
    {
    LsCacheP lsCache = location.GetCacheP();
    BeAssert(nullptr != lsCache);
    if (nullptr == lsCache)
        return nullptr;

    auto iter = lsCache->m_loadedComponents.find(location);
    if (iter != lsCache->m_loadedComponents.end())
        return iter->second.get();

    LsComponentPtr comp = cacheLoadComponent (location);
    if (comp.IsNull())
        return nullptr;

    lsCache->m_loadedComponents[location] = comp;
    return comp.get();
    }