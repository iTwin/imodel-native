/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/linestyle/LsCache.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <DgnPlatform/DgnCore/LsLocal.h>

static LsComponent*  _loadComponent (LsRscReader* reader);
static  bool    s_loadingComponents = false;


USING_NAMESPACE_BENTLEY_SQLITE

/*=================================================================================**//**
* @bsiclass                                                     Keith.Bentley   01/03
+===============+===============+===============+===============+===============+======*/
struct LsComponentKey
{
private:
    LsComponentPtr m_entry;

public:
    LsComponent* GetValue() const {return m_entry.get();}

    LsComponentKey (LsComponent const* entry) {m_entry = const_cast<LsComponent*>(entry);}
    LsComponentKey () {}

    inline int Compare (LsComponentKey* other) const
        {
        const LsComponent*  otherEntry = other->GetValue ();

        if (otherEntry == GetValue())
            return _EQ_;
        if (!m_entry.IsValid())
            return _LT_;
        if (NULL == otherEntry)
            return _GT_;

        LsLocation const* thisComp  = GetValue()->GetLocation();
        LsLocation const* otherComp = otherEntry->GetLocation();

        if (thisComp->GetFileKey() != otherComp->GetFileKey())
            return  thisComp->GetFileKey() > otherComp->GetFileKey() ? _GT_ : _LT_;

        if (thisComp->GetRscType() != otherComp->GetRscType())
            return  thisComp->GetRscType() > otherComp->GetRscType() ? _GT_ : _LT_;

        if (thisComp->GetIdentKey() != otherComp->GetIdentKey())
            return  thisComp->GetIdentKey() > otherComp->GetIdentKey() ? _GT_ : _LT_;

        return  _EQ_;
        }

    bool operator< (LsComponentKey other) const {return Compare (&other) <  _EQ_;}
};

/*=================================================================================**//**
* @bsiclass                                                     Keith.Bentley   03/03
+===============+===============+===============+===============+===============+======*/
struct LsComponentTree : bset<LsComponentKey>
{
public:
    void AddToCompCache (LsComponent* comp) {insert (comp);}
    void DeleteMatchingComponent (LsComponent* compareComponent) {erase (compareComponent); }
    void GetComponentList (T_LsComponents& components, intptr_t fileKey, uint32_t componentType);
    void Empty ();
};

static LsComponentTree    s_lsCompCache;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void LsComponentTree::GetComponentList (T_LsComponents& components, intptr_t fileKey, uint32_t componentType)
    {
    for (auto it=begin(), done=end(); it!=done; ++it)
        {
        LsComponentP     comp = it->GetValue();
    
        if (comp->GetLocation ()->GetFileKey () != fileKey)
            continue;
        
        if (0 != componentType && (uint32_t)comp->GetLocation ()->GetRscType () != componentType)
            continue;
        
        components.push_back (comp);
        }
    }

/*---------------------------------------------------------------------------------**//**
* Empty the (one and only) tree of Linestyle Components. To do this, we need to first 
* drop all references to them in the LineStyles in all of the NameTrees for all open 
* DgnFiles.
* @bsimethod                                                    Keith.Bentley   03/03
+---------------+---------------+---------------+---------------+---------------+------*/
void LsComponentTree::Empty ()
    {
    // if there's no entries, we're done.
    if (empty())
        return;

    // run through all of the LineStyle entries in all LsMap's dropping any references to LsComponents (we're about to delete them all).
    LsMap::DropAllComponentRefs();

    clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
static LsComponent* lookupComponent (LsComponent* in)
    {
    BeAssert (0 < in->GetRefCount());
    auto node = s_lsCompCache.find(in);
    return  node!=s_lsCompCache.end() ? node->GetValue() : NULL;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
LsDgnDbReader::~LsDgnDbReader ()
    {
    FREE_AND_CLEAR (m_rsc);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsRscReader*   LsRscReader::GetRscReader (const LsLocation* source, DgnDbR project)
    {

    switch (source->GetSourceType())
        {
        case LsLocationType::DgnDb:
            return  new LsDgnDbReader (source, project);
        }

    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/92
+---------------+---------------+---------------+---------------+---------------+------*/
static LsComponent* cacheLoadComponent(LsLocation*location)
    {
    LsRscReader* reader = LsRscReader::GetRscReader (location, *location->GetDgnDb());

    if (NULL == reader)
        return  NULL;

    LsComponent* component = _loadComponent (reader);
    delete  reader;

    return component;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
void LsComponent::GetNextComponentId (uint32_t& id, DgnDbR project, PropertySpec spec)
    {
    SqlPrintfString sql("SELECT  max(Id) from " BEDB_TABLE_Property " where Namespace='%s' and Name='%s'", spec.GetNamespace(), spec.GetName());
    Statement stmt;
    stmt.Prepare(project, sql.GetUtf8CP());
    DbResult result = stmt.Step();
    if (BE_SQLITE_DONE != result && BE_SQLITE_ROW != result)
        {
        id = 1;
        return;
        }

    id = (uint32_t)stmt.GetValueInt(0) + 1;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2012
//---------------------------------------------------------------------------------------
LineStyleStatus LsComponent::AddComponentAsProperty (uint32_t& componentId, DgnDbR project, PropertySpec spec, void const*data, uint32_t dataSize)
    {
    GetNextComponentId (componentId, project, spec);

    if (project.SaveProperty (spec, data, dataSize, componentId, 0) != BE_SQLITE_OK)
        return LINESTYLE_STATUS_SQLITE_Error;

    return LINESTYLE_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
LineStyleStatus LsComponent::GetComponentList(T_LsComponents& components, intptr_t fileKey, uint32_t componentType)
    {
    s_lsCompCache.GetComponentList (components, fileKey, componentType);
    return LINESTYLE_STATUS_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
LsComponent::LsComponent (DgnDbR project, uint32_t componentType, uint32_t componentId) : m_isDirty (false)
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
void            LsComponent::CopyDescription (CharP buffer) const
    {
    m_descr.ConvertToLocaleChars (buffer, LS_MAX_DESCR);
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
        SetDescription (WString(lcRsc->descr, false).c_str());

    CreateFromRsrc (lcRsc);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/92
+---------------+---------------+---------------+---------------+---------------+------*/
LsStrokePatternComponentP LsStrokePatternComponent::LoadStrokePatternComponent
(
LsRscReader*    reader
)
    {
    LineCodeRsc*        lcRsc = (LineCodeRsc*) reader->GetRsc();
    const LsLocation*   source = reader->GetSource();

    //  This used to handle LsInternalComponent of line code 0 setting resource type to LsElementType::LineCode.
    
    LsStrokePatternComponent* strokeComp = new LsStrokePatternComponent (source);
    strokeComp->Init (lcRsc);

    s_lsCompCache.AddToCompCache (strokeComp);

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

        componentInfo->type = (uint32_t)loc->GetRscType ();
        componentInfo->id = loc->GetRscID ();
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

    LineJoint*  joints = (LineJoint*) _alloca (nPoints * sizeof(LineJoint));
    LineJoint::FromVertices (joints, inPoints, nPoints, &normal, NULL, NULL);

    DPoint3d*   offsetPts = (DPoint3d*) _alloca (nPoints * sizeof (DPoint3d));
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
LsRscReader*    reader
)
    {
    LineStyleRsc*   lsRsc = reader->GetRsc();
    if (NULL == lsRsc)
        return  NULL;

    LsCompoundComponent*  compound = new LsCompoundComponent (reader->GetSource());
    compound->SetDescription (WString(lsRsc->descr, false).c_str());

    s_lsCompCache.AddToCompCache (compound);

    LsLocation  tmpLocation;

    for (uint32_t i=0; i < lsRsc->nComp; i++)
        {
        tmpLocation.GetCompoundComponentLocation (reader, i);
        
        LsOffsetComponent offsetComp (lsRsc->component[i].offset, LineStyleCacheManager::GetSubComponent (&tmpLocation, reader->GetDgnDb()));
        
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
    uint32_t id = (uint32_t) pLocation->GetIdentKey();

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
    tmpLocation.SetFrom (&location, (uint32_t)LsResourceType::Internal);

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
    project->Styles ().LineStyles().Update (DgnStyleId(m_styleNumber), _GetName(), GetLocation()->GetRscID(),
                                        static_cast <uint16_t> (GetLocation()->GetRscType()), GetAttributes(), m_unitDef);

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// This does not transfer the component or update the component ID. That has to be
// done separately.
//
// @bsimethod                                                   John.Gooding    11/2012
//--------------+------------------------------------------------------------------------
StatusInt LsDefinition::TransferStyleTableEntry(DgnStyleId& newId, DgnDbR targetProject) const
    {
    targetProject.Styles ().LineStyles().Insert (newId, _GetName(), GetLocation()->GetRscID(),
                                             static_cast <uint16_t> (GetLocation()->GetRscType()), GetAttributes(), m_unitDef);

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
void LsDefinition::RemapComponentId (uint32_t newId)
    {
    m_location.UpdateRscID(newId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsStrokePatternComponentP  LsInternalComponent::LoadInternalComponent
(
LsRscReader*    reader
)
    {
    const LsLocation* location = reader->GetSource();

    LsLocation  tmpLocation;
    tmpLocation.SetFrom (location, (uint32_t)LsResourceType::Internal);
    LsInternalComponent*  comp = new LsInternalComponent (location);
    
    comp->Init (NULL);

    //  The original code set its type back to match the original type so it would 
    //  be found in the search but I don't understand when they differ.
    //  BeAssert (reader->GetRscType() == LsResourceType::Internal);

    s_lsCompCache.AddToCompCache (comp);
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
    LsComponent searchComp (&m_location);
    searchComp.AddRef(); // we never want this to be deleted!

    LsComponentP    component = lookupComponent (&searchComp);

    if (NULL == component)
        {
        s_loadingComponents = true;  // Prevent cache unloading due to Level Libraries.
        component = cacheLoadComponent (&nonConstThis->m_location);
        s_loadingComponents = false;

        if (NULL == component)
            {
            nonConstThis->m_componentLookupFailed = true;
            return NULL;
            }
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
BentleyStatus       LsDgnDbReader::_LoadDefinition ()
    {
    if (NULL != m_rsc)
        return SUCCESS;

    HighPriorityOperationBlock highPriority; // see comments in BeSQLite.h
    m_rscType = (uint32_t)m_source->GetRscType();

    switch ((LsElementType)m_rscType)
        {
        case LsElementType::LineCode:
            LsStrokePatternComponent::CreateRscFromDgnDb ((LineCodeRsc**)&m_rsc, m_dgndb, m_source->GetRscID(), false);
            break;

        case LsElementType::Compound:
            LsCompoundComponent::CreateRscFromDgnDb ((LineStyleRsc**)&m_rsc, m_dgndb, m_source->GetRscID(), false);
            break;

        case LsElementType::LinePoint:
            LsPointComponent::CreateRscFromDgnDb ((LinePointRsc**)&m_rsc, m_dgndb, m_source->GetRscID(), false);
            break;

        case LsElementType::PointSymbol:
        //  case LS_ELEMENT_POINTSYMBOLV7:
            LsSymbolComponent::CreateRscFromDgnDb ((PointSymRsc**)&m_rsc, m_dgndb, m_source->GetRscID(), false);
            break;

        case LsElementType::Internal:
            break;
        }

    return  (NULL == m_rsc) ? ERROR : SUCCESS;
    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/03
+---------------+---------------+---------------+---------------+---------------+------*/
static LsComponent*  _loadComponent (LsRscReader* reader)
    {
    if (SUCCESS != reader->_LoadDefinition())
        {
        if (LsElementType::Internal == (LsElementType)reader->GetRscType())
            return LsInternalComponent::LoadInternalComponent (reader);

        return NULL;
        }

    switch ((LsElementType)reader->GetRscType())
        {
        case LsElementType::Compound:
            return LsCompoundComponent::LoadCompoundComponent (reader);

        case LsElementType::LineCode:
            return LsStrokePatternComponent::LoadStrokePatternComponent (reader);
            break;

        case LsElementType::LinePoint:
            return LsPointComponent::LoadLinePoint (reader);

        case LsElementType::PointSymbol:
            return LsSymbolComponent::LoadPointSym (reader);
            break;
        }

    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
* Get a pointer to a component in the component cache. If the component is not in the cache it will be loaded.
* @bsimethod                                                    JimBartlett     01/92
+---------------+---------------+---------------+---------------+---------------+------*/
LsComponentP LineStyleCacheManager::GetSubComponent(LsLocationCP location, DgnDbR project)
    {
    // A resource file handle of zero is illegal when loading sub-components
    if (!location->IsValid())
        return  NULL;

    LsComponent searchComp (location);
    searchComp.AddRef(); // we never want this to be deleted!

    LsComponent* found = lookupComponent (&searchComp);
    if (NULL != found)
        return  found;

    return  cacheLoadComponent (const_cast <LsLocationP> (location));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ChuckKirschman  02/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStyleCacheManager::CacheFree
(
)
    {
    // since there's only one component cache, the only way to delete the entries for a modelref is to delete ALL entries.
    // That's suboptimal.  This function is called from the LS Editor and when a file is closed.  It's the latter case that
    // caught me.  It turns out that if there are level libraries specified in a DGNLIB that contains line styles, and there
    // is a style where the level is missing (from older code that didn't check line style levels) then when the level library
    // is closed all the caches are cleared, which is disastrous during a load.
    if (!s_loadingComponents)
        s_lsCompCache.Empty ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ChuckKirschman  02/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStyleCacheManager::CacheDelete
(
LsLocation const*   searchLocation,
int                 option      /* Currently unused */
)
    {
    // since there's only one component cache, the only way to delete the entries for a modelref is to delete ALL entries.
    // That's OK, this is only used for the linestyle editor and the cache will just get repopulated the next time the components are referenced.
    s_lsCompCache.Empty ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ChuckKirschman  09/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStyleCacheManager::CacheDeleteComponent
(
LsComponent&    compareComponent,
int             option      /* Currently unused */
)
    {
    s_lsCompCache.DeleteMatchingComponent (&compareComponent);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ChuckKirschman  02/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStyleCacheManager::CacheDelete
(
uint32_t        rscFile,
uint32_t        rscType,
uint32_t        rscID,
int             option      /* Currently unsed */
)
    {
#ifdef DGNV10FORMAT_CHANGES_WIP_LINESTYLES
    LsComponent compareComponent (rscFile, rscType, rscID);
    CacheDeleteComponent (compareComponent, option);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ChuckKirschman  02/03
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStyleCacheManager::CacheDelete
(
DgnDbP          dgnFile,
long            lsType, 
DgnElementId    elementID,
int             option      /* Currently unsed */
)
    {
#ifdef DGNV10FORMAT_CHANGES_WIP_LINESTYLES
    LsComponent compareComponent (dgnFile, lsType, elementID);
    return CacheDeleteComponent (compareComponent, option);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* This function is used by the editors to load a temporary line style into the cache
* so it can be displayed in the preview window.
* @bsimethod                                                    JimBartlett     08/92
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LineStyleCacheManager::CacheInsert (DgnDbP dgnFile, long compType, DgnElementId compID, void* pRsc, long option)
    {
#ifdef DGNPROJECT_MODELS_CHANGES_WIP
    DgnModelP modelRef = dgnFile->GetFirstDgnModel();

    CacheDelete (dgnFile, compType, compID, option);

    LsLocation      tmpLocation;
    tmpLocation.SetLocation (dgnFile, compType, compID);

    LsRscReader* reader = LsRscReader::GetRscReader (&tmpLocation, modelRef);

    if (NULL == reader)
        return  ERROR;

    reader->SetRsc ((LineStyleRsc*)pRsc);
    LsComponent* component = _loadComponent (reader);
    reader->SetRsc (NULL);
    delete  reader;

    return  NULL != component ? SUCCESS : ERROR;
#endif
    return  ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* This function is used by the editors to load a temporary line style into the cache
* so it can be displayed in the preview window.
* @bsimethod                                                    JimBartlett     08/92
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LineStyleCacheManager::CacheInsert (RscFileHandle rscFile, long rscType, uint32_t rscId, DgnDbP dgnFile, void* pRsc, long option)
    {
#ifdef DGNPROJECT_MODELS_CHANGES_WIP
    DgnModelP modelRef = dgnFile->GetFirstDgnModel();

    CacheDelete (rscFile, rscType, rscId, option);

    LsLocation      tmpLocation;
    tmpLocation.SetLocation (rscFile, rscType, rscId);

    LsRscReader* reader = LsRscReader::GetRscReader (&tmpLocation, modelRef);

    if (NULL == reader)
        return  ERROR;

    reader->SetRsc ((LineStyleRsc*)pRsc);
    LsComponent* component = _loadComponent (reader);
    reader->SetRsc (NULL);
    delete  reader;

    return  NULL != component ? SUCCESS : ERROR;
#endif
    return  ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    09/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void            LineStyleCacheManager::CacheAdd (LsComponent* comp)
    {
    s_lsCompCache.AddToCompCache (comp);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ChuckKirschman  01/01
+---------------+---------------+---------------+---------------+---------------+------*/
LsElementType BentleyApi::remapRscTypeToElmType  /* <= Elm type */
(
uint32_t    rscType     /* => Resource type */
)
    {
    LsElementType elmType = (LsElementType)rscType;

    switch  ((LsResourceType)rscType)
        {
        case LsResourceType::Compound:
            elmType = LsElementType::Compound;
            break;

        case LsResourceType::LineCode:
            elmType = LsElementType::LineCode;
            break;

        case LsResourceType::LinePoint:
            elmType = LsElementType::LinePoint;
            break;

        case LsResourceType::PointSymbol:
        case LsResourceType::PointSymbolV7:
            elmType = LsElementType::PointSymbol;
            break;

        case LsResourceType::Internal:
            elmType = LsElementType::Internal;
            break;
        }

    return elmType;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    ChuckKirschman  01/01
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t BentleyApi::remapElmTypeToRscType  /* <= Rsc type */
(
LsElementType    elmType     /* => Elm type */
)
    {
    //  This function is a vestige of the support for line styles from resource files
    //  and from dgn files. In V10 they only come from projects.
    return (uint32_t)elmType;
    }

