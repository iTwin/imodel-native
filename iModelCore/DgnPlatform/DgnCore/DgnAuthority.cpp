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
DgnAuthorityId DgnAuthorities::QueryAuthorityId (Utf8StringCR name) const
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


    DgnAuthorityPtr sourceAuthority = m_sourceDb.Authorities().LoadAuthority(source);
    if (sourceAuthority.IsNull())
        {
        BeDataAssert(false && "Missing source authority");
        return source;
        }

    dest = m_destDb.Authorities().QueryAuthorityId(sourceAuthority->GetName());
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
DgnElement::Code DgnAuthority::_CloneCodeForImport(DgnElementCR srcElem, DgnModelR destModel, DgnImportContext& importer) const
    {
    return importer.IsBetweenDbs() ? srcElem.GetCode() : DgnElement::Code();
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

    if (importer.GetDestinationDb().Authorities().QueryAuthorityId(GetName()).IsValid())
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
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnAuthorities::Update(DgnAuthorityR auth)
    {
    if (!auth.GetAuthorityId().IsValid())
        return DgnDbStatus::InvalidId;

    Utf8String props = auth.SerializeProperties();

    Statement stmt(m_dgndb, "UPDATE " DGN_TABLE(DGN_CLASSNAME_Authority) " SET Props=? WHERE Id=?");
    stmt.BindText(1, props, Statement::MakeCopy::No);
    stmt.BindId(2, auth.GetAuthorityId());

    return BE_SQLITE_DONE == stmt.Step() ? DgnDbStatus::Success : DgnDbStatus::WriteError;
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
DgnElement::Code DgnAuthority::GenerateDefaultCode(DgnElementCR el)
    {
    auto elemId = el.GetElementId();
    if (!elemId.IsValid())
        return DgnElement::Code();

    Utf8PrintfString val("%s%u-%u", el.GetElementClass()->GetName().c_str(), elemId.GetRepositoryId().GetValue(), (uint32_t)(0xffffffff & elemId.GetValue()));
    return DgnElement::Code(DgnAuthority::LocalId(), val);
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
DgnElement::Code NamespaceAuthority::CreateCode(Utf8StringCR authorityName, Utf8StringCR value, DgnDbCR dgndb)
    {
    auto authId = dgndb.Authorities().QueryAuthorityId(authorityName);
    return authId.IsValid() ? DgnAuthority::CreateCode(authId, value) : DgnElement::Code();
    }

