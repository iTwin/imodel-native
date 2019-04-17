/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include "DgnCoreLog.h"
#include <BeJpeg/BeJpeg.h>
#include <DgnPlatform/DgnMarkupProject.h>
#include <DgnPlatform/ViewDefinition.h>

#define QV_RGBA_FORMAT   0
#define QV_BGRA_FORMAT   1
#define QV_RGB_FORMAT    2
#define QV_BGR_FORMAT    3

#define MARKUPEXTERNALLINK_LinkedElementId "LinkedElementId"

#define CATEGORY_RedlineImage "RedlineImage"

static WCharCP s_markupDgnDbExt   = L".markupdb";
static Utf8CP  s_projectType      = "Markup";

DOMAIN_DEFINE_MEMBERS(MarkupDomain)

BEGIN_BENTLEY_DGN_NAMESPACE
namespace dgn_ModelHandler
    {
    HANDLER_DEFINE_MEMBERS(SpatialRedline)
    HANDLER_DEFINE_MEMBERS(Redline)
    }

namespace dgn_ElementHandler
    {
    HANDLER_DEFINE_MEMBERS(MarkupExternalLinkHandler)
    HANDLER_DEFINE_MEMBERS(MarkupExternalLinkGroupHandler)
    HANDLER_DEFINE_MEMBERS(RedlineViewDef);
    HANDLER_DEFINE_MEMBERS(RedlineElementHandler);
    }
END_BENTLEY_DGN_NAMESPACE

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
void MarkupExternalLink::_BindWriteParams(ECSqlStatement& stmt, ForInsert forInsert)
    {
    T_Super::_BindWriteParams(stmt, forInsert);
    stmt.BindId(stmt.GetParameterIndex(MARKUPEXTERNALLINK_LinkedElementId), m_linkedElementId);
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
// @bsimethod                                                   Jeff.Marker     11/2015
//---------------------------------------------------------------------------------------
void MarkupExternalLink::_ToJson(JsonValueR out, JsonValueCR opts) const 
    {
    T_Super::_ToJson(out, opts);
    if (m_linkedElementId.IsValid())
        out[json_linkedElementId()] = m_linkedElementId.ToHexStr();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2015
//---------------------------------------------------------------------------------------
void MarkupExternalLink::_FromJson(JsonValueR val)
    {
    T_Super::_FromJson(val);
    if (val.isMember(json_linkedElementId()))
        m_linkedElementId.FromJson(val[json_linkedElementId()]);
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
    RegisterHandler(dgn_ElementHandler::RedlineElementHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::RedlineViewDef::GetHandler());
    RegisterHandler(dgn_ElementHandler::MarkupExternalLinkHandler::GetHandler());
    RegisterHandler(dgn_ElementHandler::MarkupExternalLinkGroupHandler::GetHandler());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
static void createImageCategory(DefinitionModelR model)
    {
    DrawingCategory imageCategory(model, CATEGORY_RedlineImage, DgnCategory::Rank::System);
    DgnSubCategory::Appearance appearance;
    appearance.SetInvisible (false);
    appearance.SetColor (ColorDef(0xff,0xff,0xfe));
    appearance.SetWeight (0);
    appearance.SetTransparency (0);
    appearance.SetDontSnap(true);
    appearance.SetDontLocate(true);
    appearance.SetDisplayPriority(-50); // Use a negative display priority that is not too near the minimim.
    imageCategory.Insert(appearance);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                           Sam.Wilson             01/2016
//---------------------------------------------------------------------------------------
static void createRedlineCodeSpec(DgnDbR db)
    {
    CodeSpecPtr codeSpec = CodeSpec::Create(db, MARKUP_AUTHORITY_Redline, CodeScopeSpec::CreateModelScope());
    if (codeSpec.IsValid())
        codeSpec->Insert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void MarkupDomain::_OnSchemaImported(DgnDbR db) const
    {
    createImageCategory(db.GetDictionaryModel());
    createRedlineCodeSpec(db);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialRedlineViewController::SpatialRedlineViewController(SpatialRedlineModel& rdlModel, SpatialViewController& subjectView, OrthographicViewDefinition& redlineViewDef, bvector<SpatialRedlineModelP> const& otherRdlsToView)
    : 
    T_Super(redlineViewDef),
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
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialRedlineViewController::SynchWithSubjectViewController()
    {
#if defined (NEEDS_WORK_TARGET_MODEL)
    // There can only be one set of view flags. It will be used to initialize the viewport and qv. 
    // *** EXPERIMENTAL: Here, I force a couple of flags to suit the redline view better. Does this cause too much of a change in the subject view??
    m_viewFlags = m_subjectView.GetViewFlags();
    m_viewFlags.m_weights = true;
    m_viewFlags.m_acsTriad = true;
    m_viewFlags.m_grid = true;
#endif
    }

#if defined (NEEDS_WORK_CONTINUOUS_RENDER)
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
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool SpatialRedlineViewController::_Allow3dManipulations() const {return m_subjectView.Allow3dManipulations();}

#if defined (NEEDS_WORK_TARGET_MODEL)
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
void SpatialRedlineViewController::_StoreState() const 
    {
    m_subjectView._StoreState();
    }
#endif

#ifdef WIP_RDL_QUERYVIEWS
bool SpatialRedlineViewController::_IsInSet (int nVal, BeSQLite::DbValue const* vals) const {return m_subjectView._IsInSet(nVal,vals);}

int32_t SpatialRedlineViewController::_GetMaxElementFactor() {return m_subjectView._GetMaxElementFactor();}
double SpatialRedlineViewController::_GetMinimumSizePixels (DrawPurpose updateType) {return m_subjectView._GetMinimumSizePixels (updateType);}
uint64_t SpatialRedlineViewController::_GetMaxElementMemory () {return m_subjectView._GetMaxElementMemory();}
#endif

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
* @bsimethod                                                    Paul.Connelly   11/17
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialRedlineViewController::_SetViewedModels(DgnModelIdSet const& models)
    {
    m_subjectView.SetViewedModels(models);
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

        //  draw subject model using *this* view controller
        m_subjectView.DrawView(context);

        //  restore the redline model as the normal target
        m_targetModelIsInSubjectView = false;
        }

    //  Draw redline model
    T_Super::_DrawView(context);

    //  Draw additional redline models
    for (auto rdlModel : m_otherRdlsInView)
        context.VisitDgnModel(*rdlModel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/13
+---------------+---------------+---------------+---------------+---------------+------*/
RedlineViewController::RedlineViewController(RedlineViewDefinition const& rdlViewDef) : T_Super(rdlViewDef)
    {;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/13
+---------------+---------------+---------------+---------------+---------------+------*/
RedlineViewController::~RedlineViewController() 
    {
    }

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
        status = BE_SQLITE_ERROR_BadDbProfile;
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
DbResult DgnMarkupProject::InitializeNewMarkupProject(BeFileNameCR fileName, CreateDgnMarkupProjectParams const& mpp)
    {
    OpenParams oparams(OpenMode::ReadWrite);
    DbResult status = DoOpenDgnDb(fileName, oparams);

    if (BE_SQLITE_OK != status)
        {
        BeFileName::BeDeleteFile(fileName);
        return  status;
        }

    //  ------------------------------------------------------------------
    //  Set Markup-specific project properties
    //  ------------------------------------------------------------------
    SavePropertyString(DgnProjectProperty::ProjectType(), s_projectType);   // identifies the .bim as a Markup project.

    if (mpp.GetSpatialRedlining())
        {
        SavePropertyString(DgnProjectProperty::IsSpatialRedline(), "true");
        }

    //  ------------------------------------------------------------------
    //  Mark all pre-existing models and views as internal. They will never be used directly by the app or the user.
    //  ------------------------------------------------------------------
    for (auto const& entry : ViewDefinition::MakeIterator(*this))
        {
        auto cpView = ViewDefinition::Get(*this, entry.GetId());
        auto pView = cpView.IsValid() ? cpView->MakeCopy<ViewDefinition>() : nullptr;
        if (pView.IsValid())
            {
            pView->SetIsPrivate(true);
            pView->Update();
            }
        }

    if (true)
        {
        Statement stmt;
        stmt.Prepare(*this, "UPDATE " BIS_TABLE(BIS_CLASS_Model) " SET IsPrivate=0");
        stmt.Step();
        }

    // Flush transaction table to be able to import markup schema
    SaveChanges();
    Revisions().StartCreateRevision();
    Revisions().FinishCreateRevision();

    if (SchemaStatus::Success != MarkupDomain::GetDomain().ImportSchema(*this))
        return BE_SQLITE_ERROR;

    SaveChanges();

    // POST CONDITIONS
    BeAssert(!mpp.GetSpatialRedlining() || IsSpatialRedlineProject());

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnMarkupProjectPtr DgnMarkupProject::CreateDgnDb(DbResult* result, BeFileNameCR projectFileName, CreateDgnMarkupProjectParams const& params)
    {
    // Switch file name to use markup extension
    BeFileName fileName(projectFileName);
    supplyDefaultExtension(fileName, s_markupDgnDbExt);

    // Copy seed, or create new
    DgnDbPtr db;
    if (!params.m_seedDb.empty())
        {
        BeFileName::BeCopyFile(params.m_seedDb, fileName, !params.GetOverwriteExisting());
        OpenParams oparams(OpenMode::ReadWrite);
        db = DgnDb::OpenDgnDb(nullptr, fileName, oparams);

        // Must have its own unique GUID.
        BeGuid guid;
        guid.Create();
        db->ChangeDbGuid(guid);
        db->SaveChanges();
        }
    else
        {
        db = DgnDb::CreateDgnDb(nullptr, fileName, params);
        }

    if (db.IsNull())
        return nullptr;

    // Ensure standalone briefcase if master
    if (db->GetBriefcaseId().IsMasterId())
        {
        db->SetAsBriefcase(BeBriefcaseId(BeBriefcaseId::Standalone()));
        db->SaveChanges();
        db->CloseDb();
        }

    db = nullptr;

    DbResult ALLOW_NULL_OUTPUT (stat, result);

    DgnMarkupProjectPtr markupProject = new DgnMarkupProject();

    stat = markupProject->InitializeNewMarkupProject(fileName, params);

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
* @bsimethod                                    Sam.Wilson                      04/13
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialRedlineModelPtr SpatialRedlineModel::Create(DgnMarkupProjectR markupProject, Utf8CP name, SpatialModelCR subjectViewTargetModel)
    {
    if (nullptr == markupProject.Domains().FindDomain(MARKUP_SCHEMA_NAME))
        {
        BeAssert(false && "Must have Markup domain registered in order to create a redline model");
        return nullptr;
        }

    DgnClassId rmodelClassId = DgnClassId(markupProject.Schemas().GetClassId(MARKUP_SCHEMA_NAME, MARKUP_CLASSNAME_SpatialRedlineModel));

    SpatialRedlineModelPtr rdlModel = new SpatialRedlineModel(SpatialRedlineModel::CreateParams(markupProject, rmodelClassId, DgnElementId() /* WIP: Which element? */));
    if (!rdlModel.IsValid())
        {
        DGNCORELOG->error("SpatialRedlineModel::CreateModel failed");
        BeAssert(false && "SpatialRedlineModel::CreateModel failed");
        return nullptr;
        }

    UnitDefinition mu = subjectViewTargetModel.GetFormatter().GetMasterUnits();
    UnitDefinition su = subjectViewTargetModel.GetFormatter().GetSubUnits();
    rdlModel->GetFormatterR().SetUnits(mu, su);
    return rdlModel;
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
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/13
+---------------+---------------+---------------+---------------+---------------+------*/
void RedlineViewController::_DrawView(ViewContextR context)
    {
    T_Super::_DrawView(context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/14
+---------------+---------------+---------------+---------------+---------------+------*/
ViewController* RedlineViewController::Create(DgnDbStatus* openStatusIn, RedlineViewDefinition& rdlViewDef)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(openStatus, openStatusIn);

    auto markupProject = dynamic_cast<DgnMarkupProject*>(&rdlViewDef.GetDgnDb());

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

    RedlineModelPtr redlineModel = markupProject->Models().Get<RedlineModel>(rdlViewDef.GetBaseModelId());
    if (!redlineModel.IsValid())
        {
        openStatus = DgnDbStatus::NotFound;
        return nullptr;
        }

    return new RedlineViewController(rdlViewDef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/13
+---------------+---------------+---------------+---------------+---------------+------*/
RedlineViewDefinitionPtr RedlineViewDefinition::Create(DgnDbStatus* outCreateStatus, RedlineModelR model, DVec2dCR viewSize)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(createStatus, outCreateStatus);

    if (!model.GetModelId().IsValid())
        {
        createStatus = DgnDbStatus::BadModel;
        BeAssert(false && "RedlineModel must be persistent");
        return nullptr;
        }

    auto& db = model.GetDgnDb();

    if (nullptr == db.Domains().FindDomain(MARKUP_SCHEMA_NAME))
        {
        createStatus = DgnDbStatus::MissingDomain;
        BeAssert(false && "Markup domain not registered");
        return nullptr;
        }

    auto redline = db.Elements().Get<Redline>(model.GetModeledElementId());
    if (!redline.IsValid())
        {
        createStatus = DgnDbStatus::BadModel;
        BeAssert(false && "Can't find modeled Redline element from redline");
        return nullptr;
        }

    if (!redline->GetElementId().IsValid())
        {
        createStatus = DgnDbStatus::BadModel;
        BeAssert(false && "Redline element must be persistent");
        return nullptr;
        }

    DefinitionModelR dictionary = db.GetDictionaryModel();
    RefCountedPtr<DisplayStyle2d> displayStyle = new DisplayStyle2d(dictionary);
    displayStyle->SetBackgroundColor(ColorDef::White());
    RedlineViewDefinitionPtr view = new RedlineViewDefinition(dictionary, redline->GetCode().GetValueUtf8().c_str(), 
                                                              model.GetModelId(), *new CategorySelector(dictionary, redline->GetCode().GetValueUtf8().c_str()), *displayStyle);

    //  The view always has the same name as the redline and its model
    DgnCode code = CreateCode(dictionary, redline->GetCode().GetValueUtf8());
    if (!view->IsValidCode(code))
        {
        createStatus = DgnDbStatus::InvalidName;
        BeDataAssert(false && "redline element must have a code that is valid for its view to use as its code");
        return nullptr;
        }

    view->SetCode(code);

    //  The origin of a RedlineViewDefinition is always 0,0.
    view->SetOrigin2d(DPoint2d::FromZero());
    view->SetDelta2d(viewSize);

    ViewFlags flags;
    flags.SetRenderMode(RenderMode::Wireframe);
    flags.SetShowWeights(true);
    flags.SetShowFill(false);
    view->GetDisplayStyle().SetViewFlags(flags);

    auto& catsel = view->GetCategorySelector();
    for (ElementIteratorEntryCR categoryEntry : DrawingCategory::MakeIterator(db))
        catsel.AddCategory(categoryEntry.GetId<DgnCategoryId>());

    catsel.AddCategory(DgnCategory::QueryCategoryId(db, DrawingCategory::CreateCode(dictionary, CATEGORY_RedlineImage)));

    return view;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RedlineModel::ImageDef::FromJson(JsonValueCR json)
    {
    auto textureId = json[json_textureId()].asUInt64(0);
    if (0 != textureId)
        m_textureId = DgnTextureId(textureId);
    else
        m_textureId.Invalidate(); // to avoid assertions...

    m_origin.x = json[json_originX()].asDouble(0.0);
    m_origin.y = json[json_originY()].asDouble(0.0);
    m_size.x = json[json_sizeX()].asDouble(0.0);
    m_size.y = json[json_sizeY()].asDouble(0.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value RedlineModel::ImageDef::ToJson() const
    {
    Json::Value json;
    json.SetOrRemoveUInt64(json_textureId(), m_textureId.GetValueUnchecked(), 0);
    json.SetOrRemoveDouble(json_originX(), m_origin.x, 0.0);
    json.SetOrRemoveDouble(json_originY(), m_origin.y, 0.0);
    json.SetOrRemoveDouble(json_sizeX(), m_size.x, 0.0);
    json.SetOrRemoveDouble(json_sizeY(), m_size.y, 0.0);

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RedlineModel::_OnSaveJsonProperties()
    {
    T_Super::_OnSaveJsonProperties();
    SetJsonProperties(json_imageDef(), m_imageDef.ToJson());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
void RedlineModel::_OnLoadedJsonProperties()
    {
    T_Super::_OnLoadedJsonProperties();
    m_imageDef.FromJson(GetJsonProperties(json_imageDef()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void RedlineModel::StoreImage(DgnTextureCR texture, DPoint2dCR origin, DVec2dCR size)
    {
    BeAssert(texture.GetTextureId().IsValid());
    m_imageDef = ImageDef(texture.GetTextureId(), origin, size);
    Update();
    }

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
SpatialRedlineViewControllerPtr SpatialRedlineViewController::InsertView(SpatialRedlineModel& model, OrthographicViewController& subjectView)
    {
    DgnViews::View view(DgnViewType::Physical, GetViewSubType(), model.GetModelId(),model.GetModelName());

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
    subjectView._StoreToDefinition(json);
    controller->_LoadFromDefinition(json);
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
* @bsimethod                                    Shaun.Sewall                    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DocumentListModelPtr DgnMarkupProject::GetRedlineListModel()
    {
    DgnCode partitionCode = DocumentPartition::CreateCode(*Elements().GetRootSubject(), "Redlines");
    DgnModelId modelId = Models().QuerySubModelId(partitionCode);

    if (modelId.IsValid())
        return Models().Get<DocumentListModel>(modelId);

    DocumentPartitionCPtr partition = DocumentPartition::CreateAndInsert(*Elements().GetRootSubject(), partitionCode.GetValueUtf8CP());
    return partition.IsValid() ? DocumentListModel::CreateAndInsert(*partition) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ViewControllerPtr RedlineViewDefinition::_SupplyController() const
    {
    return new RedlineViewController(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
Render::MaterialP RedlineViewController::LoadBackgroundMaterial(ViewContextR context)
    {
    // Create the material proactively so we don't repeat all this work every frame if texture cannot be created...
    if (m_backgroundMaterial.IsNull())
        {
        Render::Material::CreateParams matParams;
        matParams.SetDiffuseColor(ColorDef::White());
        matParams.SetShadows(false);
        matParams.SetAmbient(1.0);
        matParams.SetDiffuse(0.0);

        auto def = GetImageDef();
        auto texture = context.GetRenderSystem()->_GetTexture(def.m_textureId, GetDgnDb());
        if (texture.IsValid())
            {
            Render::TextureMapping::Params mapParams;
            Render::TextureMapping::Trans2x3 transform(0.0, 1.0, 0.0, 1.0, 0.0, 0.0);
            mapParams.SetTransform(&transform);
            matParams.MapTexture(*texture, mapParams);
            }

        m_backgroundMaterial = context.GetRenderSystem()->_CreateMaterial(matParams, GetDgnDb());
        }

    return m_backgroundMaterial->HasTextureMapping() ? m_backgroundMaterial.get() : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/17
+---------------+---------------+---------------+---------------+---------------+------*/
RedlineModel::ImageDef RedlineViewController::GetImageDef() const
    {
    RedlineModel::ImageDef def;
    auto model = static_cast<RedlineModelP>(GetViewedModel());
    if (nullptr != model)
        def = model->GetImageDef();

    return def;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
RedlinePtr Redline::Create(DgnDbStatus* outCreateStatus, DocumentListModelCR model, Utf8StringCR name)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(createStatus, outCreateStatus);
    DgnCode code = CreateCode(model, name);
    if (!code.IsValid())
        {
        BeAssert(false && "A code is required");
        createStatus = DgnDbStatus::InvalidName;
        return nullptr;
        }
    Redline::CreateParams params(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), code);
    return new Redline(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
RedlineModelPtr RedlineModel::Create(DgnDbStatus* outCreateStatus, Redline& doc)
    {
    // DgnDbStatus ALLOW_NULL_OUTPUT(createStatus, outCreateStatus);
    RedlineModel::CreateParams params(doc.GetDgnDb(), QueryClassId(doc.GetDgnDb()), doc.GetElementId());
    return new RedlineModel(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_ElementHandler::MarkupExternalLinkHandler::_RegisterPropertyAccessors(ECSqlClassInfo& params, ECN::ClassLayoutCR layout)
    {
    T_Super::_RegisterPropertyAccessors(params, layout);

    params.RegisterPropertyAccessors(layout, MARKUPEXTERNALLINK_LinkedElementId, 
        [](ECValueR value, DgnElementCR elIn)
            {
            auto const& el = (MarkupExternalLink const&)elIn;
            value.SetNavigationInfo(el.GetLinkedElementId());
            return DgnDbStatus::Success;
            },

        [](DgnElementR elIn, ECValueCR value)
            {
            if (value.IsNull())
                {
                BeAssert(false);
                return DgnDbStatus::BadArg;
                }
            auto& el = (MarkupExternalLink&)elIn;
            el.SetLinkedElementId(value.GetNavigationInfo().GetId<DgnCategoryId>());
            return DgnDbStatus::Success;
            });
    }
