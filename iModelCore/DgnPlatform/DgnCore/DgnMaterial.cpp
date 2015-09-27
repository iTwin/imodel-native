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

static uintptr_t  s_nextQvMaterialId;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ray.Bentley                   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
uintptr_t DgnMaterials::AddQvMaterialId(DgnMaterialId materialId) const { return (m_qvMaterialIds[materialId] = ++s_nextQvMaterialId); }
uintptr_t DgnMaterials::GetQvMaterialId(DgnMaterialId materialId) const
    {
    auto const&   found = m_qvMaterialIds.find(materialId);

    return (found == m_qvMaterialIds.end()) ? 0 : found->second; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ray.Bentley                   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnMaterials::Material::GetAsset(JsonValueR value, Utf8CP keyWord) const
    {
    Json::Value root;

    if (!Json::Reader::Parse(GetValue(), root) ||
        (value = root[keyWord]).isNull())
        return ERROR;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ray.Bentley                   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void  DgnMaterials::Material::SetAsset(JsonValueCR value, Utf8CP keyWord)
    {
    Json::Value root;

    if (!Json::Reader::Parse(GetValue(), root))
        root = Json::Value(Json::ValueType::objectValue);

    root[keyWord] = value;

    SetValue(Json::FastWriter::ToString(root).c_str());
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
    Utf8String sqlString = MakeSqlString("SELECT count(*) FROM " DGN_TABLE(DGN_CLASSNAME_Material));
    Statement sql;
    sql.Prepare(*m_db, sqlString.c_str());
    return (BE_SQLITE_ROW == sql.Step()) ? static_cast<size_t>(sql.GetValueInt(0)) : 0;
    }

DgnMaterialId DgnMaterials::Iterator::Entry::GetId() const {Verify(); return m_sql->GetValueId<DgnMaterialId>(0);}
Utf8CP DgnMaterials::Iterator::Entry::GetName() const {Verify(); return m_sql->GetValueText(1);}
Utf8CP DgnMaterials::Iterator::Entry::GetPalette() const {Verify(); return m_sql->GetValueText(2);}
Utf8CP DgnMaterials::Iterator::Entry::GetDescr() const {Verify(); return m_sql->GetValueText(3);}
Utf8CP DgnMaterials::Iterator::Entry::GetValue() const {Verify(); return m_sql->GetValueText(4);}
DgnMaterialId DgnMaterials::Iterator::Entry::GetParentId() const {Verify(); return m_sql->GetValueId<DgnMaterialId>(5);}

#include <DgnPlatform/DgnCore/MaterialElement.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

namespace dgn_ElementHandler
{
    HANDLER_DEFINE_MEMBERS(Material);
}

END_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_ElementHandler::Material::_GetClassParams(ECSqlClassParams& params)
    {
    T_Super::_GetClassParams(params);
    params.Add("Descr");
    params.Add("Data");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnMaterial::_BindInsertParams(BeSQLite::EC::ECSqlStatement& stmt) 
    {
    auto status = T_Super::_BindInsertParams(stmt);
    if (DgnDbStatus::Success == status)
        status = BindParams(stmt);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnMaterial::_BindUpdateParams(ECSqlStatement& stmt)
    {
    auto status = T_Super::_BindUpdateParams(stmt);
    if (DgnDbStatus::Success == status)
        status = BindParams(stmt);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnMaterial::_ExtractSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ExtractSelectParams(stmt, params);
    if (DgnDbStatus::Success == status)
        m_data.Init(stmt.GetValueText(params.GetSelectIndex("Descr")), stmt.GetValueText(params.GetSelectIndex("Data")));
    
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnMaterial::BindParams(ECSqlStatement& stmt)
    {
    if (ECSqlStatus::Success != stmt.BindText(stmt.GetParameterIndex("Descr"), m_data.m_descr.c_str(), IECSqlBinder::MakeCopy::No)
        || ECSqlStatus::Success != stmt.BindText(stmt.GetParameterIndex("Data"), m_data.m_value.c_str(), IECSqlBinder::MakeCopy::No))
        return DgnDbStatus::BadArg;
    else
        return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnMaterial::_CopyFrom(DgnElementCR el) 
    {
    T_Super::_CopyFrom(el);
    auto other = dynamic_cast<DgnMaterialCP>(&el);
    BeAssert(nullptr != other);
    if (nullptr != other)
        m_data = other->m_data;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnMaterial::_SetParentId(DgnElementId parentId) 
    {
    if (parentId.IsValid())
        {
        // parent must be another material
        auto stmt = GetDgnDb().GetPreparedECSqlStatement("SELECT count(*) FROM " DGN_SCHEMA(DGN_CLASSNAME_MaterialElement) " WHERE ECInstanceId=?");
        if (!stmt.IsValid())
            return DgnDbStatus::InvalidParent;

        stmt->BindId(1, parentId);
        if (BE_SQLITE_ROW != stmt->Step() || 1 != stmt->GetValueInt(0))
            return DgnDbStatus::InvalidParent;
        }

    // TODO? Base implementation doesn't check for cycles...
    return T_Super::_SetParentId(parentId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnAuthority::Code makeMaterialCode(Utf8StringCR palette, Utf8StringCR material, DgnDbR db)
    {
    // ###TODO?
    //  Should we automatically create the material authority when initializing the DB?
    //  Do we need a special MaterialAuthority class?
    static const Utf8CP s_authorityName = "DgnMaterialAuthority";
    auto auth = db.Authorities().Get<NamespaceAuthority>(s_authorityName);
    if (auth.IsNull())
        {
        auto newAuth = NamespaceAuthority::CreateNamespaceAuthority(s_authorityName, db);
        db.Authorities().Insert(*newAuth);
        auth = db.Authorities().Get<NamespaceAuthority>(s_authorityName);
        }

    BeAssert(auth.IsValid());
    return auth.IsValid() ? auth->CreateCode(material, palette) : DgnAuthority::Code();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnMaterial::CreateParams::CreateParams(DgnDbR db, DgnModelId modelId, Utf8StringCR paletteName, Utf8StringCR materialName, Utf8StringCR value, DgnElementId parentMaterialId, Utf8StringCR descr)
    : DgnMaterial::CreateParams(db, modelId, DgnMaterial::QueryDgnClassId(db), makeMaterialCode(paletteName, materialName, db), DgnElementId(), parentMaterialId, value, descr)
    {
    //
    }

