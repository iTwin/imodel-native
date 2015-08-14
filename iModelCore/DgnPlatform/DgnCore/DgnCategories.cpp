/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnCategories.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnCategories::Insert(DgnCategories::Category& row, DgnCategories::SubCategory::Appearance const& appearance)
    {
    if (row.IsValid() || !IsValidCode(row.GetCode()))
        {
        BeAssert (false);
        return  BE_SQLITE_ERROR;
        }

    // NOTE: we need to get the CategoryId from the SubCategory table, not the category table. That's because the DgnCategoryId
    // is equal to the default DgnSubCategoryId, but there are more entries in the SubCategory table than the category table.
    DbResult status = m_dgndb.GetServerIssuedId(row.m_categoryId, DGN_TABLE(DGN_CLASSNAME_SubCategory), "Id");

    Statement stmt;
    stmt.Prepare (m_dgndb, "INSERT INTO " DGN_TABLE(DGN_CLASSNAME_Category) " (Id,Code,Label,Descr,Rank,Scope) VALUES(?,?,?,?,?,?)");

    stmt.BindId   (1, row.GetCategoryId());
    stmt.BindText (2, row.GetCode(), Statement::MakeCopy::No);
    stmt.BindText (3, row.GetLabel(), Statement::MakeCopy::No);
    stmt.BindText (4, row.GetDescription(), Statement::MakeCopy::No);
    stmt.BindInt  (5, (int) row.GetRank());
    stmt.BindInt  (6, (int) row.GetScope());

    status = stmt.Step();
    if (BE_SQLITE_DONE != status)
        return status;

    SubCategory subCategory (row.GetCategoryId(), DgnCategories::DefaultSubCategoryId(row.GetCategoryId()), "", appearance);
    return InsertSubCategory (subCategory);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      04/12
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnCategories::Update(Category const& row)
    {
    if (!row.IsValid() || !IsValidCode(row.GetCode()))
        return  BE_SQLITE_ERROR;

    Statement stmt;
    stmt.Prepare (m_dgndb, "UPDATE " DGN_TABLE(DGN_CLASSNAME_Category) " SET Code=?,Label=?,Descr=?,Rank=?,Scope=? WHERE Id=?");

    stmt.BindText (1, row.GetCode(), Statement::MakeCopy::No);
    stmt.BindText (2, row.GetLabel(), Statement::MakeCopy::No);
    stmt.BindText (3, row.GetDescription(), Statement::MakeCopy::No);
    stmt.BindInt  (4, (int) row.GetRank());
    stmt.BindInt  (5, (int) row.GetScope());
    stmt.BindId   (6, row.GetCategoryId());

    DbResult status = stmt.Step();
    BeDataAssert (BE_SQLITE_DONE==status);
    return (BE_SQLITE_DONE==status) ? BE_SQLITE_OK : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnCategories::Delete(DgnCategoryId categoryId)
    {
    Statement stmt;
    stmt.Prepare (m_dgndb, "DELETE FROM " DGN_TABLE(DGN_CLASSNAME_Category) " WHERE Id=?");
    stmt.BindId (1, categoryId);
    const auto status = stmt.Step ();
    return (BE_SQLITE_DONE == status) ? BE_SQLITE_OK : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId DgnCategories::QueryCategoryId(Utf8CP code) const
    {
    CachedStatementPtr stmt;
    m_dgndb.GetCachedStatement (stmt, "SELECT Id FROM " DGN_TABLE(DGN_CLASSNAME_Category) " WHERE Code=?");
    stmt->BindText (1, code, Statement::MakeCopy::No);
    return (BE_SQLITE_ROW != stmt->Step()) ? DgnCategoryId() : stmt->GetValueId<DgnCategoryId>(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId DgnCategories::QueryCategoryId(DgnSubCategoryId subCategoryId) const
    {
    BeSQLite::HighPriorityOperationBlock highPriorityOperationBlock; // See comments on HighPriorityOperationBlock

    CachedStatementPtr stmt;
    m_dgndb.GetCachedStatement (stmt, "SELECT CategoryId FROM " DGN_TABLE(DGN_CLASSNAME_SubCategory) " WHERE Id=?");

    stmt->BindId(1, subCategoryId);
    return (BE_SQLITE_ROW == stmt->Step()) ? stmt->GetValueId<DgnCategoryId>(0) : DgnCategoryId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    02/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId DgnCategories::QueryCategoryId(DgnElementId elementId) const
    {
    CachedStatementPtr statementPtr;
    GetDgnDb().GetCachedStatement(statementPtr, "SELECT CategoryId FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE Id=?");
    statementPtr->BindId(1, elementId);
    return (BE_SQLITE_ROW != statementPtr->Step()) ? DgnCategoryId() : statementPtr->GetValueId<DgnCategoryId>(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId DgnCategories::QueryHighestId() const
    {
    Statement stmt;
    stmt.Prepare (m_dgndb, "SELECT max(Id) FROM " DGN_TABLE(DGN_CLASSNAME_Category));
    return (BE_SQLITE_ROW != stmt.Step()) ? DgnCategoryId() : stmt.GetValueId<DgnCategoryId>(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategories::Category DgnCategories::Query(DgnCategoryId id) const
    {
    if (!id.IsValid())
        return Category();

    // This has no effect unless there is a range tree query occurring during update dynamics.  See comments
    // on HighPriorityOperationBlock for more information.
    BeSQLite::HighPriorityOperationBlock highPriorityOperationBlock;
    CachedStatementPtr stmt;
    m_dgndb.GetCachedStatement (stmt, "SELECT Code,Label,Descr,Rank,Scope FROM " DGN_TABLE(DGN_CLASSNAME_Category) " WHERE Id=?");
    stmt->BindId(1, id);

    Category category;
    if (BE_SQLITE_ROW == stmt->Step())
        {
        category.m_categoryId = id;
        category.m_code.AssignOrClear (stmt->GetValueText(0));
        category.m_label.AssignOrClear (stmt->GetValueText(1));
        category.m_description.AssignOrClear (stmt->GetValueText(2));
        category.m_rank = (Rank) stmt->GetValueInt(3);
        category.m_scope = (Scope) stmt->GetValueInt(4);
        }

    return category;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategories::Iterator::const_iterator DgnCategories::Iterator::begin() const
    {
    BeSQLite::HighPriorityOperationBlock highPriorityOperationBlock;
    if (!m_stmt.IsValid())
        {
        Utf8String sqlString = MakeSqlString("SELECT Id,Code,Label,Descr,Rank,Scope FROM " DGN_TABLE(DGN_CLASSNAME_Category));
        m_db->GetCachedStatement (m_stmt, sqlString.c_str());
        m_params.Bind(*m_stmt);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry (m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     05/2012
//---------------------------------------------------------------------------------------
size_t DgnCategories::Iterator::QueryCount() const
    {
    Utf8String sqlString = MakeSqlString("SELECT count(*) FROM " DGN_TABLE(DGN_CLASSNAME_Category));

    Statement sql;
    sql.Prepare (*m_db, sqlString.c_str());

    return ((BE_SQLITE_ROW != sql.Step()) ? 0 : sql.GetValueInt (0));
    }

DgnCategoryId DgnCategories::Iterator::Entry::GetCategoryId() const {Verify(); return m_sql->GetValueId<DgnCategoryId>(0);}
Utf8CP DgnCategories::Iterator::Entry::GetCode() const {Verify(); return m_sql->GetValueText(1);}
Utf8CP DgnCategories::Iterator::Entry::GetLabel() const {Verify(); return m_sql->GetValueText(2);}
Utf8CP DgnCategories::Iterator::Entry::GetDescription() const {Verify(); return m_sql->GetValueText(3);}
DgnCategories::Rank DgnCategories::Iterator::Entry::GetRank() const {Verify(); return (Rank) m_sql->GetValueInt(4);}
DgnCategories::Scope DgnCategories::Iterator::Entry::GetScope() const {Verify();return (Scope) m_sql->GetValueInt(5);}

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
DbResult DgnCategories::InsertSubCategory (SubCategory& subCategory)
    {
    if (!subCategory.GetCategoryId().IsValid())
        {
        BeAssert (false);
        return BE_SQLITE_ERROR;
        }

    if (!subCategory.GetSubCategoryId().IsValid())
        {
        DbResult rc = m_dgndb.GetServerIssuedId(subCategory.m_subCategoryId, DGN_TABLE(DGN_CLASSNAME_SubCategory), "Id");
        if (BE_SQLITE_OK != rc)
            return rc;
        }

    Statement stmt;
    stmt.Prepare (m_dgndb, "INSERT INTO " DGN_TABLE(DGN_CLASSNAME_SubCategory) " (Id,CategoryId,Code,Label,Descr,Props) VALUES(?,?,?,?,?,?)");

    stmt.BindId (1, subCategory.GetSubCategoryId());
    stmt.BindId (2, subCategory.GetCategoryId());

    if (!subCategory.IsDefaultSubCategory()) // default SubCategories don't have a code/label/descr
        {
        if (!IsValidCode(subCategory.GetCode()))
            return BE_SQLITE_ERROR;

        stmt.BindText (3, subCategory.GetCode(), Statement::MakeCopy::No);
        stmt.BindText (4, subCategory.GetLabel(), Statement::MakeCopy::No);
        stmt.BindText (5, subCategory.GetDescription(), Statement::MakeCopy::No);
        }
    stmt.BindText (6, subCategory.GetAppearance().ToJson(), Statement::MakeCopy::Yes);

    DbResult status = stmt.Step();
    return (BE_SQLITE_DONE==status) ? BE_SQLITE_OK : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnCategories::UpdateSubCategory (SubCategory const& subCategory)
    {
    Statement stmt;
    stmt.Prepare (m_dgndb, "UPDATE " DGN_TABLE(DGN_CLASSNAME_SubCategory) " SET Code=?,Descr=?,Props=? WHERE Id=?");

    if (!subCategory.IsDefaultSubCategory()) // default SubCategories don't have a code/descr
        {
        if (!IsValidCode(subCategory.GetCode()))
            return BE_SQLITE_ERROR;

        stmt.BindText (1, subCategory.GetCode(), Statement::MakeCopy::No);
        stmt.BindText (2, subCategory.GetLabel(), Statement::MakeCopy::No);
        stmt.BindText (3, subCategory.GetDescription(), Statement::MakeCopy::No);
        }
    stmt.BindText (4, subCategory.GetAppearance().ToJson(), Statement::MakeCopy::Yes);
    stmt.BindId (5, subCategory.GetSubCategoryId());

    DbResult status = stmt.Step();
    BeDataAssert (BE_SQLITE_DONE==status);
    return (BE_SQLITE_DONE==status) ? BE_SQLITE_OK : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnCategories::DeleteSubCategory(DgnSubCategoryId subCategoryId)
    {
    Statement stmt;
    stmt.Prepare (m_dgndb, "DELETE FROM " DGN_TABLE(DGN_CLASSNAME_SubCategory) " WHERE Id=? AND CategoryId!=?"); // don't allow the default SubCategory to be deleted
    stmt.BindId (1, subCategoryId);
    stmt.BindId (2, subCategoryId); // a default SubCategory has the same Id and CategoryId
    return BE_SQLITE_DONE == stmt.Step() ? BE_SQLITE_OK : BE_SQLITE_ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategoryId DgnCategories::QuerySubCategoryId (DgnCategoryId categoryId, Utf8CP subCategoryCode) const
    {
    CachedStatementPtr stmt;
    m_dgndb.GetCachedStatement (stmt, "SELECT Id FROM " DGN_TABLE(DGN_CLASSNAME_SubCategory) " WHERE CategoryId=? AND Code=?");
    stmt->BindId (1, categoryId);
    stmt->BindText (2, subCategoryCode, Statement::MakeCopy::No);
    return (BE_SQLITE_ROW != stmt->Step()) ? DgnSubCategoryId() : stmt->GetValueId<DgnSubCategoryId>(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    03/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategoryId DgnCategories::QuerySubCategoryId(DgnGeomPartId geomPartId) const
    {
    CachedStatementPtr statementPtr;
    GetDgnDb().GetCachedStatement(statementPtr, "SELECT SubCategoryId FROM " DGN_TABLE(DGN_CLASSNAME_GeomPart) " WHERE Id=?");
    statementPtr->BindId(1, geomPartId);
    return (BE_SQLITE_ROW != statementPtr->Step()) ? DgnSubCategoryId() : statementPtr->GetValueId<DgnSubCategoryId>(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategories::SubCategory DgnCategories::QuerySubCategory(DgnSubCategoryId subCategoryId) const
    {
    if (!subCategoryId.IsValid())
        return SubCategory();

    BeSQLite::HighPriorityOperationBlock highPriorityOperationBlock; // See comments on HighPriorityOperationBlock

    CachedStatementPtr stmt;
    m_dgndb.GetCachedStatement (stmt, "SELECT CategoryId,Code,Label,Descr,Props FROM " DGN_TABLE(DGN_CLASSNAME_SubCategory) " WHERE Id=?");
    stmt->BindId(1, subCategoryId);

    SubCategory subCategory;
    if (BE_SQLITE_ROW == stmt->Step())
        {
        int col=0;
        subCategory.m_categoryId = stmt->GetValueId<DgnCategoryId>(col++);
        subCategory.m_subCategoryId = subCategoryId;
        subCategory.m_code.AssignOrClear(stmt->GetValueText(col++));
        subCategory.m_label.AssignOrClear(stmt->GetValueText(col++));
        subCategory.m_description.AssignOrClear(stmt->GetValueText(col++));
        subCategory.m_appearance.FromJson(stmt->GetValueText(col++));
        }

    return subCategory;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategories::SubCategoryIterator::const_iterator DgnCategories::SubCategoryIterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        bool whereCategory = m_categoryId.IsValid();

        Utf8String selectSql = "SELECT CategoryId,Id,Code,Label,Descr,Props FROM " DGN_TABLE(DGN_CLASSNAME_SubCategory);
        if (whereCategory)
            selectSql.append(" WHERE CategoryId=?");

        Utf8String sqlString = MakeSqlString(selectSql.c_str(), whereCategory);

        m_db->GetCachedStatement (m_stmt, sqlString.c_str());

        if (whereCategory)
            m_stmt->BindId(1, m_categoryId);

        m_params.Bind(*m_stmt);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry (m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

DgnCategoryId DgnCategories::SubCategoryIterator::Entry::GetCategoryId() const {Verify(); return m_sql->GetValueId<DgnCategoryId>(0);}
DgnSubCategoryId DgnCategories::SubCategoryIterator::Entry::GetSubCategoryId() const {Verify(); return m_sql->GetValueId<DgnSubCategoryId>(1);}
Utf8CP DgnCategories::SubCategoryIterator::Entry::GetCode() const {Verify(); return m_sql->GetValueText(2);}
Utf8String DgnCategories::SubCategoryIterator::Entry::GetLabel() const {Verify(); return m_sql->GetValueText(3);}
Utf8CP DgnCategories::SubCategoryIterator::Entry::GetDescription() const {Verify(); return m_sql->GetValueText(4);}
DgnCategories::SubCategory::Appearance DgnCategories::SubCategoryIterator::Entry::GetAppearance() const {Verify(); return DgnCategories::SubCategory::Appearance(m_sql->GetValueText(5));}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnCategories::SubCategory::Appearance::FromJson(Utf8StringCR jsonStr)
    {
    Init();

    Json::Value val (Json::objectValue);
    if (!Json::Reader::Parse(jsonStr, val))
        return;

    m_invisible = val.get(APPEARANCE_Invisible, false).asBool();
    m_color  = ColorDef(val[APPEARANCE_Color].asUInt());
    m_weight = val[APPEARANCE_Weight].asUInt();
    if (val.isMember(APPEARANCE_Style))
        m_style  = DgnStyleId(val[APPEARANCE_Style].asInt64());
    else
        m_style.Invalidate();
    m_displayPriority = val[APPEARANCE_Priority].asInt();
    m_transparency = val[APPEARANCE_Transparency].asDouble();

    if (val.isMember(APPEARANCE_Material))
        m_material = DgnMaterialId(val[APPEARANCE_Material].asInt64());
    else
        m_material.Invalidate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/13
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnCategories::SubCategory::Appearance::ToJson() const
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
bool DgnCategories::SubCategory::Appearance::operator==(Appearance const& other) const
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
void DgnCategories::SubCategory::Override::ToJson(JsonValueR outValue) const
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
void DgnCategories::SubCategory::Override::FromJson(JsonValueCR val)
    {
    Init();

    if (val.isMember(APPEARANCE_Invisible))    SetInvisible(val[APPEARANCE_Invisible].asBool());
    if (val.isMember(APPEARANCE_Color))        SetColor(ColorDef(val[APPEARANCE_Color].asUInt()));
    if (val.isMember(APPEARANCE_Weight))       SetWeight(val[APPEARANCE_Weight].asUInt());
    if (val.isMember(APPEARANCE_Style))        SetStyle(DgnStyleId(val[APPEARANCE_Style].asInt64()));
    if (val.isMember(APPEARANCE_Material))     SetMaterial(DgnMaterialId(val[APPEARANCE_Material].asInt64()));
    if (val.isMember(APPEARANCE_Priority))     SetDisplayPriority(val[APPEARANCE_Priority].asInt());
    if (val.isMember(APPEARANCE_Transparency)) SetTransparency(val[APPEARANCE_Transparency].asDouble());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnCategories::SubCategory::Override::ApplyTo(Appearance& appear) const
    {
    if (m_flags.m_invisible)    appear.SetInvisible(m_value.IsInvisible());
    if (m_flags.m_color)        appear.SetColor(m_value.GetColor());
    if (m_flags.m_weight)       appear.SetWeight(m_value.GetWeight());
    if (m_flags.m_style)        appear.SetStyle(m_value.GetStyle());
    if (m_flags.m_material)     appear.SetMaterial(m_value.GetMaterial());
    if (m_flags.m_priority)     appear.SetDisplayPriority(m_value.GetDisplayPriority());
    if (m_flags.m_transparency) appear.SetTransparency(m_value.GetTransparency());
    }

static Utf8CP ToUtf8CP(Utf8CP s) {return s;}
static Utf8CP ToUtf8CP(Utf8StringCR str) {return str.c_str();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
static DgnSubCategoryId importSubCategory(DgnDbR targetDb, DgnCategoryId targetcatid, T const& sourceSubCat)
    {
    DgnCategories::SubCategory::Appearance sourceAppearance = sourceSubCat.GetAppearance();
    // *** WIP_IMPORT: Translate style IDS in sourceAppearance

    DgnCategories::SubCategory targetsubcat(targetcatid, DgnSubCategoryId(), sourceSubCat.GetCode(), sourceAppearance, sourceSubCat.GetDescription(), ToUtf8CP(sourceSubCat.GetLabel()));
    targetDb.Categories().InsertSubCategory(targetsubcat);
    return targetsubcat.GetSubCategoryId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
static DgnSubCategoryId importSubCategory(DgnRemapTables& remap, DgnDbR destDb, DgnCategoryId destCategoryId, T const& sourceSubCat)
    {
    DgnSubCategoryId destSubCategoryId = remap.Find(sourceSubCat.GetSubCategoryId());
    if (destSubCategoryId.IsValid())
        return destSubCategoryId;

    if (sourceSubCat.GetSubCategoryId() == DgnCategories::DefaultSubCategoryId(sourceSubCat.GetCategoryId()))
        destSubCategoryId = DgnCategories::DefaultSubCategoryId(destCategoryId);
    else
        {
        destSubCategoryId = destDb.Categories().QuerySubCategoryId(destCategoryId, sourceSubCat.GetCode());
        if (!destCategoryId.IsValid())
            destSubCategoryId = importSubCategory(destDb, destCategoryId, sourceSubCat);
        }

    if (!destSubCategoryId.IsValid())
        return destSubCategoryId;

    return remap.Add(sourceSubCat.GetSubCategoryId(), destSubCategoryId);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnCategoryId importCategory(DgnDbR targetDb, DgnDbR sourceDb, DgnCategories::Category const& sourcecat)
    {
    DgnCategories::SubCategory::Appearance targetAppearance = sourceDb.Categories().QuerySubCategory(DgnCategories::DefaultSubCategoryId(sourcecat.GetCategoryId())).GetAppearance();
    // *** WIP_IMPORT: Translate style IDS in targetAppearance

    DgnCategories::Category targetcat(sourcecat.GetCode(), DgnCategories::Scope::Physical, sourcecat.GetDescription());
    targetDb.Categories().Insert(targetcat, targetAppearance);
    return targetcat.GetCategoryId();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnSubCategoryId DgnCategories::Import(DgnRemapTables& remap, DgnCategoryId destCategoryId, DgnDbR sourceDb, DgnSubCategoryId sourceSubCategoryId)
    {
    DgnCategories::SubCategory sourceSubCat = sourceDb.Categories().QuerySubCategory(sourceSubCategoryId);
    if (!sourceSubCat.IsValid())
        return DgnSubCategoryId();
    return importSubCategory(remap, GetDgnDb(), destCategoryId, sourceSubCat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCategoryId DgnCategories::Import(DgnRemapTables& remap, DgnDbR sourceDb, DgnCategoryId sourceCategoryId)
    {
    DgnCategoryId destCategoryId = remap.Find(sourceCategoryId);
    if (!destCategoryId.IsValid())
        {
        DgnCategories::Category sourcecat = sourceDb.Categories().Query(sourceCategoryId);

        destCategoryId = QueryCategoryId(sourcecat.GetCode());
        if (!destCategoryId.IsValid())
            {
            destCategoryId = importCategory(GetDgnDb(), sourceDb, sourcecat);
            if (!destCategoryId.IsValid())
                return DgnCategoryId();
            }

        remap.Add(sourceCategoryId, destCategoryId);
        }

    for (auto const& sourceSubCat : sourceDb.Categories().MakeSubCategoryIterator(sourceCategoryId))
        {
        importSubCategory(remap, GetDgnDb(), destCategoryId, sourceSubCat);
        }

    return destCategoryId;
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
    return GetDestinationDb().Categories().Import(m_remap, GetSourceDb(), source);
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
    return GetDestinationDb().Categories().Import(m_remap, destCategoryId, GetSourceDb(), source);
    }