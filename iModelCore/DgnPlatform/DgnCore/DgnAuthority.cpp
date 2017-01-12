/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnAuthority.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
DgnAuthorityId DgnImportContext::_RemapAuthorityId(DgnAuthorityId source)
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
    : m_dgndb(params.m_dgndb), m_authorityId(params.m_id), m_classId(params.m_classId), m_name(params.m_name), m_scopeSpec(params.m_scopeSpec)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnAuthority::_ToPropertiesJson(JsonValueR json) const
    {
    Json::Value fragmentSpecArray(Json::arrayValue);
    for (CodeFragmentSpecCR fragmentSpec : GetFragmentSpecs())
        fragmentSpecArray.append(fragmentSpec.ToJson());

    json["fragmentSpecs"] = fragmentSpecArray;
    json["scopeSpecType"] = (int)GetScope().GetType();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnAuthority::_FromPropertiesJson(JsonValueCR json)
    {
    JsonValueCR fragmentSpecArrayJson = json["fragmentSpecs"];
    if (!fragmentSpecArrayJson.isNull())
        {
        for (JsonValueCR fragmentSpecJson : fragmentSpecArrayJson)
            GetFragmentSpecsR().push_back(CodeFragmentSpec::FromJson(fragmentSpecJson));
        }

    JsonValueCR scopeSpecTypeJson = json["scopeSpecType"];
    if (!scopeSpecTypeJson.isNull())
        SetScope(CodeScopeSpec((CodeScopeSpec::Type)scopeSpecTypeJson.asInt()));
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
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus NullAuthority::Insert(DgnDbR db)
    {
    AuthorityHandlerR handler = dgn_AuthorityHandler::Null::GetHandler();
    CreateParams params(db, db.Domains().GetClassId(handler), BIS_AUTHORITY_NullAuthority);
    DgnAuthorityPtr authority = handler.Create(params);
    return authority.IsValid() ? authority->Insert() : DgnDbStatus::InvalidCodeAuthority;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    01/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthorityPtr DgnAuthority::Create(DgnDbR db, Utf8CP authorityName, CodeScopeSpecCR scopeSpec)
    {
    AuthorityHandlerR handler = dgn_AuthorityHandler::Authority::GetHandler();
    CreateParams params(db, db.Domains().GetClassId(handler), authorityName, DgnAuthorityId(), scopeSpec);
    return handler.Create(params).get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DatabaseScopeAuthorityPtr DatabaseScopeAuthority::Create(Utf8CP authorityName, DgnDbR dgndb)
    {
    AuthorityHandlerR hdlr = dgn_AuthorityHandler::DatabaseScope::GetHandler();
    CreateParams params(dgndb, dgndb.Domains().GetClassId(hdlr), authorityName, DgnAuthorityId(), CodeScopeSpec::CreateRepositoryScope());
    return static_cast<DatabaseScopeAuthority*>(hdlr.Create(params).get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
ModelScopeAuthorityPtr ModelScopeAuthority::Create(Utf8CP authorityName, DgnDbR db)
    {
    AuthorityHandlerR handler = dgn_AuthorityHandler::ModelScope::GetHandler();
    CreateParams params(db, db.Domains().GetClassId(handler), authorityName, DgnAuthorityId(), CodeScopeSpec::CreateModelScope());
    return static_cast<ModelScopeAuthority*>(handler.Create(params).get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
ElementScopeAuthorityPtr ElementScopeAuthority::Create(Utf8CP authorityName, DgnDbR db)
    {
    AuthorityHandlerR handler = dgn_AuthorityHandler::ElementScope::GetHandler();
    CreateParams params(db, db.Domains().GetClassId(handler), authorityName, DgnAuthorityId(), CodeScopeSpec::CreateParentElementScope());
    return static_cast<ElementScopeAuthority*>(handler.Create(params).get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode DatabaseScopeAuthority::CreateCode(Utf8CP authorityName, DgnDbCR db, Utf8StringCR value, Utf8StringCR nameSpace)
    {
    DatabaseScopeAuthorityCPtr authority = db.Authorities().Get<DatabaseScopeAuthority>(authorityName);
    BeAssert(authority.IsValid());
    return authority.IsValid() ? authority->CreateCode(value, nameSpace) : DgnCode();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode ModelScopeAuthority::CreateCode(Utf8CP authorityName, DgnModelCR model, Utf8StringCR value)
    {
    ModelScopeAuthorityCPtr authority = model.GetDgnDb().Authorities().Get<ModelScopeAuthority>(authorityName);
    BeAssert(authority.IsValid() && model.GetModelId().IsValid());
    return authority.IsValid() && model.GetModelId().IsValid() ? authority->CreateCode(model.GetModelId(), value) : DgnCode();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode ModelScopeAuthority::CreateCode(Utf8CP authorityName, DgnDbCR db, DgnModelId modelId, Utf8StringCR value)
    {
    ModelScopeAuthorityCPtr authority = db.Authorities().Get<ModelScopeAuthority>(authorityName);
    BeAssert(authority.IsValid() && db.Models().GetModel(modelId).IsValid());
    return authority.IsValid() && modelId.IsValid() ? authority->CreateCode(modelId, value) : DgnCode();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode ModelScopeAuthority::CreateCode(DgnModelCR model, Utf8StringCR value) const
    {
    return CreateCode(model.GetModelId(), value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode ElementScopeAuthority::CreateCode(Utf8CP authorityName, DgnElementCR scopeElement, Utf8StringCR value)
    {
    ElementScopeAuthorityCPtr authority = scopeElement.GetDgnDb().Authorities().Get<ElementScopeAuthority>(authorityName);
    BeAssert(authority.IsValid() && scopeElement.GetElementId().IsValid());
    return authority.IsValid() && scopeElement.GetElementId().IsValid() ? authority->CreateCode(scopeElement.GetElementId(), value) : DgnCode();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode ElementScopeAuthority::CreateCode(Utf8CP authorityName, DgnDbR db, DgnElementId scopeElementId, Utf8StringCR value)
    {
    ElementScopeAuthorityCPtr authority = db.Authorities().Get<ElementScopeAuthority>(authorityName);
    BeAssert(authority.IsValid() && db.Elements().GetElement(scopeElementId).IsValid());
    return authority.IsValid() && scopeElementId.IsValid() ? authority->CreateCode(scopeElementId, value) : DgnCode();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode ElementScopeAuthority::CreateCode(DgnElementCR scopeElement, Utf8StringCR value) const
    {
    return CreateCode(scopeElement.GetElementId(), value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnDbStatus insertDatabaseScopeAuthority(Utf8CP authorityName, DgnDbR db)
    {
    DatabaseScopeAuthorityPtr authority = DatabaseScopeAuthority::Create(authorityName, db);
    if (!authority.IsValid())
        return DgnDbStatus::InvalidCodeAuthority;

    return authority->Insert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnDbStatus insertModelScopeAuthority(Utf8CP authorityName, DgnDbR db)
    {
    ModelScopeAuthorityPtr authority = ModelScopeAuthority::Create(authorityName, db);
    if (!authority.IsValid())
        return DgnDbStatus::InvalidCodeAuthority;

    return authority->Insert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnDbStatus insertElementScopeAuthority(Utf8CP authorityName, DgnDbR db)
    {
    ElementScopeAuthorityPtr authority = ElementScopeAuthority::Create(authorityName, db);
    if (!authority.IsValid())
        return DgnDbStatus::InvalidCodeAuthority;

    return authority->Insert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::CreateAuthorities()
    {
    if ((DgnDbStatus::Success != NullAuthority::Insert(*this)) ||
        // DatabaseScopeAuthorities
        (DgnDbStatus::Success != insertDatabaseScopeAuthority(BIS_AUTHORITY_AnnotationFrameStyle, *this)) ||
        (DgnDbStatus::Success != insertDatabaseScopeAuthority(BIS_AUTHORITY_AnnotationLeaderStyle, *this)) ||
        (DgnDbStatus::Success != insertDatabaseScopeAuthority(BIS_AUTHORITY_AnnotationTextStyle, *this)) ||
        (DgnDbStatus::Success != insertDatabaseScopeAuthority(BIS_AUTHORITY_CategorySelector, *this)) ||
        (DgnDbStatus::Success != insertDatabaseScopeAuthority(BIS_AUTHORITY_DisplayStyle, *this)) ||
        (DgnDbStatus::Success != insertDatabaseScopeAuthority(BIS_AUTHORITY_GeometryPart, *this)) ||
        (DgnDbStatus::Success != insertDatabaseScopeAuthority(BIS_AUTHORITY_LightDefinition, *this)) ||
        (DgnDbStatus::Success != insertDatabaseScopeAuthority(BIS_AUTHORITY_LineStyle, *this)) ||
        (DgnDbStatus::Success != insertDatabaseScopeAuthority(BIS_AUTHORITY_MaterialElement, *this)) ||
        (DgnDbStatus::Success != insertDatabaseScopeAuthority(BIS_AUTHORITY_ModelSelector, *this)) ||
        (DgnDbStatus::Success != insertDatabaseScopeAuthority(BIS_AUTHORITY_Session, *this)) ||
        (DgnDbStatus::Success != insertDatabaseScopeAuthority(BIS_AUTHORITY_SpatialCategory, *this)) ||
        (DgnDbStatus::Success != insertDatabaseScopeAuthority(BIS_AUTHORITY_TextAnnotationSeed, *this)) ||
        (DgnDbStatus::Success != insertDatabaseScopeAuthority(BIS_AUTHORITY_Texture, *this)) ||
        (DgnDbStatus::Success != insertDatabaseScopeAuthority(BIS_AUTHORITY_TrueColor, *this)) ||
        (DgnDbStatus::Success != insertDatabaseScopeAuthority(BIS_AUTHORITY_ViewDefinition, *this)) || 
        // ModelScopeAuthorities
        (DgnDbStatus::Success != insertModelScopeAuthority(BIS_AUTHORITY_Drawing, *this)) ||
        (DgnDbStatus::Success != insertModelScopeAuthority(BIS_AUTHORITY_DrawingCategory, *this)) ||
        (DgnDbStatus::Success != insertModelScopeAuthority(BIS_AUTHORITY_LinkElement, *this)) ||
        (DgnDbStatus::Success != insertModelScopeAuthority(BIS_AUTHORITY_Sheet, *this)) ||
        // ElementScopeAuthorities
        (DgnDbStatus::Success != insertElementScopeAuthority(BIS_AUTHORITY_InformationPartitionElement, *this)) ||
        (DgnDbStatus::Success != insertElementScopeAuthority(BIS_AUTHORITY_SubCategory, *this)) ||
        (DgnDbStatus::Success != insertElementScopeAuthority(BIS_AUTHORITY_Subject, *this)))
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode DgnCode::CreateEmpty()
    {
    return NullAuthority::CreateCode();
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
DgnDbStatus DgnAuthority::_ValidateCode(DgnElementCR element) const
    {
    DgnCodeCR code = element.GetCode();
    if (!code.IsValid())
        return DgnDbStatus::InvalidName;

    BeAssert(code.GetAuthority() == GetAuthorityId());
    if (code.GetAuthority() != GetAuthorityId())
        return DgnDbStatus::InvalidCodeAuthority;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnAuthority::_RegenerateCode(DgnCodeR regeneratedCode, DgnElementCR element) const
    {
    regeneratedCode = element.GetCode(); 
    return DgnDbStatus::Success; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ModelScopeAuthority::_ValidateCode(DgnElementCR element) const
    {
    DgnCodeCR code = element.GetCode();
    if (code.GetValue().empty() || code.GetNamespace().empty())
        return DgnDbStatus::InvalidName;

    return T_Super::_ValidateCode(element);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ElementScopeAuthority::_ValidateCode(DgnElementCR element) const
    {
    DgnCodeCR code = element.GetCode();
    if (code.GetValue().empty() || code.GetNamespace().empty())
        return DgnDbStatus::InvalidName;

    return T_Super::_ValidateCode(element);
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
DgnDbStatus DgnAuthority::ValidateCode(DgnElementCR element) const
    {
    return GetAuthorityHandler()._IsRestrictedAction(RestrictedAction::ValidateCode) ? DgnDbStatus::MissingHandler : _ValidateCode(element);
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
DgnDbStatus DgnAuthority::RegenerateCode(DgnCodeR code, DgnElementCR element) const
    {
    return GetAuthorityHandler()._IsRestrictedAction(RestrictedAction::RegenerateCode) ? DgnDbStatus::MissingHandler : _RegenerateCode(code, element);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP DgnCode::Iterator::Options::GetECSql() const
    {
#define SELECT_CODE_COLUMNS_FROM "SELECT CodeAuthority.Id,CodeValue,CodeNamespace,ECInstanceId FROM "
#define SELECT_ELEMENT_CODES SELECT_CODE_COLUMNS_FROM BIS_SCHEMA(BIS_CLASS_Element)
#define EXCLUDE_EMPTY_CODES " WHERE CodeValue IS NOT NULL"

    return m_includeEmpty ? SELECT_ELEMENT_CODES : SELECT_ELEMENT_CODES EXCLUDE_EMPTY_CODES;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode::Iterator::Iterator(DgnDbR db, Options options)
    {
    Prepare(db, options.GetECSql(), 3);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    01/17
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value CodeFragmentSpec::ToJson() const
    {
    Json::Value json(Json::objectValue);
    json["type"] = (int)GetType();
    json["prompt"] = GetPrompt();
    json["inSequenceMask"] = IsInSequenceMask();
    json["minChars"] = GetMinChars();
    json["maxChars"] = GetMaxChars();

    switch (GetType())
        {
        case Type::FixedString:
            json["fixedString"] = m_param1;
            break;

        case Type::PropertyValue:
            json["propertyName"] = m_param1;
            break;

        case Type::ElementTypeCode:
        case Type::SequenceNumber:
            // no additional data to write
            break;

        default:
            BeAssert(false); // unexpected type
            break;
        }

    return json;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    01/17
+---------------+---------------+---------------+---------------+---------------+------*/
CodeFragmentSpec CodeFragmentSpec::FromJson(JsonValueCR json)
    {
    CodeFragmentSpec fragmentSpec;
    fragmentSpec.SetType((CodeFragmentSpec::Type)json["type"].asInt());
    fragmentSpec.SetPrompt(json["prompt"].asCString());
    fragmentSpec.SetInSequenceMask(json["inSequenceMask"].asBool());
    fragmentSpec.SetMinChars(json.get("minChars", MIN_MinChars).asInt());
    fragmentSpec.SetMaxChars(json.get("maxChars", MAX_MaxChars).asInt());

    switch (fragmentSpec.GetType())
        {
        case Type::FixedString:
            fragmentSpec.SetParam1(json["fixedString"].asCString());
            break;

        case Type::PropertyValue:
            fragmentSpec.SetParam1(json["propertyName"].asCString());
            break;

        case Type::ElementTypeCode:
        case Type::SequenceNumber:
            // no additional data to read
            break;

        default:
            BeAssert(false); // unexpected type
            break;
        }

    return fragmentSpec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    01/17
+---------------+---------------+---------------+---------------+---------------+------*/
CodeFragmentSpec CodeFragmentSpec::FromFixedString(Utf8CP fixedString, Utf8CP prompt, bool inSequenceMask)
    {
    if (!fixedString || !*fixedString)
        return CodeFragmentSpec();

    CodeFragmentSpec fragmentSpec;
    fragmentSpec.SetType(Type::FixedString);
    fragmentSpec.SetPrompt(prompt);
    fragmentSpec.SetInSequenceMask(inSequenceMask);
    fragmentSpec.SetMinChars(strlen(fixedString));
    fragmentSpec.SetMaxChars(strlen(fixedString));
    fragmentSpec.SetParam1(fixedString);
    return fragmentSpec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    01/17
+---------------+---------------+---------------+---------------+---------------+------*/
CodeFragmentSpec CodeFragmentSpec::FromElementTypeCode(Utf8CP prompt, bool inSequenceMask)
    {
    CodeFragmentSpec fragmentSpec;
    fragmentSpec.SetType(Type::ElementTypeCode);
    fragmentSpec.SetPrompt(prompt);
    fragmentSpec.SetInSequenceMask(inSequenceMask);
    return fragmentSpec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    01/17
+---------------+---------------+---------------+---------------+---------------+------*/
CodeFragmentSpec CodeFragmentSpec::FromSequenceNumber(Utf8CP prompt)
    {
    CodeFragmentSpec fragmentSpec;
    fragmentSpec.SetType(Type::SequenceNumber);
    fragmentSpec.SetPrompt(prompt);
    fragmentSpec.SetInSequenceMask(false); // by definition, the sequence number is not part of the sequence mask
    return fragmentSpec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    01/17
+---------------+---------------+---------------+---------------+---------------+------*/
CodeFragmentSpec CodeFragmentSpec::FromPropertyValue(Utf8CP propertyName, Utf8CP prompt, bool inSequenceMask)
    {
    if (!propertyName || !*propertyName)
        return CodeFragmentSpec();

    CodeFragmentSpec fragmentSpec;
    fragmentSpec.SetType(Type::PropertyValue);
    fragmentSpec.SetPrompt(prompt);
    fragmentSpec.SetInSequenceMask(inSequenceMask);
    fragmentSpec.SetParam1(propertyName);
    return fragmentSpec;
    }
