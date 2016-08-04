/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnAuthority.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthorityId DgnAuthorities::QueryAuthorityId (Utf8CP name) const
    {
    Statement stmt (m_dgndb, "SELECT Id FROM " BIS_TABLE(BIS_CLASS_Authority) " WHERE Name=?");
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
DgnDbStatus DgnAuthority::_CloneCodeForImport(DgnCodeR code, DgnElementCR srcElem, DgnModelR destModel, DgnImportContext& importer) const
    {
    code = importer.IsBetweenDbs() ? srcElem.GetCode() : DgnCode();
    return DgnDbStatus::Success;
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
    auto status = m_dgndb.GetServerIssuedId(newId, BIS_TABLE(BIS_CLASS_Authority), "Id");
    if (BE_SQLITE_OK != status)
        return DgnDbStatus::WriteError; // NEEDSWORK...can we communicate this more meaningfully?

    Utf8String propsStr = auth.SerializeProperties();

    Statement stmt(m_dgndb, "INSERT INTO " BIS_TABLE(BIS_CLASS_Authority) " (Id,Name,Properties,ECClassId) VALUES(?,?,?,?)");
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
    m_dgndb.GetCachedStatement(stmt, "SELECT Name,Properties,ECClassId FROM " BIS_TABLE(BIS_CLASS_Authority) " WHERE Id=?");
    stmt->BindId(1, id);

    if (BE_SQLITE_ROW != stmt->Step())
        {
        status = DgnDbStatus::InvalidId;
        return nullptr;
        }

    Utf8String name = stmt->GetValueText(0);
    Utf8String props = stmt->GetValueText(1);
    DgnClassId classId = stmt->GetValueId<DgnClassId>(2);

    AuthorityHandlerP handler = dgn_AuthorityHandler::Authority::FindHandler(m_dgndb, classId);
    if (nullptr == handler)
        {
        status = DgnDbStatus::MissingHandler;
        return nullptr;
        }

    DgnAuthority::CreateParams params(m_dgndb, classId, name.c_str(), id);
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
    : m_dgndb(params.m_dgndb), m_authorityId(params.m_id), m_classId(params.m_classId), m_name(params.m_name)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnAuthority::_ToPropertiesJson(JsonValueR json) const
    {
    // no base properties (used to have URI)
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnAuthority::_FromPropertiesJson(JsonValueCR json)
    {
    // no base properties (used to have URI)
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnAuthority::ReadProperties(Utf8StringCR jsonStr)
    {
    Json::Value props(Json::objectValue);
    if (Json::Reader::Parse(jsonStr, props))
        _FromPropertiesJson(props);
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
RefCountedPtr<NamespaceAuthority> NamespaceAuthority::CreateNamespaceAuthority(Utf8CP authorityName, DgnDbR dgndb)
    {
    auto& hdlr = dgn_AuthorityHandler::Namespace::GetHandler();
    CreateParams params(dgndb, dgndb.Domains().GetClassId(hdlr), authorityName);
    return static_cast<NamespaceAuthority*>(hdlr.Create(params).get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode NamespaceAuthority::CreateCode(Utf8CP authorityName, Utf8StringCR value, DgnDbR dgndb, Utf8StringCR nameSpace)
    {
    auto auth = dgndb.Authorities().Get<NamespaceAuthority>(authorityName);
    BeDataAssert(auth.IsValid());
    return auth.IsValid() ? auth->CreateCode(value, nameSpace) : DgnCode();
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct SystemAuthority
{
    enum BuiltinId
    {
        Local = 1LL,
        Material = 2LL,
        Category = 3LL,
        Resource = 4LL,    // Resources with a single name unique within a DgnDb, e.g. text styles, light definitions...namespace=resource type
        TrueColor = 5LL,
        Model = 6LL,
        Component = 7LL,    // Component instances. Code value is combination of component name, component parameter set, and unique integer ID

        // ............     Introduce new BuiltinIds here
        
        GeometryPart = 64LL,
        // NOTE: Don't introduce BuiltinIds > 64 as this will cause DgnDb file format upgrade/remapping issues
    };

    struct Info
    {
        Utf8CP name;
        BuiltinId which;
        AuthorityHandlerR handler;
    };

    static DgnAuthorityId GetId(BuiltinId which) { return DgnAuthorityId((uint64_t)which); }
    static DgnCode CreateCode(BuiltinId which, Utf8StringCR value, Utf8StringCR nameSpace="") { return DgnCode(GetId(which), value, nameSpace); }
    static DgnCode CreateCode(BuiltinId which, Utf8StringCR value, BeInt64Id nameSpaceId);

    static DbResult Insert(DgnDbR db, Statement& stmt, Info const& info)
        {
        stmt.BindId(1, GetId(info.which));
        stmt.BindText(2, info.name, Statement::MakeCopy::No);
        stmt.BindId(3, db.Domains().GetClassId(info.handler));

        DbResult result = stmt.Step();
        BeAssert(BE_SQLITE_DONE == result);
        return result;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode SystemAuthority::CreateCode(BuiltinId which, Utf8StringCR value, BeInt64Id nameSpaceId)
    {
    if (!nameSpaceId.IsValid())
        return DgnCode();

    return DgnCode(GetId(which), value, nameSpaceId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::CreateAuthorities()
    {
    Json::Value authorityProps(Json::objectValue);
    Utf8String authorityJson; // no base properties...

    Statement statement(*this, "INSERT INTO " BIS_TABLE(BIS_CLASS_Authority) " (Id,Name,ECClassId,Properties) VALUES (?,?,?,?)");
    statement.BindText(4, authorityJson, Statement::MakeCopy::No);

    SystemAuthority::Info infos[] =
        {
            { "Local", SystemAuthority::Local, dgn_AuthorityHandler::Local::GetHandler() },
            { "DgnMaterials", SystemAuthority::Material, dgn_AuthorityHandler::Material::GetHandler() },
            { "DgnCategories", SystemAuthority::Category, dgn_AuthorityHandler::Category::GetHandler() },
            { "DgnResources", SystemAuthority::Resource, dgn_AuthorityHandler::Resource::GetHandler() },
            { "DgnColors", SystemAuthority::TrueColor, dgn_AuthorityHandler::TrueColor::GetHandler() },
            { "DgnModels", SystemAuthority::Model, dgn_AuthorityHandler::Model::GetHandler() },
            { "DgnComponent", SystemAuthority::Component, dgn_AuthorityHandler::Component::GetHandler() },
            { "DgnGeometryPart", SystemAuthority::GeometryPart, dgn_AuthorityHandler::GeometryPart::GetHandler() },
        };

    for (auto const& info : infos)
        {
        DbResult result = SystemAuthority::Insert(*this, statement, info);
        if (BE_SQLITE_DONE != result)
            return result;

        statement.Reset();
        }

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode CategoryAuthority::CreateCategoryCode(Utf8StringCR name, Utf8StringCR nameSpace)
    {
    return SystemAuthority::CreateCode(SystemAuthority::Category, name, nameSpace);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode CategoryAuthority::CreateSubCategoryCode(DgnCategoryId categoryId, Utf8StringCR name)
    {
    return SystemAuthority::CreateCode(SystemAuthority::Category, name, categoryId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode DgnSubCategory::CreateSubCategoryCode(DgnCategoryCR cat, Utf8StringCR name)
    {
    return CreateSubCategoryCode(cat.GetCategoryId(), name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode MaterialAuthority::CreateMaterialCode(Utf8StringCR palette, Utf8StringCR material)
    {
    return SystemAuthority::CreateCode(SystemAuthority::Material, material, palette);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode TrueColorAuthority::CreateTrueColorCode(Utf8StringCR name, Utf8StringCR book)
    {
    return SystemAuthority::CreateCode(SystemAuthority::TrueColor, name, book);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode ResourceAuthority::CreateResourceCode(Utf8StringCR name, Utf8StringCR nameSpace)
    {
    return SystemAuthority::CreateCode(SystemAuthority::Resource, name, nameSpace);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthorityId ResourceAuthority::GetResourceAuthorityId() { return SystemAuthority::GetId(SystemAuthority::Resource); }
DgnAuthorityId MaterialAuthority::GetMaterialAuthorityId() { return SystemAuthority::GetId(SystemAuthority::Material); }
DgnAuthorityId CategoryAuthority::GetCategoryAuthorityId() { return SystemAuthority::GetId(SystemAuthority::Category); }
DgnAuthorityId GeometryPartAuthority::GetGeometryPartAuthorityId() { return SystemAuthority::GetId(SystemAuthority::GeometryPart); }
DgnAuthorityId TrueColorAuthority::GetTrueColorAuthorityId() { return SystemAuthority::GetId(SystemAuthority::TrueColor); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool validateResourceCode(DgnCode const& code, Utf8CP nameSpace)
    {
    return code.GetAuthority() == SystemAuthority::GetId(SystemAuthority::Resource) && code.GetNamespace().Equals(nameSpace) && !code.GetValue().empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewDefinition::IsValidCode(DgnCode const& code)
    {
    return validateResourceCode(code, BIS_CLASS_ViewDefinition);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode DgnTexture::CreateTextureCode(Utf8StringCR name)
    {
    // unnamed textures are supported.
    return name.empty() ? DgnCode::CreateEmpty() : ResourceAuthority::CreateResourceCode(name, BIS_CLASS_Texture);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode ModelAuthority::CreateModelCode(Utf8StringCR modelName, Utf8StringCR nameSpace)
    {
    // ###TODO_CODES: Silently replace illegal characters?
    Utf8String trimmed(modelName);
    trimmed.Trim();

    return SystemAuthority::CreateCode(SystemAuthority::Model, trimmed, nameSpace);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode ComponentAuthority::CreateVariationCode(Utf8StringCR slnId, Utf8StringCR name)
    {
    return SystemAuthority::CreateCode(SystemAuthority::Component, slnId, name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ComponentDef::IsComponentVariationCode(DgnCode const& icode)
    {
    return icode.GetAuthority() == SystemAuthority::GetId(SystemAuthority::Component);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode GeometryPartAuthority::CreateGeometryPartCode(Utf8StringCR name, Utf8StringCR ns)
    {
    return SystemAuthority::CreateCode(SystemAuthority::GeometryPart, name, ns);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode DgnCode::CreateEmpty()
    {
    // The default code is not unique and has no special meaning.
    return SystemAuthority::CreateCode(SystemAuthority::Local, "");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode::DgnCode(DgnAuthorityId id, Utf8StringCR value, BeInt64Id namespaceId) : m_authority(id), m_value(value)
    {
    Utf8Char buf[0x11] = { 0 };
    BeStringUtilities::FormatUInt64(buf, _countof(buf), namespaceId.GetValue(), HexFormatOptions());
    m_nameSpace.assign(buf);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnCode::From(DgnAuthorityId id, Utf8StringCR value, Utf8StringCR nameSpace)
    {
    m_authority = id;
    m_value = value;
    m_nameSpace = nameSpace;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnCode::operator<(DgnCode const& rhs) const
    {
    if (GetAuthority().GetValueUnchecked() != rhs.GetAuthority().GetValueUnchecked())
        return GetAuthority().GetValueUnchecked() < rhs.GetAuthority().GetValueUnchecked();

    int cmp = GetValue().CompareTo(rhs.GetValue());
    if (0 != cmp)
        return cmp < 0;

    return GetNamespace().CompareTo(rhs.GetNamespace()) < 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ICodedEntity::SetCode(DgnCode const& newCode)
    {
    DgnCode oldCode = GetCode();
    if (oldCode == newCode)
        return DgnDbStatus::Success;

    DgnDbStatus status = _SetCode(newCode);
    if (DgnDbStatus::Success != status)
        return status;

    status = ValidateCode();
    if (DgnDbStatus::Success != status)
        _SetCode(oldCode);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthorityCPtr ICodedEntity::GetCodeAuthority() const
    {
    return GetDgnDb().Authorities().GetAuthority(GetCode().GetAuthority());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ICodedEntity::ValidateCode() const
    {
    DgnAuthorityCPtr auth = GetCodeAuthority();
    if (auth.IsNull() || !SupportsCodeAuthority(*auth))
        return DgnDbStatus::InvalidCodeAuthority;
    else
        return auth->ValidateCode(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnAuthority::_ValidateCode(ICodedEntityCR entity) const
    {
    DgnCode const& code = entity.GetCode();
    if (!code.IsValid())
        return DgnDbStatus::InvalidName;

    BeAssert(code.GetAuthority() == GetAuthorityId());
    if (code.GetAuthority() != GetAuthorityId())
        return DgnDbStatus::InvalidCodeAuthority;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ResourceAuthority::_ValidateCode(ICodedEntityCR entity) const
    {
    auto status = T_Super::_ValidateCode(entity);
    if (DgnDbStatus::Success != status)
        return status;

    // Only elements are supported...
    auto el = entity.ToDgnElement();
    if (nullptr == el)
        return DgnDbStatus::InvalidCodeAuthority;

    // Namespace must match ECClass name (or subclass thereof)
    ECClassCP ecClass = el->GetElementClass();
    BeAssert(nullptr != ecClass);
    if (nullptr == ecClass)
        return DgnDbStatus::BadElement;
    else if (!ecClass->Is(entity.GetCode().GetNamespace().c_str()))
        return DgnDbStatus::InvalidName;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometryPartAuthority::_ValidateCode(ICodedEntityCR entity) const
    {
    DgnDbStatus status = T_Super::_ValidateCode(entity);
    if (DgnDbStatus::Success == status && nullptr == entity.ToDgnElement())
        status = DgnDbStatus::InvalidCodeAuthority;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ModelAuthority::_ValidateCode(ICodedEntityCR entity) const
    {
    auto status = T_Super::_ValidateCode(entity);
    if (DgnDbStatus::Success != status)
        return status;

    if (nullptr == entity.ToDgnModel())
        return DgnDbStatus::InvalidCodeAuthority;

    return DgnModels::IsValidName(entity.GetCode().GetValue()) ? DgnDbStatus::Success : DgnDbStatus::InvalidName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CategoryAuthority::_ValidateCode(ICodedEntityCR entity) const
    {
    auto status = T_Super::_ValidateCode(entity);
    if (DgnDbStatus::Success != status)
        return status;

    auto el = entity.ToDgnElement();
    if (nullptr == el)
        return DgnDbStatus::InvalidCodeAuthority;

    DgnCode const& code = entity.GetCode();

    // Reject illegal characters...
    if (!DgnCategory::IsValidName(code.GetValue()))
        return DgnDbStatus::InvalidName;

    // Category codes have no namespace
    DgnCategoryCP cat = dynamic_cast<DgnCategoryCP>(el);
    if (nullptr != cat)
        return code.GetNamespace().empty() ? DgnDbStatus::Success : DgnDbStatus::InvalidName;

    // Only categories and sub-categories are supported...
    DgnSubCategoryCP subcat = dynamic_cast<DgnSubCategoryCP>(el);
    if (nullptr == subcat)
        return DgnDbStatus::InvalidCodeAuthority;

    // all sub-category codes have namespace = category ID
    uint64_t categoryIdVal;
    if (SUCCESS != BeStringUtilities::ParseUInt64(categoryIdVal, code.GetNamespace().c_str()) || subcat->GetCategoryId().GetValue() != categoryIdVal)
        return DgnDbStatus::InvalidName;

    // ###TODO_CODES? From obsolete DgnSubCategory::_SetCode():
    if (el->GetElementId().IsValid()) // (_SetCode is called during copying. In that case, this SubCategory does not yet have an ID.)
        {
        // default sub-category has same name as category
        DgnCategoryCPtr cat = DgnCategory::QueryCategory(subcat->GetCategoryId(), subcat->GetDgnDb());
        if (!cat.IsValid())
            return DgnDbStatus::InvalidCategory;
        else if ((code.GetValue().Equals(cat->GetCategoryName()) != subcat->IsDefaultSubCategory()))
            return DgnDbStatus::InvalidName;
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnModel::_SupportsCodeAuthority(DgnAuthorityCR auth) const
    {
    // No reason not to allow customized model code authorities.
    return SystemAuthority::GetId(SystemAuthority::Local) != auth.GetAuthorityId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t DgnAuthority::RestrictedAction::Parse(Utf8CP name)
    {
    if (0 == BeStringUtilities::Stricmp(name, "validatecode"))
        return ValidateCode;
    else if (0 == BeStringUtilities::Stricmp(name, "regeneratecode"))
        return RegenerateCode;
    else if (0 == BeStringUtilities::Stricmp(name, "clonecode"))
        return CloneCode;
    else
        return T_Super::Parse(name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnAuthority::ValidateCode(ICodedEntityCR entity) const
    {
    return GetAuthorityHandler()._IsRestrictedAction(RestrictedAction::ValidateCode) ? DgnDbStatus::MissingHandler : _ValidateCode(entity);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnAuthority::CloneCodeForImport(DgnCodeR code, DgnElementCR src, DgnModelR model, DgnImportContext& context) const
    {
    return GetAuthorityHandler()._IsRestrictedAction(RestrictedAction::CloneCode) ? DgnDbStatus::MissingHandler : _CloneCodeForImport(code, src, model, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnAuthority::RegenerateCode(DgnCodeR code, ICodedEntityCR codedEntity) const
    {
    return GetAuthorityHandler()._IsRestrictedAction(RestrictedAction::RegenerateCode) ? DgnDbStatus::MissingHandler : _RegenerateCode(code, codedEntity);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP DgnCode::Iterator::Options::GetECSql() const
    {
#define SELECT_CODE_COLUMNS_FROM "SELECT [CodeAuthorityId],[CodeValue],[CodeNamespace],ECInstanceId FROM "
#define SELECT_ELEMENT_CODES SELECT_CODE_COLUMNS_FROM BIS_SCHEMA(BIS_CLASS_Element)
#define SELECT_MODEL_CODES SELECT_CODE_COLUMNS_FROM BIS_SCHEMA(BIS_CLASS_Model)
#define SELECT_MODEL_AND_ELEMENT_CODES SELECT_ELEMENT_CODES " UNION ALL " SELECT_MODEL_CODES
#define EXCLUDE_EMPTY_CODES " WHERE [CodeValue] IS NOT NULL"
#define SELECT_MODEL_AND_ELEMENT_CODES_EXCLUDE_EMPTY SELECT_ELEMENT_CODES EXCLUDE_EMPTY_CODES " UNION ALL " SELECT_MODEL_CODES EXCLUDE_EMPTY_CODES

    switch (m_include)
        {
        case Include::Elements:
            return m_includeEmpty ? SELECT_ELEMENT_CODES : SELECT_ELEMENT_CODES EXCLUDE_EMPTY_CODES;
        case Include::Models:
            return m_includeEmpty ? SELECT_MODEL_CODES : SELECT_MODEL_CODES EXCLUDE_EMPTY_CODES;
        default:
            return m_includeEmpty ? SELECT_MODEL_AND_ELEMENT_CODES : SELECT_MODEL_AND_ELEMENT_CODES_EXCLUDE_EMPTY;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode::Iterator::Iterator(DgnDbR db, Options options)
    {
    Prepare(db, options.GetECSql(), 3);
    }

