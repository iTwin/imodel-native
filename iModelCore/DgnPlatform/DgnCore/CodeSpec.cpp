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

        dest = destCodeSpec->GetCodeSpecId();
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

    CreateParams params(importer.GetDestinationDb(), GetName().c_str());
    CodeSpecPtr clone = dgn_CodeSpecHandler::CodeSpec::GetHandler().Create(params);
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

    Statement stmt(m_dgndb, "INSERT INTO " BIS_TABLE(BIS_CLASS_CodeSpec) " (Id,Name,Properties) VALUES(?,?,?)");
    stmt.BindId(1, newId);
    stmt.BindText(2, codeSpec.GetName(), Statement::MakeCopy::No);
    stmt.BindText(3, propsStr, Statement::MakeCopy::No);

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
    DgnDbStatus ALLOW_NULL_OUTPUT(status, outResult);

    if (!id.IsValid())
        {
        status = DgnDbStatus::InvalidId;
        return nullptr;
        }

    CachedStatementPtr stmt;
    m_dgndb.GetCachedStatement(stmt, "SELECT Name,Properties FROM " BIS_TABLE(BIS_CLASS_CodeSpec) " WHERE Id=?");
    stmt->BindId(1, id);

    if (BE_SQLITE_ROW != stmt->Step())
        {
        status = DgnDbStatus::InvalidId;
        return nullptr;
        }

    Utf8String name = stmt->GetValueText(0);
    Utf8String props = stmt->GetValueText(1);

    CodeSpec::CreateParams params(m_dgndb, name.c_str(), id);
    CodeSpecPtr codeSpec = dgn_CodeSpecHandler::CodeSpec::GetHandler().Create(params);
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
    : m_dgndb(params.m_dgndb), m_codeSpecId(params.m_id), m_name(params.m_name), m_scopeSpec(params.m_scopeSpec)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void CodeSpec::ToPropertiesJson(JsonValueR json) const
    {
    if (!m_specProperties.empty())
        json[json_spec()] = m_specProperties;

    json[json_version()] = BeVersion(1, 0).ToMajorMinorString();
    json[json_scopeSpec()] = GetScope();

    Json::Value fragmentSpecArray(Json::arrayValue);
    for (CodeFragmentSpecCR fragmentSpec : GetFragmentSpecs())
        fragmentSpecArray.append(fragmentSpec);

    if (fragmentSpecArray.size() > 0)
        json[json_fragmentSpecs()] = fragmentSpecArray;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    01/17
+---------------+---------------+---------------+---------------+---------------+------*/
void CodeSpec::FromPropertiesJson(JsonValueCR json)
    {
    m_specProperties = json[json_spec()];
    SetScope((CodeScopeSpecCR)json[json_scopeSpec()]);

    JsonValueCR fragmentSpecArrayJson = json[json_fragmentSpecs()];
    if (!fragmentSpecArrayJson.isNull())
        {
        for (JsonValueCR fragmentSpecJson : fragmentSpecArrayJson)
            GetFragmentSpecsR().push_back((CodeFragmentSpecCR)fragmentSpecJson);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void CodeSpec::ReadProperties(Utf8StringCR jsonStr)
    {
    FromPropertiesJson(Json::Value::From(jsonStr));
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
* @bsimethod                                                    Shaun.Sewall    01/17
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpecPtr CodeSpec::Create(DgnDbR db, Utf8CP codeSpecName, CodeScopeSpecCR scopeSpec)
    {
    CreateParams params(db, codeSpecName, CodeSpecId(), scopeSpec);
    return dgn_CodeSpecHandler::CodeSpec::GetHandler().Create(params).get();
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
* @bsimethod                                                    Shaun.Sewall    03/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId CodeSpec::GetScopeElementId(DgnElementCR element) const
    {
    switch (GetScope().GetType())
        {
        case CodeScopeSpec::Type::Repository:            
            return element.GetDgnDb().Elements().GetRootSubjectId();          

        case CodeScopeSpec::Type::Model:            
            return element.GetModel()->GetModeledElementId(); 

        case CodeScopeSpec::Type::ParentElement:    
            return element.GetParentId();                     

        default:
            BeAssert(false);                                            
            return DgnElementId();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnDb::CreateCodeSpecs()
    {
    if (// CodeSpecs with Repository scope
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_NullCodeSpec, CodeScopeSpec::CreateRepositoryScope(), CodeSpec::GetNullCodeSpecId())) ||
        // CodeSpecs with Model scope
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_AnnotationFrameStyle, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_AnnotationLeaderStyle, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_AnnotationTextStyle, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_AuxCoordSystem2d, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_AuxCoordSystem3d, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_AuxCoordSystemSpatial, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_CategorySelector, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_ColorBook, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_DisplayStyle, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_Drawing, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_DrawingCategory, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_GeometryPart, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_GraphicalType2d, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_LineStyle, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_LinkElement, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_MaterialElement, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_ModelSelector, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_PhysicalType, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_Sheet, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_SpatialCategory, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_SpatialLocationType, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_TemplateRecipe2d, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_TemplateRecipe3d, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_TextAnnotationSeed, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_Texture, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_ViewDefinition, CodeScopeSpec::CreateModelScope())) || 
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
* @bsimethod                                                    Shaun.Sewall    03/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode CodeSpec::CreateCode(Utf8CP codeSpecName, DgnModelCR scopeModel, Utf8StringCR value)
    {
    CodeSpecCPtr codeSpec = scopeModel.GetDgnDb().CodeSpecs().GetCodeSpec(codeSpecName);
    BeAssert(codeSpec.IsValid() && codeSpec->IsModelScope());
    DgnElementCPtr scopeElement = scopeModel.GetModeledElement();
    return codeSpec.IsValid() && scopeElement.IsValid() ? codeSpec->CreateCode(*scopeElement, value) : DgnCode();
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
* @bsimethod                                                    Shaun.Sewall    04/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode CodeSpec::CreateCode(DgnModelCR scopeModel, Utf8StringCR value) const
    {
    BeAssert(scopeModel.GetModelId().IsValid() && IsModelScope());
    return scopeModel.GetModelId().IsValid() && !value.empty() ? DgnCode(GetCodeSpecId(), value, scopeModel.GetModelId()) : DgnCode();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    01/17
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode CodeSpec::CreateCode(DgnElementCR scopeElement, Utf8StringCR value) const
    {
    BeAssert(scopeElement.GetElementId().IsValid());
    return scopeElement.GetElementId().IsValid() && !value.empty() ? DgnCode(GetCodeSpecId(), value, scopeElement.GetElementId()) : DgnCode();
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
