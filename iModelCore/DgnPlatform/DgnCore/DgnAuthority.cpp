/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnAuthority.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthorityId DgnAuthorities::QueryAuthorityId (Utf8CP name) const
    {
    Statement stmt (m_dgndb, "SELECT Id FROM " DGN_TABLE(DGN_CLASSNAME_Authority) " WHERE Name=?");
    stmt.BindText (1, name, Statement::MakeCopy::No);
    return BE_SQLITE_ROW == stmt.Step() ? stmt.GetValueId<DgnAuthorityId>(0) : DgnAuthorityId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthorityId DgnImportContext::RemapAuthorityId(DgnAuthorityId source)
    {
    if (!IsBetweenDbs())
        return source;

    DgnAuthorityId dest = m_remap.Find(source);
    if (dest.IsValid())
        return dest;

    DgnAuthorityCPtr sourceAuthority = m_sourceDb.Authorities().GetAuthority(source);
    if (sourceAuthority.IsNull())
        {
        BeDataAssert(false && "Missing source authority");
        return source;
        }

    dest = m_destDb.Authorities().QueryAuthorityId(sourceAuthority->GetName().c_str());
    if (!dest.IsValid())
        {
        DgnAuthorityPtr destAuthority = DgnAuthority::Import(nullptr, *sourceAuthority, *this);
        if (destAuthority.IsNull())
            {
            BeDataAssert(false && "Invalid source authority");
            return source;
            }
        else
            {
            dest = destAuthority->GetAuthorityId();
            }
        }

    return m_remap.Add(source, dest);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnAuthority::Insert()
    {
    return GetDgnDb().Authorities().Insert(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthority::Code DgnAuthority::_CloneCodeForImport(DgnElementCR srcElem, DgnModelR destModel, DgnImportContext& importer) const
    {
    return importer.IsBetweenDbs() ? srcElem.GetCode() : DgnAuthority::Code();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthorityPtr DgnAuthority::Import(DgnDbStatus* outResult, DgnAuthorityCR src, DgnImportContext& importer)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT (status, outResult);

    BeAssert(&src.GetDgnDb() == &importer.GetSourceDb());
    if (!importer.IsBetweenDbs())
        {
        status = DgnDbStatus::DuplicateName;
        return nullptr;
        }

    DgnAuthorityPtr dest = src._CloneForImport(&status, importer);
    if (dest.IsNull() || DgnDbStatus::Success != (status = dest->Insert()))
        return nullptr;

    importer.AddAuthorityId(src.GetAuthorityId(), dest->GetAuthorityId());

    status = DgnDbStatus::Success;
    return dest;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthorityPtr DgnAuthority::_CloneForImport(DgnDbStatus* outResult, DgnImportContext& importer) const
    {
    DgnDbStatus ALLOW_NULL_OUTPUT (status, outResult);

    if (importer.GetDestinationDb().Authorities().QueryAuthorityId(GetName().c_str()).IsValid())
        {
        status = DgnDbStatus::DuplicateName;
        return nullptr;
        }

    auto classId = GetClassId();
    if (importer.IsBetweenDbs())
        classId = importer.RemapClassId(classId);

    CreateParams params(importer.GetDestinationDb(), classId, GetName().c_str());
    DgnAuthorityPtr clone = GetAuthorityHandler().Create(params);
    if (!clone.IsValid())
        {
        status = DgnDbStatus::NotFound; // better error code...?
        return nullptr;
        }

    Json::Value props(Json::objectValue);
    _ToPropertiesJson(props);
    clone->_FromPropertiesJson(props);

    status = DgnDbStatus::Success;
    return clone;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnAuthorities::Insert(DgnAuthorityR auth)
    {
    if (auth.GetAuthorityId().IsValid())
        return DgnDbStatus::IdExists;

    auth.m_name.Trim();
    if (QueryAuthorityId(auth.GetName().c_str()).IsValid())
        return DgnDbStatus::DuplicateName;

    DgnAuthorityId newId;
    auto status = m_dgndb.GetServerIssuedId(newId, DGN_TABLE(DGN_CLASSNAME_Authority), "Id");
    if (BE_SQLITE_OK != status)
        return DgnDbStatus::WriteError; // NEEDSWORK...can we communicate this more meaningfully?

    Utf8String propsStr = auth.SerializeProperties();

    Statement stmt(m_dgndb, "INSERT INTO " DGN_TABLE(DGN_CLASSNAME_Authority) " (Id,Name,Props,ECClassId) VALUES(?,?,?,?)");
    stmt.BindId(1, newId);
    stmt.BindText(2, auth.GetName(), Statement::MakeCopy::No);
    stmt.BindText(3, propsStr, Statement::MakeCopy::No);
    stmt.BindId(4, auth.GetClassId());

    if (BE_SQLITE_DONE != stmt.Step())
        return DgnDbStatus::WriteError;

    auth.m_authorityId = newId;

    BeDbMutexHolder _v(m_mutex);
    m_loadedAuthorities.push_back(DgnAuthorityPtr(&auth));

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthorityPtr DgnAuthorities::LoadAuthority(DgnAuthorityId id, DgnDbStatus* outResult)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT (status, outResult);

    if (!id.IsValid())
        {
        status = DgnDbStatus::InvalidId;
        return nullptr;
        }

    CachedStatementPtr stmt;
    m_dgndb.GetCachedStatement(stmt, "SELECT Name,Props,ECClassId FROM " DGN_TABLE(DGN_CLASSNAME_Authority) " WHERE Id=?");
    stmt->BindId(1, id);

    if (BE_SQLITE_ROW != stmt->Step())
        {
        status = DgnDbStatus::InvalidId;
        return nullptr;
        }

    Utf8String name = stmt->GetValueText(0);
    Utf8String props = stmt->GetValueText(1);
    DgnClassId classId = DgnClassId(stmt->GetValueInt64(2));

    AuthorityHandlerP handler = dgn_AuthorityHandler::Authority::FindHandler(m_dgndb, classId);
    if (nullptr == handler)
        {
        status = DgnDbStatus::MissingHandler;
        return nullptr;
        }

    DgnAuthority::CreateParams params(m_dgndb, classId, name.c_str(), nullptr, id);
    DgnAuthorityPtr auth = handler->Create(params);
    if (auth.IsNull())
        {
        status = DgnDbStatus::NotFound;
        return nullptr;
        }

    auth->ReadProperties(props);

    status = DgnDbStatus::Success;
    return auth;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthorityCPtr DgnAuthorities::GetAuthority(DgnAuthorityId id)
    {
    if (!id.IsValid())
        return nullptr;

    DgnAuthorityPtr authority;

    BeDbMutexHolder _v(m_mutex);
    auto found = std::find_if(m_loadedAuthorities.begin(), m_loadedAuthorities.end(), [&id](DgnAuthorityPtr const& arg) { return arg->GetAuthorityId() == id; });
    if (m_loadedAuthorities.end() != found)
        {
        authority = *found;
        }
    else
        {
        authority = LoadAuthority(id, nullptr);
        BeDataAssert(authority.IsValid());
        if (authority.IsValid())
            m_loadedAuthorities.push_back(authority);
        }

    return authority.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthorityCPtr DgnAuthorities::GetAuthority(Utf8CP name)
    {
    // good chance it's already loaded - check there before running a query
    auto found = std::find_if(m_loadedAuthorities.begin(), m_loadedAuthorities.end(), [&name](DgnAuthorityPtr const& arg) { return arg->GetName().Equals(name); });
    return m_loadedAuthorities.end() != found ? (*found).get() : GetAuthority(QueryAuthorityId(name));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthority::DgnAuthority(CreateParams const& params)
    : m_dgndb(params.m_dgndb), m_authorityId(params.m_id), m_classId(params.m_classId), m_name(params.m_name), m_uri(params.m_uri)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnAuthority::_ToPropertiesJson(JsonValueR json) const
    {
    json["uri"] = m_uri;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnAuthority::_FromPropertiesJson(JsonValueCR json)
    {
    m_uri = BeJsonUtilities::CStringFromStringValue(json["uri"], "");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnAuthority::ReadProperties(Utf8StringCR jsonStr)
    {
    Json::Value props(Json::objectValue);
    if (Json::Reader::Parse(jsonStr, props))
        _FromPropertiesJson(props);
    else
        BeAssert(false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnAuthority::SerializeProperties() const
    {
    Json::Value props(Json::objectValue);
    _ToPropertiesJson(props);
    return Json::FastWriter::ToString(props);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
AuthorityHandlerP dgn_AuthorityHandler::Authority::FindHandler(DgnDbCR dgndb, DgnClassId classId)
    {
    DgnDomain::Handler* handler = dgndb.Domains().LookupHandler(classId);
    if (nullptr == handler)
        handler = dgndb.Domains().FindHandler(classId, dgndb.Domains().GetClassId(GetHandler()));

    return nullptr != handler ? handler->_ToAuthorityHandler() : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
AuthorityHandlerR DgnAuthority::GetAuthorityHandler() const
    {
    return *dgn_AuthorityHandler::Authority::FindHandler(m_dgndb, m_classId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<NamespaceAuthority> NamespaceAuthority::CreateNamespaceAuthority(Utf8CP authorityName, DgnDbR dgndb, Utf8CP uri)
    {
    auto& hdlr = dgn_AuthorityHandler::Namespace::GetHandler();
    CreateParams params(dgndb, dgndb.Domains().GetClassId(hdlr), authorityName, uri);
    return static_cast<NamespaceAuthority*>(hdlr.Create(params).get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthority::Code NamespaceAuthority::CreateCode(Utf8CP authorityName, Utf8StringCR value, DgnDbR dgndb, Utf8StringCR nameSpace)
    {
    auto auth = dgndb.Authorities().Get<NamespaceAuthority>(authorityName);
    BeDataAssert(auth.IsValid());
    return auth.IsValid() ? auth->CreateCode(value, nameSpace) : DgnAuthority::Code();
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
enum BuiltinAuthority : uint64_t
{   
    BuiltinAuthority_Local = 1LL,
    BuiltinAuthority_Material = 2LL,
    BuiltinAuthority_LightDefinition = 3LL,
    BuiltinAuthority_Category = 4LL,
    BuiltinAuthority_Texture = 5LL,
};

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct BuiltinAuthorityInfo
{
    Utf8CP name;
    BuiltinAuthority which;
    AuthorityHandlerR handler;
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
static DbResult insertAuthority(DgnDbR db, Statement& stmt, DgnAuthorityId id, Utf8CP name, AuthorityHandlerR handler)
    {
    stmt.BindId(1, id);
    stmt.BindText(2, name, Statement::MakeCopy::No);
    stmt.BindId(3, db.Domains().GetClassId(handler));

    DbResult result = stmt.Step();
    BeAssert(BE_SQLITE_DONE == result);
    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::CreateAuthorities()
    {
    Json::Value authorityProps(Json::objectValue);
    authorityProps["uri"] = "";
    Utf8String authorityJson = Json::FastWriter::ToString(authorityProps);

    Statement statement(*this, "INSERT INTO " DGN_TABLE(DGN_CLASSNAME_Authority) " (Id,Name,ECClassId,Props) VALUES (?,?,?,?)");
    statement.BindText(4, authorityJson, Statement::MakeCopy::No);

    BuiltinAuthorityInfo builtins[] =
        {
            { "Local", BuiltinAuthority_Local, dgn_AuthorityHandler::Local::GetHandler() },
            { "DgnMaterials", BuiltinAuthority_Material, dgn_AuthorityHandler::Namespace::GetHandler() },
            { "DgnLightDefinitions", BuiltinAuthority_LightDefinition, dgn_AuthorityHandler::Namespace::GetHandler() },
            { "DgnCategories", BuiltinAuthority_Category, dgn_AuthorityHandler::Namespace::GetHandler() },
            { "DgnTextures", BuiltinAuthority_Texture, dgn_AuthorityHandler::Namespace::GetHandler() },
        };

    for (auto const& builtin : builtins)
        {
        DbResult result = insertAuthority(*this, statement, DgnAuthorityId((uint64_t)builtin.which), builtin.name, builtin.handler);
        if (BE_SQLITE_DONE != result)
            return result;

        statement.Reset();
        }

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T> static RefCountedCPtr<T> getBuiltinAuthority(BuiltinAuthority which, DgnDbR db)
    {
    return db.Authorities().Get<T>(DgnAuthorityId((uint64_t)which));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthority::Code DgnCategory::CreateCategoryCode(Utf8StringCR name, DgnDbR db)
    {
    auto auth = getBuiltinAuthority<NamespaceAuthority>(BuiltinAuthority_Category, db);
    BeAssert(auth.IsValid());
    return auth.IsValid() ? auth->CreateCode(name) : Code();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthority::Code DgnSubCategory::CreateSubCategoryCode(DgnCategoryId categoryId, Utf8StringCR name, DgnDbR db)
    {
    auto cat = DgnCategory::QueryCategory(categoryId, db);
    return cat.IsValid() ? CreateSubCategoryCode(*cat, name, db) : Code();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthority::Code DgnSubCategory::CreateSubCategoryCode(DgnCategoryCR cat, Utf8StringCR name, DgnDbR db)
    {
    auto auth = getBuiltinAuthority<NamespaceAuthority>(BuiltinAuthority_Category, db);
    return auth.IsValid() ? auth->CreateCode(name, cat.GetCategoryName()) : Code();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthority::Code DgnMaterial::CreateMaterialCode(Utf8StringCR palette, Utf8StringCR material, DgnDbR db)
    {
    auto auth = getBuiltinAuthority<NamespaceAuthority>(BuiltinAuthority_Material, db);
    BeAssert(auth.IsValid());
    return auth.IsValid() ? auth->CreateCode(material, palette) : DgnAuthority::Code();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthority::Code LightDefinition::CreateLightDefinitionCode(Utf8StringCR name, DgnDbR db)
    {
    auto auth = getBuiltinAuthority<NamespaceAuthority>(BuiltinAuthority_LightDefinition, db);
    BeAssert(auth.IsValid());
    return auth.IsValid() ? auth->CreateCode(name) : DgnAuthority::Code();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthority::Code DgnTexture::CreateTextureCode(Utf8StringCR name, DgnDbR db)
    {
    auto auth = getBuiltinAuthority<NamespaceAuthority>(BuiltinAuthority_Texture, db);
    BeAssert(auth.IsValid());
    return auth.IsValid() ? auth->CreateCode(name) : DgnAuthority::Code();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthority::Code DgnAuthority::GenerateDefaultCode(DgnElementCR el)
    {
    auto elemId = el.GetElementId();
    if (!elemId.IsValid())
        return DgnAuthority::Code();

    Utf8PrintfString val("%" PRIu32 "-%" PRIu64, elemId.GetRepositoryId().GetValue(), (uint64_t)(0xffffffffffLL & elemId.GetValue()));
    return DgnAuthority::Code(DgnAuthorityId((uint64_t)BuiltinAuthority_Local), val, el.GetElementClass()->GetName());
    }


