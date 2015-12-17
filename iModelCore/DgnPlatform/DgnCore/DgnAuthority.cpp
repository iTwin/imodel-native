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
DgnAuthority::Code NamespaceAuthority::CreateCode(Utf8CP authorityName, Utf8StringCR value, DgnDbR dgndb, Utf8StringCR nameSpace)
    {
    auto auth = dgndb.Authorities().Get<NamespaceAuthority>(authorityName);
    BeDataAssert(auth.IsValid());
    return auth.IsValid() ? auth->CreateCode(value, nameSpace) : DgnAuthority::Code();
    }

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

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
    };

    struct Info
    {
        Utf8CP name;
        BuiltinId which;
        AuthorityHandlerR handler;
    };

    static DgnAuthorityId GetId(BuiltinId which) { return DgnAuthorityId((uint64_t)which); }
    static DgnAuthority::Code CreateCode(BuiltinId which, Utf8StringCR value, Utf8StringCR nameSpace="") { return DgnAuthority::Code(GetId(which), value, nameSpace); }
    static DgnAuthority::Code CreateCode(BuiltinId which, Utf8StringCR value, BeInt64Id nameSpaceId);
    template<typename T> static RefCountedCPtr<T> Get(BuiltinId which, DgnDbR db) { return db.Authorities().Get<T>(GetId(which)); }

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

END_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthority::Code SystemAuthority::CreateCode(BuiltinId which, Utf8StringCR value, BeInt64Id nameSpaceId)
    {
    if (!nameSpaceId.IsValid())
        return DgnAuthority::Code();

    Utf8Char buf[0x11] = { 0 };
    BeStringUtilities::FormatUInt64(buf, _countof(buf), nameSpaceId.GetValue(), HexFormatOptions());
    return DgnAuthority::Code(GetId(which), value, buf);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::CreateAuthorities()
    {
    Json::Value authorityProps(Json::objectValue);
    Utf8String authorityJson; // no base properties...

    Statement statement(*this, "INSERT INTO " DGN_TABLE(DGN_CLASSNAME_Authority) " (Id,Name,ECClassId,Props) VALUES (?,?,?,?)");
    statement.BindText(4, authorityJson, Statement::MakeCopy::No);

    SystemAuthority::Info infos[] =
        {
            { "Local", SystemAuthority::Local, dgn_AuthorityHandler::Local::GetHandler() },
            { "DgnMaterials", SystemAuthority::Material, dgn_AuthorityHandler::Namespace::GetHandler() },
            { "DgnCategories", SystemAuthority::Category, dgn_AuthorityHandler::Namespace::GetHandler() },
            { "DgnResources", SystemAuthority::Resource, dgn_AuthorityHandler::Namespace::GetHandler() },
            { "DgnColors", SystemAuthority::TrueColor, dgn_AuthorityHandler::Namespace::GetHandler() },
            { "DgnModels", SystemAuthority::Model, dgn_AuthorityHandler::Namespace::GetHandler() },
            { "DgnComponent", SystemAuthority::Component, dgn_AuthorityHandler::Namespace::GetHandler() },
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
DgnAuthority::Code DgnCategory::CreateCategoryCode(Utf8StringCR name)
    {
    return SystemAuthority::CreateCode(SystemAuthority::Category, name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthority::Code DgnSubCategory::CreateSubCategoryCode(DgnCategoryId categoryId, Utf8StringCR name)
    {
    return SystemAuthority::CreateCode(SystemAuthority::Category, name, categoryId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthority::Code DgnSubCategory::CreateSubCategoryCode(DgnCategoryCR cat, Utf8StringCR name)
    {
    return CreateSubCategoryCode(cat.GetCategoryId(), name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthority::Code DgnMaterial::CreateMaterialCode(Utf8StringCR palette, Utf8StringCR material)
    {
    return SystemAuthority::CreateCode(SystemAuthority::Material, material, palette);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthority::Code DgnTrueColor::CreateColorCode(Utf8StringCR name, Utf8StringCR book)
    {
    return SystemAuthority::CreateCode(SystemAuthority::TrueColor, name, book);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnAuthority::Code createResourceCode(Utf8StringCR name, Utf8CP nameSpace)
    {
    return SystemAuthority::CreateCode(SystemAuthority::Resource, name, nameSpace);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool validateResourceCode(DgnAuthority::Code const& code, Utf8CP nameSpace)
    {
    return code.GetAuthority() == SystemAuthority::GetId(SystemAuthority::Resource) && code.GetNamespace().Equals(nameSpace) && !code.GetValue().empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthority::Code LightDefinition::CreateLightDefinitionCode(Utf8StringCR name)
    {
    return createResourceCode(name, DGN_CLASSNAME_LightDefinition);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthority::Code ViewDefinition::CreateCode(Utf8StringCR name)
    {
    return createResourceCode(name, DGN_CLASSNAME_ViewDefinition);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ViewDefinition::IsValidCode(Code const& code)
    {
    return validateResourceCode(code, DGN_CLASSNAME_ViewDefinition);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthority::Code DgnTexture::CreateTextureCode(Utf8StringCR name)
    {
    // unnamed textures are supported.
    return name.empty() ? DgnAuthority::CreateDefaultCode() : createResourceCode(name, DGN_CLASSNAME_Texture);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthority::Code DgnTexture::_GenerateDefaultCode()
    {
    // unnamed textures are supported.
    return DgnAuthority::CreateDefaultCode();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthority::Code AnnotationTextStyle::CreateCodeFromName(Utf8CP nameCP)
    {
    Utf8String name;
    name.AssignOrClear(nameCP);

    return createResourceCode(name, DGN_CLASSNAME_AnnotationTextStyle);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   John.Gooding    12/2015
//---------------------------------------------------------------------------------------
DgnAuthority::Code LineStyleElement::CreateCodeFromName(Utf8CP nameCP)
    {
    Utf8String name;
    name.AssignOrClear(nameCP);

    return createResourceCode(name, DGN_CLASSNAME_LineStyle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthority::Code AnnotationFrameStyle::CreateCodeFromName(Utf8CP nameCP)
    {
    Utf8String name;
    name.AssignOrClear(nameCP);

    return createResourceCode(name, DGN_CLASSNAME_AnnotationFrameStyle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthority::Code AnnotationLeaderStyle::CreateCodeFromName(Utf8CP nameCP)
    {
    Utf8String name;
    name.AssignOrClear(nameCP);

    return createResourceCode(name, DGN_CLASSNAME_AnnotationLeaderStyle);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthority::Code TextAnnotationSeed::CreateCodeFromName(Utf8CP nameCP)
    {
    Utf8String name;
    name.AssignOrClear(nameCP);

    return createResourceCode(name, DGN_CLASSNAME_TextAnnotationSeed);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthority::Code DgnModel::CreateModelCode(Utf8StringCR modelName)
    {
    Utf8String trimmed(modelName);
    trimmed.Trim();

    return SystemAuthority::CreateCode(SystemAuthority::Model, trimmed);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::Code ComponentDef::CreateVariationCode(Utf8StringCR slnId)
    {
    return SystemAuthority::CreateCode(SystemAuthority::Component, slnId, GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ComponentDef::IsCapturedSolutionCode(DgnElement::Code const& icode)
    {
    return icode.GetAuthority() == SystemAuthority::GetId(SystemAuthority::Component);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthority::Code DgnAuthority::CreateDefaultCode()
    {
    // The default code is not unique and has no special meaning.
    return SystemAuthority::CreateCode(SystemAuthority::Local, "");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnAuthority::Code::From(DgnAuthorityId id, Utf8StringCR value, Utf8StringCR nameSpace)
    {
    m_authority = id;
    m_value = value;
    m_nameSpace = nameSpace;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool AuthorityIssuedCode::operator<(AuthorityIssuedCode const& rhs) const
    {
    if (GetAuthority().GetValueUnchecked() != rhs.GetAuthority().GetValueUnchecked())
        return GetAuthority().GetValueUnchecked() < rhs.GetAuthority().GetValueUnchecked();

    int cmp = GetValue().CompareTo(rhs.GetValue());
    if (0 != cmp)
        return cmp < 0;

    return GetNamespace().CompareTo(rhs.GetNamespace()) < 0;
    }

