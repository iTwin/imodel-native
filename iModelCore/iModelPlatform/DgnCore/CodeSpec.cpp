/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpecId DgnCodeSpecs::QueryCodeSpecId (Utf8CP name) const
    {
    Statement stmt (m_dgndb, "SELECT Id FROM " BIS_TABLE(BIS_CLASS_CodeSpec) " WHERE Name=?");
    stmt.BindText (1, name, Statement::MakeCopy::No);
    return BE_SQLITE_ROW == stmt.Step() ? stmt.GetValueId<CodeSpecId>(0) : CodeSpecId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CodeSpec::Insert()
    {
    return GetDgnDb().CodeSpecs().Insert(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus CodeSpec::CloneCodeForImport(DgnCodeR code, DgnElementCR srcElem, DgnModelR destModel, DgnImportContext& importer) const
    {
    code = importer.IsBetweenDbs() ? srcElem.GetCode() : DgnCode();
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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

    BeJsDocument props;
    ToPropertiesJson(props);
    clone->FromPropertiesJson(props);

    status = DgnDbStatus::Success;
    return clone;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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

    Statement stmt(m_dgndb, "INSERT INTO " BIS_TABLE(BIS_CLASS_CodeSpec) " (Id,Name,JsonProperties) VALUES(?,?,?)");
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
* @bsimethod
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
    m_dgndb.GetCachedStatement(stmt, "SELECT Name,JsonProperties FROM " BIS_TABLE(BIS_CLASS_CodeSpec) " WHERE Id=?");
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpecCPtr DgnCodeSpecs::GetCodeSpec(Utf8CP name)
    {
    // good chance it's already loaded - check there before running a query
    auto found = std::find_if(m_loadedCodeSpecs.begin(), m_loadedCodeSpecs.end(), [&name](CodeSpecPtr const& arg) { return arg->GetName().Equals(name); });
    return m_loadedCodeSpecs.end() != found ? (*found).get() : GetCodeSpec(QueryCodeSpecId(name));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpec::CodeSpec(CreateParams const& params) : m_dgndb(params.m_dgndb), m_codeSpecId(params.m_id), m_name(params.m_name), m_scopeSpec(params.m_scopeSpec) {
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void CodeSpec::ToPropertiesJson(BeJsValue json) const {
    if (!m_specProperties.empty())
        json[json_spec()].From(m_specProperties);

    json[json_version()] = BeVersion(1, 0).ToMajorMinorString();
    m_scopeSpec.ToJSON(json[json_scopeSpec()]);
}

/*---------------------------------------------------------------------------------**/ /**
 * @bsimethod
 +---------------+---------------+---------------+---------------+---------------+------*/
void CodeSpec::FromPropertiesJson(BeJsConst json) {
    m_specProperties.From(json[json_spec()]);
    m_scopeSpec.FromJSON(json[json_scopeSpec()]);
}

/*---------------------------------------------------------------------------------**/ /**
 * @bsimethod
 +---------------+---------------+---------------+---------------+---------------+------*/
void CodeSpec::ReadProperties(Utf8StringCR jsonStr) {
    FromPropertiesJson(BeJsDocument(jsonStr));
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String CodeSpec::SerializeProperties() const
    {
    BeJsDocument props;
    ToPropertiesJson(props);
    return props.Stringify();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpecPtr CodeSpec::Create(DgnDbR db, Utf8CP codeSpecName, CodeScopeSpecCR scopeSpec)
    {
    CreateParams params(db, codeSpecName, CodeSpecId(), scopeSpec);
    return dgn_CodeSpecHandler::CodeSpec::GetHandler().Create(params).get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpecPtr CodeSpec::Create(DgnDbR db, Utf8CP codeSpecName, BeJsConst jsonProperties)
    {
    if (!jsonProperties.isMember(CodeSpec::json_spec()) && !jsonProperties.isMember(CodeSpec::json_scopeSpec())) // quick sanity check of incoming JSON
        return nullptr;

    CreateParams params(db, codeSpecName, CodeSpecId());
    CodeSpecPtr codeSpec = dgn_CodeSpecHandler::CodeSpec::GetHandler().Create(params).get();
    if (codeSpec.IsValid())
        codeSpec->FromPropertiesJson(jsonProperties);

    return codeSpec;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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

        case CodeScopeSpec::Type::RelatedElement: // WIP: evaulate relationship
        default:
            BeAssert(false);
            return DgnElementId();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_ModelSelector, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_PhysicalMaterial, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_PhysicalType, CodeScopeSpec::CreateModelScope())) ||
        (DgnDbStatus::Success != insertCodeSpec(*this, BIS_CODESPEC_RenderMaterial, CodeScopeSpec::CreateModelScope())) ||
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode CodeSpec::CreateCode(DgnDbR db, Utf8CP codeSpecName, Utf8StringCR value)
    {
    CodeSpecCPtr codeSpec = db.CodeSpecs().GetCodeSpec(codeSpecName);
    BeAssert(codeSpec.IsValid());
    return codeSpec.IsValid() ? codeSpec->CreateCode(value) : DgnCode();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode CodeSpec::CreateCode(Utf8StringCR value) const
    {
    BeAssert(IsRepositoryScope());
    return IsRepositoryScope() ? DgnCode(GetCodeSpecId(), GetDgnDb().Elements().GetRootSubjectId(), value) : DgnCode();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode CodeSpec::CreateCode(Utf8CP codeSpecName, DgnModelCR scopeModel, Utf8StringCR value)
    {
    CodeSpecCPtr codeSpec = scopeModel.GetDgnDb().CodeSpecs().GetCodeSpec(codeSpecName);
    BeAssert(codeSpec.IsValid() && codeSpec->IsModelScope());
    return codeSpec.IsValid() ? codeSpec->CreateCode(scopeModel, value) : DgnCode();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode CodeSpec::CreateCode(Utf8CP codeSpecName, DgnElementCR scopeElement, Utf8StringCR value)
    {
    CodeSpecCPtr codeSpec = scopeElement.GetDgnDb().CodeSpecs().GetCodeSpec(codeSpecName);
    BeAssert(codeSpec.IsValid());
    return codeSpec.IsValid() ? codeSpec->CreateCode(scopeElement, value) : DgnCode();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode CodeSpec::CreateCode(DgnModelCR scopeModel, Utf8StringCR value) const
    {
    DgnElementCPtr scopeElement = scopeModel.GetModeledElement();
    BeAssert(scopeElement.IsValid() && IsModelScope());
    return scopeElement.IsValid() && !value.empty() ? CreateCode(*scopeElement, value) : DgnCode();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode CodeSpec::CreateCode(DgnElementCR scopeElement, Utf8StringCR value) const {
    if (value.empty())
        return DgnCode();

    return scopeElement.GetElementId().IsValid() ? DgnCode(GetCodeSpecId(), scopeElement.GetElementId(), value) : DgnCode();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8Char DgnCodeValue::Fold(Utf8Char ch) {
    constexpr Utf8Char foldDelta = 'a' - 'A';
    if (ch >= 'A' && ch <= 'Z')
        ch += foldDelta;

    return ch;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCodeValue::CompareResult DgnCodeValue::Compare(Utf8Char lhs, Utf8Char rhs) {
    lhs = Fold(lhs);
    rhs = Fold(rhs);
    return lhs == rhs ? CompareResult::Equal : (lhs < rhs ? CompareResult::Less : CompareResult::Greater);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCodeValue::CompareResult DgnCodeValue::Compare(Utf8StringCR lhs, Utf8StringCR rhs)
    {
    size_t lhsSize = lhs.size(),
           rhsSize = rhs.size(),
           minSize = std::min(lhsSize, rhsSize);

    for (size_t i = 0; i < minSize; i++)
        {
        CompareResult cmp = Compare(lhs[i], rhs[i]);
        if (CompareResult::Equal != cmp)
            return cmp;
        }

    return lhsSize == rhsSize ? CompareResult::Equal : (lhsSize < rhsSize ? CompareResult::Less : CompareResult::Greater);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnCodeValue::Equals(Utf8CP str) const
    {
    for (size_t i = 0; i < size(); i++)
        {
        if (0 == str[i] || CompareResult::Equal != Compare(m_value[i], str[i]))
            return false;
        }

    return 0 == str[size()];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode DgnCode::CreateEmpty()
    {
    return DgnCode(CodeSpec::GetNullCodeSpecId(), DgnElementId((uint64_t)1LL), ""); // codeScope=RootSubject, codeValue=null
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId DgnCode::GetScopeElementId(DgnDbR db) const
    {
    uint64_t scopeElementId;
    if (BentleyStatus::SUCCESS == BeStringUtilities::ParseUInt64(scopeElementId, m_scope.c_str()))
        return DgnElementId(scopeElementId);

    return DgnElementId((uint64_t)1LL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnCode::RelocateToDestinationDb(DgnImportContext& importer)
    {
    m_specId = importer.RemapCodeSpecId(m_specId);

    uint64_t scopeElementId;
    if (BentleyStatus::SUCCESS == BeStringUtilities::ParseUInt64(scopeElementId, m_scope.c_str()))
        m_scope = importer.FindElementId(DgnElementId(scopeElementId)).ToHexStr();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnCode::operator<(DgnCodeCR rhs) const {
    if (GetCodeSpecId().GetValueUnchecked() != rhs.GetCodeSpecId().GetValueUnchecked())
        return GetCodeSpecId().GetValueUnchecked() < rhs.GetCodeSpecId().GetValueUnchecked();

    switch (GetValue().CompareTo(rhs.GetValue())) {
    case DgnCodeValue::CompareResult::Less:
        return true;
    case DgnCodeValue::CompareResult::Greater:
        return false;
    }

    return GetScopeString().CompareTo(rhs.GetScopeString()) < 0;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnCode::ToJson(BeJsValue val) const {
    val[json_spec()] = m_specId;
    val[json_scope()] = m_scope;
    val[json_value()] = m_value.GetUtf8();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode DgnCode::FromJson(BeJsConst value, DgnDbCR db) {
    DgnCode val = CreateEmpty();
    val.m_value = value[json_value()].asString();

    auto specJson = value[json_spec()];
    val.m_specId = CodeSpecId(specJson.asUInt64());
    if (!val.m_specId.IsValid()) {
        // the code spec id supplied wasn't valid. See if maybe they passed the name of the codeSpec.
        // if that doesn't work, use the id of the Null code spec.
        auto specPtr = db.CodeSpecs().GetCodeSpec(specJson.asCString());
        val.m_specId = specPtr.IsValid() ? specPtr->GetCodeSpecId() : CodeSpec::GetNullCodeSpecId();
    }

    // the code scope must refer to an existing element. The json may have an ElementId or FederationGuid
    auto scopeJson = value[json_scope()];
    auto scopeStr = scopeJson.asCString();

    // Note: this is necessary since GetId64 parses strings that start with number successfully and can get confused with Guids.
    // ElementIds in JSON must start with "0x"
    auto validId = scopeStr && (scopeStr[0] == '0' && (scopeStr[1] == 'X' || scopeStr[1] == 'x'));

    auto scopeId = scopeJson.GetId64<DgnElementId>();
    if (!validId || !db.Elements().ElementExists(scopeId)) {
        BeGuid scopeFederationGuid;
        DgnElementCPtr scopeEl;
        if (BentleyStatus::SUCCESS == scopeFederationGuid.FromString(scopeJson.asCString()))
            scopeEl = db.Elements().QueryElementByFederationGuid(scopeFederationGuid);
        if (!scopeEl.IsValid())
            throw std::invalid_argument("invalid code scope");// the value didn't refer to an element either by ElementId or FederationGuid

        scopeId = scopeEl->GetElementId();
    }
    val.m_scope = scopeId.ToHexStr();
    return val;
}
