/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/linestyle/LsCache.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <DgnPlatform/LsLocal.h>

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
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsComponentReader*   LsComponentReader::GetRscReader (const LsLocation* source, DgnDbR project)
    {
    return  new LsComponentReader (source, project);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
void LsComponentReader::GetJsonValue(JsonValueR componentDef)
    {
    if (!Json::Reader::Parse(m_jsonSource, componentDef))
        return;
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
// @bsimethod                                                   John.Gooding    09/2015
//---------------------------------------------------------------------------------------
void LsComponent::UpdateLsOkayForTextureGeneration(LsOkayForTextureGeneration&current, LsOkayForTextureGeneration const&newValue)
    {
    //  The larger the value the worse it is (NotAllowed > ChangeRequired).  We want to record the worst value.
    if (newValue > current)
        current = newValue;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
void LsComponent::ExtractDescription(JsonValueCR result)
    {
    m_descr = LsJsonHelpers::GetString(result, "descr", "");
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
void LsComponent::SaveToJson(Json::Value& result) const
    {
    result.clear();
    Utf8String descr = GetDescription();
    if (descr.SizeInBytes() > 0)
        result["descr"] = descr.c_str();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
void LsComponent::GetNextComponentNumber (uint32_t& id, DgnDbR project, BeSQLite::PropertySpec spec)
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

    //  NOTNOW -- unclear what will happen to the magic values.  For now, avoid any collision with them.
    id = std::max(stmt.GetValueInt(0) + 1, MAX_LINECODE+1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
LineStyleStatus LsComponent::AddRasterComponentAsJson (LsComponentId& componentId, DgnDbR project, JsonValueCR jsonDefIn, uint8_t const*imageData, uint32_t dataSize)
    {
    //  First put out the raster image
    BeSQLite::PropertySpec spec = LineStyleProperty::RasterImage();

    uint32_t rasterImageNumber;
    GetNextComponentNumber (rasterImageNumber, project, spec);

    if (project.SaveProperty (spec, imageData, dataSize, rasterImageNumber, 0) != BE_SQLITE_OK)
        return LINESTYLE_STATUS_SQLITE_Error;

    spec = LineStyleProperty::RasterComponent();
    Json::Value jsonValue(jsonDefIn);
    jsonValue["imageId"] = rasterImageNumber;

    uint32_t componentNumber;
    GetNextComponentNumber (componentNumber, project, spec);
    Utf8String data = Json::FastWriter::ToString(jsonValue);

    if (project.SavePropertyString (spec, data.c_str(), componentNumber, 0) != BE_SQLITE_OK)
        return LINESTYLE_STATUS_SQLITE_Error;

    componentId = LsComponentId(LsComponentType::RasterImage, componentNumber);

    return LINESTYLE_STATUS_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
LineStyleStatus LsComponent::AddComponentAsJsonProperty (LsComponentId& componentId, DgnDbR project, LsComponentType componentType, JsonValueCR jsonValue)
    {
    BeSQLite::PropertySpec spec = LineStyleProperty::Compound();

    switch (componentType)
        {
        case LsComponentType::Compound:
            break;
        case LsComponentType::LineCode:
            spec = LineStyleProperty::LineCode();
            break;
        case LsComponentType::LinePoint:
            spec = LineStyleProperty::LinePoint();
            break;
        case LsComponentType::PointSymbol:
            spec = LineStyleProperty::PointSym();
            break;
        case LsComponentType::RasterImage:
            BeAssert(false && "use AddRasterComponentAsJson to add RasterImage");
            componentId = LsComponentId();
            return LINESTYLE_STATUS_ConvertingComponent;
        default:
            BeAssert(false && "invalid component type");
            componentId = LsComponentId();
            return LINESTYLE_STATUS_ConvertingComponent;
        }
    uint32_t componentNumber;
    GetNextComponentNumber (componentNumber, project, spec);

    Utf8String data = Json::FastWriter::ToString(jsonValue);

    if (project.SavePropertyString (spec, data.c_str(), componentNumber, 0) != BE_SQLITE_OK)
        return LINESTYLE_STATUS_SQLITE_Error;

    componentId = LsComponentId(componentType, componentNumber);
    return LINESTYLE_STATUS_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
LsComponent::LsComponent (DgnDbR project, LsComponentId componentId) : m_isDirty (false)
    {
    m_location.SetLocation (project, componentId);
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
LineStyleStatus LsStrokePatternComponent::CreateFromJson(LsStrokePatternComponentP* newLC, Json::Value const & jsonDef, LsLocationCP location)
    {
    LsStrokePatternComponentP retval = new LsStrokePatternComponent(location);
    retval->ExtractDescription(jsonDef);

    JsonValueCR strokes = jsonDef["strokes"];
    uint32_t nStrokes = strokes.size();

    if (nStrokes == 0)
        {
        // create a default solid stroke
        retval->AppendStroke (fc_hugeVal, true);
        }
    else
        {
        retval->m_nStrokes = nStrokes;
        if (retval->m_nStrokes > 32)
            retval->m_nStrokes = 32;

        for (uint32_t i = 0; i < retval->m_nStrokes; i++)
            {
            LsStroke&   pStroke = retval->m_strokes[i];
            JsonValueCR jsonStroke = strokes[i];
            double length = LsJsonHelpers::GetDouble(jsonStroke, "length", 0);
            double width = LsJsonHelpers::GetDouble(jsonStroke, "orgWidth", 0);
            double endWidth = LsJsonHelpers::GetDouble(jsonStroke, "endWidth", width);
            uint32_t strokeMode = LsJsonHelpers::GetUInt32(jsonStroke, "strokeMode", 0);
            uint32_t widthMode = LsJsonHelpers::GetUInt32(jsonStroke, "widthMode", 0);
            uint32_t capMode = LsJsonHelpers::GetUInt32(jsonStroke, "capMode", 0);
            
            pStroke.Init (length, width, endWidth, (LsStroke::WidthMode)widthMode, (LsCapMode)capMode);
            pStroke.SetIsDash (strokeMode & LCSTROKE_DASH);
            pStroke.SetIsRigid (TO_BOOL (strokeMode & LCSTROKE_RAY));
            pStroke.SetIsStretchable (TO_BOOL(strokeMode & LCSTROKE_SCALE));
            pStroke.SetIsDashFirst (pStroke.IsDash() ^ TO_BOOL(strokeMode & LCSTROKE_SINVERT));
            pStroke.SetIsDashLast  (pStroke.IsDash() ^ TO_BOOL(strokeMode & LCSTROKE_EINVERT));
            }

        uint32_t options = LsJsonHelpers::GetUInt32(jsonDef, "options", 0);
        uint32_t maxIterate = LsJsonHelpers::GetUInt32(jsonDef, "maxIter", 0);
        double phase = LsJsonHelpers::GetDouble(jsonDef, "phase", 0);
        retval->SetIterationLimit ((options & LCOPT_ITERATION) ? maxIterate : 0);
        retval->SetIterationMode ((options & LCOPT_ITERATION) ? true : false);
        retval->SetSegmentMode ((options & LCOPT_SEGMENT) ? true : false);

        if (!retval->IsRigid())
            {
            if (0 == (options & (LCOPT_AUTOPHASE | LCOPT_CENTERSTRETCH)) && 0.0 != phase)
                {
                retval->SetDistancePhase (phase);
                }
            else if (options & LCOPT_AUTOPHASE)
                {
                retval->SetFractionalPhase (phase);
                }
            else if (options & LCOPT_CENTERSTRETCH)
                {
                retval->SetCenterPhaseMode();
                }
            }
        }

    retval->CalcPatternLength();
    retval->PostCreate ();

    *newLC = retval;
    return LINESTYLE_STATUS_Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
void LsStrokePatternComponent::SaveToJson(Json::Value& result) const
    {
    LsComponent::SaveToJson(result);
    double phase = 0.0;
    uint32_t options = 0;
    uint32_t  maxIterate = 0;

    // Set phase, maxIterate, and options
    if (HasIterationLimit ())
        {
        maxIterate = GetIterationLimit ();
        options |= LCOPT_ITERATION;
        }

    if (IsSingleSegment())
        options |= LCOPT_SEGMENT;

    switch (GetPhaseMode ())
        {
        case PHASEMODE_Fixed:
            phase = GetDistancePhase ();
            break;
        case PHASEMODE_Fraction:
            phase = GetFractionalPhase ();
            options |= LCOPT_AUTOPHASE;
            break;
        case PHASEMODE_Center:
            phase = 0;
            options |= LCOPT_CENTERSTRETCH;
            break;
        }

    if (phase != 0)
        result["phase"] = phase;

    if (options != 0)
        result["options"] = options;

    if (maxIterate != 0)
        result["maxIter"] = maxIterate;

    Json::Value strokes(Json::arrayValue);
    for (uint32_t index = 0; index<m_nStrokes; ++index)
        {
        LsStroke const& stroke = m_strokes[index];
        Json::Value  entry(Json::objectValue);
        entry["length"] = stroke.m_length;
        if (stroke.m_orgWidth != 0)
            entry["orgWidth"] = stroke.m_orgWidth;
        if (stroke.m_endWidth != stroke.m_orgWidth)
            entry["endWidth"] = stroke.m_endWidth;
        if (stroke.m_strokeMode != 0)
            entry["strokeMode"] = stroke.m_strokeMode;
        if (stroke.m_widthMode != 0)
            entry["widthMode"] = stroke.m_widthMode;
        if (stroke.m_capMode != 0)
            entry["capMode"] = stroke.m_capMode;

        uint8_t strokeMode = 0;
        if (stroke.IsDash ())
            strokeMode  |= LCSTROKE_DASH;
        if (stroke.IsDashFirst () != stroke.IsDash())
            strokeMode  |= LCSTROKE_SINVERT;
        if (stroke.IsDashLast () != stroke.IsDash())
            strokeMode  |= LCSTROKE_EINVERT;
        if (stroke.IsRigid ())
            strokeMode  |= LCSTROKE_RAY;
        if (stroke.IsStretchable ())
            strokeMode  |= LCSTROKE_SCALE;

        if (strokeMode != 0)
            entry["strokeMode"] = strokeMode;

        strokes[index] = entry;
        }

    result["strokes"] = strokes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JimBartlett     08/92
+---------------+---------------+---------------+---------------+---------------+------*/
LsStrokePatternComponentP LsStrokePatternComponent::LoadStrokePatternComponent
(
LsComponentReader*    reader
)
    {
    Json::Value      jsonValue;
    reader->GetJsonValue(jsonValue);

    LsStrokePatternComponentP compPtr;
    
    LsStrokePatternComponent::CreateFromJson(&compPtr, jsonValue, reader->GetSource());

    return  compPtr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2015
//---------------------------------------------------------------------------------------
void LsCompoundComponent::_StartTextureGeneration() const
    {
    m_okayForTextureGeneration = LsOkayForTextureGeneration::Unknown; 
    for (LsOffsetComponent const & comp : m_components)
        comp.m_subComponent->_StartTextureGeneration();
    }
  
//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    11/2015
//---------------------------------------------------------------------------------------
void LsCompoundComponent::_QuerySymbology (SymbologyQueryResults& results) const
    {
    for (LsOffsetComponent const & comp : m_components)
        {
        comp.m_subComponent->_QuerySymbology(results);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2015
//---------------------------------------------------------------------------------------
LsOkayForTextureGeneration LsCompoundComponent::_IsOkayForTextureGeneration() const 
    {
    if (m_okayForTextureGeneration != LsOkayForTextureGeneration::Unknown)
        return m_okayForTextureGeneration;

    m_okayForTextureGeneration = LsOkayForTextureGeneration::NoChangeRequired; 

    for (LsOffsetComponent const & comp : m_components)
        {
        if (comp.m_subComponent->_IsOkayForTextureGeneration() > m_okayForTextureGeneration)
            UpdateLsOkayForTextureGeneration(m_okayForTextureGeneration, comp.m_subComponent->_IsOkayForTextureGeneration());
        }

    return m_okayForTextureGeneration;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2015
//---------------------------------------------------------------------------------------
 LsComponentPtr LsCompoundComponent::_GetForTextureGeneration() const
    {
    _IsOkayForTextureGeneration();
    BeAssert(LsOkayForTextureGeneration::NotAllowed != m_okayForTextureGeneration); //  The caller should have checked this.
    if (LsOkayForTextureGeneration::NoChangeRequired == m_okayForTextureGeneration)
        return const_cast<LsCompoundComponentP>(this);

    LsCompoundComponentP retval = new LsCompoundComponent(*this);
    for (LsOffsetComponent& comp : retval->m_components)
        comp.m_subComponent = comp.m_subComponent->_GetForTextureGeneration();

    m_okayForTextureGeneration = LsOkayForTextureGeneration::NoChangeRequired;
    return retval;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2015
//---------------------------------------------------------------------------------------
LsCompoundComponent::LsCompoundComponent(LsCompoundComponentCR source) : LsComponent(&source), m_postProcessed(false)
    {
    m_size = source.m_size;
    m_okayForTextureGeneration = LsOkayForTextureGeneration::Unknown;
    for (LsOffsetComponent const& child: source.m_components)
        m_components.push_back(child);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsCompoundComponent::LsCompoundComponent (LsLocation const *pLocation) :
            LsComponent (pLocation), m_postProcessed (false)
    {
    m_okayForTextureGeneration = LsOkayForTextureGeneration::Unknown;
    m_postProcessed = false;
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
StatusInt       LsCompoundComponent::_DoStroke (LineStyleContextR context, DPoint3dCP inPoints, int nPoints, LineStyleSymbCP modifiers) const
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
        //
        // Judging from the previous comment, this should have been testing for Internal.  It absolutely should be testing for
        // internal when computing the size that is required for generating a texture. If we find cases where it should not be 
        // testing for internal, then we need to track 2 separate sizes.
        if (!GetComponentCP(compNum)->_IsContinuous() && GetComponentCP(compNum)->GetComponentType() != LsComponentType::Internal)
            {
            if (GetComponentCP(compNum)->_GetLength() > m_size.x)
                m_size.x = GetComponentCP(compNum)->_GetLength();
            }

        double offset    = fabs (GetOffset (compNum));
        //  NEEDSWORK_LINESTYLE_UNITS
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
LsCompoundComponentP  LsCompoundComponent::LoadCompoundComponent(LsComponentReader* reader)
    {
    Json::Value jsonValue;
    reader->GetJsonValue(jsonValue);

    LsCompoundComponentP compPtr;
    LsCompoundComponent::CreateFromJson(&compPtr, jsonValue, reader->GetSource());

    return compPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsInternalComponent::LsInternalComponent (LsLocation const *pLocation) :
            LsStrokePatternComponent (pLocation)
    {
    uint32_t id = pLocation->GetComponentId().GetValue();
    m_hardwareLineCode = 0;
    if (id <= MAX_LINECODE)
#if defined (WIP_LINESTYLE)
        m_hardwareLineCode = id;
#else
        //  I am not certain that geometry textures will ever support line codes
        m_hardwareLineCode = 0;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   03/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       LsInternalComponent::_DoStroke (LineStyleContextR context, DPoint3dCP inPoints, int nPoints, LineStyleSymbCP modifiers) const
    {
#if defined (WIP_LINESTYLE)
    if (GetLocation ()->IsInternalDefault ())
        return LsStrokePatternComponent::_DoStroke (context, inPoints, nPoints, modifiers);

    GraphicParams saveMatSymb;
    saveMatSymb = *context->GetGraphicParams (); // Copy current GraphicParams

#if defined (NEEDS_WORK_DGNITEM)
    // Keep GeometryParams and GraphicParams in synch...operations like drop lstyle use GeometryParams not GraphicParams.
    GeometryParamsStateSaver saveState (*context->GetCurrentGeometryParams (), false, false, false, true, false);
#endif

    // It's important to set the style via GeometryParams, not GraphicParams, for printing to work correctly.
    context->GetCurrentGeometryParams ()->SetLineStyle (style);
    context->CookGeometryParams ();
    context->GetIDrawGeom ().ActivateGraphicParams (context->GetGraphicParams ()); // Activate the new matsymb

    // Style override that caused this linestyle to be used needs to be cleared in order to use the correct raster pattern for the strokes. 
    OvrGraphicParamsP ovrMatSymb = context->GetOverrideGraphicParams ();
    uint32_t    saveFlags = ovrMatSymb->GetFlags ();

    if (0 != (saveFlags & MATSYMB_OVERRIDE_Style))
        {
        ovrMatSymb->SetFlags (saveFlags & ~MATSYMB_OVERRIDE_Style);
        context->GetIDrawGeom ().ActivateOverrideGraphicParams (ovrMatSymb);
        }

    context->GetIDrawGeom ().AddLineString (nPoints, inPoints, NULL); // Draw the linestring

    // Restore GraphicParams to previous state, GeometryParams will be restored in GeometryParamsStateSaver destructor...
    context->GetIDrawGeom ().ActivateGraphicParams (&saveMatSymb);

    if (0 != (saveFlags & MATSYMB_OVERRIDE_Style))
        {
        ovrMatSymb->SetFlags (saveFlags);
        context->GetIDrawGeom ().ActivateOverrideGraphicParams (ovrMatSymb);
        }

    return SUCCESS;
#else

    #if defined(NOTNOW)
    int32_t style = GetHardwareStyle ();
    if (style >= MIN_LINECODE || style <= MAX_LINECODE)
        {
        //  I would like to do this here but it doesn't work with geometry textures.  I don't know if that will be fixed.
        //  Need to save and restore the GraphicParams
        GraphicParamsR params = context.GetGraphicParamsR();
        params.SetLinePixels(Render::GraphicParams::LinePixels(style));
        context.GetGraphicR().ActivateGraphicParams(params);
        context.GetGraphicR().AddLineString (nPoints, inPoints);
        return BSISUCCESS;
        }
    #endif

    return LsStrokePatternComponent::_DoStroke (context, inPoints, nPoints, modifiers);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    John.Gooding                    10/2009
+---------------+---------------+---------------+---------------+---------------+------*/
LsInternalComponentPtr LsInternalComponent::CreateInternalComponent(LsLocation& location)
    {
    LsInternalComponent*  comp = new LsInternalComponent (&location);
    comp->m_isDirty = true;

    comp->AppendStroke (fc_hugeVal, true);
    comp->CalcPatternLength();
    comp->PostCreate ();

    return comp;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2012
//--------------+------------------------------------------------------------------------
StatusInt LsDefinition::UpdateStyleTable () const
    {
    DgnDbP project = GetLocation()->GetDgnDb();
    project->Styles ().LineStyles().Update (DgnStyleId(m_styleId), _GetName(), GetLocation()->GetComponentId(), GetAttributes(), m_unitDef);

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   01/03
+---------------+---------------+---------------+---------------+---------------+------*/
LsStrokePatternComponentP  LsInternalComponent::LoadInternalComponent(LsComponentReader*reader)
    {
    const LsLocation* location = reader->GetSource();

    LsInternalComponent*  comp = new LsInternalComponent (location);
    
    comp->AppendStroke (fc_hugeVal, true);
    comp->CalcPatternLength();
    comp->PostCreate ();

    //  The original code set its type back to match the original type so it would 
    //  be found in the search but I don't understand when they differ.
    //  BeAssert (reader->GetRscType() == LsResourceType::Internal);

    return  comp;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    07/2015
//---------------------------------------------------------------------------------------
LsRasterImageComponent* LsRasterImageComponent::LoadRasterImage  (LsComponentReader* reader)
    {
    Json::Value      jsonValue;
    reader->GetJsonValue(jsonValue);

    LsRasterImageComponentP newComp;
    LsRasterImageComponent::CreateFromJson(&newComp, jsonValue, reader->GetSource());

    return newComp;
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
#if defined(NEEDSWORK_LINESTYLE_MODELREF)
    if (NULL == modelRef)
        return;    //  defer post-processing until there is a request that supplies a modelRef.
#endif

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
    LsLocation  location = nonConstThis->m_location;

    LsComponentP    component = DgnLineStyles::GetLsComponent (location);
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
    if (m_jsonSource.size() > 0)
        return SUCCESS;

    BeSQLite::PropertySpec spec = LineStyleProperty::Compound();

    switch (m_source->GetComponentType())
        {
        case LsComponentType::Compound:
            break;
        case LsComponentType::LineCode:
            spec = LineStyleProperty::LineCode();
            break;
        case LsComponentType::LinePoint:
            spec = LineStyleProperty::LinePoint();
            break;
        case LsComponentType::PointSymbol:
            spec = LineStyleProperty::PointSym();
            break;
        case LsComponentType::RasterImage:
            spec = LineStyleProperty::RasterComponent();
            break;
        case LsComponentType::Internal:
            return ERROR;

        default:
            BeAssert(false && "invalid component type");
        }

    GetDgnDb().QueryProperty(m_jsonSource, spec, GetSource()->GetComponentId().GetValue());
    return  (m_jsonSource.size() == 0) ? ERROR : SUCCESS;
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

        case LsComponentType::RasterImage:
            return LsRasterImageComponent::LoadRasterImage (reader);
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
LsComponent* DgnLineStyles::GetLsComponent(LsLocationCR location)
    {
    DgnLineStyles& dgnLineStyles = location.GetDgnDb()->Styles().LineStyles();

    auto iter = dgnLineStyles.m_loadedComponents.find(location);
    if (iter != dgnLineStyles.m_loadedComponents.end())
        return iter->second.get();

    LsComponentPtr comp = cacheLoadComponent (location);
    if (comp.IsNull())
        return nullptr;

    dgnLineStyles.m_loadedComponents[location] = comp;
    return comp.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    10/2015
//---------------------------------------------------------------------------------------
LsComponentPtr DgnLineStyles::GetLsComponent(LsComponentId componentId)
    {
    if (!componentId.IsValid())
        return nullptr;

    LsLocation   location;
    location.SetLocation(m_dgndb, componentId);
    return GetLsComponent(location);
    }