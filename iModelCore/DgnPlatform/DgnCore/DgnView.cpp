/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnView.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#define PROPNAME_Descr "Descr"
#define PROPNAME_Source "Source"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

namespace dgn_ElementHandler
{
    HANDLER_DEFINE_MEMBERS(DrawingViewDef);
    HANDLER_DEFINE_MEMBERS(SheetViewDef);
    HANDLER_DEFINE_MEMBERS(OrthographicViewDef);
    HANDLER_DEFINE_MEMBERS(CameraViewDef);
    HANDLER_DEFINE_MEMBERS(ModelSelectorDef);
    HANDLER_DEFINE_MEMBERS(CategorySelectorDef);
    HANDLER_DEFINE_MEMBERS(DisplayStyleDef);
    HANDLER_DEFINE_MEMBERS(DisplayStyle3dDef);
}

END_BENTLEY_DGNPLATFORM_NAMESPACE

namespace ViewProperties
{
    static Utf8CP str_ModelSelector() {return "ModelSelector";}
    static Utf8CP str_CategorySelector() {return "CategorySelector";}
    static Utf8CP str_DisplayStyle() {return "DisplayStyle";}
    static Utf8CP str_BackgroundColor(){return "backgroundColor";}
    static Utf8CP str_ViewFlags() {return "viewflags";}
    static Utf8CP str_Styles() {return "Styles";}
    static Utf8CP str_Details() {return "Details";}
    static Utf8CP str_SubCategory() {return "SubCategory";}
    static Utf8CP str_SubCategoryOverrides() {return "SubCategoryOvr";}
    static Utf8CP str_LensAngle() {return "LensAngle";}
    static Utf8CP str_FocusDistance() {return "FocusDistance";}
    static Utf8CP str_EyePoint() {return "EyePoint";}
    static Utf8CP str_BaseModel() {return "BaseModel";}
    static Utf8CP str_Origin() {return "Origin";}
    static Utf8CP str_Extents() {return "Extents";}
    static Utf8CP str_RotationAngle() {return "RotationAngle";}
    static Utf8CP str_Yaw() {return "Yaw";}
    static Utf8CP str_Pitch() {return "Pitch";}           
    static Utf8CP str_Roll() {return "Roll";}           
    static Utf8CP str_AspectSkew() {return "AspectSkew";}           
    static Utf8CP str_Width() {return "width";}           
    static Utf8CP str_Height() {return "height";}           
    static Utf8CP str_Format() {return "format";}           
    static Utf8CP str_Jpeg() {return "jpeg";}           
    static Utf8CP str_Png() {return "png";}           
};

using namespace ViewProperties;

HANDLER_EXTENSION_DEFINE_MEMBERS(ViewControllerOverride)

BEGIN_UNNAMED_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
template<class T> static RefCountedPtr<T> getAndCopy(DgnDbR db, DgnElementId id)
    {
    BeAssert(id.IsValid());
    auto el = db.Elements().Get<T>(id);
    if (!el.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    return el->template MakeCopy<T>();
    }
END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnViewId ViewDefinition::QueryViewId(DgnCode const& code, DgnDbR db)
    {
    DgnElementId elemId = db.Elements().QueryElementIdByCode(code);
    return DgnViewId(elemId.GetValueUnchecked());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewControllerPtr ViewDefinition::LoadViewController(DgnViewId viewId, DgnDbR db)
    {
    auto view = QueryView(viewId, db);
    return view.IsValid() ? view->LoadViewController() : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewControllerPtr ViewDefinition::LoadViewController(bool allowOverrides) const
    {
    ViewControllerOverride* ovr = allowOverrides ? ViewControllerOverride::Cast(GetElementHandler()) : nullptr;
    ViewControllerPtr controller = ovr ? ovr->_SupplyController(*this) : nullptr;

    if (controller.IsNull())
        controller = _SupplyController();

    if (!controller.IsValid())
        return nullptr;

    controller->LoadState();
    return controller;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewDefinition::_EqualState(ViewDefinitionR other) 
    {
    SaveState();
    other.SaveState();

    if (!GetCategorySelector().EqualState(other.GetCategorySelector()))
        return false;

    if (!GetDisplayStyle().EqualState(other.GetDisplayStyle()))
        return false;

    return m_details == other.m_details;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition::SaveState() const
    {
    if (m_displayStyle.IsValid() && !m_displayStyle->GetElementId().IsValid())
        {
        m_displayStyle->Insert();
        m_dirty = true;
        }

    if (m_categorySelector.IsValid() && !m_categorySelector->GetElementId().IsValid())
        {
        m_categorySelector->Insert();
        m_dirty = true;
        }

    if (!m_dirty)
        return;

    ViewDefinitionR ncThis = const_cast<ViewDefinitionR>(*this);
    if (m_displayStyle.IsValid())
        ncThis.SetPropertyValue(str_DisplayStyle(), m_displayStyle->GetElementId());
    if (m_categorySelector.IsValid())
        ncThis.SetPropertyValue(str_CategorySelector(), m_categorySelector->GetElementId());

    ncThis.SetPropertyValue(str_Details(), Json::FastWriter::ToString(m_details).c_str());
    m_dirty=false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId ViewDefinition::GetCategorySelectorId() const
    {
    return GetPropertyValueId<DgnElementId>(str_CategorySelector());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
CategorySelectorR ViewDefinition::GetCategorySelector()
    {
    if (!m_categorySelector.IsValid())
        m_categorySelector = getAndCopy<CategorySelector>(m_dgndb, GetCategorySelectorId());

    return *m_categorySelector;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId ViewDefinition::GetDisplayStyleId() const
    {
    return GetPropertyValueId<DgnElementId>(str_DisplayStyle());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayStyleR ViewDefinition::GetDisplayStyle()
    {
    if (!m_displayStyle.IsValid())
        m_displayStyle = getAndCopy<DisplayStyle>(m_dgndb, GetDisplayStyleId());

    return *m_displayStyle;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewDefinition::_LoadFromDb() 
    {
    auto stat = T_Super::_LoadFromDb();
    if (DgnDbStatus::Success != stat)
        return stat;

    Json::Reader::Parse(GetPropertyValueString(str_Details()), m_details);
    m_dirty = false;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition::_CopyFrom(DgnElementCR el) 
    {
    auto& other = static_cast<ViewDefinitionCR>(el);
    other.SaveState();

    T_Super::_CopyFrom(el);

    m_categorySelector = other.m_categorySelector.IsValid() ? other.m_categorySelector->MakeCopy<CategorySelector>() : nullptr;
    m_displayStyle = other.m_displayStyle.IsValid() ? other.m_displayStyle->MakeCopy<DisplayStyle>() : nullptr;
    m_details = other.m_details;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
ViewControllerPtr OrthographicViewDefinition::_SupplyController() const
    {
    return new OrthographicViewController(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewControllerPtr CameraViewDefinition::_SupplyController() const
    {
    return new CameraViewController(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewControllerPtr SheetViewDefinition::_SupplyController() const
    {
    return new SheetViewController(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewControllerPtr DrawingViewDefinition::_SupplyController() const
    {
    return new DrawingViewController(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
double DrawingViewDefinition::GetAspectRatioSkew() const
    {
    auto& val = GetDetail(str_AspectSkew());
    return val.isNull() ? 1.0 : val.asDouble();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnDbStatus lockElement(DgnElementCR ele)
    {
    LockRequest lockReq;
    lockReq.Insert(ele, LockLevel::Exclusive);
    return (RepositoryStatus::Success == ele.GetDgnDb().BriefcaseManager().AcquireLocks(lockReq).Result())? DgnDbStatus::Success: DgnDbStatus::LockNotHeld;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewDefinition2d::OnModelDelete(DgnDbR db, DgnModelId mid)
    {
    auto findViewsStmt = db.GetPreparedECSqlStatement("SELECT ECInstanceId FROM " BIS_SCHEMA("ViewDefinition2d") " WHERE BaseModel=?");
    if (!findViewsStmt.IsValid())
        {
        BeAssert(false);
        return DgnDbStatus::BadRequest;
        }
    findViewsStmt->BindId(1, mid);
    while (BE_SQLITE_ROW == findViewsStmt->Step())
        {
        auto viewId = findViewsStmt->GetValueId<DgnViewId>(0);
        auto view = QueryView(viewId, db);
        if (view.IsValid())
            {
            lockElement(*view);
            view->Delete();
            }
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition2d::_RemapIds(DgnImportContext& importer)
    {
    T_Super::_RemapIds(importer);
    if (importer.IsBetweenDbs())
        {
        // We're not going to deep-copy the model in. We're expecting the user already copied it.
        m_baseModelId  = importer.FindModelId(GetBaseModelId());

        // NOTE: We're not copying or remapping the settings from the db's properties table...
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition2d::SaveViewDef2d()
    {
    SetPropertyValue(str_BaseModel(), m_baseModelId);
    SetPropertyValue(str_Origin(), m_origin);
    SetPropertyValue(str_Extents(), m_delta);
    SetPropertyValue(str_RotationAngle(), Angle::FromRadians(m_rotAngle).Degrees());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewDefinition2d::_LoadFromDb() 
    {
    auto stat = T_Super::_LoadFromDb();
    if (DgnDbStatus::Success != stat)
        return stat;

    m_baseModelId = GetPropertyValueId<DgnModelId>(str_BaseModel());
    m_origin = GetPropertyValueDPoint2d(str_Origin());
    m_delta = DVec2d::From(GetPropertyValueDPoint2d(str_Extents()));
    m_rotAngle = Angle::FromDegrees(GetPropertyValueDouble(str_RotationAngle())).Radians();

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition2d::_CopyFrom(DgnElementCR el) 
    {
    T_Super::_CopyFrom(el);
    auto const& other = (ViewDefinition2d const&) el;
    m_baseModelId = other.m_baseModelId;
    m_origin = other.m_origin;
    m_delta = other.m_delta;
    m_rotAngle = other.m_rotAngle;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewDefinition2d::_EqualState(ViewDefinitionR in) 
    {
    auto const& other= (ViewDefinition2d const&) in;
    if (m_baseModelId != other.m_baseModelId || !m_origin.IsEqual(other.m_origin) || !m_delta.IsEqual(other.m_delta) || m_rotAngle != other.m_rotAngle)
        return false;

    return T_Super::_EqualState(in);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
ViewDefinition::Iterator::Iterator(DgnDbR db, Options const& options)
    {
    static const Utf8CP s_ecsql = "SELECT ECInstanceId,[CodeValue],[" PROPNAME_Source "]," PROPNAME_Descr ",GetECClassId() FROM " BIS_SCHEMA("ViewDefinition");

    Utf8CP ecsql = s_ecsql;
    Utf8String customECSql;
    if (!options.IsEmpty())
        {
        customECSql = s_ecsql;
        customECSql.append(options.ToString());

        ecsql = customECSql.c_str();
        }

    Prepare(db, ecsql, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
static void appendSourceClause(Utf8StringR str, ViewDefinition::Iterator::Options::Source source)
    {
#ifdef ECSQL_SUPPORTS_BITWISE_OPS // It doesn't. NEEDSWORK.
    Utf8Char buf[0x20];
    BeStringUtilities::FormatUInt64(buf, static_cast<uint64_t>(m_source));

    str.append("0 != (" PROPNAME_Source " & ");
    str.append(buf);
    str.append(1, ')');
#else
    // ECSql doesn't support bitwise operators...
    typedef ViewDefinition::Iterator::Options::Source Source;
    bool user {0 != static_cast<uint8_t>(source & Source::User) };
    bool gen  {0 != static_cast<uint8_t>(source & Source::Generated) };
    bool priv {0 != static_cast<uint8_t>(source & Source::Private) };

    if (!user && !gen && !priv)
        return;

    // Parens in case we're preceded by an AND...
    str.append(1, '(');

    Utf8Char buf[3];
    if (user)
        {
        BeStringUtilities::FormatUInt64(buf, static_cast<uint64_t>(Source::User));
        str.append(PROPNAME_Source "=").append(buf);
        }

    if (gen)
        {
        if (user)
            str.append(" OR ");

        BeStringUtilities::FormatUInt64(buf, static_cast<uint64_t>(Source::Generated));
        str.append(PROPNAME_Source "=").append(buf);
        }

    if (priv)
        {
        if (user || gen)
            str.append(" OR ");

        BeStringUtilities::FormatUInt64(buf, static_cast<uint64_t>(Source::Private));
        str.append(PROPNAME_Source "=").append(buf);
        }
    
    str.append(1, ')');
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ViewDefinition::Iterator::Options::ToString() const
    {
    Utf8String str;
    if (IsEmpty())
        return str;

    if (!m_customECSql.empty())
        {
        // mutually exclusive with the other options
        if (!m_customECSql.StartsWith(" "))
            str.append(1, ' ');

        str.append(m_customECSql);
        return str;
        }

    // WHERE
    if (Source::All != m_source)
        {
        str.append(str.empty() ? " WHERE " : " AND ");
        appendSourceClause(str, m_source);
        }

    // ORDER BY
    if (Order::Ascending == m_order)
        {
        str.append(" ORDER BY [CodeValue]");
        }

    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ViewDefinition::QueryCount(DgnDbR db, Iterator::Options const& opts)
    {
    static const Utf8CP s_ecsql = "SELECT count(*) FROM " BIS_SCHEMA("ViewDefinition");
    
    Utf8CP ecsql = s_ecsql;
    Utf8String customECSql;
    if (!opts.IsEmpty())
        {
        customECSql = s_ecsql;
        customECSql.append(opts.ToString());
        ecsql = customECSql.c_str();
        }

    CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement(ecsql);
    return stmt.IsValid() && BE_SQLITE_ROW == stmt->Step() ? stmt->GetValueInt(0) : 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T_Desired> static bool isEntryOfClass(ViewDefinition::Entry const& entry)
    {
    DgnDbP db = entry.GetDgnDb();
    if (nullptr == db)
        return false;

    auto entryClass = db->Schemas().GetECClass(entry.GetClassId());
    auto desiredClass = db->Schemas().GetECClass(T_Desired::QueryClassId(*db));
    return nullptr != entryClass && nullptr != desiredClass && entryClass->Is(desiredClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewDefinition::Entry::IsOrthographicView() const {return isEntryOfClass<OrthographicViewDefinition>(*this);}
bool ViewDefinition::Entry::IsCameraView() const {return isEntryOfClass<CameraViewDefinition>(*this);}
bool ViewDefinition::Entry::IsSpatialView() const {return isEntryOfClass<SpatialViewDefinition>(*this);}
bool ViewDefinition::Entry::IsDrawingView() const {return isEntryOfClass<DrawingViewDefinition>(*this);}
bool ViewDefinition::Entry::IsSheetView() const {return isEntryOfClass<SheetViewDefinition>(*this);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void CategorySelector::_CopyFrom(DgnElementCR in) 
    {
    T_Super::_CopyFrom(in);
    auto const& other = (CategorySelector const&) in;
    m_categories = other.m_categories; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CategorySelector::_InsertInDb()
    {
    auto status = T_Super::_InsertInDb();
    if (DgnDbStatus::Success != status)
        return status;

    return WriteCategories();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool CategorySelector::EqualState(CategorySelectorCR other) const 
    {
    return m_categories == other.m_categories;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CategorySelector::_OnUpdate(DgnElementCR el)
    {
    auto status = T_Super::_OnUpdate(el);
    if (DgnDbStatus::Success != status)
        return status;
    
    auto delExisting = GetDgnDb().GetPreparedECSqlStatement("DELETE FROM " BIS_SCHEMA("CategorySelectorRefersToCategories") " WHERE (SourceECInstanceId=?)");
    delExisting->BindId(1, GetElementId());
    delExisting->Step();

    return WriteCategories();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CategorySelector::WriteCategories() 
    {
    if (!GetElementId().IsValid())
        {
        BeAssert(false);
        return DgnDbStatus::MissingId;
        }

    auto statement = GetDgnDb().GetPreparedECSqlStatement("INSERT INTO " BIS_SCHEMA("CategorySelectorRefersToCategories") " (SourceECInstanceId,TargetECInstanceId) VALUES (?,?)");
    if (!statement.IsValid())
        return DgnDbStatus::WriteError;

    for (auto id : m_categories)
        {
        statement->Reset();
        statement->ClearBindings();
        statement->BindId(1, GetElementId());
        statement->BindId(2, id);
        if (BE_SQLITE_DONE != statement->Step())
            return DgnDbStatus::WriteError;
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CategorySelector::_LoadFromDb() 
    {
    auto stat = T_Super::_LoadFromDb();
    if (stat  != DgnDbStatus::Success)
        return stat;

    auto statement = GetDgnDb().GetPreparedECSqlStatement("SELECT TargetECInstanceId FROM " BIS_SCHEMA("CategorySelectorRefersToCategories") " WHERE SourceECInstanceId=?");
    if (!statement.IsValid())
        return DgnDbStatus::BadSchema;

    statement->BindId(1, GetElementId());

    while (BE_SQLITE_ROW == statement->Step())
        m_categories.insert(statement->GetValueId<DgnCategoryId>(0));

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Bill.Steinbock                  10/2016
//---------------------------------------------------------------------------------------
DgnElementIdSet CategorySelector::QuerySelectors(DgnDbR db)
    {
    DgnElementIdSet ids;

    CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement("SELECT ECInstanceId FROM " BIS_SCHEMA(BIS_CLASS_CategorySelector));
    if (stmt.IsValid())
        {
        while (BE_SQLITE_ROW == stmt->Step())
            ids.insert(stmt->GetValueId<DgnElementId>(0));
        }

    return ids;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ModelSelector::OnModelDelete(DgnDbR db, DgnModelId mid)
    {
    // Detect all ModelSelectors that include this model
    auto statement = db.GetPreparedECSqlStatement("SELECT SourceECInstanceId FROM " BIS_SCHEMA(BIS_REL_ModelSelectorRefersToModels) " WHERE TargetECInstanceId=?");
    if (!statement.IsValid())
        {
        BeAssert(false);
        return DgnDbStatus::BadRequest;
        }
    statement->BindId(1, mid);
    while (BE_SQLITE_ROW == statement->Step())
        {
        auto selId = statement->GetValueId<DgnElementId>(0);
        auto selector = db.Elements().Get<ModelSelector>(selId);
        if (!selector.IsValid())
            continue;

        //  If the this ModelSelector contains *ONLY* this model, it is about to become empty. Delete it.
        if (selector->GetModels().size() == 1)
            {
            lockElement(*selector);
            selector->Delete();
            }
        }
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ModelSelector::_OnDelete() const
    {
    // Delete all 3d views that are based on on this selector
    auto statement = GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId FROM " BIS_SCHEMA("SpatialViewDefinition") " WHERE ModelSelector=?");
    statement->BindId(1, GetElementId());
    if (BE_SQLITE_ROW == statement->Step())
        {
        auto view = ViewDefinition::QueryView(statement->GetValueId<DgnViewId>(0), GetDgnDb());
        if (view.IsValid())
            view->Delete();
        }
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ModelSelector::_CopyFrom(DgnElementCR rhsElement) 
    {
    T_Super::_CopyFrom(rhsElement);

    auto const& rhs = (ModelSelector const&)rhsElement;
    m_models = rhs.m_models; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ModelSelector::_InsertInDb()
    {
    auto status = T_Super::_InsertInDb();
    if (DgnDbStatus::Success != status)
        return status;

    return WriteModels();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ModelSelector::_OnUpdate(DgnElementCR el)
    {
    auto status = T_Super::_OnUpdate(el);
    if (DgnDbStatus::Success != status)
        return status;
    
    auto delExisting = GetDgnDb().GetPreparedECSqlStatement("DELETE FROM " BIS_SCHEMA(BIS_REL_ModelSelectorRefersToModels) " WHERE (SourceECInstanceId=?)");
    delExisting->BindId(1, GetElementId());
    delExisting->Step();

    return WriteModels();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ModelSelector::WriteModels() 
    {
    if (!GetElementId().IsValid())
        {
        BeAssert(false);
        return DgnDbStatus::MissingId;
        }

    auto statement = GetDgnDb().GetPreparedECSqlStatement("INSERT INTO " BIS_SCHEMA(BIS_REL_ModelSelectorRefersToModels) " (SourceECInstanceId,TargetECInstanceId) VALUES (?,?)");
    if (!statement.IsValid())
        return DgnDbStatus::WriteError;

    for (auto id : m_models)
        {
        statement->Reset();
        statement->ClearBindings();
        statement->BindId(1, GetElementId());
        statement->BindId(2, id);
        if (BE_SQLITE_DONE != statement->Step())
            return DgnDbStatus::WriteError;
        }
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ModelSelector::_LoadFromDb() 
    {
    auto stat = T_Super::_LoadFromDb();
    if (DgnDbStatus::Success != stat)
        return stat;

    auto statement = GetDgnDb().GetPreparedECSqlStatement("SELECT TargetECInstanceId FROM " BIS_SCHEMA(BIS_REL_ModelSelectorRefersToModels) " WHERE SourceECInstanceId=?");
    if (!statement.IsValid())
        {
        BeAssert(false);
        return DgnDbStatus::BadSchema;
        }

    statement->BindId(1, GetElementId());
    while (BE_SQLITE_ROW == statement->Step())
        {
        auto id = statement->GetValueId<DgnModelId>(0);
        m_models.insert(id);
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition3d::SaveViewDef3d()
    {
    SetPropertyValue(str_Origin(), m_origin);
    SetPropertyValue(str_Extents(), m_extents);

    YawPitchRollAngles angles;
    YawPitchRollAngles::TryFromRotMatrix(angles, m_rotation);
    SetPropertyValueYpr(angles, str_Yaw(), str_Pitch(), str_Roll());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewDefinition3d::_EqualState(ViewDefinitionR in) 
    {
    auto& other = (ViewDefinition3dR) in;
    if (!m_origin.IsEqual(other.m_origin) || !m_extents.IsEqual(other.m_extents) || !m_rotation.IsEqual(other.m_rotation))
        return false;

    return T_Super::_EqualState(other);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ViewDefinition3d::_LoadFromDb() 
    {
    auto stat = T_Super::_LoadFromDb();
    if (DgnDbStatus::Success != stat)
        return stat;

    m_origin = GetPropertyValueDPoint3d(str_Origin());
    m_extents = DVec3d::From(GetPropertyValueDPoint3d(str_Extents()));
    m_rotation = GetPropertyValueYpr(str_Yaw(), str_Pitch(), str_Roll()).ToRotMatrix();
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition3d::_CopyFrom(DgnElementCR el)
    {
    T_Super::_CopyFrom(el);

    auto const& other = (ViewDefinition3d const&) el;
    m_origin = other.m_origin;
    m_extents = other.m_extents;
    m_rotation = other.m_rotation;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult ViewDefinition::SaveThumbnail(Point2d size, Render::ImageSourceCR source) const
    {
    Json::Value val;
    val[str_Width()] = size.x;
    val[str_Height()] = size.y;
    val[str_Format()] = (source.GetFormat() == ImageSource::Format::Jpeg) ? str_Jpeg() : str_Png();

    DbResult rc = m_dgndb.SaveProperty(DgnViewProperty::ViewThumbnail(), Json::FastWriter().ToString(val), source.GetByteStream().GetData(), source.GetByteStream().GetSize(), GetViewId().GetValue());
    return rc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ImageSource ViewDefinition::ReadThumbnail() const
    {
    ImageSource image;

    DgnDbR db = GetDgnDb();
    uint32_t bytes;
    auto stat = db.QueryPropertySize(bytes, DgnViewProperty::ViewThumbnail(), GetViewId().GetValue());
    if (BE_SQLITE_ROW != stat)
        return image;

    Utf8String jsonStr;
    stat = db.QueryProperty(jsonStr, DgnViewProperty::ViewThumbnail(), GetViewId().GetValue());
    if (BE_SQLITE_ROW != stat)
        return image;

    Json::Value value;
    Json::Reader::Parse(jsonStr, value);
    image.SetFormat(value[str_Format()] == str_Jpeg() ? ImageSource::Format::Jpeg : ImageSource::Format::Png);
    image.GetByteStreamR().Resize(bytes);
    stat = db.QueryProperty(image.GetByteStreamR().GetDataP(), bytes, DgnViewProperty::ViewThumbnail(), GetViewId().GetValue());
    BeAssert(BE_SQLITE_ROW == stat);
    return image;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
Point2d ViewDefinition::GetThumbnailSize() const
    {
    Point2d size = {0,0};

    DgnDbR db = GetDgnDb();
    Utf8String jsonStr;
    auto stat = db.QueryProperty(jsonStr, DgnViewProperty::ViewThumbnail(), GetViewId().GetValue());
    if (BE_SQLITE_ROW != stat)
        return size;

    Json::Value value;
    Json::Reader::Parse(jsonStr, value);
    size.x = value[str_Width()].asInt();
    size.y = value[str_Height()].asInt();
    return size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ViewDefinition::DeleteThumbnail() const
    {
    GetDgnDb().DeleteProperty(DgnViewProperty::ViewThumbnail(), GetViewId().GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialViewDefinition::UpdateModelSelectorId()
    {
    if (!m_modelSelector.IsValid())
        return;

    if (!m_modelSelector->GetElementId().IsValid())
        m_modelSelector->Insert();

    SetPropertyValue(str_ModelSelector(), m_modelSelector->GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId SpatialViewDefinition::GetModelSelectorId() const
    {
    return GetPropertyValueId<DgnElementId>(str_ModelSelector());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ModelSelectorR SpatialViewDefinition::GetModelSelector()
    {
    if (!m_modelSelector.IsValid())
        m_modelSelector = getAndCopy<ModelSelector>(m_dgndb, GetModelSelectorId());

    return *m_modelSelector;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void SpatialViewDefinition::_CopyFrom(DgnElementCR el) 
    {
    T_Super::_CopyFrom(el);
    auto other = static_cast<SpatialViewDefinitionCP>(&el);

    m_modelSelector = other->m_modelSelector.IsValid() ? other->m_modelSelector->MakeCopy<ModelSelector>() : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool SpatialViewDefinition::_EqualState(ViewDefinitionR in) 
    {
    if (!T_Super::_EqualState(in))
        return false;

    auto& other = (SpatialViewDefinition&) in;
    return GetModelSelector().EqualState(other.GetModelSelector());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void CameraViewDefinition::SaveCamera()
    {
    SetPropertyValue(str_EyePoint(), GetEyePoint());
    SetPropertyValue(str_LensAngle(), GetLensAngle());
    SetPropertyValue(str_FocusDistance(), GetFocusDistance());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void CameraViewDefinition::_CopyFrom(DgnElementCR el)
    {
    T_Super::_CopyFrom(el);
    auto other = static_cast<CameraViewDefinitionCP>(&el);
    m_camera = other->m_camera;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CameraViewDefinition::_LoadFromDb() 
    {
    auto stat = T_Super::_LoadFromDb();
    if (DgnDbStatus::Success != stat)
        return stat;

    m_camera.SetLensAngle(GetPropertyValueDouble(str_LensAngle()));
    m_camera.SetFocusDistance(GetPropertyValueDouble(str_FocusDistance()));
    m_camera.SetEyePoint(GetPropertyValueDPoint3d(str_EyePoint()));
    m_camera.ValidateLens();
    
    VerifyFocusPlane();
    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool CameraViewDefinition::_EqualState(ViewDefinitionR in) 
    {
    if (!T_Super::_EqualState(in))
        return false;

    auto& other = (CameraViewDefinition&) in;
    return m_camera.IsEqual(other.m_camera);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ViewDefinition::SetStandardViewRotation(StandardView standardView)
    {
    RotMatrix rMatrix;
    if (!bsiRotMatrix_getStandardRotation(&rMatrix, static_cast<int>(standardView)))
        return  ERROR;

    SetRotation(rMatrix);
    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* load a subcategory appearance into its unmodified state
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategory::Appearance DisplayStyle::LoadSubCategory(DgnSubCategoryId id) const
    {
    auto unmodified = DgnSubCategory::QuerySubCategory(id, GetDgnDb());
    BeAssert(unmodified.IsValid());
    if (!unmodified.IsValid())
        return DgnSubCategory::Appearance();

    BeMutexHolder _v(m_mutex);
    auto const& result = m_subCategories.Insert(id, unmodified->GetAppearance());
    if (!result.second)
        result.first->second = unmodified->GetAppearance(); // we already had this SubCategory; change it to unmodified state

    return unmodified->GetAppearance();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle::OverrideSubCategory(DgnSubCategoryId id, DgnSubCategory::Override const& ovr)
    {
    if (!id.IsValid())
        return;

    m_dirty = true;
    auto result = m_subCategoryOverrides.Insert(id, ovr);
    if (!result.second)
        result.first->second = ovr; // we already had this override; change it.

    LoadSubCategory(id); // To ensure none of the previous overrides are still active, we reload the original SubCategory

    // now apply this override to the unmodified SubCategory appearance
    auto it = m_subCategories.find(id);
    if (it != m_subCategories.end())
        ovr.ApplyTo(it->second);
    else
        BeAssert(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategory::Appearance DisplayStyle::GetSubCategoryAppearance(DgnSubCategoryId subCategoryId) const
    {
    if (true)
        {
        BeMutexHolder _v(m_mutex);
        auto const entry = m_subCategories.find(subCategoryId);
        if (entry != m_subCategories.end())
            return entry->second;
        }

    return LoadSubCategory(subCategoryId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle::DropSubCategoryOverride(DgnSubCategoryId id)
    {
    m_dirty = true;
    m_subCategoryOverrides.erase(id);
    LoadSubCategory(id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DisplayStyle::_LoadFromDb() 
    {
    auto stat = T_Super::_LoadFromDb();
    if (DgnDbStatus::Success != stat)
        return stat;

    Json::Reader::Parse(GetPropertyValueString(str_Styles()), m_styles);
    m_viewFlags.FromJson(GetStyle(str_ViewFlags()));

    JsonValueCR overrides = GetStyle(str_SubCategoryOverrides());
    for (Json::ArrayIndex i = 0; i<overrides.size(); ++i)
        {
        JsonValueCR val = overrides[i];
        DgnSubCategoryId subCategoryId(val[str_SubCategory()].asUInt64());
        if (!subCategoryId.IsValid())
            {
            BeDataAssert(false && "SubCategoryOverride refers to missing SubCategory");
            continue;
            }
        OverrideSubCategory(subCategoryId, DgnSubCategory::Override(val));
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle::_CopyFrom(DgnElementCR el) 
    {
    auto& other = static_cast<DisplayStyleCR>(el);
    other.SaveStyles();
    T_Super::_CopyFrom(el);

    m_styles = other.m_styles;
    m_viewFlags = other.m_viewFlags;
    m_subCategories = other.m_subCategories;
    m_subCategoryOverrides = other.m_subCategoryOverrides;
    m_dirty = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle::SaveStyles() const
    {
    if (!m_dirty)
        return;

    auto& ncThis = const_cast<DisplayStyleR>(*this);
    ncThis.SetStyle(str_ViewFlags(), m_viewFlags.ToJson());

    if (m_subCategoryOverrides.empty())
        {
        ncThis.RemoveStyle(str_SubCategoryOverrides());
        }
    else
        {
        Json::Value ovrJson;
        int i=0;
        for (auto const& it : m_subCategoryOverrides)
            {
            ovrJson[i][str_SubCategory()] = it.first.GetValue();
            it.second.ToJson(ovrJson[i++]);
            }
        ncThis.SetStyle(str_SubCategoryOverrides(), ovrJson);
        }

    if (ColorDef::Black() == GetBackgroundColor())
        ncThis.RemoveStyle(str_BackgroundColor());

    ncThis.SetPropertyValue(str_Styles(), Json::FastWriter::ToString(m_styles).c_str());
    m_dirty = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ColorDef DisplayStyle::GetBackgroundColor() const
    {
    JsonValueCR val = GetStyle(str_BackgroundColor());
    return val.isNull() ? ColorDef::Black() : ColorDef(val.asUInt());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DisplayStyle::SetBackgroundColor(ColorDef val) 
    {
    SetStyle(str_BackgroundColor(), Json::Value(val.GetValue()));
    }

OrthographicViewControllerPtr OrthographicViewDefinition::LoadViewController(bool o) const {auto vc = T_Super::LoadViewController(o); return vc.IsValid() ? vc->ToOrthographicViewP() : nullptr;}
CameraViewControllerPtr CameraViewDefinition::LoadViewController(bool o) const {auto vc = T_Super::LoadViewController(o); return vc.IsValid() ? vc->ToCameraViewP() : nullptr;}
DrawingViewControllerPtr DrawingViewDefinition::LoadViewController(bool o) const {auto vc = T_Super::LoadViewController(o); return vc.IsValid() ? vc->ToDrawingViewP() : nullptr;}
SheetViewControllerPtr SheetViewDefinition::LoadViewController(bool o) const {auto vc = T_Super::LoadViewController(o); return vc.IsValid() ? vc->ToSheetViewP() : nullptr;}
