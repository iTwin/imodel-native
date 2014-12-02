/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnFileIO/DgnModel.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#ifdef NDEBUG
#   define REPORT_ID_REMAP(errmsg)            // if we're not in debug mode, just ignore the problem
#else
//#   define REPORT_ID_REMAP(errmsg) toolSubsystem_printf ("Invalid ID %llu (" ## errmsg ## "), will be remapped\n", elemId);
#if defined (BEIJING_WIP_DATA_ASSERT) // must turn this off in order to run tests
#   define REPORT_ID_REMAP(errmsg) BeAssert (0 && L"Invalid ID: " errmsg);              // for debugging
#else
#   define REPORT_ID_REMAP(errmsg) toolSubsystem_wprintf (L"Invalid ID %llu ("  errmsg  L"), will be remapped\n", elemId);
#endif
#endif

enum
    {
    MIN_GRAPHIC_ELM_WORDS = sizeof(DgnElement) / 2,
    MIN_ELEMENT_WORDS     = sizeof(DgnElementHeader) / 2,
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    11/00
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentElementRefList::PersistentElementRefList (ElementListHandlerR listHandler, DgnModelP dgnModel) : m_listHandler(listHandler), m_dgnModel(dgnModel)
    {
    m_wasFilled = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    11/00
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentElementRefList::~PersistentElementRefList ()
    {
    ReleaseAllElements();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    11/00
+---------------+---------------+---------------+---------------+---------------+------*/
void PersistentElementRefList::ReleaseAllElements()
    {
    m_wasFilled  = false;   // this must be before we release all elementrefs
    m_ownedElems.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
ElementRefVec PersistentElementRefList::MakeElementRefVec() const
    {
    ElementRefVec elements;
    elements.reserve (m_ownedElems.size());
    for (auto const& it : m_ownedElems)
        {
        if (!it.second->IsDeleted())
            elements.push_back (it.second.get());
        }

    return elements;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   02/04
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus ElementListHandler::CheckElementIntegrity (DgnElementR el)
    {
#if defined (NEEDS_WORK_DGNITEM)
    // don't allow type 0 elements or elements with invalid sizes
    if (el.GetLegacyType() < 1)
        return DGNMODEL_STATUS_BadElement;

    bool   isGraphics = el.IsGraphic();
    UInt32 minSize    = isGraphics ? MIN_GRAPHIC_ELM_WORDS : MIN_ELEMENT_WORDS;

    bool   is3d       = isGraphics ? el.Is3d() : false;
    switch (el.GetLegacyType())
        {
        case CELL_HEADER_ELM:
            minSize = is3d ? sizeof (Cell_3d) : sizeof (Cell_2d);
            break;

        case LINE_ELM:
            minSize = is3d ? sizeof (Line_3d) : sizeof (Line_2d);
            break;

        case LINE_STRING_ELM:
        case SHAPE_ELM:
        case CURVE_ELM:
        case BSPLINE_POLE_ELM:
        case POINT_STRING_ELM:
            minSize = is3d ? sizeof (Line_String_3d) : sizeof (Line_String_2d);

        case GROUP_DATA_ELM:
            break;

        case TEXT_NODE_ELM:
            minSize = is3d ? sizeof (Text_node_3d) : sizeof (Text_node_2d);
            break;

        case LEV_SYM_ELM:
            break;

        case CMPLX_STRING_ELM:
        case CMPLX_SHAPE_ELM:
            minSize = sizeof (Complex_string);
            break;

        case ELLIPSE_ELM:
            minSize = is3d ? sizeof(Ellipse_3d) : sizeof(Ellipse_2d);
            break;

        case ARC_ELM:
            minSize = is3d ? sizeof(Arc_3d) : sizeof(Arc_2d);
            break;

        case TEXT_ELM:
            minSize = (is3d ? sizeof(Text_3d) : sizeof(Text_2d)) - 6;  // for some reason, there's six characters in the structure.
            break;

        case SURFACE_ELM:
        case SOLID_ELM:
            minSize = sizeof(Surface) - 6;
            break;

        case CONE_ELM:
            minSize = sizeof (Cone_3d);
            break;

        case BSPLINE_SURFACE_ELM:
            minSize = sizeof (Bspline_surface);
            break;

        case BSURF_BOUNDARY_ELM:
            minSize = sizeof (Bsurf_boundary);
            break;

        case BSPLINE_KNOT_ELM:
            minSize = sizeof (Bspline_knot) - sizeof (double);
            break;

        case BSPLINE_CURVE_ELM:
            minSize = sizeof (Bspline_curve);
            break;

        case BSPLINE_WEIGHT_ELM:
            minSize = sizeof (Bspline_weight);
            break;

        case DIMENSION_ELM:
            minSize = sizeof (DimensionElm);
            break;

        case SHAREDCELL_DEF_ELM:
            minSize = sizeof (SharedCellDef);
            break;

        case SHARED_CELL_ELM:
            minSize = sizeof (SharedCell);
            break;

        case MULTILINE_ELM:
            minSize = sizeof (MlineElm);
            break;

        #ifdef REMOVED_IN_GRAPHITE
        case ATTRIBUTE_ELM:
            minSize = offsetof (AttributeElm, iCodePage);
            break;
        #endif

        case MICROSTATION_ELM:
            break;

        case RASTER_FRAME_ELM:
            {
            RasterFrameElm * pType94Elm = (RasterFrameElm *)&el;

            if (pType94Elm->version >= V10_RASTER_FRAME_FIRSTVERSION)
                {
                minSize = sizeof(RasterFrameElm);
                }
            }
            break;

        case MESH_HEADER_ELM:
            minSize = sizeof (Mesh_header);
            break;

        case MATRIX_HEADER_ELM:
            minSize = sizeof (Matrix_header);
            break;

        case MATRIX_INT_DATA_ELM:
        case MATRIX_DOUBLE_DATA_ELM:
            minSize = offsetof (Matrix_double_data, data);
            break;
        }

    // If the attr-offset is too small, correct it.
    if (minSize > (el.GetAttributeOffset()*2))
        el.SetAttributeOffset(minSize/2);

    if ((el.GetSizeWords() * 2) < minSize)
        return DGNMODEL_STATUS_UndersizeElement;
#endif

    if (el.GetSizeWords() < el.GetAttributeOffset())
        return  DGNMODEL_STATUS_InvalidAttrOffset;

    if (!_VerifyDimensionality (el))
        {
#if defined (DEBUG_2D3D_PROBLEMS)
        toolSubsystem_printf ("ERROR - adding element (type=%d) with wrong is3D flag, rejecting\n");
        BeAssert(0);
#endif
        return DGNMODEL_STATUS_2d3dMismatch;
        }

    return  DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* verify that each element of an element descriptor has a valid type and size
* and that, if it is a complex header element, critical stuff like the component count is set correctly.
* @bsimethod                                                    BrienBastings   06/00
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus ElementListHandler::_VerifyElemDescr (MSElementDescrR descr)
    {
    DgnElementR element = descr.ElementR();

    // Always check the element, some minor fixes may be possible.
    DgnModelStatus status = CheckElementIntegrity (element);
    if (DGNMODEL_STATUS_Success != status)
        return status;

    return DGNMODEL_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    02/02
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementRefR ElementListHandler::CreateNewElementRef (DgnModelR dgnModel, MSElementDescrR descr, AddElementOptions const& opts)
    {
    DgnElementP element = &descr.ElementR();

    PersistentElementRefP persistentRef = NULL;
    DgnElementRefP thisElRef = persistentRef = dgnModel.GetDgnProject().Models().ElementPool().AllocateElementRef(dgnModel, element->GetElementId());

    if (!opts.IsLoadFromFile())
        thisElRef->SetDirtyFlags (ElementRef::DIRTY_ElemData);

    thisElRef->m_elementId = element->GetElementId();
    thisElRef->SaveElement (element);

    thisElRef->_GetHeaderFieldsFrom(descr);
    descr.SetElementRef(thisElRef);    // save the elementref into the descriptor

    return *thisElRef;
    }

/*---------------------------------------------------------------------------------**//**
* test all of the rules about values for new elementIDs and return true or false for this value.
* @bsimethod                                                    KeithBentley    02/02
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnModels::IsValidNewElementId (ElementId elemId)
    {
    // invalid means not assigned yet
    if (!elemId.IsValid())
        return  false;

    return elemId>=GetLowestNewElementId() && elemId<=GetHighestElementId() && !IsElementIdUsed(elemId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
AddNewElementOptions::AddNewElementOptions(DgnModelR model) : AddElementOptions(true, model) 
    {
    m_createTime = osTime_getCurrentMillis();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
void AddElementOptions::_ResolveHandler (MSElementDescrR descr) const
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void AddElementOptions::_OnAdded(PersistentElementRefR elRef, bool isGraphicsList) const 
    {
    PersistentElementRefList* list = m_model.GetGraphicElementsP();
    if (list)
        list->_OnElementAdded(elRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
void AddElementOptions::_ValidateIds(DgnProjectR dgnFile, MSElementDescrR descr) const
    {
    DgnElementP el = &descr.ElementR();
    if (0 == el->GetElementId().GetValueUnchecked())
        el->SetElementId(dgnFile.Models().MakeNewElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
void AddNewElementOptions::_ValidateIds(DgnProjectR dgnFile, MSElementDescrR descr) const
    {
    DgnElementP el = &descr.ElementR();
    if (!dgnFile.Models().IsValidNewElementId(el->GetElementId()))
        el->SetElementId(dgnFile.Models().MakeNewElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus AddElementOptions::_VerifyElemDescr(ElementListHandlerR list, MSElementDescrR descr) const
    {
    return list._VerifyElemDescr (descr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/11
+---------------+---------------+---------------+---------------+---------------+------*/
void AddNewElementOptions::_OnAdded(PersistentElementRefR elRef, bool isGraphicsList) const
    {
    T_Super::_OnAdded (elRef, isGraphicsList);
    elRef.GetDgnModelP()->ElementAdded (elRef, isGraphicsList);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModel::DgnModel (DgnProjectR project, DgnModelId modelId, Utf8CP name) : m_project(project), m_modelId(modelId), m_modelInfo()
    {
    m_readonly          = false;
    m_isGeoReprojected  = false;
    m_graphicElems      = NULL;
    m_mark              = false;

    m_name.AssignOrClear(name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/09
+---------------+---------------+---------------+---------------+---------------+------*/
ModelInfoCR DgnModel::GetModelInfo () const {return m_modelInfo;}
ModelInfoR DgnModel::GetModelInfoR () {return m_modelInfo;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnModel::AddAppData (DgnModelAppData::Key const& key, DgnModelAppData* obj)
    {
    return  m_appData.AddAppData (key, obj, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnModel::DropAppData (DgnModelAppData::Key const& key)
    {
    return  m_appData.DropAppData (key, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelAppData* DgnModel::FindAppData (DgnModelAppData::Key const& key) const
    {
    return  m_appData.FindAppData (key);
    }

struct EmptyCaller
    {
    DgnModelR m_model;
    EmptyCaller (DgnModelR model) : m_model(model) {}
    bool CallHandler (DgnModelAppData& handler) const {return handler._OnEmpty (m_model);}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnModel::NotifyOnEmpty()
    {
    if (!IsFilled())
        return false;

    m_appData.CallAllDroppable (EmptyCaller (*this), *this);
    return  true;
    }

struct EmptiedCaller
    {
    DgnModelR m_model;
    EmptiedCaller (DgnModelR model) : m_model(model) {}
    bool CallHandler (DgnModelAppData& handler) const {return handler._OnEmptied (m_model);}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::Empty ()
    {
    if (IsFilled())
        m_appData.CallAllDroppable (EmptiedCaller (*this), *this);
    }

struct CleanupCaller
    {
    DgnModelR m_model;
    CleanupCaller (DgnModelR model) : m_model(model) {}
    void CallHandler (DgnModelAppData& handler) const {handler._OnCleanup (m_model);}
    };

/*---------------------------------------------------------------------------------**//**
* Destructor for DgnModel. Free all memory allocated to this DgnModel.
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModel::~DgnModel ()
    {
    m_appData.CallAll (CleanupCaller(*this));
    m_appData.m_list.clear ();  // (some handlers may have deleted themselves. Don't allow Empty to call handlers.)

    Empty ();
    DELETE_AND_CLEAR (m_graphicElems);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_ToSettingsJson(Json::Value& val) const {m_modelInfo.ToSettingsJson(val);}
void DgnModel::_ToPropertiesJson(Json::Value& val) const            {m_modelInfo.ToPropertiesJson(val);}
void DgnModel::_FromPropertiesJson(Json::Value const& val) {m_modelInfo.FromPropertiesJson(val);}
void DgnModel::_FromSettingsJson(Json::Value const& val)   {m_modelInfo.FromSettingsJson(val);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel2d::_ToPropertiesJson(Json::Value& val) const 
    {
    T_Super::_ToPropertiesJson(val);
    JsonUtils::DPoint2dToJson (val["globalOrigin"], m_globalOrigin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel2d::_FromPropertiesJson(Json::Value const& val) 
    {
    T_Super::_FromPropertiesJson(val);
    JsonUtils::DPoint2dFromJson (m_globalOrigin, val["globalOrigin"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void SectionDrawingModel::_ToPropertiesJson(Json::Value& val) const
    {
    T_Super::_ToPropertiesJson (val);

    JsonUtils::TransformToJson (val["transform_from_drawing_lcs"], m_transformFromDrawingLCS);
    val["zlow"] = m_zrange.low;
    val["zhigh"] = m_zrange.high;
    val["annotation_scale"] = m_annotationScale;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void SectionDrawingModel::_FromPropertiesJson(Json::Value const& val) 
    {
    if (val.isMember ("transform_from_drawing_lcs"))
        JsonUtils::TransformFromJson (m_transformFromDrawingLCS, val["transform_from_drawing_lcs"]);
    else
        m_transformFromDrawingLCS.InitIdentity();

    if (val.isMember ("zlow"))
        {
        m_zrange.low = JsonUtils::GetDouble (val["zlow"], 0);
        m_zrange.high = JsonUtils::GetDouble (val["zhigh"], 0);
        }
    else
        {
        m_zrange.InitNull();
        }

    if (val.isMember ("annotation_scale"))
        m_annotationScale = val["annotation_scale"].asDouble();
    else
        m_annotationScale = 1.0;

    m_modelInfo.FromPropertiesJson(val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Transform SectionDrawingModel::GetFlatteningMatrix (double zdelta) const
    {
    if (m_zrange.IsNull())
        return Transform::FromIdentity();

    double nearlyZero = 1.0e-12;
    
    RotMatrix   flattenRMatrix;
    flattenRMatrix.initFromScaleFactors (1.0, 1.0, nearlyZero);

    RotMatrix   mustBeInvertible;
    while (!mustBeInvertible.InverseOf (flattenRMatrix))
        {
        printf ("nearlyZero=%lg too small\n", nearlyZero);
        nearlyZero *= 10;
        flattenRMatrix.initFromScaleFactors (1.0, 1.0, nearlyZero);
        }

    auto trans = Transform::From (flattenRMatrix);

    if (zdelta != 0.0)
        {
        auto xlat = Transform::FromIdentity();
        xlat.SetTranslation (DPoint3d::FromXYZ(0,0,zdelta));
        trans = Transform::FromProduct (xlat, trans);  // flatten and then translate (in z)
        }

    return trans;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnModel::_LoadFromDb()
    {
    Utf8String settings;
    DbResult  result = QueryProperty (settings, DgnModelProperty::ModelSettings());
    if (BE_SQLITE_ROW != result)
        {
        BeAssert(false);
        return ERROR;
        }

    Json::Value  settingsJson(Json::objectValue);
    if (!Json::Reader::Parse(settings, settingsJson))
        {
        BeAssert(false);
        return ERROR;
        }

    _FromSettingsJson(settingsJson);

    Utf8String props;
    result = QueryProperty (props, DgnModelProperty::ModelProperties());
    if (BE_SQLITE_ROW != result)
        {
        BeAssert(false);
        return ERROR;
        }

    Json::Value  propsJson(Json::objectValue);
    if (!Json::Reader::Parse(props, propsJson))
        {
        BeAssert(false);
        return ERROR;
        }

    _FromPropertiesJson(propsJson);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult DgnModel::SaveProperties()
    {
    Json::Value propJson(Json::objectValue);
    _ToPropertiesJson(propJson);

    return SavePropertyString(DgnModelProperty::ModelProperties(), Json::FastWriter::ToString(propJson));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult DgnModel::SaveSettings()
    {
    Json::Value settingsJson (Json::objectValue);
    _ToSettingsJson(settingsJson);

    return SavePropertyString(DgnModelProperty::ModelSettings(), Json::FastWriter::ToString(settingsJson));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult DgnModel::CopyPropertiesFrom (DgnModelCR source)
    {
    Json::Value props;
    source._ToPropertiesJson (props);
    _FromPropertiesJson(props);

    Json::Value settings;
    source._ToSettingsJson(settings);
    _FromSettingsJson(settings);

    SaveProperties();
    return SaveSettings();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicElementRefList* DgnModel::GetGraphicElementsP(bool create) const 
    {
    if (NULL==m_graphicElems && create) 
        {
        DgnModelP ncThis = (DgnModelP) this;
        ncThis->m_graphicElems = (GraphicElementRefList*) _GetGraphicElementHandler()._CreateList(ncThis);
        }

    return m_graphicElems;
    }

/*---------------------------------------------------------------------------------**//**
* Allocate and initialize a range tree for this model.
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
void GraphicElementRefList::AllocateRangeIndex ()
    {
    if (NULL == m_rangeIndex)
        m_rangeIndex = new ElemRangeIndex (*m_dgnModel);
    }

struct ElementAddedCaller
    {
    DgnModelR   m_model;
    PersistentElementRef&  m_elem;
    bool        m_isGraphicsList;
    ElementAddedCaller (DgnModelR c, PersistentElementRef& e, bool g) : m_model(c), m_elem(e), m_isGraphicsList(g) {}
    void CallHandler (DgnModelAppData& handler) const {handler._OnElementAdded (m_model, m_elem, m_isGraphicsList);}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::ElementAdded (PersistentElementRef& elem, bool isGraphicsList)
    {
    m_appData.CallAll (ElementAddedCaller (*this, elem, isGraphicsList));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::OnElementDeletedFromDb (PersistentElementRefR elem, bool canceled)
    {
    PersistentElementRefList* list = m_graphicElems;
    if (list)
        list->_OnElementDeletedFromDb(elem, canceled);
    }

/*---------------------------------------------------------------------------------**//**
* Get the range of this model if the DgnRangeTree has previously been loaded. Otherwise, return ERROR.
* @bsimethod                                                    KeithBentley    02/01
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt  DgnModel::GetRangeIfKnown (DRange3dR range)
    {
    ElemRangeIndexP index = GetRangeIndexP(false);

    return (NULL == index) ? ERROR : index->GetRangeIfKnown(range);
    }

/*---------------------------------------------------------------------------------**//**
* determine which PersistentElementRefList should hold this element descriptor
* @bsimethod                                                    KeithBentley    02/01
+---------------+---------------+---------------+---------------+---------------+------*/
ElementListHandlerR DgnModel::DetermineListHandler (MSElementDescrR descr)
    {
    return GetGraphicElementsP(true)->GetListHandler();
    }

struct FilledCaller
    {
    DgnModelR m_model;
    FilledCaller (DgnModelR model) : m_model(model) {}
    void CallHandler (DgnModelAppData& handler) const {handler._OnFilled (m_model);}
    };

/*---------------------------------------------------------------------------------**//**
* Called after the process of filling a model is complete. Provides an opportunity for application to
* do things that have to happen after a model is completely filled.
* @bsimethod                                                    KeithBentley    02/01
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::ModelFillComplete()
    {
    m_appData.CallAll (FilledCaller(*this));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    02/01
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelStatus DgnModel::AddElementDescr (MSElementDescrR descr, AddElementOptions const& opts)
    {
    return m_readonly ? DGNMODEL_STATUS_ReadOnly : DetermineListHandler(descr).AddElementToModel (*this, descr, opts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnModel::SetReadOnly (bool newState)
    {
    return  (m_readonly = newState);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/07
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::ChangeToReadOnly ()
    {
    BeAssert (!m_readonly);

    SetReadOnly (true);
    }

/*---------------------------------------------------------------------------------**//**
* Check to see whether sections of the model has been filled from the file yet.
* @bsimethod                                                    KeithBentley    12/00
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnModel::IsFilled() const
    {
    return m_graphicElems && m_graphicElems->WasFilled();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    02/01
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 DgnModel::GetElementCount() const
    {
    UInt32  total = 0;

    if (NULL != GetGraphicElementsP())
        total += GetGraphicElementsP()->CountElements();

    return total;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/13
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 PersistentElementRefList::CountElements() const
    {
    UInt32 count = 0;

    for (const_iterator it=begin(); it!=end(); ++it)
        ++count;

    return  count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GraphicElementRefList::InsertRangeElement (PersistentElementRefP elemRef, bool updateDerivedRange)
    {
    if (NULL == m_rangeIndex || elemRef == NULL)
        return ERROR;

    DgnElementCR el = *elemRef->_GetElemPtrC();

    if (updateDerivedRange && elemRef->HasDerivedRange())
        elemRef->UpdateDerivedRange ();

    m_rangeIndex->AddRangeElement(elemRef, el);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
void GraphicElementRefList::RemoveRangeElement (PersistentElementRefR elemRef)
    {
    if (NULL != m_rangeIndex)
        m_rangeIndex->RemoveElement(&elemRef);
    }

/*=================================================================================**//**
* Helper class allows DerivedElementRange to access private DgnModel data without
* making DerivedElementRange a friend struct of DgnModel. The goal is to allow access
* to model.m_rangeIndex only for the purpose of DerivedElementRange update.
* (Note: We cannot make DerivedElementRange::OnDerivedRangeChangePre a friend struct of DgnModel,
* because this method is protected. We don't want to make DgnModel a friend struct of DerivedElementRange.)
* @bsiclass                                                     Sam.Wilson      04/2008
+===============+===============+===============+===============+===============+======*/
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
struct DerivedElementRangeUtils
{
    static void RemoveRangeElement (DgnModelR model, PersistentElementRef& elem)
        {
        if (model.GetRangeIndexP(false))
            model.GetRangeIndexP(false)->RemoveElement (&elem);
        }
}; 
END_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void DerivedElementRange::OnDerivedRangeChangePre (PersistentElementRef& elem)
    {
    DerivedElementRangeUtils::RemoveRangeElement (*elem.GetDgnModelP(), elem);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void DerivedElementRange::OnDerivedRangeChangePost (PersistentElementRef& elem)
    {
    // The new derived element range is in place. Use it to insert the element in the range index.
    GraphicElementRefList* graphics = elem.GetDgnModelP()->GetGraphicElementsP();
    if (NULL != graphics)
        graphics->InsertRangeElement (&elem, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
void GraphicElementRefList::ClearRangeIndex()
    {
    DELETE_AND_CLEAR (m_rangeIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
ElemRangeIndexP GraphicElementRefList::GetRangeIndexP(bool create)
    {
    if (NULL == m_rangeIndex && create)
        {
        AllocateRangeIndex();
        for (auto const& thisEl : m_ownedElems)
            InsertRangeElement(thisEl.second.get(), false);
        }
    return m_rangeIndex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/07
+---------------+---------------+---------------+---------------+---------------+------*/
int GraphicElementRefList::GetRangeStamp()
    {
    return m_rangeIndex ? m_rangeIndex->GetStamp() : -1;
    }

/*---------------------------------------------------------------------------------**//**
* Called whenever the graphics for an element (could possibly have) changed.
* If a cached presentation of the graphics is stored on the ElementRefP, delete it. Also
* remove its range from the range tree
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::ElementChanged (DgnElementRef& elRef, ElemRefChangeReason reason)
    {
    // if there are qvElems on this ElementRefP, delete them
    elRef.ForceElemChanged (false, reason);

    if (ELEMREF_CHANGE_REASON_Modify != reason || ELEMENT_REF_TYPE_Persistent != elRef.GetRefType())
        return;

    PersistentElementRefList* list = GetGraphicElementsP();
    if (list)
        list->_OnElementModify((PersistentElementRefR) elRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/11
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentElementRefP PersistentElementRefList::FindElementById (ElementId id)
    {
    auto it = m_ownedElems.find (id);
    return it == m_ownedElems.end() ? NULL : it->second.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentElementRefP DgnModel::FindElementById (ElementId id)
    {
    return GetDgnProject().Models().FindElementById (id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   05/04
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::ClearAllQvElems ()
    {
    DgnModel::ElementRefIterator iter (this);
    for (PersistentElementRefP elRef = iter.GetFirstElementRef(); NULL != elRef && !iter.HitEOF(); elRef = iter.GetNextElementRef(true))
        elRef->ForceElemChanged (true, ELEMREF_CHANGE_REASON_ClearQVData);
    }

bool DgnModel::IsReadOnly () const {return m_readonly;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentElementRefP PersistentElementRefListIterator::GetFirstElementRef (PersistentElementRefList* elList, bool wantDeleted)
    {
    if (NULL == elList)
        return  NULL;

    m_map = &elList->m_ownedElems;
    m_it  = m_map->begin();

    PersistentElementRefP currElement = GetCurrentElementRef();

    if (currElement && (!wantDeleted && currElement->IsDeleted()))
        currElement = GetNextElementRef (wantDeleted);

    return  currElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentElementRefP PersistentElementRefListIterator::GetNextElementRef (bool wantDeleted)
    {
    if (!IsValid())
        return  NULL;
    
    while (true)
        {
        ++m_it;

        PersistentElementRefP currElement = GetCurrentElementRef();

        if (!wantDeleted && currElement && currElement->IsDeleted())
            continue;

        return  currElement;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentElementRefP PersistentElementRefListIterator::SetCurrentElementRef (PersistentElementRefP toElm)
    {
    if (!IsValid())
        return  NULL;

    m_it = m_map->find (toElm->GetElementId());
    return GetCurrentElementRef();
    }

/*---------------------------------------------------------------------------------**//**
* Set up iterator to walk the elements in this model only.
* @bsimethod                                                    SamWilson       10/00
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModel::ElementRefIterator::ElementRefIterator (DgnModelP dgnModel)
    {
    m_model = dgnModel;
    m_state = ITERATING_GraphicElms;
    m_iter.Invalidate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    01/01
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModel::ElementRefIterator::ElementRefIterator (DgnModel::ElementRefIterator* source) : m_iter (source->m_iter)
    {
    m_model    = source->m_model;
    m_state    = source->m_state;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentElementRefP DgnModel::ElementRefIterator::GetFirstElementRef (DgnModelP dgnModel, bool wantDeleted)
    {
    m_model = dgnModel;

    PersistentElementRefP  thisElm;
    m_state = ITERATING_GraphicElms;
    if (NULL != (thisElm = m_iter.GetFirstElementRef (m_model->m_graphicElems, wantDeleted)))
        return thisElm;

    m_state = ITERATING_HitEOF;
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentElementRefP DgnModel::ElementRefIterator::GetNextElementRef (bool wantDeleted)
    {
    PersistentElementRef *el;

    if (HitEOF ())          // this happens if caller keeps calling iterator after EOF has been reached
        return NULL;

    if (NULL != (el = m_iter.GetNextElementRef (wantDeleted)))
        return  el;

    m_state = ITERATING_HitEOF;
    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/13
+---------------+---------------+---------------+---------------+---------------+------*/
static PersistentElementRefList* getMyList(PersistentElementRefP el) 
    {
    if (NULL == el)
        return  NULL;
    DgnModelP dgnModel = el->GetDgnModelP();
    return  dgnModel->GetGraphicElementsP(false);
    }

/*---------------------------------------------------------------------------------**//**
* set the current element for the iterator
* @bsimethod                                                    KeithBentley    10/00
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnModel::ElementRefIterator::SetCurrentElm (PersistentElementRefP toElm)
    {
    PersistentElementRefList* toList = getMyList(toElm);
    if (NULL == toList)
        return  false;

    // quick case where list doesn't change
    PersistentElementRefP curr=m_iter.GetCurrentElementRef();
    if ((NULL != curr) && (m_state != ITERATING_HitEOF) && (toList == getMyList(curr)))
        {
        m_iter.SetCurrentElementRef (toElm);
        return  true;
        }

    // otherwise, figure out which list it's in
    DgnModelP dgnModel = toList->MyModel();

    m_state = ITERATING_GraphicElms;

    if (dgnModel)
        m_model = dgnModel;

    m_iter.SetCurrentElementRef (toElm);
    return  true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    KeithBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::ElementRefIterator::SetAtEOF ()
    {
    m_iter.Invalidate();
    m_state = ITERATING_HitEOF;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModel::ElementRefIterator& DgnModel::ElementRefIterator::operator++ ()
    {
    GetNextElementRef ();
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentElementRefP DgnModel::ElementRefIterator::operator* () const
    {
    return *m_iter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnModel::ElementRefIterator::operator!= (DgnModel::ElementRefIterator const& rhs) const
    {
    if (m_model != rhs.m_model)
        return true;

    if (HitEOF() && rhs.HitEOF())   // at EOF == at EOF, regardless of how each iterator got there
        return false;

    return m_iter != rhs.m_iter;
    }

#if defined (NEEDS_WORK_DGNITEM)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModel::ElementRefIterator DgnModel::ElementsCollection::begin () const
    {
    DgnModel::ElementRefIterator it (const_cast<DgnModelP>(&m_model));
    it.GetFirstElementRef ();
    return it;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModel::ElementRefIterator DgnModel::ElementsCollection::end () const
    {
    DgnModel::ElementRefIterator it (const_cast<DgnModelP>(&m_model));
    it.SetAtEOF ();
    return it;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentElementRefListIterator& PersistentElementRefListIterator::operator++ ()
    {
    GetNextElementRef ();
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool PersistentElementRefListIterator::operator== (PersistentElementRefListIterator const& rhs) const
    {
    // two invalid iterator are equal
    return (!IsValid() && !rhs.IsValid()) || (m_map == rhs.m_map && m_it == rhs.m_it);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentElementRefListIterator PersistentElementRefList::begin () const
    {
    PersistentElementRefListIterator it;
    it.GetFirstElementRef (const_cast<PersistentElementRefList*>(this));
    return it;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      05/2009
+---------------+---------------+---------------+---------------+---------------+------*/
PersistentElementRefListIterator PersistentElementRefList::end () const
    {
    return PersistentElementRefListIterator ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/07
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP DgnModel::GetModelName () const
    {
    return m_name.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d DgnModel::_GetGlobalOrigin() const
    {
    return GetDgnProject().Units().GetProjectOrigin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BarryBentley    05/01
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnModel::_Dump (int nestLevel, bool brief) const
    {
    WString     fileName;
    WString     fileExt;
    char        readWrite[4];

    toolSubsystem_wprintf (L"\n%2d ", nestLevel);
    while (nestLevel-- > 0)
        toolSubsystem_wprintf (L"|   ");

    fileName[0] = '\0';
    fileExt[0] = '\0';

    strcpy (readWrite, "??");

    Utf8CP modelName = GetModelName();
    if (NULL == modelName)
        modelName = "";

    DgnProjectR dgnFile = GetDgnProject();

    strcpy (readWrite, IsReadOnly() ? "RO" : "RW");

    BeFileName::ParseName (NULL, NULL, &fileName, &fileExt, dgnFile.GetFileName().c_str());
    fileName.append (L".");
    fileName.append (fileExt);
    
    toolSubsystem_wprintf (L"%d, model 0x%06x file 0x%06x [%ls], model %ls, %hs", _GetModelType(), this, &dgnFile, fileName.c_str(), modelName, readWrite);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/11
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult DgnModel::QueryProperty (void* value, UInt32 size, DgnModelPropertySpecCR spec, UInt64 id)
    {
    return GetDgnProject().Models().QueryProperty (value, size, GetModelId(), spec, id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/11
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult DgnModel::SaveProperty (DgnModelPropertySpecCR spec, void const* value, UInt32 size, UInt64 id)
    {
    return GetDgnProject().Models().SaveProperty (GetModelId(), spec, value, size, id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/11
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult DgnModel::QueryProperty (Utf8StringR value, DgnModelPropertySpecCR spec, UInt64 id)
    {
    return GetDgnProject().Models().QueryProperty (value, GetModelId(), spec, id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/11
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult DgnModel::SavePropertyString (DgnModelPropertySpecCR spec, Utf8StringCR value, UInt64 id)
    {
    return GetDgnProject().Models().SavePropertyString (GetModelId(), spec, value, id);
    }

bool DgnModel::IsLocked () const {return GetModelInfo().GetIsLocked();}
bool DgnModel::Is3d () const {return _Is3d();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/11
+---------------+---------------+---------------+---------------+---------------+------*/
SectioningViewControllerPtr SectionDrawingModel::GetSourceView()
    {
    auto sectioningViewId = GetDgnProject().ViewGeneratedDrawings().QuerySourceView (GetModelId());
    return dynamic_cast<SectioningViewController*>(GetDgnProject().Views().LoadViewController(sectioningViewId, DgnViews::FillModels::No).get());
    }
