/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/CodeSpec.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpecId DgnCodeSpecs::QueryCodeSpecId (Utf8CP name) const
    {
    Statement stmt (m_dgndb, "SELECT Id FROM " BIS_TABLE(BIS_CLASS_CodeSpec) " WHERE Name=?");
    stmt.BindText (1, name, Statement::MakeCopy::No);
    return BE_SQLITE_ROW == stmt.Step() ? stmt.GetValueId<CodeSpecId>(0) : CodeSpecId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpecId DgnImportContext::_RemapCodeSpecId(CodeSpecId source)
    {
    if (!IsBetweenDbs())
        return source;

    CodeSpecId dest = m_remap.Find(source);
    if (dest.IsValid())
        return dest;

    CodeSpecCPtr sourceCodeSpec = m_sourceDb.CodeSpecs().GetCodeSpec(source);
    if (sourceCodeSpec.IsNull())
        {
        BeDataAssert(false && "Missing source CodeSpec");
        return source;
        }

    dest = m_destDb.CodeSpecs().QueryCodeSpecId(sourceCodeSpec->GetName().c_str());
    if (!dest.IsValid())
        {
        CodeSpecPtr destCodeSpec = CodeSpec::Import(nullptr, *sourceCodeSpec, *this);
        if (destCodeSpec.IsNull())
            {
            BeDataAssert(false && "Invalid source CodeSpec");
            return source;
            }
        else
            {
            dest = destCodeSpec->GetCodeSpecId();
            }
        }

    return m_remap.Add(source, dest);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CodeSpec::Insert()
    {
    return GetDgnDb().CodeSpecs().Insert(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CodeSpec::CloneCodeForImport(DgnCodeR code, DgnElementCR srcElem, DgnModelR destModel, DgnImportContext& importer) const
    {
    code = importer.IsBetweenDbs() ? srcElem.GetCode() : DgnCode();
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpecPtr CodeSpec::Import(DgnDbStatus* outResult, CodeSpecCR src, DgnImportContext& importer)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT (status, outResult);

    BeAssert(&src.GetDgnDb() == &importer.GetSourceDb());
    if (!importer.IsBetweenDbs())
        {
        status = DgnDbStatus::DuplicateName;
        return nullptr;
        }

    CodeSpecPtr dest = src.CloneForImport(&status, importer);
    if (dest.IsNull() || DgnDbStatus::Success != (status = dest->Insert()))
        return nullptr;

    importer.AddCodeSpecId(src.GetCodeSpecId(), dest->GetCodeSpecId());

    status = DgnDbStatus::Success;
    return dest;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpecPtr CodeSpec::CloneForImport(DgnDbStatus* outResult, DgnImportContext& importer) const
    {
    DgnDbStatus ALLOW_NULL_OUTPUT (status, outResult);

    if (importer.GetDestinationDb().CodeSpecs().QueryCodeSpecId(GetName().c_str()).IsValid())
        {
        status = DgnDbStatus::DuplicateName;
        return nullptr;
        }

    auto classId = GetClassId();
    if (importer.IsBetweenDbs())
        classId = importer.RemapClassId(classId);

    CreateParams params(importer.GetDestinationDb(), classId, GetName().c_str());
    CodeSpecPtr clone = GetCodeSpecHandler().Create(params);
    if (!clone.IsValid())
        {
        status = DgnDbStatus::NotFound; // better error code...?
        return nullptr;
        }

    Json::Value props(Json::objectValue);
    ToPropertiesJson(props);
    clone->FromPropertiesJson(props);

    status = DgnDbStatus::Success;
    return clone;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnCodeSpecs::Insert(CodeSpecR codeSpec)
    {
    if (codeSpec.GetCodeSpecId().IsValid())
        return DgnDbStatus::IdExists;

    codeSpec.m_name.Trim();
    if (QueryCodeSpecId(codeSpec.GetName().c_str()).IsValid())
        return DgnDbStatus::DuplicateName;

    CodeSpecId newId;
    auto status = m_dgndb.GetServerIssuedId(newId, BIS_TABLE(BIS_CLASS_CodeSpec), "Id");
    if (BE_SQLITE_OK != status)
        return DgnDbStatus::WriteError; // NEEDSWORK...can we communicate this more meaningfully?

    Utf8String propsStr = codeSpec.SerializeProperties();

    Statement stmt(m_dgndb, "INSERT INTO " BIS_TABLE(BIS_CLASS_CodeSpec) " (Id,Name,Properties,ECClassId) VALUES(?,?,?,?)");
    stmt.BindId(1, newId);
    stmt.BindText(2, codeSpec.GetName(), Statement::MakeCopy::No);
    stmt.BindText(3, propsStr, Statement::MakeCopy::No);
    stmt.BindId(4, codeSpec.GetClassId());

    if (BE_SQLITE_DONE != stmt.Step())
        return DgnDbStatus::WriteError;

    codeSpec.m_codeSpecId = newId;

    BeDbMutexHolder _v(m_mutex);
    m_loadedCodeSpecs.push_back(CodeSpecPtr(&codeSpec));

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpecPtr DgnCodeSpecs::LoadCodeSpec(CodeSpecId id, DgnDbStatus* outResult)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT (status, outResult);

    if (!id.IsValid())
        {
        status = DgnDbStatus::InvalidId;
        return nullptr;
        }

    CachedStatementPtr stmt;
    m_dgndb.GetCachedStatement(stmt, "SELECT Name,Properties,ECClassId FROM " BIS_TABLE(BIS_CLASS_CodeSpec) " WHERE Id=?");
    stmt->BindId(1, id);

    if (BE_SQLITE_ROW != stmt->Step())
        {
        status = DgnDbStatus::InvalidId;
        return nullptr;
        }

    Utf8String name = stmt->GetValueText(0);
    Utf8String props = stmt->GetValueText(1);
    DgnClassId classId = stmt->GetValueId<DgnClassId>(2);

    CodeSpecHandlerP handler = dgn_CodeSpecHandler::CodeSpec::FindHandler(m_dgndb, classId);
    if (nullptr == handler)
        {
        status = DgnDbStatus::MissingHandler;
        return nullptr;
        }

    CodeSpec::CreateParams params(m_dgndb, classId, name.c_str(), id);
    CodeSpecPtr codeSpec = handler->Create(params);
    if (codeSpec.IsNull())
        {
        status = DgnDbStatus::NotFound;
        return nullptr;
        }

    codeSpec->ReadProperties(props);

    status = DgnDbStatus::Success;
    return codeSpec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpecCPtr DgnCodeSpecs::GetCodeSpec(CodeSpecId id)
    {
    if (!id.IsValid())
        return nullptr;

    CodeSpecPtr codeSpec;

    BeDbMutexHolder _v(m_mutex);
    auto found = std::find_if(m_loadedCodeSpecs.begin(), m_loadedCodeSpecs.end(), [&id](CodeSpecPtr const& arg) { return arg->GetCodeSpecId() == id; });
    if (m_loadedCodeSpecs.end() != found)
        {
        codeSpec = *found;
        }
    else
        {
        codeSpec = LoadCodeSpec(id, nullptr);
        BeDataAssert(codeSpec.IsValid());
        if (codeSpec.IsValid())
            m_loadedCodeSpecs.push_back(codeSpec);
        }

    return codeSpec.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpecCPtr DgnCodeSpecs::GetCodeSpec(Utf8CP name)
    {
    // good chance it's already loaded - check there before running a query
    auto found = std::find_if(m_loadedCodeSpecs.begin(), m_loadedCodeSpecs.end(), [&name](CodeSpecPtr const& arg) { return arg->GetName().Equals(name); });
    return m_loadedCodeSpecs.end() != found ? (*found).get() : GetCodeSpec(QueryCodeSpecId(name));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpec::CodeSpec(CreateParams const& params)
    : m_dgndb(params.m_dgndb), m_codeSpecId(params.m_id), m_classId(params.m_classId), m_name(params.m_name), m_scopeSpec(params.m_scopeSpec), m_registrySuffix(params.m_registrySuffix)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void CodeSpec::ToPropertiesJson(JsonValueR json) const
    {
    Json::Value fragmentSpecArray(Json::arrayValue);
    for (CodeFragmentSpecCR fragmentSpec : GetFragmentSpecs())
        fragmentSpecArray.append(fragmentSpec.ToJson());

    if (fragmentSpecArray.size() > 0)
        json["fragmentSpecs"] = fragmentSpecArray;

    json["specVersion"] = BeVersion(1, 0).ToMajorMinorString();
    json["scopeSpecType"] = (int)GetScope().GetType();

    if (!m_registrySuffix.empty())
        json["registrySuffix"] = m_registrySuffix;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void CodeSpec::FromPropertiesJson(JsonValueCR json)
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

    JsonValueCR registrySuffixJson = json["registrySuffix"];
    if (!registrySuffixJson.isNull())
        m_registrySuffix = registrySuffixJson.asCString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void CodeSpec::ReadProperties(Utf8StringCR jsonStr)
    {
    Json::Value props(Json::objectValue);
    if (Json::Reader::Parse(jsonStr, props))
        FromPropertiesJson(props);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CodeSpec::SerializeProperties() const
    {
    Json::Value props(Json::objectValue);
    ToPropertiesJson(props);
    return Json::FastWriter::ToString(props);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpecHandlerP dgn_CodeSpecHandler::CodeSpec::FindHandler(DgnDbCR dgndb, DgnClassId classId)
    {
    DgnDomain::Handler* handler = dgndb.Domains().LookupHandler(classId);
    if (nullptr == handler)
        handler = dgndb.Domains().FindHandler(classId, dgndb.Domains().GetClassId(GetHandler()));

    return nullptr != handler ? handler->_ToCodeSpecHandler() : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpecHandlerR CodeSpec::GetCodeSpecHandler() const
    {
    return *dgn_CodeSpecHandler::CodeSpec::FindHandler(m_dgndb, m_classId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    01/17
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpecPtr CodeSpec::Create(DgnDbR db, Utf8CP codeSpecName, CodeScopeSpecCR scopeSpec, Utf8CP registrySuffix)
    {
    CodeSpecHandlerR handler = dgn_CodeSpecHandler::CodeSpec::GetHandler();
    CreateParams params(db, db.Domains().GetClassId(handler), codeSpecName, CodeSpecId(), scopeSpec, registrySuffix);
    return handler.Create(params).get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    01/17
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnDbStatus insertCodeSpec(DgnDbR db, Utf8CP name, CodeScopeSpecCR scope, CodeSpecId expectedId=CodeSpecId())
    {
    CodeSpecPtr codeSpec = CodeSpec::Create(db, name, scope);
    if (!codeSpec.IsValid())
        return DgnDbStatus::InvalidCodeSpec;

    DgnDbStatus insertStatus = codeSpec->Insert();
    if (expectedId.IsValid() && (expectedId != codeSpec->GetCodeSpecId()))
        return DgnDbStatus::InvalidId;

    return insertStatus;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::CreateCodeSpecs()
    {
    if (// CodeSpecs with Repository scope
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_NullCodeSpec, CodeScopeSpec::CreateRepositoryScope(), CodeSpec::GetNullCodeSpecId())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_AnnotationFrameStyle, CodeScopeSpec::CreateRepositoryScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_AnnotationLeaderStyle, CodeScopeSpec::CreateRepositoryScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_AnnotationTextStyle, CodeScopeSpec::CreateRepositoryScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_CategorySelector, CodeScopeSpec::CreateRepositoryScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_DisplayStyle, CodeScopeSpec::CreateRepositoryScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_GeometryPart, CodeScopeSpec::CreateRepositoryScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_LightDefinition, CodeScopeSpec::CreateRepositoryScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_LineStyle, CodeScopeSpec::CreateRepositoryScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_MaterialElement, CodeScopeSpec::CreateRepositoryScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_ModelSelector, CodeScopeSpec::CreateRepositoryScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_Session, CodeScopeSpec::CreateRepositoryScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_SpatialCategory, CodeScopeSpec::CreateRepositoryScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_TextAnnotationSeed, CodeScopeSpec::CreateRepositoryScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_Texture, CodeScopeSpec::CreateRepositoryScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_TrueColor, CodeScopeSpec::CreateRepositoryScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_ViewDefinition, CodeScopeSpec::CreateRepositoryScope())) || 
        // CodeSpecs with Model scope
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_Drawing, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_DrawingCategory, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_LinkElement, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_Sheet, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_PhysicalType, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_PhysicalRecipe, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_GraphicalType2d, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_GraphicalRecipe2d, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_SpatialLocationType, CodeScopeSpec::CreateModelScope())) ||
        // CodeSpecs with ParentElement scope
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_InformationPartitionElement, CodeScopeSpec::CreateParentElementScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_SubCategory, CodeScopeSpec::CreateParentElementScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_Subject, CodeScopeSpec::CreateParentElementScope())))
        {
        BeAssert(false);
        return BE_SQLITE_ERROR;
        }

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    01/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode CodeSpec::CreateCode(DgnDbR db, Utf8CP codeSpecName, Utf8StringCR value, Utf8StringCR scope)
    {
    CodeSpecCPtr codeSpec = db.CodeSpecs().GetCodeSpec(codeSpecName);
    BeAssert(codeSpec.IsValid());
    return codeSpec.IsValid() ? codeSpec->CreateCode(value, scope) : DgnCode();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    01/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode CodeSpec::CreateCode(DgnDbR db, Utf8CP codeSpecName, Utf8StringCR value)
    {
    CodeSpecCPtr codeSpec = db.CodeSpecs().GetCodeSpec(codeSpecName);
    BeAssert(codeSpec.IsValid());
    return codeSpec.IsValid() ? codeSpec->CreateCode(value) : DgnCode();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    01/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode CodeSpec::CreateCode(Utf8StringCR value) const
    {
    BeAssert(IsRepositoryScope());
    return IsRepositoryScope() ? DgnCode(GetCodeSpecId(), value, GetDgnDb().Elements().GetRootSubjectId()) : DgnCode();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    01/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode CodeSpec::CreateCode(Utf8CP codeSpecName, DgnElementCR scopeElement, Utf8StringCR value)
    {
    CodeSpecCPtr codeSpec = scopeElement.GetDgnDb().CodeSpecs().GetCodeSpec(codeSpecName);
    BeAssert(codeSpec.IsValid());
    return codeSpec.IsValid() ? codeSpec->CreateCode(scopeElement, value) : DgnCode();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    01/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode CodeSpec::CreateCode(DgnElementCR scopeElement, Utf8StringCR value) const
    {
    BeAssert(scopeElement.GetElementId().IsValid());
    return scopeElement.GetElementId().IsValid() ? DgnCode(GetCodeSpecId(), value, scopeElement.GetElementId()) : DgnCode();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode DgnCode::CreateEmpty()
    {
    return DgnCode(CodeSpec::GetNullCodeSpecId(), "", "");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode::DgnCode(CodeSpecId id, Utf8StringCR value, BeInt64Id scopeId) : m_codeSpecId(id), m_value(value)
    {
    Utf8Char buf[0x11] = { 0 };
    BeStringUtilities::FormatUInt64(buf, _countof(buf), scopeId.GetValue(), HexFormatOptions());
    m_scope.assign(buf);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnCode::From(CodeSpecId id, Utf8StringCR value, Utf8StringCR scope)
    {
    m_codeSpecId = id;
    m_value = value;
    m_scope = scope;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnCode::operator<(DgnCode const& rhs) const
    {
    if (GetCodeSpecId().GetValueUnchecked() != rhs.GetCodeSpecId().GetValueUnchecked())
        return GetCodeSpecId().GetValueUnchecked() < rhs.GetCodeSpecId().GetValueUnchecked();

    int cmp = GetValue().CompareTo(rhs.GetValue());
    if (0 != cmp)
        return cmp < 0;

    return GetScope().CompareTo(rhs.GetScope()) < 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CodeSpec::ValidateCode(DgnElementCR element) const
    {
    DgnCodeCR code = element.GetCode();
    if (!code.IsValid())
        return DgnDbStatus::InvalidName;

    if (IsModelScope() || IsParentElementScope())
        {
        if (code.GetValue().empty() || code.GetScope().empty())
            return DgnDbStatus::InvalidName;
        }

    BeAssert(code.GetCodeSpecId() == GetCodeSpecId());
    if (code.GetCodeSpecId() != GetCodeSpecId())
        return DgnDbStatus::InvalidCodeSpec;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP DgnCode::Iterator::Options::GetECSql() const
    {
#define SELECT_CODE_COLUMNS_FROM "SELECT CodeSpec.Id,CodeValue,CodeScope,ECInstanceId FROM "
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

    if (MIN_MinChars != GetMinChars())
        json["minChars"] = GetMinChars();

    if (MAX_MaxChars != GetMaxChars())
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
