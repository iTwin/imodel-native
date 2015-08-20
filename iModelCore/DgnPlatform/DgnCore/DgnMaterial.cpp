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
DbResult DgnMaterials::Insert(Material& material)
    {
    DgnMaterialId newId;

    auto status = m_dgndb.GetServerIssuedId(newId, DGN_TABLE(DGN_CLASSNAME_Material), "Id");
    if (status != BE_SQLITE_OK)
        {
        BeAssert(false);
        return status;
        }

    material.m_id=newId;
    Statement stmt(m_dgndb, "INSERT INTO " DGN_TABLE(DGN_CLASSNAME_Material) " (Id,Value,Name,Palette,Descr,ParentId) VALUES(?,?,?,?,?,?)");

    stmt.BindId(1, newId);
    stmt.BindText(2, material.GetValue(), Statement::MakeCopy::No);
    stmt.BindText(3, material.GetName(), Statement::MakeCopy::No);
    stmt.BindText(4, material.GetPalette(), Statement::MakeCopy::No);
    stmt.BindText(5, material.GetDescr(), Statement::MakeCopy::No);
    stmt.BindId(6, material.GetParentId());

    status = stmt.Step();
    BeAssert(BE_SQLITE_DONE==status);
    return (BE_SQLITE_DONE==status) ? BE_SQLITE_OK : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnMaterials::Update(Material const& material) const
    {
    if (!material.IsValid())
        return  BE_SQLITE_ERROR;

    Statement stmt;
    stmt.Prepare(m_dgndb, "UPDATE " DGN_TABLE(DGN_CLASSNAME_Material) " SET Value=?,Descr=?,ParentId WHERE Id=?");

    stmt.BindText(1, material.GetValue(), Statement::MakeCopy::No);
    stmt.BindText(2, material.GetDescr(), Statement::MakeCopy::No);
    stmt.BindId(3, material.GetParentId());
    stmt.BindId(4, material.GetId());

    DbResult status = stmt.Step();
    BeDataAssert(BE_SQLITE_DONE==status);
    return (BE_SQLITE_DONE==status) ? BE_SQLITE_OK : status;
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
* @bsimethod                                    Keith.Bentley                   12/10
+---------------+---------------+---------------+---------------+---------------+------*/
DgnMaterials::Iterator::const_iterator DgnMaterials::Iterator::begin() const
    {
    if (!m_stmt.IsValid())
        {
        Utf8String sqlString = MakeSqlString("SELECT Id,Name,Palette,Descr,Value FROM " DGN_TABLE(DGN_CLASSNAME_Model));
        m_db->GetCachedStatement(m_stmt, sqlString.c_str());
        m_params.Bind(*m_stmt);
        }
    else
        {
        m_stmt->Reset();
        }

    return Entry(m_stmt.get(), BE_SQLITE_ROW == m_stmt->Step());
    }

DgnMaterialId DgnMaterials::Iterator::Entry::GetId() const {Verify(); return m_sql->GetValueId<DgnMaterialId>(0);}
Utf8CP DgnMaterials::Iterator::Entry::GetName() const {Verify(); return m_sql->GetValueText(1);}
Utf8CP DgnMaterials::Iterator::Entry::GetPalette() const {Verify(); return m_sql->GetValueText(2);}
Utf8CP DgnMaterials::Iterator::Entry::GetDescr() const {Verify(); return m_sql->GetValueText(3);}
Utf8CP DgnMaterials::Iterator::Entry::GetValue() const {Verify(); return m_sql->GetValueText(4);}
