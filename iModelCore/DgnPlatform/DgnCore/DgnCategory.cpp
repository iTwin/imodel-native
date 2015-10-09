/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnCategory.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#define CAT_PROP_Descr      "Descr"
#define CAT_PROP_Scope      "Scope"
#define CAT_PROP_Rank       "Rank"
#define SUBCAT_PROP_Descr   "Descr"
#define SUBCAT_PROP_Props   "Props"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

namespace dgn_ElementHandler
{
HANDLER_DEFINE_MEMBERS(Category);
HANDLER_DEFINE_MEMBERS(SubCategory);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Category::_GetClassParams(ECSqlClassParams& params)
    {
    T_Super::_GetClassParams(params);
    params.Add(CAT_PROP_Descr);
    params.Add(CAT_PROP_Scope);
    params.Add(CAT_PROP_Rank);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void SubCategory::_GetClassParams(ECSqlClassParams& params)
    {
    T_Super::_GetClassParams(params);
    params.Add(SUBCAT_PROP_Descr);
    params.Add(SUBCAT_PROP_Props);
    }
}

END_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnCategory::_BindInsertParams(BeSQLite::EC::ECSqlStatement& stmt)
    {
    auto status = T_Super::_BindInsertParams(stmt);
    return DgnDbStatus::Success == status ? BindParams(stmt) : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnCategory::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& stmt)
    {
    auto status = T_Super::_BindUpdateParams(stmt);
    return DgnDbStatus::Success == status ? BindParams(stmt) : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnCategory::_ExtractSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ExtractSelectParams(stmt, params);
    if (DgnDbStatus::Success == status)
        {
        m_data.Init(
            static_cast<Scope>(stmt.GetValueInt(params.GetSelectIndex(CAT_PROP_Scope))),
            static_cast<Rank>(stmt.GetValueInt(params.GetSelectIndex(CAT_PROP_Rank))),
            stmt.GetValueText(params.GetSelectIndex(CAT_PROP_Descr))
            );
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnCategory::BindParams(ECSqlStatement& stmt)
    {
    if (ECSqlStatus::Success != stmt.BindText(stmt.GetParameterIndex(CAT_PROP_Descr), m_data.m_descr.c_str(), IECSqlBinder::MakeCopy::No)
        || ECSqlStatus::Success != stmt.BindInt(stmt.GetParameterIndex(CAT_PROP_Scope), static_cast<int32_t>(m_data.m_scope))
        || ECSqlStatus::Success != stmt.BindInt(stmt.GetParameterIndex(CAT_PROP_Rank), static_cast<int32_t>(m_data.m_rank)))
        return DgnDbStatus::BadArg;
    else
        return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnCategory::_CopyFrom(DgnElementCR el)
    {
    T_Super::_CopyFrom(el);
    auto other = dynamic_cast<DgnCategoryCP>(&el);
    BeAssert(nullptr != other);
    if (nullptr != other)
        m_data = other->m_data;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategory::CreateParams::CreateParams(DgnDbR db, Utf8StringCR name, Scope scope, Rank rank, Utf8StringCR descr)
    : T_Super(db, QueryDgnClassId(db), CreateCategoryCode(name, db)), m_data(scope, rank, descr)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId DgnCategory::QueryCategoryId(Code const& code, DgnDbR db)
    {
    return DgnCategoryId(db.Elements().QueryElementIdByCode(code).GetValueUnchecked());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnCategory::_OnChildDelete(DgnElementCR child) const
    {
    // Default sub-category cannot be deleted.
    return GetDefaultSubCategoryId() == DgnSubCategoryId(child.GetElementId().GetValueUnchecked()) ? DgnDbStatus::ParentBlockedChange : T_Super::_OnChildDelete(child);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnCategory::_OnChildImport(DgnElementCR child, DgnModelR destModel, DgnImportContext& importer) const
    {
    BeAssert(false && "Import the category before importing subcategories"); // ###TODO?
    return DgnDbStatus::ParentBlockedChange;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnCategory::_OnInserted(DgnElementP copiedFrom) const
    {
    // Create the default sub-category.
    DgnSubCategoryId defaultSubCatId = GetDefaultSubCategoryId();
    BeAssert(defaultSubCatId.IsValid());
    BeAssert(!GetDgnDb().Elements().GetElement(defaultSubCatId).IsValid());

    DgnSubCategory defaultSubCat(DgnSubCategory::CreateParams(GetDgnDb(), GetCategoryId(), GetCategoryName(), DgnSubCategory::Appearance()));

    DgnSubCategoryCPtr persistentSubCat = defaultSubCat.Insert();
    BeAssert(persistentSubCat.IsValid());
    BeAssert(persistentSubCat->GetSubCategoryId() == defaultSubCatId);
    BeAssert(persistentSubCat->GetCategoryId() == GetCategoryId());

    UNUSED_VARIABLE(persistentSubCat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryCPtr DgnCategory::Insert(DgnSubCategory::Appearance const& app, DgnDbStatus* stat)
    {
    auto cat = GetDgnDb().Elements().Insert<DgnCategory>(*this, stat);
    if (cat.IsValid())
        {
        DgnSubCategoryCPtr subCat = DgnSubCategory::QuerySubCategory(GetDefaultSubCategoryId(), GetDgnDb());
        BeAssert(subCat.IsValid());
        if (subCat.IsValid())
            {
            auto updatedSubCat = subCat->MakeCopy<DgnSubCategory>();
            updatedSubCat->GetAppearanceR() = app;
            updatedSubCat->Update();
            }
        }

    return cat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategoryId DgnCategory::GetDefaultSubCategoryId(DgnCategoryId catId)
    {
    // hackity hacky hack - assume sequential assignment of element IDs and that the default sub-category is the next 
    return catId.IsValid() ? DgnSubCategoryId(catId.GetValueUnchecked() + 1) : DgnSubCategoryId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryIdSet DgnCategory::QueryCategories(DgnDbR db)
    {
    DgnCategoryIdSet ids;

    CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement("SELECT ECInstanceId FROM " DGN_SCHEMA(DGN_CLASSNAME_Category));
    if (stmt.IsValid())
        {
        while (BE_SQLITE_ROW == stmt->Step())
            ids.insert(stmt->GetValueId<DgnCategoryId>(0));
        }

    return ids;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DgnCategory::QueryCount(DgnDbR db)
    {
    CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement("SELECT count(*) FROM " DGN_SCHEMA(DGN_CLASSNAME_Category));
    return stmt.IsValid() && BE_SQLITE_ROW == stmt->Step() ? stmt->GetValueInt(0) : 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId DgnCategory::QueryFirstCategoryId(DgnDbR db)
    {
    CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement("SELECT ECInstanceId FROM " DGN_SCHEMA(DGN_CLASSNAME_Category) " LIMIT 1");
    return stmt.IsValid() && BE_SQLITE_ROW == stmt->Step() ? stmt->GetValueId<DgnCategoryId>(0) : DgnCategoryId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId DgnCategory::QueryHighestCategoryId(DgnDbR db)
    {
    CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement("SELECT max(ECInstanceId) FROM " DGN_SCHEMA(DGN_CLASSNAME_Category));
    return stmt.IsValid() && BE_SQLITE_ROW == stmt->Step() ? stmt->GetValueId<DgnCategoryId>(0) : DgnCategoryId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId DgnCategory::QueryElementCategoryId(DgnElementId elemId, DgnDbR db)
    {
    CachedStatementPtr stmt;
    db.GetCachedStatement(stmt, "SELECT CategoryId FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE Id=?");
    stmt->BindId(1, elemId);
    return BE_SQLITE_ROW == stmt->Step() ? stmt->GetValueId<DgnCategoryId>(0) : DgnCategoryId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnCategory::_SetCode(Code const& code)
    {
    return code.GetNameSpace().empty() ? T_Super::_SetCode(code) : DgnDbStatus::BadArg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthority::Code DgnCategory::_GenerateDefaultCode()
    {
    BeAssert(false && "Creator of a category must set its code");
    return Code();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnSubCategory::_BindInsertParams(BeSQLite::EC::ECSqlStatement& stmt)
    {
    auto status = T_Super::_BindInsertParams(stmt);
    return DgnDbStatus::Success == status ? BindParams(stmt) : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnSubCategory::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& stmt)
    {
    auto status = T_Super::_BindUpdateParams(stmt);
    return DgnDbStatus::Success == status ? BindParams(stmt) : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnSubCategory::_ExtractSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ExtractSelectParams(stmt, params);
    if (DgnDbStatus::Success == status)
        {
        m_data.m_descr = stmt.GetValueText(params.GetSelectIndex(SUBCAT_PROP_Descr));
        m_data.m_appearance.FromJson(stmt.GetValueText(params.GetSelectIndex(SUBCAT_PROP_Props)));
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnSubCategory::BindParams(ECSqlStatement& stmt)
    {
    // default sub-categories don't have a description
    if (!IsDefaultSubCategory() && ECSqlStatus::Success != stmt.BindText(stmt.GetParameterIndex(SUBCAT_PROP_Descr), m_data.m_descr.c_str(), IECSqlBinder::MakeCopy::No))
        return DgnDbStatus::BadArg;
    else if (ECSqlStatus::Success != stmt.BindText(stmt.GetParameterIndex(SUBCAT_PROP_Props), m_data.m_appearance.ToJson().c_str(), IECSqlBinder::MakeCopy::Yes))
        return DgnDbStatus::BadArg;
    else
        return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnSubCategory::_SetCode(Code const& code)
    {
    // default sub-category has same name as category
    // all sub-category codes have namespace = category name
    DgnCategoryCPtr cat = DgnCategory::QueryCategory(GetCategoryId(), GetDgnDb());
    if (cat.IsNull() || !code.GetNameSpace().Equals(cat->GetCategoryName()) || (code.GetValue().Equals(cat->GetCategoryName()) != IsDefaultSubCategory()))
        return DgnDbStatus::BadArg;
    else
        return T_Super::_SetCode(code);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnSubCategory::_CopyFrom(DgnElementCR el)
    {
    T_Super::_CopyFrom(el);
    auto other = dynamic_cast<DgnSubCategoryCP>(&el);
    BeAssert(nullptr != other);
    if (nullptr != other)
        m_data = other->m_data;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategory::CreateParams::CreateParams(DgnDbR db, DgnCategoryId catId, Utf8StringCR name, Appearance const& app, Utf8StringCR descr)
    : T_Super(db, QueryDgnClassId(db), CreateSubCategoryCode(catId, name, db), catId), m_data(app, descr)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategoryId DgnSubCategory::QuerySubCategoryId(Code const& code, DgnDbR db)
    {
    return DgnSubCategoryId(db.Elements().QueryElementIdByCode(code).GetValueUnchecked());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnSubCategory::_SetParentId(DgnElementId parentId)
    {
    // a sub-category cannot be re-parented. Its parent must be a category. The parent category is set up when we create + insert the subcategory.
    return DgnDbStatus::InvalidParent;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId DgnSubCategory::QueryCategoryId(DgnSubCategoryId subCatId, DgnDbR db)
    {
    BeAssert(subCatId.IsValid());

    BeSQLite::HighPriorityOperationBlock highPriorityOperationBlock; // See comments on HighPriorityOperationBlock

    CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement("SELECT ParentId FROM " DGN_SCHEMA(DGN_CLASSNAME_SubCategory) " WHERE ECInstanceId=? LIMIT 1");
    if (stmt.IsValid())
        {
        stmt->BindId(1, subCatId);
        if (BE_SQLITE_ROW == stmt->Step())
            return stmt->GetValueId<DgnCategoryId>(0);
        }

    BeAssert(false && "Subcategory has no parent category");
    return DgnCategoryId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategoryIdSet DgnSubCategory::QuerySubCategories(DgnDbR db, DgnCategoryId catId)
    {
    DgnSubCategoryIdSet ids;

    Utf8String ecsql("SELECT ECInstanceId FROM " DGN_SCHEMA(DGN_CLASSNAME_SubCategory));
    if (catId.IsValid())
        ecsql.append(" WHERE [ParentId]=?");

    CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement(ecsql.c_str());
    if (stmt.IsValid())
        {
        if (catId.IsValid())
            stmt->BindId(1, catId);

        while (BE_SQLITE_ROW == stmt->Step())
            ids.insert(stmt->GetValueId<DgnSubCategoryId>(0));
        }

    return ids;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DgnSubCategory::QueryCount(DgnDbR db, DgnCategoryId catId)
    {
    size_t count = 0;
    Utf8String ecsql("SELECT count(*) FROM " DGN_SCHEMA(DGN_CLASSNAME_SubCategory));
    if (catId.IsValid())
        ecsql.append (" WHERE [ParentId]=?");

    CachedECSqlStatementPtr stmt = db.GetPreparedECSqlStatement(ecsql.c_str());
    if (stmt.IsValid())
        {
        if (catId.IsValid())
            stmt->BindId(1, catId);

        if (BE_SQLITE_ROW == stmt->Step())
            count = stmt->GetValueInt(0);
        }

    return count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnSubCategory::_OnInsert()
    {
    // A sub-category requires a parent category.
    DgnCategoryId catId(GetParentId().GetValueUnchecked());
    DgnCategoryCPtr cat = catId.IsValid() ? DgnCategory::QueryCategory(catId, GetDgnDb()) : nullptr;
    return cat.IsValid() ? T_Super::_OnInsert() : DgnDbStatus::InvalidParent;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnSubCategory::IsDefaultSubCategory() const
    {
    return DgnCategory::GetDefaultSubCategoryId(GetCategoryId()) == GetSubCategoryId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthority::Code DgnSubCategory::_GenerateDefaultCode()
    {
    DgnCategoryCPtr cat = IsDefaultSubCategory() ? DgnCategory::QueryCategory(GetCategoryId(), GetDgnDb()) : nullptr;
    if (cat.IsValid())
        return CreateSubCategoryCode(*cat, cat->GetCategoryName(), GetDgnDb());

    BeAssert(false && "The creator of a sub-category must set its code");
    return Code();
    }

static Utf8CP APPEARANCE_Invisible  = "invisible";
static Utf8CP APPEARANCE_Color      = "color";
static Utf8CP APPEARANCE_Weight     = "weight";
static Utf8CP APPEARANCE_Style      = "style";
static Utf8CP APPEARANCE_Priority   = "priority";
static Utf8CP APPEARANCE_Material   = "material";
static Utf8CP APPEARANCE_Transparency = "transp";

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnSubCategory::Appearance::FromJson(Utf8StringCR jsonStr)
    {
    Init();

    Json::Value val(Json::objectValue);
    if (!Json::Reader::Parse(jsonStr, val))
        return;

    m_invisible = val.get(APPEARANCE_Invisible, false).asBool();
    m_color  = ColorDef(val[APPEARANCE_Color].asUInt());
    m_weight = val[APPEARANCE_Weight].asUInt();
    if (val.isMember(APPEARANCE_Style))
        m_style  = DgnStyleId(val[APPEARANCE_Style].asUInt64());
    else
        m_style.Invalidate();
    m_displayPriority = val[APPEARANCE_Priority].asInt();
    m_transparency = val[APPEARANCE_Transparency].asDouble();

    if (val.isMember(APPEARANCE_Material))
        m_material = DgnMaterialId(val[APPEARANCE_Material].asUInt64());
    else
        m_material.Invalidate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnSubCategory::Appearance::ToJson() const
    {
    Json::Value val;

    if (m_invisible)            val[APPEARANCE_Invisible] = true;
    if (ColorDef::Black() != m_color)  val[APPEARANCE_Color]  = m_color.GetValue();
    if (0 != m_weight)          val[APPEARANCE_Weight] = m_weight;
    if (m_style.IsValid())      val[APPEARANCE_Style]  = m_style.GetValue();
    if (0 != m_displayPriority) val[APPEARANCE_Priority] = m_displayPriority;
    if (m_material.IsValid())   val[APPEARANCE_Material] = m_material.GetValue();
    if (0.0 != m_transparency)  val[APPEARANCE_Transparency] = m_transparency;

    return Json::FastWriter::ToString(val);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnSubCategory::Appearance::operator==(Appearance const& other) const
    {
    return m_invisible==other.m_invisible &&
           m_color==other.m_color && 
           m_weight==other.m_weight && 
           m_style==other.m_style && 
           m_displayPriority==other.m_displayPriority && 
           m_material==other.m_material && 
           m_transparency==other.m_transparency;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnSubCategory::Override::ToJson(JsonValueR outValue) const
    {
    if (m_flags.m_invisible)    outValue[APPEARANCE_Invisible] = m_value.IsInvisible();
    if (m_flags.m_color)        outValue[APPEARANCE_Color] = m_value.GetColor().GetValue();
    if (m_flags.m_weight)       outValue[APPEARANCE_Weight] = m_value.GetWeight();
    if (m_flags.m_style)        outValue[APPEARANCE_Style] = m_value.GetStyle().GetValue();
    if (m_flags.m_material)     outValue[APPEARANCE_Material] = m_value.GetMaterial().GetValue();
    if (m_flags.m_priority)     outValue[APPEARANCE_Priority] = m_value.GetDisplayPriority();
    if (m_flags.m_transparency) outValue[APPEARANCE_Transparency] = m_value.GetTransparency();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnSubCategory::Override::FromJson(JsonValueCR val)
    {
    Init();

    if (val.isMember(APPEARANCE_Invisible))    SetInvisible(val[APPEARANCE_Invisible].asBool());
    if (val.isMember(APPEARANCE_Color))        SetColor(ColorDef(val[APPEARANCE_Color].asUInt()));
    if (val.isMember(APPEARANCE_Weight))       SetWeight(val[APPEARANCE_Weight].asUInt());
    if (val.isMember(APPEARANCE_Style))        SetStyle(DgnStyleId(val[APPEARANCE_Style].asUInt64()));
    if (val.isMember(APPEARANCE_Material))     SetMaterial(DgnMaterialId(val[APPEARANCE_Material].asUInt64()));
    if (val.isMember(APPEARANCE_Priority))     SetDisplayPriority(val[APPEARANCE_Priority].asInt());
    if (val.isMember(APPEARANCE_Transparency)) SetTransparency(val[APPEARANCE_Transparency].asDouble());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnSubCategory::Override::ApplyTo(Appearance& appear) const
    {
    if (m_flags.m_invisible)    appear.SetInvisible(m_value.IsInvisible());
    if (m_flags.m_color)        appear.SetColor(m_value.GetColor());
    if (m_flags.m_weight)       appear.SetWeight(m_value.GetWeight());
    if (m_flags.m_style)        appear.SetStyle(m_value.GetStyle());
    if (m_flags.m_material)     appear.SetMaterial(m_value.GetMaterial());
    if (m_flags.m_priority)     appear.SetDisplayPriority(m_value.GetDisplayPriority());
    if (m_flags.m_transparency) appear.SetTransparency(m_value.GetTransparency());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnCategory::_OnImported(DgnElementCR original, DgnImportContext& importer) const
    {
    if (importer.IsBetweenDbs())
        {
        BeAssert(nullptr != dynamic_cast<DgnCategoryCP>(&original));

        for (auto const& srcSubCatId : DgnSubCategory::QuerySubCategories(importer.GetSourceDb(), DgnCategoryId(original.GetElementId().GetValue())))
            importer.RemapSubCategory(GetCategoryId(), srcSubCatId);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnSubCategory::_RemapIds(DgnImportContext& importer)
    {
    // *** WIP_IMPORT: Translate style IDS in sourceAppearance
    T_Super::_RemapIds(importer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId DgnCategory::ImportCategory(DgnCategoryId srcCatId, DgnImportContext& importer, DgnRemapTables& remap)
    {
    DgnCategoryId dstCatId = remap.Find(srcCatId);
    if (dstCatId.IsValid())
        return dstCatId;

    DgnCategoryCPtr srcCat = DgnCategory::QueryCategory(srcCatId, importer.GetSourceDb());
    BeAssert(srcCat.IsValid());
    if (!srcCat.IsValid())
        return DgnCategoryId();

    dstCatId = QueryCategoryId(srcCat->GetCategoryName(), importer.GetDestinationDb());
    if (!dstCatId.IsValid())
        {
        auto importedElem = srcCat->Import(nullptr, importer.GetDestinationDb().GetDictionaryModel(), importer);
        if (importedElem.IsValid())
            dstCatId = DgnCategoryId(importedElem->GetElementId().GetValue());
        }

    if (dstCatId.IsValid())
        remap.Add(srcCatId, dstCatId);

    return dstCatId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategoryId DgnSubCategory::ImportSubCategory(DgnSubCategoryId srcSubCatId, DgnCategoryId dstCatId, DgnImportContext& importer, DgnRemapTables& remap)
    {
    DgnSubCategoryId dstSubCatId = remap.Find(srcSubCatId);
    if (dstSubCatId.IsValid())
        return dstSubCatId;

    DgnSubCategoryCPtr srcSubCat = DgnSubCategory::QuerySubCategory(srcSubCatId, importer.GetSourceDb());
    BeAssert(srcSubCat.IsValid());
    if (srcSubCat.IsNull())
        return dstSubCatId;

    if (srcSubCat->IsDefaultSubCategory())
        {
        dstSubCatId = DgnCategory::GetDefaultSubCategoryId(dstCatId);
        DgnSubCategoryCPtr dstSubCat = DgnSubCategory::QuerySubCategory(dstSubCatId, importer.GetDestinationDb());
        BeAssert(dstSubCat.IsValid() && "Importing a category should have created a default sub-category");
        if (dstSubCat.IsValid())
            {
            BeAssert(dstSubCat->IsDefaultSubCategory());

            // Since we must create the default sub-category as soon as the category is inserted, we now need to update its properties to reflect the source sub-category

            // *** WIP_IMPORT: Translate style IDS in sourceAppearance
            UNUSED_VARIABLE(dstSubCat);
            }
        }
    else
        {
        auto dstSubCat = srcSubCat->Import(nullptr, importer.GetDestinationDb().GetDictionaryModel(), importer);
        if (dstSubCat.IsValid())
            dstSubCatId = DgnSubCategoryId(dstSubCat->GetElementId().GetValue());
        }

    if (dstSubCatId.IsValid())
        remap.Add(srcSubCatId, dstSubCatId);

    return dstSubCatId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId DgnImportContext::RemapCategory(DgnCategoryId source)
    {
    if (!IsBetweenDbs())
        return source;

    DgnCategoryId dest = m_remap.Find(source);
    if (dest.IsValid())
        return dest;

    return DgnCategory::ImportCategory(source, *this, m_remap);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategoryId DgnImportContext::RemapSubCategory(DgnCategoryId destCategoryId, DgnSubCategoryId source)
    {
    if (!IsBetweenDbs())
        return source;

    DgnSubCategoryId dest = m_remap.Find(source);
    if (dest.IsValid())
        return dest;

    return DgnSubCategory::ImportSubCategory(source, destCategoryId, *this, m_remap);
    }

