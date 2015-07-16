/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnMarkupProject.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include "DgnCoreLog.h"
#include <DgnPlatform/DgnCore/DgnMarkupProject.h>
#include <BeJpeg/BeJpeg.h>

#define QV_RGBA_FORMAT   0
#define QV_BGRA_FORMAT   1
#define QV_RGB_FORMAT    2
#define QV_BGR_FORMAT    3

static WCharCP s_markupDgnDbExt   = L".markupdb";
static Utf8CP  s_projectType      = "Markup";

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus queryPropertyAsJson(JsonValueR json, DgnDbCR project, DgnModelId mid, RedlineModelProperty::ProjectProperty const& propSpec, uint64_t id)
    {
    Utf8String str;
    if (project.QueryProperty(str, propSpec, mid.GetValue(), id) != BE_SQLITE_ROW)
        return BSIERROR;

    if (!Json::Reader::Parse(str, json))
        {
        BeAssert(false);
        return BSIERROR;
        }
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnMarkupProject::QueryPropertyAsJson(JsonValueR json, DgnModelCR model, RedlineModelProperty::ProjectProperty const& propSpec, uint64_t id) const
    {
    return queryPropertyAsJson(json, *this, model.GetModelId(), propSpec, id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnMarkupProject::SavePropertyFromJson(DgnModelCR model, RedlineModelProperty::ProjectProperty const& propSpec, JsonValueCR json, uint64_t id)
    {
    SavePropertyString(propSpec, Json::FastWriter::ToString(json), model.GetModelId().GetValue(), id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnMarkupProject::QueryPropertyAsJson(JsonValueR json, DgnMarkupProjectProperty::ProjectProperty const& propSpec, uint64_t id) const
    {
    Utf8String str;
    if (QueryProperty(str, propSpec, 0, id) != BE_SQLITE_ROW)
        return BSIERROR;

    if (!Json::Reader::Parse(str, json))
        {
        BeAssert(false);
        return BSIERROR;
        }
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId DgnMarkupProject::GetFirstViewOf(DgnModelId mid)
    {
    for (auto const& view : Views().MakeIterator())
        {
        if (view.GetBaseModelId() == mid)
            return view.GetDgnViewId();
        }
    return DgnViewId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId RedlineModel::GetFirstView()
    {
    auto db = GetDgnMarkupProject();
    return db? db->GetFirstViewOf(GetModelId()): DgnViewId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId PhysicalRedlineModel::GetFirstView()
    {
    auto db = GetDgnMarkupProject();
    return db? db->GetFirstViewOf(GetModelId()): DgnViewId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalRedlineViewController::PhysicalRedlineViewController(PhysicalRedlineModel& rdlModel, PhysicalViewController& subjectView, DgnViewId physicalRedlineViewId) 
    : 
    PhysicalViewController(*rdlModel.GetDgnMarkupProject(), physicalRedlineViewId.IsValid()? physicalRedlineViewId: rdlModel.GetFirstView()),
    m_subjectView(subjectView)
    {
#ifndef NDEBUG
    if (physicalRedlineViewId.IsValid())
        {
        DgnViewId assocViewId = rdlModel.GetFirstView();
        BeAssert(!assocViewId.IsValid() || assocViewId == physicalRedlineViewId);
        }
#endif

    // By default, users of this view controller should target the redline model and we should set up the viewport based on the redline model.
    m_targetModelIsInSubjectView = false;

    SynchWithSubjectViewController();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalRedlineViewController::~PhysicalRedlineViewController()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void PhysicalRedlineViewController::_OnViewOpened(DgnViewportR vp)
    {
    // Setup a view aligned ACS that all points/snaps will be projected to...
    if (!m_auxCoordSys.IsValid())
        m_auxCoordSys = IACSManager::GetManager().CreateACS ();

    DPoint3d    origin = DPoint3d::From(0.5, 0.5, 1.0);

    vp.NpcToWorld(&origin, &origin, 1);
    m_auxCoordSys->SetOrigin(origin);
    m_auxCoordSys->SetRotation(_GetRotation());

    T_Super::_OnViewOpened(vp);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void PhysicalRedlineViewController::SynchWithSubjectViewController()
    {
    // There can only be one set of view flags. It will be used to initialize the viewport and qv. 
    // *** EXPERIMENTAL: Here, I force a couple of flags to suit the redline view better. Does this cause too much of a change in the subject view??
    m_viewFlags = m_subjectView.GetViewFlags();
    m_viewFlags.weights = true;
    m_viewFlags.acs = true;
    m_viewFlags.grid = true;

    // tricky: PhysicalRedlineViewController must seem to override the background color 
    //         just so that its _GetBackgroundColor will be called by ResolveBGColor, 
    //         so that it can forward the call to the subject viewcontroller.
    m_viewFlags.bgColor = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
IAuxCoordSysP PhysicalRedlineViewController::_GetAuxCoordinateSystem() const
    {
    // Redline views have their own ACS
    return T_Super::_GetAuxCoordinateSystem();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef PhysicalRedlineViewController::_GetBackgroundColor() const
    {
    // There can only be one background color
    return m_subjectView.ResolveBGColor();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d PhysicalRedlineViewController::_GetOrigin() const
    {
    return m_subjectView.GetOrigin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
DVec3d PhysicalRedlineViewController::_GetDelta() const
    {
    return m_subjectView.GetDelta();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
RotMatrix PhysicalRedlineViewController::_GetRotation() const
    {
    return m_subjectView.GetRotation();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void PhysicalRedlineViewController::_SetOrigin(DPoint3dCR org)
    {
    T_Super::_SetOrigin(org);
    m_subjectView.SetOrigin(org);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void PhysicalRedlineViewController::_SetDelta(DVec3dCR delta)
    {
    T_Super::_SetDelta(delta);
    m_subjectView.SetDelta(delta);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void PhysicalRedlineViewController::_AdjustAspectRatio(double aspect, bool expandView)
    {
    T_Super::_AdjustAspectRatio(aspect, expandView);
    m_subjectView.AdjustAspectRatio(aspect, expandView);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d PhysicalRedlineViewController::_GetTargetPoint() const
    {
    return m_subjectView.GetTargetPoint();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool PhysicalRedlineViewController::_Allow3dManipulations() const {return m_subjectView.Allow3dManipulations();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void PhysicalRedlineViewController::_SetRotation(RotMatrixCR rot)
    {
    T_Super::_SetRotation(rot);
    m_subjectView.SetRotation(rot);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      03/14
//---------------------------------------------------------------------------------------
void PhysicalRedlineViewController::_SaveToSettings(JsonValueR jsonObj) const 
    {
    m_subjectView._SaveToSettings(jsonObj);
    }

#ifdef WIP_RDL_QUERYVIEWS
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void PhysicalRedlineViewController::_OnHealUpdate(DgnViewportR viewport, ViewContextR context, bool fullHeal) {return m_subjectView.OnHealUpdate(viewport, context, fullHeal);}
void PhysicalRedlineViewController::_OnFullUpdate(DgnViewportR viewport, ViewContextR context, FullUpdateInfo& info) {return m_subjectView.OnFullUpdate(viewport, context, info);}
void PhysicalRedlineViewController::_OnDynamicUpdate(DgnViewportR viewport, ViewContextR context, DynamicUpdateInfo& info) {return m_subjectView.OnDynamicUpdate(viewport, context, info);}
bool PhysicalRedlineViewController::_WantElementLoadStart(DgnViewportR viewport, double currentTime, double lastQueryTime, uint32_t maxElementsDrawnInDynamicUpdate, Frustum const& queryFrustum) {return WantElementLoadStart(viewport,currentTime,lastQueryTime,maxElementsDrawnInDynamicUpdate,queryFrustum);}
void PhysicalRedlineViewController::_OnCategoryChange() {return m_subjectView.OnCategoryChange();}
void PhysicalRedlineViewController::_ChangeModelDisplay(DgnModelId modelId, bool onOff) {return m_subjectView.ChangeModelDisplay(modelId, onOff);}
void PhysicalRedlineViewController::_DrawView(ViewContextR context) {return m_subjectView.DrawView(context);}
uint32_t PhysicalRedlineViewController::_GetMaxElementsToLoad() {return GetMaxElementsToLoad();}
BeSQLite::DbResult PhysicalRedlineViewController::_Load() {return m_subjectView.Load();}
Utf8String PhysicalRedlineViewController::_GetRTreeMatchSql(DgnViewportR viewport) {return GetRTreeMatchSql(viewport);}
int32_t PhysicalRedlineViewController::_GetMaxElementFactor() {return GetMaxElementFactor();}
double PhysicalRedlineViewController::_GetMinimumSizePixels(DrawPurpose updateType) {return GetMinimumSizePixels(updateType);}
uint64_t PhysicalRedlineViewController::_GetMaxElementMemory() {return GetMaxElementMemory();}
#endif

bool PhysicalRedlineViewController::_DrawOverlayDecorations(IndexedViewportR viewport) { return m_subjectView._DrawOverlayDecorations(viewport) || T_Super::_DrawOverlayDecorations(viewport); }
bool PhysicalRedlineViewController::_DrawZBufferedDecorations(IndexedViewportR viewport) { return m_subjectView._DrawZBufferedDecorations(viewport) || T_Super::_DrawZBufferedDecorations(viewport); }
void PhysicalRedlineViewController::_DrawBackgroundGraphics(ViewContextR context) { m_subjectView._DrawBackgroundGraphics(context); }
void PhysicalRedlineViewController::_DrawZBufferedGraphics(ViewContextR context) { m_subjectView._DrawZBufferedGraphics(context); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2014
//---------------------------------------------------------------------------------------
void PhysicalRedlineViewController::_DrawElement(ViewContextR context, GeometricElementCR element)
    {
    if (m_targetModelIsInSubjectView)
        m_subjectView._DrawElement(context, element);
    else
        T_Super::_DrawElement(context, element);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    09/2014
//---------------------------------------------------------------------------------------
void PhysicalRedlineViewController::_DrawElementFiltered(ViewContextR context, GeometricElementCR element, DPoint3dCP pts, double size)
    {
    if (m_targetModelIsInSubjectView)
        m_subjectView._DrawElementFiltered(context, element, pts, size);
    else
        T_Super::_DrawElementFiltered(context, element, pts, size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP PhysicalRedlineViewController::_GetTargetModel() const
    {
    return m_targetModelIsInSubjectView? m_subjectView.GetTargetModel(): T_Super::_GetTargetModel();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbR PhysicalRedlineViewController::_GetDgnDb() const
    {
    return m_targetModelIsInSubjectView? m_subjectView.GetDgnDb(): T_Super::_GetDgnDb();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d PhysicalRedlineViewController::_GetViewedExtents() const
    {
    AxisAlignedBox3d subjectRange = m_subjectView.GetViewedExtents();
    AxisAlignedBox3d rdlRange = T_Super::_GetViewedExtents();
    AxisAlignedBox3d fullRange;
    fullRange.UnionOf(rdlRange, subjectRange);
    return fullRange;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void PhysicalRedlineViewController::_RestoreFromSettings(JsonValueCR settings)
    {
    T_Super::_RestoreFromSettings(settings);
    SynchWithSubjectViewController();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2014
//---------------------------------------------------------------------------------------
void PhysicalRedlineViewController::_OnHealUpdate(DgnViewportR viewport, ViewContextR context, bool fullHeal)
    {
    T_Super::_OnHealUpdate(viewport, context, fullHeal);
    m_subjectView._OnHealUpdate(viewport, context, fullHeal);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2014
//---------------------------------------------------------------------------------------
void PhysicalRedlineViewController::_OnFullUpdate(DgnViewportR viewport, ViewContextR context, FullUpdateInfo& info)
    {
    T_Super::_OnFullUpdate(viewport, context, info);
    m_subjectView._OnFullUpdate(viewport, context, info);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2014
//---------------------------------------------------------------------------------------
void PhysicalRedlineViewController::_OnDynamicUpdate(DgnViewportR viewport, ViewContextR context, DynamicUpdateInfo& info)
    {
    T_Super::_OnDynamicUpdate(viewport, context, info);
    m_subjectView._OnDynamicUpdate(viewport, context, info);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2014
//---------------------------------------------------------------------------------------
void PhysicalRedlineViewController::_OnCategoryChange(bool singleEnabled)
    {
    T_Super::_OnCategoryChange(singleEnabled);
    m_subjectView._OnCategoryChange(singleEnabled);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2014
//---------------------------------------------------------------------------------------
void PhysicalRedlineViewController::_ChangeModelDisplay(DgnModelId modelId, bool onOff)
    {
    m_subjectView._ChangeModelDisplay(modelId, onOff);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void PhysicalRedlineViewController::_DrawView(ViewContextR context)
    {
    //  Draw subject model
        {
        //  set up to draw subject model 
        ViewContext::ContextMark mark(&context);
        m_targetModelIsInSubjectView = true;   // causes GetTargetModel to return subject view's target model
        BeAssert(GetTargetModel() == m_subjectView.GetTargetModel());

        //  draw subject model using *this* view controller
        m_subjectView.DrawView(context);

        //  restore the redline model as the normal target
        m_targetModelIsInSubjectView = false;
        BeAssert(GetTargetModel() == T_Super::GetTargetModel());
        }

    BeAssert(GetTargetModel() == T_Super::GetTargetModel() && "redline model is the normal target of this view controller");

    //  Draw redline model
    T_Super::_DrawView(context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnMarkupProject::SavePropertyFromJson(DgnMarkupProjectProperty::ProjectProperty const& propSpec, JsonValueCR json, uint64_t id)
    {
    SavePropertyString(propSpec, Json::FastWriter::ToString(json), 0, id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/13
+---------------+---------------+---------------+---------------+---------------+------*/
RedlineViewController::RedlineViewController(RedlineModel& rdlModel, DgnViewId viewId) 
    : 
    SheetViewController(*rdlModel.GetDgnMarkupProject(), viewId.IsValid()? viewId: rdlModel.GetFirstView()),
    m_enableViewManipulation(false),
    m_drawBorder(true)
    {;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/13
+---------------+---------------+---------------+---------------+---------------+------*/
RedlineViewController::~RedlineViewController() 
    {
    }

// Disallow pan, zoom, and rotate. While pan is harmless, mouse-wheel goes crazy when we disallow zoom and allow pan.
void RedlineViewController::_SetDelta(DVec3dCR delta)      {if (true || m_enableViewManipulation) T_Super::_SetDelta(delta);}
void RedlineViewController::_SetOrigin(DPoint3dCR origin)  {if (true || m_enableViewManipulation) T_Super::_SetOrigin(origin);}
void RedlineViewController::_SetRotation(RotMatrixCR rot)  {if (true || m_enableViewManipulation) T_Super::_SetRotation(rot);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
static void supplyDefaultExtension(BeFileName& fn, WCharCP defaultExt)
    {
    WString dev, dir, name, ext;
    fn.ParseName(&dev, &dir, &name, &ext);

    if (ext.empty()) ext = defaultExt;
    
    fn.BuildName(dev.c_str(), dir.c_str(), name.c_str(), ext.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnMarkupProject::ComputeDgnProjectFileName(BeFileNameR mname, BeFileNameCR pname)
    {
    mname.SetName(BeFileName::GetDirectoryName(pname).c_str());
    mname.AppendToPath(BeFileName::GetFileNameWithoutExtension(pname).c_str());
    mname.AppendExtension(s_markupDgnDbExt+1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnMarkupProjectPtr DgnMarkupProject::OpenDgnDb(DbResult* outResult, BeFileNameCR filename, OpenParams const& openParams)
    {
    DbResult ALLOW_NULL_OUTPUT (status, outResult);

    BeFileName projectFileName(filename);
    supplyDefaultExtension(projectFileName, s_markupDgnDbExt);

    DgnMarkupProjectPtr markupProject = new DgnMarkupProject();

    status = markupProject->DoOpenDgnDb(projectFileName, openParams);
    if (BE_SQLITE_OK != status)
        return nullptr;

    Utf8String typeProperty;
    if (markupProject->QueryProperty(typeProperty, DgnProjectProperty::ProjectType()) != BE_SQLITE_ROW  ||  typeProperty != s_projectType)
        {
        status = BE_SQLITE_ERROR_BadDbSchema;
        return nullptr;
        }

    return markupProject;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnMarkupProject::CheckIsOpen()
    {
    if (IsDbOpen())
        return BSISUCCESS;

    BeAssert(false && "This function should never be called unless the project is open");
    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnMarkupProject::ConvertToMarkupProject(BeFileNameCR fileName, CreateDgnDbParams const& params)
    {
    // PRE CONDITIONS
    if (params.m_seedDb.empty())
        {
        BeAssert(false && "You must supply a seed file");
        return BE_SQLITE_ERROR;
        }

    //  ------------------------------------------------------------------
    //  Create the .markupdb file
    //  ------------------------------------------------------------------
    BeFileName projectFile(fileName);

    CreateDgnMarkupProjectParams& mpp = *(CreateDgnMarkupProjectParams*) &params;

    supplyDefaultExtension(projectFile, s_markupDgnDbExt);
    if (mpp.GetOverwriteExisting() && BeFileName::DoesPathExist(projectFile))
        {
        BeFileNameStatus fstatus = BeFileName::BeDeleteFile(projectFile);

        if (BeFileNameStatus::Success != fstatus)
            return BE_SQLITE_ERROR;
        }

    BeFileNameStatus fstatus = BeFileName::BeCopyFile(params.m_seedDb.c_str(), projectFile);

    if (BeFileNameStatus::Success != fstatus)
        return BE_SQLITE_ERROR;

    OpenParams oparams(OpenMode::ReadWrite);
    DbResult status = DoOpenDgnDb(projectFile, oparams); // open as a DgnDb

    if (BE_SQLITE_OK != status)
        {
        BeFileName::BeDeleteFile(projectFile);
        return  status;
        }

    //  ------------------------------------------------------------------
    //  Set Markup-specific project properties
    //  ------------------------------------------------------------------
    SavePropertyString(DgnProjectProperty::ProjectType(), s_projectType);   // identifies the .dgndb as a Markup project.
    SetAssociation(mpp.GetSubjectDgnProject());

    if (mpp.GetPhysicalRedlining())
        {
        SavePropertyString(DgnProjectProperty::IsPhysicalRedline(), "true");

        }

    //  ------------------------------------------------------------------
    //  Mark all pre-existing models and views as internal. They may be used
    //  as seeds, but they will never be used directly by the app or the user.
    //  ------------------------------------------------------------------
    if (true)
        {
        Statement stmt;
        // *** NEEDS WORK: Missing WHERE Id=?   
        stmt.Prepare(*this, "UPDATE " DGN_TABLE(DGN_CLASSNAME_View) " SET Source=4");  // DgnViewSource::Private
        stmt.Step();
        }

    if (true)
        {
        Statement stmt;
        // *** NEEDS WORK: Missing WHERE Id=?   
        stmt.Prepare(*this, "UPDATE " DGN_TABLE(DGN_CLASSNAME_Model) " SET Visibility=1");  // ModelIterate::All (i.e., hide when looking for models to show in the GUI)
        stmt.Step();
        }

    //  ------------------------------------------------------------------
    //  Import the needed ECSchema(s).
    //  ------------------------------------------------------------------
    if (ImportMarkupEcschema() != SUCCESS)
        return BE_SQLITE_ERROR;

    SaveSettings();
    SaveChanges();


    // POST CONDITIONS
    BeAssert(!mpp.GetPhysicalRedlining() || IsPhysicalRedlineProject());

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void RedlineModel::_ToPropertiesJson(Json::Value& val) const 
    {
    T_Super::_ToPropertiesJson(val);

    Json::Value json(Json::objectValue);
    m_assoc.ToPropertiesJson(json);
    val["RedlineModel_Assoc"] = json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void RedlineModel::_FromPropertiesJson(Json::Value const& val) 
    {
    T_Super::_FromPropertiesJson(val);
    m_assoc.FromPropertiesJson(val["RedlineModel_Assoc"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void RedlineModel::SetAssociation(DgnDbR dgnProject, ViewControllerCR projectView)
    {
    m_assoc.FromDgnProject(dgnProject, projectView, *GetDgnMarkupProject());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void RedlineModel::GetAssociation(DgnViewAssociationData& assocData) const
    {
    assocData = m_assoc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnMarkupProject::SetAssociation(DgnDbR dgnProject)
    {
    DgnProjectAssociationData assocData;
    assocData.FromDgnProject(dgnProject, *this);
    Json::Value json(Json::objectValue);
    assocData.ToPropertiesJson(json);
    SavePropertyFromJson(DgnMarkupProjectProperty::DgnProjectAssociationData(), json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnMarkupProject::GetAssociation(DgnProjectAssociationData& assocData) const
    {
    Json::Value json(Json::objectValue);
    QueryPropertyAsJson(json, DgnMarkupProjectProperty::DgnProjectAssociationData());
    assocData.FromPropertiesJson(json);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnProjectAssociationData::CheckResults DgnMarkupProject::CheckAssociation(DgnDbR dgnProject, DgnProjectAssociationData const& assocData)
    {
    DgnProjectAssociationData::CheckResults results;
    memset(&results, 0, sizeof(results));

    //  Non-directory part of the stored name must match
    BeFileName dgnProjectFileName;
    dgnProjectFileName.SetNameUtf8(dgnProject.GetDbFileName());
    BeFileName storedDgnProjectFileNameRelative;
    storedDgnProjectFileNameRelative.SetNameUtf8(assocData.GetRelativeFileName());
    if (BeFileName::GetFileNameAndExtension(dgnProjectFileName) != BeFileName::GetFileNameAndExtension(storedDgnProjectFileNameRelative))
        {
        results.NameChanged = true;
        }

    //  Stored BeGuid must match. If it's different, the DgnDb was republished. Who knows what might have changed?
    if (assocData.GetGuid() != dgnProject.GetDbGuid())
        {
        results.GuidChanged = true;
        }
    else
        {
        //  GUID matches. See if the file was changed. If it has changed, that's not necessarily a problem.
        time_t lmt;
        BeFileName::GetFileTime(NULL, NULL, &lmt, dgnProject.GetFileName());
        if (assocData.GetLastModifiedTime() < (uint64_t)lmt)
            {
            results.ContentsChanged = true;
            }
        }

    return results;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnProjectAssociationData::CheckResults DgnMarkupProject::CheckAssociation(DgnDbR dgnProject)
    {
    DgnProjectAssociationData assocData;
    GetAssociation(assocData);

    return CheckAssociation(dgnProject, assocData);    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewAssociationData::CheckResults RedlineModel::CheckAssociation(DgnDbR dgnProject)
    {
    DgnViewAssociationData assocData;
    GetAssociation(assocData);

    DgnViewAssociationData::CheckResults viewResults(GetDgnMarkupProject()->CheckAssociation(dgnProject, assocData));

    if (assocData.GetViewId().IsValid() && !dgnProject.Views().QueryView(assocData.GetViewId()).IsValid())
        {
        viewResults.ViewNotFound = true;
        }

    return viewResults;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnMarkupProjectPtr DgnMarkupProject::CreateDgnDb(DbResult* result, BeFileNameCR projectFileName, CreateDgnMarkupProjectParams const& params)
    {
    DbResult ALLOW_NULL_OUTPUT (stat, result);

    DgnMarkupProjectPtr markupProject = new DgnMarkupProject();

    stat = markupProject->ConvertToMarkupProject(projectFileName, params);
    
    if (BE_SQLITE_OK != stat)
        return nullptr;

    return  markupProject;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnMarkupProject::IsPhysicalRedlineProject() const
    {
    Utf8String rdlPropVal;
    return QueryProperty(rdlPropVal, DgnProjectProperty::IsPhysicalRedline()) == BE_SQLITE_ROW  &&  rdlPropVal == "true";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnMarkupProject::ImportMarkupEcschema()
    {
    BeFileName ecSchemasDir = T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory();
    ecSchemasDir.AppendToPath(L"ECSchemas");   // *** WIP_MARKUP - I don't want to have this hard-wired knowledge here. 

    BeFileName stdSchemasDir(ecSchemasDir);
    stdSchemasDir.AppendToPath(L"Standard");
    
    BeFileName dgnSchemasDir(ecSchemasDir);
    dgnSchemasDir.AppendToPath(L"Dgn");   // *** WIP_MARKUP - I don't want to have this hard-wired knowledge here. 

    BeFileName dgnMarkupEcSchemaFileName(NULL, dgnSchemasDir, L"DgnMarkupSchema.01.00.ecschema", L"xml"); // *** WIP_MARKUP - I don't want to have the version number hard-wired.
    BeFileName markupEcSchemaFileName(NULL, dgnSchemasDir, L"Bentley_Markup.01.00.ecschema", L"xml"); // *** WIP_MARKUP - I don't want to have the version number hard-wired.
    BeFileName markupExtEcSchemaFileName(NULL, dgnSchemasDir, L"Bentley_MarkupExtension.01.01.ecschema", L"xml"); // *** WIP_MARKUP - I don't want to have the version number hard-wired.

    // 1) Deserialize ECSchema from XML file
    ECN::ECSchemaReadContextPtr schemaContext = ECN::ECSchemaReadContext::CreateContext();
    // add schema search paths for referenced schemas.
    // This is needed in case the schema references classes from other schemas in other locations.
    schemaContext->AddSchemaPath(ecSchemasDir);
    schemaContext->AddSchemaPath(stdSchemasDir);
    schemaContext->AddSchemaPath(dgnSchemasDir);

    // Read specified schema. All referenced schemas will also be read. Schema and references
    // are available in the cache of the schemaContext
    ECN::ECSchemaPtr schema = NULL;
    ECN::SchemaReadStatus deserializeStat = ECN::ECSchema::ReadFromXmlFile(schema, markupEcSchemaFileName, *schemaContext);
    if (ECN::SCHEMA_READ_STATUS_Success != deserializeStat)
        {
        // Schema could not be read into memory. Do error handling here
        BeAssert(false && "Markup schemas should be a delivered asset");
        return ERROR;
        }
    schema = NULL;
    deserializeStat = ECN::ECSchema::ReadFromXmlFile(schema, markupExtEcSchemaFileName, *schemaContext);
    if (ECN::SCHEMA_READ_STATUS_Success != deserializeStat)
        {
        // Schema could not be read into memory. Do error handling here
        BeAssert(false && "Markup schemas should be a delivered asset");
        return ERROR;
        }
    schema = NULL;
    deserializeStat = ECN::ECSchema::ReadFromXmlFile(schema, dgnMarkupEcSchemaFileName, *schemaContext);
    if (ECN::SCHEMA_READ_STATUS_Success != deserializeStat)
        {
        // Schema could not be read into memory. Do error handling here
        BeAssert(false && "Markup schemas should be a delivered asset");
        return ERROR;
        }

    // 2) Import ECSchema (and its references) into DgnDb file. The schema and its references are available in the cache
    //    of the schema context from the XML deserialization step.
    ECDbSchemaManagerCR schemaManager = Schemas();
    BentleyStatus importStat = schemaManager.ImportECSchemas(schemaContext->GetCache());
    if (SUCCESS != importStat)
        {
        // Schema(s) could not be imported into the DgnDb file. Do error handling here
        BeAssert(false && "MarkupSchema.ecschema.xml should be a delivered asset");
        return ERROR;
        }

    // commits the changes in the DgnDb file (the imported ECSchema) to disk
    return SaveChanges() == BE_SQLITE_OK? SUCCESS: ERROR;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
ECN::ECClassCP DgnMarkupProject::GetRedlineECClass()
    {
    ECN::ECClassCP redlineClass = Schemas ().GetECClass ("DgnMarkupSchema", "Redline");
    if (redlineClass == nullptr)
        {
        BeAssert (false);
        return NULL;
        }
    return redlineClass;
    }
+---------------+---------------+---------------+---------------+---------------+------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnMarkupProject* RedlineModel::GetDgnMarkupProject() const
    {
    return dynamic_cast<DgnMarkupProject*>(&GetDgnDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnMarkupProject* PhysicalRedlineModel::GetDgnMarkupProject() const
    {
    return dynamic_cast<DgnMarkupProject*>(&GetDgnDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/13
+---------------+---------------+---------------+---------------+---------------+------*/
RedlineModel::ImageDef::ImageDef()
    {
    m_format = -1;
    m_size.Init(0,0);
    m_origin.Init(0,0);
    m_sizeInPixels.Init(0,0);
    m_topDown = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void RedlineModel::ImageDef::ToPropertiesJson(Json::Value& val) const
    {
    val["Format"]           = m_format;
    JsonUtils::Point2dToJson(val["SizeInPixels"], m_sizeInPixels);
    JsonUtils::DPoint2dToJson(val["Origin"], m_origin);
    JsonUtils::DVec2dToJson(val["Size"], m_size);
    val["TopDown"] = m_topDown;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RedlineModel::ImageDef::FromPropertiesJson(Json::Value const& val)
    {
    m_format            = val["Format"].asInt();
    JsonUtils::Point2dFromJson(m_sizeInPixels, val["SizeInPixels"]);
    JsonUtils::DPoint2dFromJson(m_origin, val["Origin"]);
    JsonUtils::DVec2dFromJson(m_size, val["Size"]);
    m_topDown = val.isMember("TopDown")? val["TopDown"].asBool(): false;
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/13
+---------------+---------------+---------------+---------------+---------------+------*/
size_t RedlineModel::ImageDef::GetSizeofPixelInBytes() const
    {
    return (m_format == QV_RGBA_FORMAT || m_format == QV_BGRA_FORMAT)? 4: 3;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/13
+---------------+---------------+---------------+---------------+---------------+------*/
size_t RedlineModel::ImageDef::GetSizeInBytes() const
    {
    return m_sizeInPixels.x * m_sizeInPixels.y * GetSizeofPixelInBytes();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool RedlineModel::ImageDef::GetIsTopDown() const
    {
    return m_topDown;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/13
+---------------+---------------+---------------+---------------+---------------+------*/
size_t RedlineModel::ImageDef::GetPitch() const
    {
    return m_sizeInPixels.x * GetSizeofPixelInBytes();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnProjectAssociationData::DgnProjectAssociationData()
    {
    m_lastModifiedTime = 0;
    }

Utf8String DgnProjectAssociationData::GetRelativeFileName() const {return m_relativeFileName;}
BeGuid DgnProjectAssociationData::GetGuid() const {return m_guid;}
uint64_t DgnProjectAssociationData::GetLastModifiedTime() const {return m_lastModifiedTime;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnProjectAssociationData::FromDgnProject(DgnDbCR dgnProject, DgnMarkupProjectCR markupProject)
    {
    WString projectPath(dgnProject.GetDbFileName(), true);
    WString markupDir = BeFileName::GetDirectoryName(BeFileName(markupProject.GetDbFileName(), true));
    WString projectRelativePath;
    BeFileName::FindRelativePath(projectRelativePath, projectPath.c_str(), markupDir.c_str());

    time_t lmt;
    BeFileName::GetFileTime(NULL, NULL, &lmt, dgnProject.GetFileName());
    
    m_guid = dgnProject.GetDbGuid();
    m_lastModifiedTime = lmt;
    BeStringUtilities::WCharToUtf8(m_relativeFileName, projectRelativePath.GetWCharCP());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnProjectAssociationData::ToPropertiesJson(Json::Value& val) const
    {
    val["LastModifiedTime"] = m_lastModifiedTime;
    val["GUID"]             = m_guid.ToString();
    val["RelativeFileName"] = m_relativeFileName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnProjectAssociationData::FromPropertiesJson(Json::Value const& val)
    {
    m_lastModifiedTime = val["LastModifiedTime"].asUInt64();
    m_relativeFileName = val["RelativeFileName"].asCString();
    m_guid.FromString(val["GUID"].asCString());
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewAssociationData::FromDgnProject(DgnDbCR dgnProject, ViewControllerCR projectView, DgnMarkupProjectCR markupProject)
    {
    T_Super::FromDgnProject(dgnProject, markupProject);

    m_viewId = projectView.GetViewId();

    projectView.SaveToSettings(m_viewGeometry);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     09/12
//---------------------------------------------------------------------------------------
static void DPoint3dFromJson(DPoint3dR point, Json::Value const& inValue)
    {
    point.x = inValue[0].asDouble();
    point.y = inValue[1].asDouble();
    point.z = inValue[2].asDouble();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewAssociationData::DgnViewAssociationData() : m_viewGeometry(Json::objectValue)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewAssociationData::GetViewOrigin(DPoint3dR origin, DgnDbCR dgnProject)
    {
    DPoint3dFromJson(origin, m_viewGeometry["origin"]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnViewAssociationData::ToPropertiesJson(Json::Value& val) const
    {
    T_Super::ToPropertiesJson(val);
    val["ViewId"] = m_viewId.GetValue();
    val["ViewGeometry"] = m_viewGeometry;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnViewAssociationData::FromPropertiesJson(Json::Value const& val)
    {
    T_Super::FromPropertiesJson(val);
    m_viewId = DgnViewId(val["ViewId"].asInt64());
    m_viewGeometry = val["ViewGeometry"];
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnViewAssociationData::FromSerializedJson(Utf8CP serializedData)
    {
    Json::Value json(Json::objectValue);
    if (!Json::Reader::Parse(serializedData, json))
        {
        BeAssert(false);
        return BSIERROR;
        }

    return FromPropertiesJson(json);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      06/13
//--------------+------------------------------------------------------------------------
DgnViewId DgnViewAssociationData::GetViewId() const {return m_viewId;}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      06/13
//--------------+------------------------------------------------------------------------
bpair<Dgn::DgnModelId,double> DgnMarkupProject::FindClosestRedlineModel(ViewControllerCR viewController)
    {
    DPoint3d viewOriginToMatch = viewController.GetOrigin();

    DgnModelId closestRedlineModelId;
    DPoint3d closestOrigin;
    double closestDistance = DBL_MAX;
    for (auto const& view : Views().MakeIterator())
        {
        RedlineModelPtr rdlModel = Models().Get<RedlineModel>(view.GetBaseModelId());
        if (rdlModel.IsValid())
            {
            DgnViewAssociationData assoc;
            rdlModel->GetAssociation(assoc);
            if (assoc.GetViewId() == viewController.GetViewId())
                {
                DPoint3d viewOrigin;
                assoc.GetViewOrigin(viewOrigin, *this);
                double d = viewOrigin.Distance(viewOriginToMatch);
                if (d < closestDistance)
                    {
                    closestRedlineModelId = rdlModel->GetModelId();
                    closestDistance = d;
                    closestOrigin = viewOrigin;
                    if (BeNumerical::Compare(d, 0.0) == 0)
                        break;
                    }
                }
            }
        }
    return make_bpair(closestRedlineModelId, closestDistance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
RedlineModelPtr RedlineModel::Create(DgnMarkupProjectR markupProject, Utf8CP name, DgnModelId templateModelId)
    {
    DgnClassId rmodelClassId = DgnClassId(markupProject.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, "RedlineModel"));
    RedlineModelPtr rdlModel = new RedlineModel(RedlineModel::CreateParams(markupProject, rmodelClassId, name, DPoint2d::FromZero()));
    if (!rdlModel.IsValid())
        return nullptr;

    //  Copy model properties and contents from template
    SheetModelPtr templateModel = markupProject.Models().Get<SheetModel>(templateModelId);
    if (templateModel.IsValid())
        {
        //  Copy model properties and settings
        rdlModel->GetPropertiesR() = templateModel->GetProperties();
        rdlModel->m_size = templateModel->GetSize();

        //  Copy elements -- *** TBD ***
        }
    return rdlModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalRedlineModelPtr PhysicalRedlineModel::Create(DgnMarkupProjectR markupProject, Utf8CP name, PhysicalModelCR subjectViewTargetModel)
    {
    DgnClassId rmodelClassId = DgnClassId(markupProject.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, "PhysicalRedlineModel"));

    PhysicalRedlineModelPtr rdlModel = new PhysicalRedlineModel(PhysicalRedlineModel::CreateParams(markupProject, rmodelClassId, name));
    if (!rdlModel.IsValid())
        {
        DGNCORELOG->error("PhysicalRedlineModel::CreateModel failed");
        BeAssert(false && "PhysicalRedlineModel::CreateModel failed");
        return nullptr;
        }

    UnitDefinition mu = subjectViewTargetModel.GetProperties().GetMasterUnits();
    UnitDefinition su = subjectViewTargetModel.GetProperties().GetSubUnits();
    rdlModel->GetPropertiesR().SetUnits(mu, su);

    return rdlModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
void DgnMarkupProject::CreateModelECProperties (DgnModelId modelId, Utf8CP modelName)
    {
    // --------------------------------------------------------------
    //  Define as many of the redline ECProperties as we can
    ECN::ECClassCP ecclass = GetRedlineECClass();
    if (NULL == ecclass)
        return;

    Utf8String ecsql ("INSERT INTO ");
    ecsql.append (ECSqlBuilder::ToECSqlSnippet (*ecclass)).append (" (RedlineModelId, [Name], CreateDate) VALUES (?, ?, ?)");
    
    ECSqlStatement statement;
    ECSqlStatus stat = statement.Prepare (*this, ecsql.c_str ());
    if (stat != ECSqlStatus::Success)
        {
        BeAssert (false);
        return;
        }
    
    stat = statement.BindInt64 (1, modelId.GetValue ());
    if (stat != ECSqlStatus::Success)
        {
        BeAssert (false);
        return;
        }
        
    stat = statement.BindText (2, modelName, IECSqlBinder::MakeCopy::No);
    if (stat != ECSqlStatus::Success)
        {
        BeAssert (false);
        return;
        }
        
    stat = statement.BindDateTime (3, DateTime::GetCurrentTimeUtc());
    if (stat != ECSqlStatus::Success)
        {
        BeAssert (false);
        return;
        }

    ECInstanceKey newECInstanceKey;
    ECSqlStepStatus stepStat = statement.Step (newECInstanceKey);
    if (stepStat != ECSqlStepStatus::Done)
        {
        BeAssert (false);
        return;
        }
    
    // --------------------------------------------------------------
    // Associate the redline model with the ECInstance.
    ECInstanceId const& newECInstanceId = newECInstanceKey.GetECInstanceId ();
    SaveProperty (RedlineModelProperty::RedlineECInstanceId(), &newECInstanceId, sizeof(ECInstanceId), modelId.GetValue());
    }
+---------------+---------------+---------------+---------------+---------------+------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void RedlineModel::StoreImageData(bvector<uint8_t> const& imageData, IViewOutput::CapturedImageInfo const& imageInfo, bool fitToX, bool compressImageProperty)
    {
    //  Grab possibly updated image definition data
    if (imageInfo.hasAlpha)
        m_imageDef.m_format = imageInfo.isBGR? QV_BGRA_FORMAT: QV_RGBA_FORMAT; 
    else
        m_imageDef.m_format = imageInfo.isBGR? QV_BGR_FORMAT: QV_RGB_FORMAT; 

    m_imageDef.m_sizeInPixels.x = (int32_t)imageInfo.width;
    m_imageDef.m_sizeInPixels.y = (int32_t)imageInfo.height;

    m_imageDef.m_topDown = imageInfo.isTopDown;

    //  Map the image into the sheet area, scaling it up or down to fit ... 
    //  but, be sure to maintain the aspect ratio of the original image.
    if (fitToX)
        {
        m_imageDef.m_size.x = m_size.x;
        m_imageDef.m_size.y = m_size.x * (m_imageDef.m_sizeInPixels.y / (double)m_imageDef.m_sizeInPixels.x);
        }
    else
        {
        m_imageDef.m_size.y = m_size.y;
        m_imageDef.m_size.x = m_size.y * (m_imageDef.m_sizeInPixels.x / (double)m_imageDef.m_sizeInPixels.y);
        }

    m_imageDef.m_origin.FromZero();
    
    //  Store the image def
    Json::Value json(Json::objectValue);
    m_imageDef.ToPropertiesJson(json);

    GetDgnMarkupProject()->SavePropertyFromJson(*this, RedlineModelProperty::ImageDef(), json);
    
    //  Store image data
    RedlineModelProperty::Spec propSpec = compressImageProperty? RedlineModelProperty::ImageData(): RedlineModelProperty::ImageData2();
    GetDgnMarkupProject()->SaveProperty(propSpec, &imageData[0], (uint32_t)imageData.size(), GetModelId().GetValue());

    // Update the sheetdef to make sure it encloses the image.
    if (!m_imageDef.m_size.AlmostEqual(DVec2d::From(m_size)))
        {
        m_size.x = std::max(m_imageDef.m_size.x, m_size.x);
        m_size.y = std::max(m_imageDef.m_size.y, m_size.y);
        }
    
    Update();

    //  Define the image texture
    DefineImageTextures(m_imageDef, imageData);

#if defined (WIP_TILED_IMAGE_TEST_TEST_TEST)
    ImageDef def1x1;
    def1x1.m_format = QV_RGB_FORMAT;
    def1x1.m_origin.x = 0;
    def1x1.m_origin.y = 0;
    def1x1.m_size.x = 10;
    def1x1.m_size.y = 10;
    def1x1.m_sizeInPixels.x = 1;
    def1x1.m_sizeInPixels.y = 1;
    char const* def = "1  ";
BeAssert(def1x1.GetSizeofPixelInBytes() == 3);
    bvector<Byte> bytes1x1;
    bytes1x1.assign(def, def+strlen(def));
    DefineImageTextures(def1x1, bytes1x1);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void RedlineModel::StoreImageDataFromJPEG (uint8_t const* jpegData, size_t jpegDataSize, IViewOutput::CapturedImageInfo const& imageInfoIn, bool fitToX)
    {
    IViewOutput::CapturedImageInfo imageInfo;
    bvector<uint8_t> rgbData;
    if (ImageUtilities::ReadImageFromJpgBuffer(rgbData, imageInfo, jpegData, jpegDataSize, imageInfoIn) != BSISUCCESS)
        return;
    StoreImageData(rgbData, imageInfo, fitToX, /*compresssImageProperty*/false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RedlineModel::LoadImageData(ImageDef& imageDef, bvector<uint8_t>& imageData)
    {
    return LoadImageData(imageDef, imageData, GetDgnDb(), GetModelId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RedlineModel::LoadImageData(ImageDef& imageDef, bvector<uint8_t>& imageData, DgnDbCR project, DgnModelId modelId)
    {
    imageData.clear();

    Json::Value json(Json::objectValue);
    if (queryPropertyAsJson(json, project, modelId, RedlineModelProperty::ImageDef(), 0) != BSISUCCESS || imageDef.FromPropertiesJson(json) != BSISUCCESS)
        return BSIERROR;

    size_t imageDataSizeInBytes = imageDef.m_sizeInPixels.x * imageDef.m_sizeInPixels.y * imageDef.GetSizeofPixelInBytes();
    imageData.resize(imageDataSizeInBytes);
    if (project.QueryProperty(&imageData[0], (uint32_t)imageData.size(), RedlineModelProperty::ImageData(), modelId.GetValue()) != BE_SQLITE_ROW)
        {
        if (project.QueryProperty(&imageData[0], (uint32_t)imageData.size(), RedlineModelProperty::ImageData2(), modelId.GetValue()) != BE_SQLITE_ROW)
            return BSIERROR;
        }

    return BSISUCCESS;
    }

// source: http://www.hackersdelight.org/hdcodetxt/flp2.c.txt
int hibit(unsigned int n) 
    {
    n |= (n >>  1);
    n |= (n >>  2);
    n |= (n >>  4);
    n |= (n >>  8);
    n |= (n >> 16);
    return n - (n >> 1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void RedlineModel::DefineImageTexturesForRow(ImageDef const& imageDef, uint8_t const* rowStart, DPoint3dCR rowOrigin, Point2dCR tileDims, uint32_t nTilesAcross)
    {
#ifdef WIP_MOSAIC
    uint32_t pitch = (uint32_t)imageDef.GetPitch(); // # bytes per row

    uint32_t fullWidth = (uint32_t) imageDef.m_sizeInPixels.x; // # pixels per row

    double pixelsToModelX = imageDef.m_size.x / imageDef.m_sizeInPixels.x;
    double tileWidthModel = tileDims.x * pixelsToModelX;  // the width of a tile in model coordinates
    uint32_t tileWidthInBytes = tileDims.x*(uint32_t)imageDef.GetSizeofPixelInBytes();  // the size of a tile's worth of data in bytes
    
    //  March tiles across the row
    Byte const* colStart = rowStart;
    DPoint3d colOrigin = rowOrigin;
    intptr_t tileid = GetBackDropTextureId() + m_tileIds.size();        // the next unused tileid
    for (uint32_t col=0; col<nTilesAcross; ++col)
        {
        if (tileDims.y != 0)
            {
            T_HOST.GetGraphicsAdmin()._DefineTile(tileid, nullptr, tileDims, false, imageDef.m_format, pitch, colStart);
            m_tileIds.push_back(tileid);
            ++tileid;
            }
        m_tileOrigins.push_back(colOrigin);
            
        colStart += tileWidthInBytes;
        colOrigin.x += tileWidthModel;                           // march across
        }

    // tack on the tall, narrow tile to the right of the last full tile in this row.
    Point2d stubColTileDims;
    stubColTileDims.x = fullWidth - (nTilesAcross*tileDims.x);
    if (stubColTileDims.x  != 0)
        {
        stubColTileDims.y = tileDims.y;
        if (tileDims.y != 0)
            {
            T_HOST.GetGraphicsAdmin()._DefineTile(tileid, nullptr, stubColTileDims, false, imageDef.m_format, pitch, colStart);
            m_tileIds.push_back(tileid);
            ++tileid;
            }
        m_tileOrigins.push_back(colOrigin);
        }

    //  Append the location of the outside corner of the row.
    colOrigin.x = rowOrigin.x + imageDef.m_size.x;
    m_tileOrigins.push_back(colOrigin);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void RedlineModel::DefineImageTextures(ImageDef const& imageDef, bvector<uint8_t> const& imageData)
    {
#ifdef WIP_MOSAIC
    // All dimensions are in PIXELS unless specified as "bytes" or "model" (i.e., distance)
    uint32_t maxTileSize = 1024;

    BeAssert(m_tileIds.empty());
    m_tileIds.clear();
    m_tileOrigins.clear();

    uint32_t pixelsX = (uint32_t) imageDef.m_sizeInPixels.x;
    uint32_t pixelsY = (uint32_t) imageDef.m_sizeInPixels.y;

    //  What is the largest SQUARE tile we can have in this image?
    uint32_t tileSize = std::min(maxTileSize, pixelsX);
    tileSize = std::min(tileSize, pixelsY);

    //  Round this down to a power of 2
    tileSize = hibit(tileSize);

    //  Compute the number of these tiles that we can fit across and up
    uint32_t nTilesAcross = pixelsX / tileSize;
    uint32_t nTilesUp     = pixelsY / tileSize;

    Point2d tileDims = {tileSize,tileSize};

    double pixelsToModelY = imageDef.m_size.y / imageDef.m_sizeInPixels.y;

    //  For each row
    DPoint3d rowOrigin = DPoint3d::From(imageDef.m_origin);       // lower left
    double layoutDirection = 1.0; 
    rowOrigin.z = -DgnViewport::GetDisplayPriorityFrontPlane();  // lowest possibly priority
    m_tilesX = 0;
    Byte const* rowStart = &imageData[0];
    uint32_t pitch = (uint32_t)imageDef.GetPitch();
        
    for (uint32_t row=0; row<nTilesUp; ++row)
        {
        //  March tiles across the columns of this row
        DefineImageTexturesForRow(imageDef, rowStart, rowOrigin, tileDims, nTilesAcross);
        rowStart += tileDims.y*pitch;
        rowOrigin.y += layoutDirection * (tileDims.y * pixelsToModelY);// march up the y-axis in model coordinates
        ++m_tilesX;
        } 

    //  rowStart now points to the remaining pixels across the top.
    Point2d topTileDims;
    topTileDims.x = tileSize;
    topTileDims.y = imageDef.m_sizeInPixels.y - (nTilesUp*tileSize);
    if (topTileDims.y != 0)
        {
        DefineImageTexturesForRow(imageDef, rowStart, rowOrigin, topTileDims, nTilesAcross);
        rowStart += topTileDims.y*pitch;
        rowOrigin.y += layoutDirection * (topTileDims.y * pixelsToModelY);// march up the y-axis in model coordinates
        ++m_tilesX;
        }

    //  Finally, append the corners of the last stub tile
    Point2d nopTileDims;
    nopTileDims.x = tileSize;
    nopTileDims.y = 0;  // TRICKY: This tells DefineImageTexturesForRow not to define textures, just push DPoint3ds
    DefineImageTexturesForRow(imageDef, nullptr, rowOrigin, nopTileDims, nTilesAcross);

#else

    //  tile corners:
    //
    //          [2]                         [3]   ^
    //                                          m_size.y
    //  m_origin[0]                         [1]   v
    //             <-------m_size.x-------->
    DPoint3d uvPts[4];
    ::memset(uvPts, 0, sizeof(uvPts));
    uvPts[0].x = uvPts[2].x = m_imageDef.m_origin.x;
    uvPts[0].y = uvPts[1].y = m_imageDef.m_origin.y;
    uvPts[1].x = uvPts[3].x = m_imageDef.m_origin.x + m_imageDef.m_size.x;
    uvPts[2].y = uvPts[3].y = m_imageDef.m_origin.y + m_imageDef.m_size.y;
    
    for (int i=0; i<_countof(uvPts); ++i)
        uvPts[i].z = -DgnViewport::GetDisplayPriorityFrontPlane();  // lowest possibly priority

    if (imageDef.GetIsTopDown())
        {
        std::swap(uvPts[0], uvPts[2]);
        std::swap(uvPts[1], uvPts[3]);
        }

    for (auto const& corner : uvPts)
        m_tileOrigins.push_back(corner);

    auto pitch = (uint32_t)imageDef.GetPitch();
    auto tileid = GetBackDropTextureId();
    Point2d tileDims;
    tileDims.x = imageDef.m_sizeInPixels.x;
    tileDims.y = imageDef.m_sizeInPixels.y;

    T_HOST.GetGraphicsAdmin()._DefineTile(tileid, nullptr, tileDims, false, imageDef.m_format, pitch, imageData.data());
    m_tileIds.push_back(tileid);
    m_tilesX = 1;

#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void RedlineModel::LoadImageDataAndDefineTexture()
    {
    if (T_HOST.GetGraphicsAdmin()._IsTextureIdDefined(GetBackDropTextureId()))
        return;
    bvector<uint8_t> imageData;
    LoadImageData(m_imageDef, imageData);
    DefineImageTextures(m_imageDef, imageData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void RedlineViewController::OnOpen(RedlineModel& targetModel)
    {
    targetModel.LoadImageDataAndDefineTexture();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void RedlineViewController::OnClose(RedlineModel& targetModel)
    {
    T_HOST.GetGraphicsAdmin()._DeleteTexture(targetModel.GetBackDropTextureId());
    }

void RedlineViewController::SetDrawBorder(bool allow) {m_drawBorder=allow;}
bool RedlineViewController::GetDrawBorder() const {return m_drawBorder;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus RedlineModel::DrawImage(ViewContextR context, DPoint3dCR viewOrg, DVec3dCR viewDelta, bool drawBorder)
    {
    // Make sure texture is defined. (I think it could get deleted when we run low on memory, so we might have to re-create it on the fly.)
    LoadImageDataAndDefineTexture();

    uintptr_t const& x = m_tileIds.front();
    int tilesY = (int)m_tileIds.size() / m_tilesX;

    context.GetIViewDraw().DrawMosaic((int)m_tilesX, tilesY, &x, &m_tileOrigins[0]);

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                10/14
+---------------+---------------+---------------+---------------+---------------+------*/
uintptr_t RedlineModel::GetBackDropTextureId() const 
    {
    return (uintptr_t) this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void RedlineViewController::_DrawView(ViewContextR context)
    {
    BeAssert(dynamic_cast<RedlineModel*> (context.GetViewport()->GetViewController().GetTargetModel()) != NULL && "RedlineViewController should be used only to draw RedlineModels!");
    RedlineModel* targetModel = (RedlineModel*) context.GetViewport()->GetViewController().GetTargetModel();

    targetModel->DrawImage(context, GetOrigin(), GetDelta(), m_drawBorder);

    T_Super::_DrawView(context);   // draws sheet border and redline graphics
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/14
+---------------+---------------+---------------+---------------+---------------+------*/
ViewController* RedlineViewController::Create(DgnDbR project, DgnViewId viewId)
    {
    auto markupProject = dynamic_cast<DgnMarkupProject*>(&project);
    if (markupProject == NULL)
        return NULL;
    auto rdlView = markupProject->Views().QueryView(viewId);
    auto rdlModel = markupProject->OpenRedlineModel(rdlView.GetBaseModelId());
    if (rdlModel == NULL)
        return NULL;

    return new RedlineViewController(*rdlModel, viewId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/14
+---------------+---------------+---------------+---------------+---------------+------*/
RedlineViewControllerPtr RedlineViewController::InsertView(RedlineModelR rdlModel, DgnViewId templateView, BSIRectCR projectViewRect, BSIRectCR imageViewRect)
    {
    DgnMarkupProject* project = rdlModel.GetDgnMarkupProject();
    if (project == NULL)
        {
        BeAssert(false);
        return NULL;
        }

    DgnClassId classId(project->Schemas().GetECClassId("dgn","RedlineView"));
    DgnViews::View view(DgnViewType::Sheet, classId, rdlModel.GetModelId(), rdlModel.GetModelName(), NULL, DgnViewSource::Generated);

    auto result = rdlModel.GetDgnMarkupProject()->Views().Insert(view);
    if (BE_SQLITE_OK != result)
        return NULL;

    auto controller = new RedlineViewController(rdlModel, view.GetId());

    controller->m_enableViewManipulation = true; // *** TRICKY: Normally, RedlineViewController::SetDelta, Origin, Rotation are disabled (to prevent user from changing camera on sheet.)

    auto templateSheet = project->Views().LoadViewController(templateView, DgnViews::FillModels::No);
    if (templateSheet.IsValid())
        {
        if (templateSheet->GetViewFlags().bgColor)
            controller->SetBackgroundColor(templateSheet->GetBackgroundColor());
        controller->SetDelta(templateSheet->GetDelta());
        controller->SetOrigin(templateSheet->GetOrigin());
        controller->GetViewedCategoriesR() = templateSheet->GetViewedCategories();
        controller->SetRotation(templateSheet->GetRotation());
        controller->GetViewFlagsR() = templateSheet->GetViewFlags();
        }
    else
        {
        DPoint3d org;
        DVec3d delta;
        AxisAlignedBox3d range = rdlModel.QueryModelRange();
        if (!range.IsEmpty())
            {
            range.Extend(range.XLength() * 0.1);   // add in a margin of 10%
            delta.Init(range.XLength(), range.YLength());
            org.Init(range.low.x, range.low.y);
            }
        else
            {
            org.Init(0,0);
            delta.Init(1024,1024);
            }

        controller->SetOrigin(org);
        controller->SetDelta(delta);

        RotMatrix rot;
        rot.InitIdentity();
        controller->SetRotation(rot);
        
        ViewFlags flags;
        memset(&flags, 0, sizeof(flags));
        controller->GetViewFlagsR() = flags;

        for (auto const& cat : rdlModel.GetDgnDb().Categories().MakeIterator())
            controller->ChangeCategoryDisplay(cat.GetCategoryId(), true);
        }
        
    controller->m_enableViewManipulation = false;

    // Align the redline view with the DgnDb view, in order to make the transition from project to redline view smoother.
    //
    // Background: Before a view is displayed, the DgnViewport will always "adjust" the view's origin and delta to get as 
    // much of the viewed area on the screen as possible. If, for example, the screen is tall and narrow while
    // the viewinfo delta is short and wide, the viewinfo origin will be shifted and its delta expanded so that
    // the largest possible swath of the viewed area is displayed in the narrow window.
    // 
    // We force the redline view to be adjusted now, and then we set the sheet origin and delta to match. That way, we
    // know that the redline model (and the image that we embed in it) will display in about the same location on the screen
    // as the DgnDb view. That makes for a smoother transition from DgnDb vew to redline view. 
    controller->AdjustAspectRatio(projectViewRect.Aspect(), true);

    controller->Save();

    return controller;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId DgnMarkupProject::CreateRedlineModelView(RedlineModelR model, DgnViewId templateView, BSIRectCR projectViewRect, BSIRectCR imageViewRect)
    {
    if (CheckIsOpen() != BSISUCCESS)
        return DgnViewId();

    auto controller = RedlineViewController::InsertView(model, templateView, projectViewRect, imageViewRect);
    if (!controller.IsValid())
        {
        BeAssert(false && "We must always generate rdl views with unique names!");
        return  DgnViewId();
        }

    //  Associate the redline model with the view.
    int64_t v = controller->GetViewId().GetValue();
    SaveProperty(RedlineModelProperty::ViewId(), &v, sizeof(v), model.GetModelId().GetValue());

    //  Adjust the model's sheet settings to fit in the view
    //model.m_origin.Init (controller->GetOrigin().x, controller->GetOrigin().y);
    model.m_size.Init(controller->GetDelta().x, controller->GetDelta().y);

    //  Apply user-specified insets. 
    //  imageViewRect is the desired location and size of the image in pixels
    //  imageViewRect - projectViewRect is a vector that applies the desired adjustment (in pixels).
    //  Ultimately, we have to adjust the view controller's origin and delta, which are in model coordinates.
    //  So, we'll have to transform this adjustment from pixels to distances. The conversion is based on the fact that
    //  the desired adjustment as a proportion of the view dimensions is the same whether stated in pixels or in distances.
    //  insetdistance.x / widthdistance.x =  insetpixels.x / widthpixels.x
    //  Therefore,
    //  insetdistance.x                   = (insetpixels.x / widthpixels.x) * widthdistance.x 
    //                                    =  insetpixels.x * (widthdistance.x/widthpixels.x)
    //                                      
    double toDistanceX = model.m_size.x / projectViewRect.Width();
    double toDistanceY = model.m_size.y / projectViewRect.Height();

    DPoint2d deltaUL;    // vector from project view origin to desired origin (upper left)
    deltaUL.x = ( imageViewRect.Left() - projectViewRect.Left()     ) * toDistanceX;
    deltaUL.y = ( imageViewRect.Top()  - projectViewRect.Top()      ) * toDistanceY;

    DPoint2d deltaBR;    // vector from project view corner to desired corner (bottom right)
    deltaBR.x = ( imageViewRect.Right() - projectViewRect.Right()   ) * toDistanceX;
    deltaBR.y = ( imageViewRect.Bottom()  - projectViewRect.Bottom()) * toDistanceY;

    //  Now restate the adjustment in terms of changes to bottom left and size

    DPoint2d deltaBL;   // vector from sheet origin to desired origin (bottom left)
    deltaBL.Init(deltaUL.x, deltaBR.y);
    DVec2d deltaSize; 
    deltaSize.Init(-deltaUL.x + deltaBR.x,  deltaUL.y + (-deltaBR.y));

    //model.m_origin.Add (deltaBL);
    model.m_size.Add(deltaSize);

    /*
    double inset = 0.1 * uorPerMaster;
    sheetDef.m_size.x -= 2*inset;
    sheetDef.m_size.y -= 2*inset;
    sheetDef.m_origin.x += inset;
    sheetDef.m_origin.y += inset;
    */

    model.Update();

    return controller->GetViewId();    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/13
DgnViewId RedlineModel::GetViewId () const
    {
    int64_t v;
    if (GetDgnDb().QueryProperty (&v, sizeof(v), RedlineModelProperty::ViewId(), GetModelId().GetValue()) != BE_SQLITE_ROW)
        return DgnViewId();
    return DgnViewId(v);
    }
+---------------+---------------+---------------+---------------+---------------+------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/14
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef WIP_PhysicalRedlineViewController // *** we need the PhysicalViewController& of the subject view in order to create a PhysicalRedlineViewController
ViewControllerPtr PhysicalRedlineViewController::Create(DgnViewType viewType, Utf8CP viewSubtype, DgnDbR project, DgnViewId viewId)
    {
    if (CheckViewSubType(viewSubtype,GetViewSubType()) != BSISUCCESS)
        return NULL;
    auto markupProject = dynamic_cast<DgnMarkupProject*>(&project);
    if (markupProject == NULL)
        return NULL;
    auto rdlView = markupProject->Views().QueryView(viewId);
    auto rdlModel = markupProject->OpenRedlineModel(rdlView.GetBaseModelId());
    if (rdlModel == NULL)
        return NULL;
    return new PhysicalRedlineViewController(*rdlModel, subjectViewController);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/14
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalRedlineViewControllerPtr PhysicalRedlineViewController::InsertView(PhysicalRedlineModel& model, PhysicalViewController& subjectView)
    {
    DgnViews::View view(DgnViewType::Physical, GetViewSubType(), model.GetModelId(),model.GetModelName(), NULL, DgnViewSource::Generated);

    auto rc = model.GetDgnMarkupProject()->Views().InsertView(view);
    if (BE_SQLITE_OK != rc)
        return NULL;

    auto controller = new PhysicalRedlineViewController(model, subjectView, view.GetId());

    //  The physical redline view is always exactly the same as the subject DgnDb view. Start it off with a copy of the subject view's settings.
    //  Note: we route the view settings through Json to avoid problems in the case where dgnView is not exactly the same type as this view controller.
    //  We know it's a sub-class of PhysicalViewController, and so it has all of the properties that we care about. 
    Json::Value json;
    subjectView._SaveToSettings(json);
    controller->_RestoreFromSettings(json);
    controller->Save();

    return controller;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId DgnMarkupProject::CreatePhysicalRedlineModelView(PhysicalRedlineModelR model, PhysicalViewControllerR dgnView)
    {
    if (CheckIsOpen() != BSISUCCESS)
        return DgnViewId();

    auto controller = PhysicalRedlineViewController::InsertView(model, dgnView);
    if (!controller.IsValid())
        {
        BeAssert(false && "We must always generate rdl views with unique names!");
        return  DgnViewId();
        }

    //  Associate the redline model with the view.
    int64_t v = controller->GetViewId().GetValue();
    SaveProperty(RedlineModelProperty::ViewId(), &v, sizeof(v), model.GetModelId().GetValue());

    return controller->GetViewId();
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/13
DgnViewId PhysicalRedlineModel::GetViewId () const
    {
    int64_t v;
    if (m_dgndb->QueryProperty (&v, sizeof(v), RedlineModelProperty::ViewId(), GetModelId().GetValue()) != BE_SQLITE_ROW)
        return DgnViewId();
    return DgnViewId(v);
    }
+---------------+---------------+---------------+---------------+---------------+------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
RedlineModelP DgnMarkupProject::CreateRedlineModel(Utf8CP name, DgnModelId templateModel)
    {
    if (CheckIsOpen() != BSISUCCESS)
        return nullptr;

    RedlineModelPtr rdlModel = RedlineModel::Create(*this, name, templateModel);
    if (!rdlModel.IsValid())
        return nullptr;

    rdlModel->Insert(); // Takes ownership of the rdlModel, adding a reference to it.

    SaveChanges();

    return rdlModel.get();      // ... that's why I can return a naked pointer to the model
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalRedlineModelP DgnMarkupProject::CreatePhysicalRedlineModel(Utf8CP name, PhysicalModelCR subjectViewTargetModel)
    {
    if (CheckIsOpen() != BSISUCCESS)
        return nullptr;

    PhysicalRedlineModelPtr rdlModel = PhysicalRedlineModel::Create(*this, name, subjectViewTargetModel);

    if (!rdlModel.IsValid())
        return nullptr;

    rdlModel->Insert(); // Takes ownership of the rdlModel, adding a reference to it.

    SaveChanges();

    return rdlModel.get();      // ... that's why I can return a naked pointer to the model
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
RedlineModelP DgnMarkupProject::OpenRedlineModel(DgnModelId mid)
    {
    if (CheckIsOpen() != BSISUCCESS)
        return NULL;

    RedlineModelPtr redlineModel = Models().Get<RedlineModel>(mid);
    if (!redlineModel.IsValid())
        return nullptr;

    //! Always fill a redline model. We never work with a subset of redline graphics.
    //! Note: even if redline model was previously loaded, it might have been emptied. So, make sure it's filled.
    redlineModel->FillModel();

    return redlineModel.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalRedlineModelP DgnMarkupProject::OpenPhysicalRedlineModel(DgnModelId mid)
    {
    if (CheckIsOpen() != BSISUCCESS)
        return NULL;

    PhysicalRedlineModelPtr redlineModel = Models().Get<PhysicalRedlineModel>(mid);
    if (!redlineModel.IsValid())
        return nullptr;

    //! Always fill a redline model. We never work with a subset of redline graphics.
    //! Note: even if redline model was previously loaded, it might have been emptied. So, make sure it's filled.
    redlineModel->FillModel();

    return redlineModel.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnMarkupProject::EmptyRedlineModel(DgnModelId mid)
    {
    if (CheckIsOpen() != BSISUCCESS)
        return BSIERROR;

    DgnModelP dgnModel = Models().FindModel(mid).get();
    if (dgnModel == NULL)
        return BSIERROR;

    dgnModel->EmptyModel();
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnMarkupProject::EmptyPhysicalRedlineModel(DgnModelId mid)
    {
    return EmptyRedlineModel(mid);
    }
