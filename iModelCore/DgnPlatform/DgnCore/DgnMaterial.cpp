/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnMaterial.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnMaterialId DgnMaterials::Insert(Material& material, DgnDbStatus* outResult)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(result, outResult);
    DgnMaterialId newId;

    auto status = m_dgndb.GetServerIssuedId(newId, DGN_TABLE(DGN_CLASSNAME_Material), "Id");
    if (status != BE_SQLITE_OK)
        {
        result = DgnDbStatus::ForeignKeyConstraint;
        return DgnMaterialId();
        }

    Statement stmt(m_dgndb, "INSERT INTO " DGN_TABLE(DGN_CLASSNAME_Material) " (Id,Value,Name,Palette,Descr,ParentId) VALUES(?,?,?,?,?,?)");
    stmt.BindId(1, newId);
    stmt.BindText(2, material.GetValue(), Statement::MakeCopy::No);
    stmt.BindText(3, material.GetName(), Statement::MakeCopy::No);
    stmt.BindText(4, material.GetPalette(), Statement::MakeCopy::No);
    stmt.BindText(5, material.GetDescr(), Statement::MakeCopy::No);
    stmt.BindId(6, material.GetParentId());

    status = stmt.Step();
    if (BE_SQLITE_DONE != status)
        {
        result = DgnDbStatus::DuplicateName;
        return DgnMaterialId();
        }

    result = DgnDbStatus::Success;
    material.m_id = newId;
    return newId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnMaterials::Delete(DgnMaterialId materialId)
    {
    Statement stmt;
    stmt.Prepare(m_dgndb, "DELETE FROM " DGN_TABLE(DGN_CLASSNAME_Material) " WHERE Id=?");
    stmt.BindId(1, materialId);
    const auto status = stmt.Step();
    return (BE_SQLITE_DONE == status) ? BE_SQLITE_OK : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnMaterials::Update(Material const& material) const
    {
    if (!material.IsValid())
        return DgnDbStatus::InvalidId;

    Statement stmt(m_dgndb, "UPDATE " DGN_TABLE(DGN_CLASSNAME_Material) " SET Value=?,Descr=?,ParentId=? WHERE Id=?");
    stmt.BindText(1, material.GetValue(), Statement::MakeCopy::No);
    stmt.BindText(2, material.GetDescr(), Statement::MakeCopy::No);
    stmt.BindId(3, material.GetParentId());
    stmt.BindId(4, material.GetId());

    DbResult status = stmt.Step();
    BeDataAssert(BE_SQLITE_DONE==status);
    return (BE_SQLITE_DONE==status) ? DgnDbStatus::Success : DgnDbStatus::WriteError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnMaterials::Material DgnMaterials::Query(DgnMaterialId id) const
    {
    if (!id.IsValid())
        return Material();

    // This has no effect unless there is a range tree query occurring during update dynamics.  See comments
    // on HighPriorityOperationBlock for more information.
    BeSQLite::HighPriorityOperationBlock highPriorityOperationBlock;
    CachedStatementPtr stmt;
    m_dgndb.GetCachedStatement(stmt, "SELECT Value,Name,Descr,Palette,ParentId FROM " DGN_TABLE(DGN_CLASSNAME_Material) " WHERE Id=?");
    stmt->BindId(1, id);

    Material material;
    if (BE_SQLITE_ROW == stmt->Step())
        {
        material.m_id = id;
        material.m_value.AssignOrClear(stmt->GetValueText(0));
        material.m_name.AssignOrClear(stmt->GetValueText(1));
        material.m_descr.AssignOrClear(stmt->GetValueText(2));
        material.m_palette.AssignOrClear(stmt->GetValueText(3));
        material.m_parentId = stmt->GetValueId<DgnMaterialId>(4);
        }

    return material;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnMaterialId DgnMaterials::QueryMaterialId(Utf8StringCR name, Utf8StringCR palette) const
    {
    Statement stmt(m_dgndb, "SELECT Id FROM " DGN_TABLE(DGN_CLASSNAME_Material) " WHERE Name=? AND Palette=?");
    stmt.BindText(1, name, Statement::MakeCopy::No);
    stmt.BindText(2, palette, Statement::MakeCopy::No);
    return BE_SQLITE_ROW != stmt.Step() ? DgnMaterialId() : stmt.GetValueId<DgnMaterialId>(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ray.Bentley                   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
uintptr_t   DgnMaterials::s_nextQvMaterialId;

uintptr_t   DgnMaterials::AddQvMaterialId (DgnMaterialId materialId) const        { return (m_qvMaterialIds[materialId] = ++s_nextQvMaterialId); }
uintptr_t   DgnMaterials::GetQvMaterialId (DgnMaterialId materialId) const
    {
    auto const&   found = m_qvMaterialIds.find(materialId);

    return (found == m_qvMaterialIds.end()) ? 0 : found->second; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ray.Bentley                   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnMaterials::Material::GetAsset (JsonValueR value, char const* keyWord) const
    {
    Json::Value root;

    if (!Json::Reader::Parse (GetValue(), root) ||
        (value = root[keyWord]).isNull())
        return ERROR;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ray.Bentley                   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void  DgnMaterials::Material::SetAsset (JsonValueCR value, char const* keyWord)
    {
    Json::Value root;

    if (!Json::Reader::Parse (GetValue(), root))
        root = Json::Value (Json::ValueType::objectValue);

    root[keyWord] = value;


    SetValue (Json::FastWriter::ToString (root).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnMaterials::Iterator::const_iterator DgnMaterials::Iterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        Utf8String sqlString = MakeSqlString("SELECT Id,Name,Palette,Descr,Value,ParentId FROM " DGN_TABLE(DGN_CLASSNAME_Material));
        m_db->GetCachedStatement(m_stmt, sqlString.c_str());
        m_params.Bind(*m_stmt);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
size_t DgnMaterials::Iterator::QueryCount() const
    {
    Utf8String sqlString = MakeSqlString ("SELECT count(*) FROM " DGN_TABLE(DGN_CLASSNAME_Material));
    Statement sql;
    sql.Prepare (*m_db, sqlString.c_str());
    return (BE_SQLITE_ROW == sql.Step()) ? static_cast<size_t>(sql.GetValueInt (0)) : 0;
    }

DgnMaterialId DgnMaterials::Iterator::Entry::GetId() const {Verify(); return m_sql->GetValueId<DgnMaterialId>(0);}
Utf8CP DgnMaterials::Iterator::Entry::GetName() const {Verify(); return m_sql->GetValueText(1);}
Utf8CP DgnMaterials::Iterator::Entry::GetPalette() const {Verify(); return m_sql->GetValueText(2);}
Utf8CP DgnMaterials::Iterator::Entry::GetDescr() const {Verify(); return m_sql->GetValueText(3);}
Utf8CP DgnMaterials::Iterator::Entry::GetValue() const {Verify(); return m_sql->GetValueText(4);}
DgnMaterialId DgnMaterials::Iterator::Entry::GetParentId() const {Verify(); return m_sql->GetValueId<DgnMaterialId>(5);}
