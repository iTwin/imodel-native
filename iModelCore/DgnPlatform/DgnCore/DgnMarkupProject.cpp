/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnMarkupProject.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include "DgnCoreLog.h"
#include <BeJpeg/BeJpeg.h>
#include <DgnPlatform/DgnMarkupProject.h>
#include <DgnPlatform/DgnView.h>

#define QV_RGBA_FORMAT   0
#define QV_BGRA_FORMAT   1
#define QV_RGB_FORMAT    2
#define QV_BGR_FORMAT    3

#define MARKUPEXTERNALLINK_LinkedElementId "LinkedElementId"

static WCharCP s_markupDgnDbExt   = L".markupdb";
static Utf8CP  s_projectType      = "Markup";

BEGIN_BENTLEY_DGN_NAMESPACE

DOMAIN_DEFINE_MEMBERS(MarkupDomain)

namespace dgn_ModelHandler
    {
    HANDLER_DEFINE_MEMBERS(SpatialRedline)
    HANDLER_DEFINE_MEMBERS(Redline)
    }

namespace dgn_ElementHandler
    {
    HANDLER_DEFINE_MEMBERS(MarkupExternalLinkHandler)
    HANDLER_DEFINE_MEMBERS(MarkupExternalLinkGroupHandler)
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    04/2016
//---------------------------------------------------------------------------------------
MarkupExternalLink::CreateParams::CreateParams(LinkModelR linkModel, DgnElementId linkedElementId /*= DgnElementId()*/) : CreateParams(Dgn::DgnElement::CreateParams(linkModel.GetDgnDb(), linkModel.GetModelId(), MarkupExternalLink::QueryClassId(linkModel.GetDgnDb())), linkedElementId)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2016
//---------------------------------------------------------------------------------------
MarkupExternalLinkCPtr MarkupExternalLink::Insert()
    {
    MarkupExternalLinkCPtr link = GetDgnDb().Elements().Insert<MarkupExternalLink>(*this);
    BeAssert(link.IsValid());
    return link;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2016
//---------------------------------------------------------------------------------------
MarkupExternalLinkCPtr MarkupExternalLink::Update()
    {
    MarkupExternalLinkCPtr link = GetDgnDb().Elements().Update<MarkupExternalLink>(*this);
    BeAssert(link.IsValid());
    return link;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    04/2016
//---------------------------------------------------------------------------------------
DgnDbStatus MarkupExternalLink::_BindInsertParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat = BindParams(statement);
    if (DgnDbStatus::Success != stat)
        return stat;
    return T_Super::_BindInsertParams(statement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    01/2016
//---------------------------------------------------------------------------------------
DgnDbStatus MarkupExternalLink::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& statement)
    {
    DgnDbStatus stat = BindParams(statement);
    if (DgnDbStatus::Success != stat)
        return stat;
    return T_Super::_BindUpdateParams(statement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    04/2016
//---------------------------------------------------------------------------------------
DgnDbStatus MarkupExternalLink::BindParams(BeSQLite::EC::ECSqlStatement& stmt)
    {
    if (ECSqlStatus::Success != stmt.BindId(stmt.GetParameterIndex(MARKUPEXTERNALLINK_LinkedElementId), m_linkedElementId))
        return DgnDbStatus::BadArg;

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    04/2016
//---------------------------------------------------------------------------------------
DgnDbStatus MarkupExternalLink::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    DgnDbStatus status = T_Super::_ReadSelectParams(stmt, params);
    if (DgnDbStatus::Success != status)
        return status;

    m_linkedElementId = stmt.GetValueId<DgnElementId>(params.GetSelectIndex(MARKUPEXTERNALLINK_LinkedElementId));
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    04/2016
//---------------------------------------------------------------------------------------
void MarkupExternalLink::_CopyFrom(DgnElementCR other)
    {
    T_Super::_CopyFrom(other);

    MarkupExternalLinkCP otherLink = dynamic_cast<MarkupExternalLinkCP> (&other);
    if (otherLink)
        m_linkedElementId = otherLink->m_linkedElementId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    04/2016
//---------------------------------------------------------------------------------------
MarkupExternalLinkGroup::CreateParams::CreateParams(LinkModelR linkModel) : CreateParams(Dgn::DgnElement::CreateParams(linkModel.GetDgnDb(), linkModel.GetModelId(), MarkupExternalLinkGroup::QueryClassId(linkModel.GetDgnDb())))
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2016
//---------------------------------------------------------------------------------------
MarkupExternalLinkGroupCPtr MarkupExternalLinkGroup::Insert()
    {
    MarkupExternalLinkGroupCPtr link = GetDgnDb().Elements().Insert<MarkupExternalLinkGroup>(*this);
    BeAssert(link.IsValid());
    return link;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2016
//---------------------------------------------------------------------------------------
MarkupExternalLinkGroupCPtr MarkupExternalLinkGroup::Update()
    {
    MarkupExternalLinkGroupCPtr link = GetDgnDb().Elements().Update<MarkupExternalLinkGroup>(*this);
    BeAssert(link.IsValid());
    return link;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/2015
//---------------------------------------------------------------------------------------
MarkupDomain::MarkupDomain() : DgnDomain(MARKUP_SCHEMA_NAME, "Markup Domain", 1)
    {
    RegisterHandler(dgn_ModelHandler::Redline::GetHandler());
    RegisterHandler(dgn_ModelHandler::SpatialRedline::GetHandler());
    RegisterHandler(dgn_ElementHandler::RedlineViewDef::GetHandler());
    RegisterHandler(dgn_ElementHandler::MarkupExternalLinkHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::MarkupExternalLinkGroupHandler::GetHandler());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
void MarkupDomain::_OnDgnDbOpened(DgnDbR db) const
    {
    if (!CustomPropertyRegistry::HasOldDgnSchema(db))
        return;

    CustomPropertyRegistry prop;
    prop.SetClass(db, "Markup", "MarkupExternalLink");
    prop.Register("LinkedElementId");
    }

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
    for (auto const& view : ViewDefinition::MakeIterator(*this))
        {
        if (view.GetBaseModelId() == mid)
            return view.GetId();
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
DgnViewId SpatialRedlineModel::GetFirstView()
    {
    auto db = GetDgnMarkupProject();
    return db? db->GetFirstViewOf(GetModelId()): DgnViewId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialRedlineViewController::SpatialRedlineViewController(SpatialRedlineModel& rdlModel, SpatialViewController& subjectView, DgnViewId physicalRedlineViewId, bvector<SpatialRedlineModelP> const& otherRdlsToView) 
    : 
    SpatialViewController (*rdlModel.GetDgnMarkupProject(), physicalRedlineViewId.IsValid()? physicalRedlineViewId: rdlModel.GetFirstView()),
    m_subjectView(subjectView),
    m_otherRdlsInView(otherRdlsToView)
    {
    // By default, users of this view controller should target the redline model and we should set up the viewport based on the redline model.
    m_targetModelIsInSubjectView = false;

    SynchWithSubjectViewController();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialRedlineViewController::~SpatialRedlineViewController()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrienBastings   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialRedlineViewController::_OnViewOpened(DgnViewportR vp)
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
void SpatialRedlineViewController::SynchWithSubjectViewController()
    {
    // There can only be one set of view flags. It will be used to initialize the viewport and qv. 
    // *** EXPERIMENTAL: Here, I force a couple of flags to suit the redline view better. Does this cause too much of a change in the subject view??
    m_viewFlags = m_subjectView.GetViewFlags();
    m_viewFlags.weights = true;
    m_viewFlags.acs = true;
    m_viewFlags.grid = true;
    }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
IAuxCoordSysP SpatialRedlineViewController::_GetAuxCoordinateSystem() const
    {
    // Redline views have their own ACS
    return T_Super::_GetAuxCoordinateSystem();
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef SpatialRedlineViewController::_GetBackgroundColor() const
    {
    // There can only be one background color
    return m_subjectView._GetBackgroundColor();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d SpatialRedlineViewController::_GetOrigin() const
    {
    return m_subjectView.GetOrigin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
DVec3d SpatialRedlineViewController::_GetDelta() const
    {
    return m_subjectView.GetDelta();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
RotMatrix SpatialRedlineViewController::_GetRotation() const
    {
    return m_subjectView.GetRotation();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialRedlineViewController::_SetOrigin(DPoint3dCR org)
    {
    T_Super::_SetOrigin(org);
    m_subjectView.SetOrigin(org);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialRedlineViewController::_SetDelta(DVec3dCR delta)
    {
    T_Super::_SetDelta(delta);
    m_subjectView.SetDelta(delta);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialRedlineViewController::_AdjustAspectRatio(double aspect, bool expandView)
    {
    T_Super::_AdjustAspectRatio(aspect, expandView);
    m_subjectView.AdjustAspectRatio(aspect, expandView);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d SpatialRedlineViewController::_GetTargetPoint() const
    {
    return m_subjectView.GetTargetPoint();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool SpatialRedlineViewController::_Allow3dManipulations() const {return m_subjectView.Allow3dManipulations();}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    04/2016
//---------------------------------------------------------------------------------------
// WIP_MERGE_John_Patterns - double PhysicalRedlineViewController::_GetPatternZOffset (ViewContextR context, ElementHandleCR eh) const {return m_subjectView.GetPatternZOffset(context, eh);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialRedlineViewController::_SetRotation(RotMatrixCR rot)
    {
    T_Super::_SetRotation(rot);
    m_subjectView.SetRotation(rot);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      03/14
//---------------------------------------------------------------------------------------
void SpatialRedlineViewController::_SaveToSettings() const 
    {
    m_subjectView._SaveToSettings();
    }

#ifdef WIP_RDL_QUERYVIEWS
bool SpatialRedlineViewController::_IsInSet (int nVal, BeSQLite::DbValue const* vals) const {return m_subjectView._IsInSet(nVal,vals);}

bool SpatialRedlineViewController::_WantElementLoadStart (ViewportR viewport, double currentTime, double lastQueryTime, uint32_t maxElementsDrawnInDynamicUpdate, Frustum const& queryFrustum) {return m_subjectView._WantElementLoadStart(viewport,currentTime,lastQueryTime,maxElementsDrawnInDynamicUpdate,queryFrustum);}
Utf8String SpatialRedlineViewController::_GetRTreeMatchSql (ViewportR viewport) {return m_subjectView._GetRTreeMatchSql(viewport);}
int32_t SpatialRedlineViewController::_GetMaxElementFactor() {return m_subjectView._GetMaxElementFactor();}
double SpatialRedlineViewController::_GetMinimumSizePixels (DrawPurpose updateType) {return m_subjectView._GetMinimumSizePixels (updateType);}
uint64_t SpatialRedlineViewController::_GetMaxElementMemory () {return m_subjectView._GetMaxElementMemory();}
#endif

ViewController::FitComplete SpatialRedlineViewController::_ComputeFitRange (FitContextR context) {return m_subjectView._ComputeFitRange(context);}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
GeometricModelP SpatialRedlineViewController::_GetTargetModel() const
    {
    return m_targetModelIsInSubjectView? m_subjectView.GetTargetModel(): T_Super::_GetTargetModel();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d SpatialRedlineViewController::_GetViewedExtents() const
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
void SpatialRedlineViewController::_RestoreFromSettings()
    {
    T_Super::_RestoreFromSettings();
    SynchWithSubjectViewController();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.wilson      10/2015
//---------------------------------------------------------------------------------------
void SpatialRedlineViewController::_OnAttachedToViewport(DgnViewportR vp)
    {
    T_Super::_OnAttachedToViewport(vp);
    m_subjectView._OnAttachedToViewport(vp);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2014
//---------------------------------------------------------------------------------------
void SpatialRedlineViewController::_OnCategoryChange(bool singleEnabled)
    {
    T_Super::_OnCategoryChange(singleEnabled);
    m_subjectView._OnCategoryChange(singleEnabled);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    08/2014
//---------------------------------------------------------------------------------------
void SpatialRedlineViewController::_ChangeModelDisplay(DgnModelId modelId, bool onOff)
    {
    m_subjectView._ChangeModelDisplay(modelId, onOff);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialRedlineViewController::_DrawView(ViewContextR context)
    {
    //  Draw subject model
        {
        //  set up to draw subject model 
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

    //  Draw additional redline models
    for (auto rdlModel : m_otherRdlsInView)
        context.VisitDgnModel(rdlModel);
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
DbResult DgnMarkupProject::ConvertToMarkupProject(BeFileNameCR fileNameIn, CreateDgnDbParams const& params)
    {
    CreateDgnMarkupProjectParams& mpp = *(CreateDgnMarkupProjectParams*) &params;

    BeFileName fileName(fileNameIn);
    supplyDefaultExtension(fileName, s_markupDgnDbExt);

    if (true)
        {
        if (!mpp.m_seedDb.empty())
            BeFileName::BeCopyFile(mpp.m_seedDb, fileName, !mpp.GetOverwriteExisting());
        else
            {
            CreateDgnDbParams params;
            params.SetOverwriteExisting (mpp.GetOverwriteExisting());
            DgnDb::CreateDgnDb (nullptr, fileName, params);
            }
        }

    OpenParams oparams(OpenMode::ReadWrite);
    DbResult status = DoOpenDgnDb(fileName, oparams);

    ChangeBriefcaseId(BeBriefcaseId(BeBriefcaseId::Standalone()));
    SaveChanges();
    CloseDb();

    status = DoOpenDgnDb(fileName, oparams);

    if (BE_SQLITE_OK != status)
        {
        BeFileName::BeDeleteFile(fileName);
        return  status;
        }

    //  ------------------------------------------------------------------
    //  Set Markup-specific project properties
    //  ------------------------------------------------------------------
    SavePropertyString(DgnProjectProperty::ProjectType(), s_projectType);   // identifies the .bim as a Markup project.
    SetAssociation(mpp.GetSubjectDgnProject());

    if (mpp.GetSpatialRedlining())
        {
        SavePropertyString(DgnProjectProperty::IsSpatialRedline(), "true");
        }

    //  ------------------------------------------------------------------
    //  Make sure that we have at least one category
    //  ------------------------------------------------------------------
    if (!DgnCategory::QueryFirstCategoryId(*this).IsValid())
        {
        DgnCategory cat(DgnCategory::CreateParams(*this, Utf8String(fileName.GetFileNameWithoutExtension()).c_str(), DgnCategory::Scope::Any));
        DgnSubCategory::Appearance defaultAppearance;
        defaultAppearance.SetColor(ColorDef(0xff, 0xff, 0xff));
        cat.Insert(defaultAppearance);
        }

    //  ------------------------------------------------------------------
    //  Mark all pre-existing models and views as internal. They may be used
    //  as seeds, but they will never be used directly by the app or the user.
    //  ------------------------------------------------------------------
    for (auto const& entry : ViewDefinition::MakeIterator(*this))
        {
        auto cpView = ViewDefinition::QueryView(entry.GetId(), *this);
        auto pView = cpView.IsValid() ? cpView->MakeCopy<ViewDefinition>() : nullptr;
        if (pView.IsValid())
            {
            pView->SetSource(DgnViewSource::Private);
            pView->Update();
            }
        }

    if (true)
        {
        Statement stmt;
        // *** NEEDS WORK: Missing WHERE Id=?   
        stmt.Prepare(*this, "UPDATE " DGN_TABLE(DGN_CLASSNAME_Model) " SET Visibility=1");  // ModelIterate::All (i.e., hide when looking for models to show in the GUI)
        stmt.Step();
        }

    if (DgnDbStatus::Success != ImportMarkupSchema())
        return BE_SQLITE_ERROR;

    SaveSettings();
    SaveChanges();

    // POST CONDITIONS
    BeAssert(!mpp.GetSpatialRedlining() || IsSpatialRedlineProject());

    return BE_SQLITE_OK;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    05/2015
//---------------------------------------------------------------------------------------
DgnDbStatus DgnMarkupProject::ImportMarkupSchema()
    {
    BeFileName schemaFile(T_HOST.GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    schemaFile.AppendToPath(MARKUP_SCHEMA_PATH);

    DgnDbStatus status = MarkupDomain::GetDomain().ImportSchema(*this, schemaFile);
    BeAssert(DgnDbStatus::Success == status);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void RedlineModel::_WriteJsonProperties(Json::Value& val) const
    {
    T_Super::_WriteJsonProperties(val);

    Json::Value json(Json::objectValue);
    m_assoc.ToPropertiesJson(json);
    val["RedlineModel_Assoc"] = json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void RedlineModel::_ReadJsonProperties(Json::Value const& val)
    {
    T_Super::_ReadJsonProperties(val);
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

    if (assocData.GetViewId().IsValid() && !ViewDefinition::QueryView(assocData.GetViewId(), dgnProject).IsValid())
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
bool DgnMarkupProject::IsSpatialRedlineProject() const
    {
    Utf8String rdlPropVal;
    return QueryProperty(rdlPropVal, DgnProjectProperty::IsSpatialRedline()) == BE_SQLITE_ROW  &&  rdlPropVal == "true";
    }

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
DgnMarkupProject* SpatialRedlineModel::GetDgnMarkupProject() const
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
Render::Image::Format RedlineModel::ImageDef::GetRenderImageFormat() const
    {
    switch (m_format)
        {
        case QV_RGBA_FORMAT: return Render::Image::Format::Rgba;
        case QV_RGB_FORMAT: return Render::Image::Format::Rgb;
        }
    BeAssert(false);
    return Render::Image::Format::Rgb;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/13
+---------------+---------------+---------------+---------------+---------------+------*/
size_t RedlineModel::ImageDef::GetSizeofPixelInBytes() const
    {
    return HasAlpha() ? 4 : 3;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool RedlineModel::ImageDef::HasAlpha() const
    { 
    return (m_format == QV_RGBA_FORMAT || m_format == QV_BGRA_FORMAT);
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

    projectView.SaveToSettings();
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
    if (m_viewId.IsValid())
        {
        val["ViewId"] = m_viewId.GetValue();
        val["ViewGeometry"] = m_viewGeometry;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnViewAssociationData::FromPropertiesJson(Json::Value const& val)
    {
    T_Super::FromPropertiesJson(val);
    if (val.isMember("ViewId"))
        {
        m_viewId = DgnViewId(val["ViewId"].asUInt64());
        m_viewGeometry = val["ViewGeometry"];
        }
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
    for (auto const& view : ViewDefinition::MakeIterator(*this))
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
    if (nullptr == markupProject.Domains().FindDomain(MARKUP_SCHEMA_NAME))
        {
        BeAssert(false && "Must have Markup domain registered in order to create a redline model");
        return nullptr;
        }

    DgnClassId rmodelClassId = DgnClassId(markupProject.Schemas().GetECClassId(MARKUP_SCHEMA_NAME, "RedlineModel"));
    RedlineModelPtr rdlModel = new RedlineModel(RedlineModel::CreateParams(markupProject, rmodelClassId, CreateModelCode(name), DPoint2d::FromZero()));
    if (!rdlModel.IsValid())
        return nullptr;

    //  Copy model properties and contents from template
    SheetModelPtr templateModel = markupProject.Models().Get<SheetModel>(templateModelId);
    if (templateModel.IsValid())
        {
        //  Copy model properties and settings
        rdlModel->GetDisplayInfoR() = templateModel->GetDisplayInfo();
        rdlModel->m_size = templateModel->GetSize();

        //  Copy elements -- *** TBD ***
        }
    return rdlModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialRedlineModelPtr SpatialRedlineModel::Create(DgnMarkupProjectR markupProject, Utf8CP name, SpatialModelCR subjectViewTargetModel)
    {
    if (nullptr == markupProject.Domains().FindDomain(MARKUP_SCHEMA_NAME))
        {
        BeAssert(false && "Must have Markup domain registered in order to create a redline model");
        return nullptr;
        }

    DgnClassId rmodelClassId = DgnClassId(markupProject.Schemas().GetECClassId(MARKUP_SCHEMA_NAME, MARKUP_CLASSNAME_SpatialRedlineModel));

    SpatialRedlineModelPtr rdlModel = new SpatialRedlineModel(SpatialRedlineModel::CreateParams(markupProject, rmodelClassId, CreateModelCode(name)));
    if (!rdlModel.IsValid())
        {
        DGNCORELOG->error("SpatialRedlineModel::CreateModel failed");
        BeAssert(false && "SpatialRedlineModel::CreateModel failed");
        return nullptr;
        }

    UnitDefinition mu = subjectViewTargetModel.GetDisplayInfo().GetMasterUnits();
    UnitDefinition su = subjectViewTargetModel.GetDisplayInfo().GetSubUnits();
    rdlModel->GetDisplayInfoR().SetUnits(mu, su);

    return rdlModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void RedlineModel::StoreImageData(Render::Image const& imageData, bool isTopDown, bool fitToX, bool compressImageProperty)
    {
    //  Grab possibly updated image definition data
    m_imageDef.m_format = (int) imageData.GetFormat();
    m_imageDef.m_sizeInPixels.x = imageData.GetWidth();
    m_imageDef.m_sizeInPixels.y = imageData.GetHeight();

    m_imageDef.m_topDown = isTopDown;

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
    GetDgnMarkupProject()->SaveProperty(propSpec, imageData.GetByteStream().GetData(), imageData.GetByteStream().GetSize(), GetModelId().GetValue());

    // Update the sheetdef to make sure it encloses the image.
    if (!m_imageDef.m_size.AlmostEqual(DVec2d::From(m_size)))
        {
        m_size.x = std::max(m_imageDef.m_size.x, m_size.x);
        m_size.y = std::max(m_imageDef.m_size.y, m_size.y);
        }
    
    Update();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void RedlineModel::StoreImageData(ImageSourceCR source, bool isTopDown, bool fitToX)
    {
    Render::Image image(source);
    StoreImageData(image, isTopDown, fitToX, /*compresssImageProperty*/false);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void RedlineViewController::OnOpen(RedlineModel& targetModel)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void RedlineViewController::OnClose(RedlineModel& targetModel)
    {
    targetModel.m_tileGraphic = nullptr;
    }

void RedlineViewController::SetDrawBorder(bool allow) {m_drawBorder=allow;}
bool RedlineViewController::GetDrawBorder() const {return m_drawBorder;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/13
+---------------+---------------+---------------+---------------+---------------+------*/
Render::GraphicBuilderPtr RedlineModel::GetImageGraphic(ViewContextR context)
    {
    if (m_tileGraphic.IsValid())
        return m_tileGraphic;

    bvector<uint8_t> imageData;
    LoadImageData(m_imageDef, imageData);
    ByteStream byteStream(imageData.data(), (uint32_t) imageData.size());
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

    for (int i = 0; i<_countof(uvPts); ++i)
        uvPts[i].z = -DgnViewport::GetDisplayPriorityFrontPlane();  // lowest possibly priority

    if (m_imageDef.GetIsTopDown())
        {
        std::swap(uvPts[0], uvPts[2]);
        std::swap(uvPts[1], uvPts[3]);
        }

    auto& rsys = context.GetViewport()->GetRenderTarget()->GetSystem();
        
    m_tileGraphic = rsys._CreateGraphic(Graphic::CreateParams(nullptr));
    auto ifmt = m_imageDef.GetRenderImageFormat();
    ColorDef color(0xff, 0, 0, 0);      // *** WIP_MARKUP - what 'color' should be used for an image??
    m_tileGraphic->SetSymbology(color, color, 0);

    Render::Image image(m_imageDef.m_sizeInPixels.x, m_imageDef.m_sizeInPixels.y, std::move(byteStream), ifmt);
    auto texture = rsys._CreateTexture(image/*, m_imageDef.HasAlpha()*/);

    m_tileGraphic->AddTile(*texture, uvPts);
        
    m_tileGraphic->Close();

    return m_tileGraphic;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void RedlineViewController::_DrawView(ViewContextR context)
    {
    BeAssert(dynamic_cast<RedlineModel*> (context.GetViewport()->GetViewController().GetTargetModel()) != NULL && "RedlineViewController should be used only to draw RedlineModels!");
    RedlineModel* targetModel = (RedlineModel*) context.GetViewport()->GetViewController().GetTargetModel();
    if (nullptr == targetModel)
        {
        BeDataAssert(false);
        return;
        }

    auto graphic = targetModel->GetImageGraphic(context);
    if (graphic.IsValid())
        context.OutputGraphic(*graphic, nullptr);

    T_Super::_DrawView(context);   // draws sheet border and redline graphics
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/14
+---------------+---------------+---------------+---------------+---------------+------*/
ViewController* RedlineViewController::Create(DgnDbStatus* openStatusIn, DgnDbR project, DgnViewId viewId)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(openStatus, openStatusIn);

    auto markupProject = dynamic_cast<DgnMarkupProject*>(&project);

    if (markupProject == nullptr)
        {
        openStatus = DgnDbStatus::NotOpen;
        return nullptr;
        }

    if (nullptr == markupProject->Domains().FindDomain(MARKUP_SCHEMA_NAME))
        {
        BeAssert(false && "Must have Markup domain registered in order to create a redline view");
        return nullptr;
        }

    auto rdlView = ViewDefinition::QueryView(viewId, *markupProject);
    if (!rdlView.IsValid())
        {
        openStatus = DgnDbStatus::ViewNotFound;
        return nullptr;
        }

    auto rdlModel = markupProject->OpenRedlineModel(&openStatus, rdlView->GetBaseModelId());
    if (rdlModel == nullptr)
        return nullptr;

    return new RedlineViewController(*rdlModel, viewId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/14
+---------------+---------------+---------------+---------------+---------------+------*/
RedlineViewControllerPtr RedlineViewController::InsertView(DgnDbStatus* insertStatusIn, RedlineModelR rdlModel, DgnViewId templateView, BSIRectCR projectViewRect, BSIRectCR imageViewRect)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(insertStatus, insertStatusIn);

    DgnMarkupProject* project = rdlModel.GetDgnMarkupProject();
    if (project == NULL)
        {
        insertStatus = DgnDbStatus::NotOpen;
        BeAssert(false);
        return NULL;
        }

    RedlineViewDefinition view(RedlineViewDefinition::CreateParams(*project, rdlModel.GetCode().GetValue().c_str(),
                ViewDefinition::Data(rdlModel.GetModelId(), DgnViewSource::Generated)));
    if (!view.Insert(&insertStatus).IsValid())
        return nullptr;

    auto controller = new RedlineViewController(rdlModel, view.GetViewId());

    controller->m_enableViewManipulation = true; // *** TRICKY: Normally, RedlineViewController::SetDelta, Origin, Rotation are disabled (to prevent user from changing camera on sheet.)

    auto templateSheet = ViewDefinition::LoadViewController(templateView, *project, ViewDefinition::FillModels::No);
    if (templateSheet.IsValid())
        {
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
        flags.weights = true;
        controller->GetViewFlagsR() = flags;

        for (auto const& catId : DgnCategory::QueryCategories(rdlModel.GetDgnDb()))
            controller->ChangeCategoryDisplay(catId, true);
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
DgnViewId DgnMarkupProject::CreateRedlineModelView(DgnDbStatus* createStatusIn, RedlineModelR model, DgnViewId templateView, BSIRectCR projectViewRect, BSIRectCR imageViewRect)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(createStatus, createStatusIn);

    if (CheckIsOpen() != BSISUCCESS)
        {
        createStatus = DgnDbStatus::NotOpen;
        return DgnViewId();
        }

    if (nullptr == Domains().FindDomain(MARKUP_SCHEMA_NAME))
        {
        BeAssert(false && "Markup domain not registered");
        createStatus = DgnDbStatus::MissingDomain;
        return DgnViewId();
        }

    auto controller = RedlineViewController::InsertView(&createStatus, model, templateView, projectViewRect, imageViewRect);
    if (!controller.IsValid())
        {
        BeAssert(false && "RedlineViewController::InsertView failed! This could be caused by using a duplicate name or by failing to register the Markup domain.");
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
    //  Recall: BSIRect has origin at top left, corner at bottom right, and +y moves DOWN the view from top to bottom
    //  So, an inset in BSIRect terms will have +ix,+iy for the origin and -ix,-iy for the corner.
    //  In the diagram below, the O is the origin, and C is the corner. io is the inset origin, and ic is the inset corner.
    //  Positive X goes across, and positive Y goes down.
    //      +X
    //     ---->
    //    |  
    // +Y |                 deltaUL.x (+)           
    //    v                O--> .............
    //       deltaUL.y (+) |                :  
    //                     v   io           :
    //                     :                :
    //                     :                :
    //                     :            ic  ^
    //                     :                | deltaBR.y (-)
    //                     :.............<--C
    //                                   deltaBR.x (-)
    //
    //  In Cartesian coordinates the view origin is at bottom left, and +y moves UP the view. So, 
    //  THE SIGN OF Y CHANGES, and the origin and corner move from ToptLeft (TL) and BottomRight (BR) to BottomLeft (BL) and TopRight (TR)
    //
    //     ^
    //  +Y |
    //     |    +X
    //      ----->       
    //                                   deltaTR.x (-) = deltaBR.x
    //                     ..............<--TR
    //                     :                |  deltaTR.y (-) = -deltaUL.y
    //                     :           itr  v
    //                     :                :
    //                     :                :
    //                     ^   ibl          :
    //      deltaBL.y (+)  |                :
    //    = -deltaBR.y     BL-->.............:
    //                      deltaBL.x (+)
    //                      = deltaUL.x      
    DPoint2d deltaBL;
    deltaBL.Init (deltaUL.x, -deltaBR.y);
    DPoint2d deltaTR;
    deltaTR.Init (deltaBR.x, -deltaUL.y);
    //
    //  Thus deltaBL.y comes out +
    //       deltaTR.y comes out -
    //
    // Now a view is actually defined terms of origin and "delta", which is its size. So, if we want to adjust the BL rightward and upward and the TR leftward and downward, 
    // we must REDUCE the delta.x coordinate by the sum of the rightward and leftward adjustments and its y coordinate by the sum of the downward and upward adjustments.
     
    DVec2d deltaSize; 
    deltaSize.Init (-(deltaBL.x + -deltaTR.x),  -(deltaBL.y + -deltaTR.y));

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
#ifdef WIP_SpatialRedlineViewController // *** we need the SpatialViewController& of the subject view in order to create a SpatialRedlineViewController
ViewControllerPtr SpatialRedlineViewController::Create(DgnViewType viewType, Utf8CP viewSubtype, DgnDbR project, DgnViewId viewId)
    {
    if (CheckViewSubType(viewSubtype,GetViewSubType()) != BSISUCCESS)
        return NULL;
    auto markupProject = dynamic_cast<DgnMarkupProject*>(&project);
    if (markupProject == NULL)
        return NULL;
    auto rdlView = ViewDefinition::QueryView(viewId, project);
    auto rdlModel = rdlView.IsValid() ? markupProject->OpenRedlineModel(rdlView.GetBaseModelId()) : nullptr;
    if (rdlModel == NULL)
        return NULL;
    if (nullptr == rdlModel->GetDgnDb().Domains().FindDomain(MARKUP_SCHEMA_NAME))
        {
        BeAssert(false && "Must have Markup domain registered in order to create a redline view");
        return nullptr;
        }
    return new SpatialRedlineViewController(*rdlModel, subjectViewController);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/14
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialRedlineViewControllerPtr SpatialRedlineViewController::InsertView(SpatialRedlineModel& model, SpatialViewController& subjectView)
    {
    DgnViews::View view(DgnViewType::Physical, GetViewSubType(), model.GetModelId(),model.GetModelName(), NULL, DgnViewSource::Generated);

    auto rc = model.GetDgnMarkupProject()->Views().InsertView(view);
    if (BE_SQLITE_OK != rc)
        return NULL;

    if (nullptr == model.GetDgnMarkupProject()->Domains().FindDomain(MARKUP_SCHEMA_NAME))
        {
        BeAssert(false && "Markup domain not registered");
        //createStatus = DgnDbStatus::MissingDomain;
        return nullptr;
        }

    auto controller = new SpatialRedlineViewController(model, subjectView, view.GetId());

    //  The physical redline view is always exactly the same as the subject DgnDb view. Start it off with a copy of the subject view's settings.
    //  Note: we route the view settings through Json to avoid problems in the case where dgnView is not exactly the same type as this view controller.
    //  We know it's a sub-class of SpatialViewController, and so it has all of the properties that we care about. 
    Json::Value json;
    subjectView._SaveToSettings(json);
    controller->_RestoreFromSettings(json);
    controller->Save();

    return controller;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId DgnMarkupProject::CreateSpatialRedlineModelView(SpatialRedlineModelR model, SpatialViewControllerR dgnView)
    {
    if (CheckIsOpen() != BSISUCCESS)
        return DgnViewId();

    if (nullptr == Domains().FindDomain(MARKUP_SCHEMA_NAME))
        {
        BeAssert(false && "Markup domain not registered");
        createStatus = DgnDbStatus::MissingDomain;
        return nullptr;
        }

    auto controller = SpatialRedlineViewController::InsertView(model, dgnView);
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
DgnViewId SpatialRedlineModel::GetViewId () const
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
RedlineModelP DgnMarkupProject::CreateRedlineModel(DgnDbStatus* createStatusIn, Utf8CP name, DgnModelId templateModel)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(createStatus, createStatusIn);

    if (CheckIsOpen() != BSISUCCESS)
        {
        createStatus = DgnDbStatus::NotOpen;
        return nullptr;
        }

    if (nullptr == Domains().FindDomain(MARKUP_SCHEMA_NAME))
        {
        BeAssert(false && "Markup domain not registered");
        createStatus = DgnDbStatus::MissingDomain;
        return nullptr;
        }

    RedlineModelPtr rdlModel = RedlineModel::Create(*this, name, templateModel);
    if (!rdlModel.IsValid())
        {
        createStatus = DgnDbStatus::BadArg; // must be a bad name
        return nullptr;
        }

    createStatus = rdlModel->Insert(); // Takes ownership of the rdlModel, adding a reference to it.
    if (DgnDbStatus::Success != createStatus)
        return nullptr; 

    SaveChanges();

    return rdlModel.get();      // ... that's why I can return a naked pointer to the model
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialRedlineModelP DgnMarkupProject::CreateSpatialRedlineModel(DgnDbStatus* createStatusIn, Utf8CP name, SpatialModelCR subjectViewTargetModel)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(createStatus, createStatusIn);

    if (CheckIsOpen() != BSISUCCESS)
        {
        createStatus = DgnDbStatus::NotOpen;
        return nullptr;
        }

    if (nullptr == Domains().FindDomain(MARKUP_SCHEMA_NAME))
        {
        BeAssert(false && "Markup domain not registered");
        createStatus = DgnDbStatus::MissingDomain;
        return nullptr;
        }

    SpatialRedlineModelPtr rdlModel = SpatialRedlineModel::Create(*this, name, subjectViewTargetModel);

    if (!rdlModel.IsValid())
        {
        createStatus = DgnDbStatus::BadArg; // must be a bad name
        return nullptr;
        }

    createStatus = rdlModel->Insert(); // Takes ownership of the rdlModel, adding a reference to it.
    if (DgnDbStatus::Success != createStatus)
        return nullptr;

    SaveChanges();

    return rdlModel.get();      // ... that's why I can return a naked pointer to the model
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
RedlineModelP DgnMarkupProject::OpenRedlineModel(DgnDbStatus* openStatusIn, DgnModelId mid)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(openStatus, openStatusIn);

    if (CheckIsOpen() != BSISUCCESS)
        {
        openStatus = DgnDbStatus::NotOpen;
        return nullptr;
        }

    RedlineModelPtr redlineModel = Models().Get<RedlineModel>(mid);
    if (!redlineModel.IsValid())
        {
        openStatus = DgnDbStatus::NotFound;
        return nullptr;
        }

    //! Always fill a redline model. We never work with a subset of redline graphics.
    //! Note: even if redline model was previously loaded, it might have been emptied. So, make sure it's filled.
    redlineModel->FillModel();

    return redlineModel.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialRedlineModelP DgnMarkupProject::OpenSpatialRedlineModel(DgnDbStatus* openStatusIn, DgnModelId mid)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(openStatus, openStatusIn);

    if (CheckIsOpen() != BSISUCCESS)
        {
        openStatus = DgnDbStatus::NotOpen;
        return nullptr;
        }

    SpatialRedlineModelPtr redlineModel = Models().Get<SpatialRedlineModel>(mid);
    if (!redlineModel.IsValid())
        {
        openStatus = DgnDbStatus::NotFound;
        return nullptr;
        }

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
BentleyStatus DgnMarkupProject::EmptySpatialRedlineModel(DgnModelId mid)
    {
    return EmptyRedlineModel(mid);
    }

END_BENTLEY_DGN_NAMESPACE
