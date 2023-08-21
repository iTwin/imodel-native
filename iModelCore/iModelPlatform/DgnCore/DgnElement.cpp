/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include "BisCoreNames.h"
#include "ElementECInstanceAdapter.h"
#include <ECObjects/ECJsonUtilities.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool hasHandler(ECN::ECClassCR cls)
    {
    return !cls.GetCustomAttributeLocal("ClassHasHandler").IsNull();
    }


/*---------------------------------------------------------------------------------**/ /**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::CallJsPostHandler(Utf8CP methodName) const {
    auto jsDb = m_dgndb.GetJsIModelDb();
    if (jsDb) {
        BeJsNapiObject arg(jsDb->Env());
        arg[json_id()] = m_elementId;
        arg[json_model()] = m_modelId;
        arg[json_federationGuid()] = m_federationGuid.ToString();
        m_dgndb.CallJsHandlerMethod(m_classId, methodName, arg);
    }
}

/*---------------------------------------------------------------------------------**/ /**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::CallJsChildPreHandler(DgnElementCR child, Utf8CP methodName) const {
    auto jsDb = m_dgndb.GetJsIModelDb();
    if (jsDb && child.m_napiObj) {
        BeJsNapiObject arg(jsDb->Env());
        arg["parentId"] = m_elementId;
        ((Napi::Object) arg).Set("childProps", *child.m_napiObj);
        m_dgndb.CallJsHandlerMethod(m_classId, methodName, arg);
    }
}

/*---------------------------------------------------------------------------------**/ /**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::CallJsChildPostHandler(DgnElementCR child, Utf8CP methodName) const {
    auto jsDb = m_dgndb.GetJsIModelDb();
    if (jsDb) {
        BeJsNapiObject arg(jsDb->Env());
        arg["parentId"] = m_elementId;
        arg["childId"] = child.m_elementId;
        m_dgndb.CallJsHandlerMethod(m_classId, methodName, arg);
    }
}

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::CallJsSubModelHandler(DgnModelCR model, Utf8CP methodName) const {
    auto jsDb = m_dgndb.GetJsIModelDb();
    if (jsDb) {
        BeJsNapiObject arg(jsDb->Env());
        arg["subModelId"] = model.GetModelId();
        m_dgndb.CallJsHandlerMethod(m_classId, methodName, arg);
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelPtr DgnElement::GetModel() const
    {
    return m_dgndb.Models().GetModel(m_modelId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelPtr DgnElement::GetSubModel() const
    {
    // The DgnModelId value for the SubModel is the same as this element's DgnElementId value
    return m_dgndb.Models().GetModel(DgnModelId(GetElementId().GetValue()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId DgnElement::GetSubModelId() const
    {
    DgnModelPtr model = GetSubModel();
    return model.IsValid() ? model->GetModelId() : DgnModelId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_OnSubModelInsert(DgnModelCR model) const {
    bool isSubModeled = GetElementClass()->Is(BIS_ECSCHEMA_NAME, BIS_CLASS_ISubModeledElement);
    if (!isSubModeled) {
        BeAssert(false && "Only ECClasses that implement bis:ISubModeledElement can have SubModels");
        return DgnDbStatus::WrongElement;
    }

    if (model.m_napiObj) {
        auto arg = Napi::Object::New(model.m_napiObj->Env());
        arg.Set("subModelProps", *model.m_napiObj);
        m_dgndb.CallJsHandlerMethod(m_classId, "onSubModelInsert", arg);
    }

    return DgnDbStatus::Success;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::AppData* DgnElement::FindAppDataInternal(AppData::Key const& key) const
    {
    auto entry = m_appData.find(&key);
    return entry==m_appData.end() ? nullptr : entry->second.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::AddAppDataInternal(AppData::Key const& key, AppData* obj) const
    {
    auto entry = m_appData.Insert(&key, obj);
    if (entry.second)
        return;

    // we already had appdata for this key. Clean up old and save new.
    entry.first->second = obj;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnElement::DropAppData(AppData::Key const& key) const
    {
    BeMutexHolder lock(GetElementsMutex());
    return 0==m_appData.erase(&key) ? ERROR : SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::ReplaceAppData(AppData::Key const& key, AppData* data) const
    {
    BeMutexHolder lock(GetElementsMutex());
    m_appData.erase(&key);
    AddAppDataInternal(key, data);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_DeleteInDb() const
    {
    CachedStatementPtr stmt=GetDgnDb().Elements().GetStatement("DELETE FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE Id=?");
    stmt->BindId(1, m_elementId);

    switch (stmt->Step())
        {
        case BE_SQLITE_CONSTRAINT_FOREIGNKEY:
            return  DgnDbStatus::ForeignKeyConstraint;

        case BE_SQLITE_DONE:
            return DgnDbStatus::Success;
        }

    return DgnDbStatus::WriteError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP DgnElement::GetElementClass() const
    {
    return GetDgnDb().Schemas().GetClass(GetElementClassId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode DgnElement::_GenerateDefaultCode() const
    {
    return DgnCode::CreateEmpty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DateTime DgnElement::QueryLastModifyTime() const
    {
    ECSqlStatement stmt;
    stmt.Prepare(GetDgnDb(), "SELECT " BIS_ELEMENT_PROP_LastMod " FROM " BIS_SCHEMA(BIS_CLASS_Element) " WHERE " BIS_ELEMENT_PROP_ECInstanceId "=?");
    stmt.BindId(1, m_elementId);
    stmt.Step();
    return stmt.GetValueDateTime(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<class T> void DgnElement::CallAppData(T const& caller) const
    {
    BeMutexHolder lock(GetElementsMutex());

    for (auto entry=m_appData.begin(); entry!=m_appData.end(); )
        {
        if (DgnElement::AppData::DropMe::Yes == caller(*entry->second, *this))
            entry = m_appData.erase(entry);
        else
            ++entry;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_OnInsert() {
    ElementHandlerR elementHandler = GetElementHandler();
    if (elementHandler.GetDomain().IsReadonly())
        return DgnDbStatus::ReadOnlyDomain;

    if (m_parent.m_id.IsValid() != m_parent.m_relClassId.IsValid()) {
        BeAssert(false); // when m_parentId.IsValid, m_parentRelClassId must be a subclass of BisCore:ElementOwnsChildElements
        return DgnDbStatus::InvalidParent;
    }

    if (!m_code.IsValid()) {
        m_code = _GenerateDefaultCode();
        if (!m_code.IsValid())
            return DgnDbStatus::InvalidCode;
    }

    if (m_code.GetValueUtf8().length() > (size_t)IModelHubConstants::MaxCodeValueLength) {
        BeAssert(false);
        LOG.errorv("Element insert rejected because code value [%s] is too long. ECClass=%s", m_code.GetValueUtf8CP(), GetHandlerECClassName());
        return DgnDbStatus::InvalidCode;
    }

    if (GetDgnDb().Elements().QueryElementIdByCode(m_code).IsValid())
        return DgnDbStatus::DuplicateCode;

    {
        BeMutexHolder lock(GetElementsMutex());
        for (auto entry = m_appData.begin(); entry != m_appData.end(); ++entry) {
            DgnDbStatus stat = entry->second->_OnInsert(*this);
            if (DgnDbStatus::Success != stat)
                return stat;
        }
    }

    return GetModel()->_OnInsertElement(*this);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnElement::_GetInfoString(Utf8CP delimiter) const
    {
    Utf8String out = GetDisplayLabel() + delimiter;

    auto geom = ToGeometrySource();
    if (geom)
        {
        DgnCategoryCPtr category = DgnCategory::Get(GetDgnDb(), geom->GetCategoryId());
        if (category.IsValid())
            out += "Category: " + category->GetDisplayLabel() + delimiter;
        }
    return out + "Model: " + GetModel()->GetName();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DrawingGraphic::GetRepresentedElement() const
    {
    auto statement = GetDgnDb().GetPreparedECSqlStatement("SELECT TargetECInstanceId FROM " BIS_SCHEMA(BIS_REL_DrawingGraphicRepresentsElement) " WHERE SourceECInstanceId=?");
    if (!statement.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    statement->BindId(1, GetElementId());
    return BE_SQLITE_ROW == statement->Step() ? GetDgnDb().Elements().GetElement(statement->GetValueId<DgnElementId>(0)) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DrawingGraphic::_GetInfoString(Utf8CP delimiter) const
    {
    DgnElementCPtr source = GetRepresentedElement();
    return source.IsValid() ? source->GetInfoString(delimiter) : T_Super::_GetInfoString(delimiter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefinitionElement::_BindWriteParams(ECSqlStatement& stmt, ForInsert forInsert)
    {
    T_Super::_BindWriteParams(stmt, forInsert);
    auto stat = stmt.BindBoolean(stmt.GetParameterIndex(prop_IsPrivate()), IsPrivate());
    UNUSED_VARIABLE(stat);
    BeAssert(ECSqlStatus::Success == stat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DefinitionElement::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParamsCR params)
    {
    auto status = T_Super::_ReadSelectParams(stmt, params);
    if (DgnDbStatus::Success != status)
        return status;

    m_isPrivate = stmt.GetValueBoolean(params.GetSelectIndex(prop_IsPrivate()));

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefinitionElement::_ToJson(BeJsValue val, BeJsConst opts) const
    {
    T_Super::_ToJson(val, opts);
    val[json_isPrivate()] = m_isPrivate;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefinitionElement::_FromJson(BeJsConst val)
    {
    T_Super::_FromJson(val);
    if (val.isMember(json_isPrivate()))
        m_isPrivate = val[json_isPrivate()].asBool();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DefinitionElement::_CopyFrom(DgnElementCR el, CopyFromOptions const& opts)
    {
    T_Super::_CopyFrom(el, opts);

    auto& other = static_cast<DefinitionElementCR>(el);
    m_isPrivate = other.m_isPrivate;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DefinitionElement::_OnInsert()
    {
    // DefinitionElements can reside *only* in a DefinitionModel
    DgnDbStatus status = GetModel()->IsDefinitionModel() ? T_Super::_OnInsert() : DgnDbStatus::WrongModel;
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DefinitionModelPtr DefinitionElement::GetDefinitionModel() const
    {
    DgnModelPtr model = GetModel();
    BeAssert(model.IsValid());
    DefinitionModelP definitionModel = model.IsValid() ? model->ToDefinitionModelP() : nullptr;
    BeAssert(nullptr != definitionModel);
    return definitionModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Subject::_OnInsert()
    {
    // All Subjects (other than the Root Subject) must have a valid parent
    if ((GetElementId() != GetDgnDb().Elements().GetRootSubjectId()) && !GetParentId().IsValid())
        return DgnDbStatus::InvalidParent;

    // Subjects can only reside in the RepositoryModel
    return DgnModel::RepositoryModelId() == GetModel()->GetModelId() ? T_Super::_OnInsert() : DgnDbStatus::WrongModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Subject::_OnUpdate(DgnElementCR original)
    {
    // All Subjects (other than the Root Subject) must have a valid parent
    if ((GetElementId() != GetDgnDb().Elements().GetRootSubjectId()) && !GetParentId().IsValid())
        return DgnDbStatus::InvalidParent;

    return T_Super::_OnUpdate(original);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Subject::_OnDelete() const
    {
    if (GetDgnDb().Elements().GetRootSubjectId() == GetElementId())
        {
        BeAssert(false); // can't delete the root Subject
        return DgnDbStatus::WrongElement;
        }

    return T_Super::_OnDelete();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Subject::_OnSubModelInsert(DgnModelCR model) const
    {
    BeAssert(false && "A Subject is not directly modeled. Insert a child InformationPartitionElement to be modeled instead.");
    return DgnDbStatus::ElementBlockedChange;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode Subject::CreateCode(SubjectCR parentSubject, Utf8StringCR name)
    {
    return CodeSpec::CreateCode(BIS_CODESPEC_Subject, parentSubject, name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SubjectPtr Subject::Create(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description)
    {
    DgnDbR db = parentSubject.GetDgnDb();
    DgnModelId modelId = DgnModel::RepositoryModelId();
    DgnClassId classId = db.Domains().GetClassId(dgn_ElementHandler::Subject::GetHandler());
    DgnElementId parentId = parentSubject.GetElementId();
    DgnClassId parentRelClassId = db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_SubjectOwnsSubjects);

    if (!classId.IsValid() || !parentId.IsValid() || !parentRelClassId.IsValid() || name.empty())
        return nullptr;

    SubjectPtr subject = new Subject(CreateParams(db, modelId, classId, CreateCode(parentSubject, name), nullptr, parentId, parentRelClassId));
    if (description && *description)
        subject->SetDescription(description);

    return subject;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SubjectCPtr Subject::CreateAndInsert(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description)
    {
    SubjectPtr subject = Create(parentSubject, name, description);
    return subject.IsValid() ? parentSubject.GetDgnDb().Elements().Insert<Subject>(*subject) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::CreateParams InformationPartitionElement::InitCreateParams(SubjectCR parentSubject, Utf8StringCR name, DgnDomain::Handler& handler)
    {
    DgnDbR db = parentSubject.GetDgnDb();
    DgnModelId modelId = DgnModel::RepositoryModelId();
    DgnClassId classId = db.Domains().GetClassId(handler);
    DgnElementId parentId = parentSubject.GetElementId();
    DgnClassId parentRelClassId = db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_SubjectOwnsPartitionElements);
    DgnCode code = CreateCode(parentSubject, name);

    if (!parentId.IsValid() || !parentRelClassId.IsValid() || !classId.IsValid() || name.empty())
        modelId.Invalidate(); // mark CreateParams as invalid

    return CreateParams(db, modelId, classId, code, nullptr, parentId, parentRelClassId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InformationPartitionElementPtr InformationPartitionElement::Create(DgnClassId classId, SubjectCR parentSubject, Utf8StringCR name, Utf8CP description)
    {
    CreateParams createParams = InitCreateParams(parentSubject, name, dgn_ElementHandler::InformationPartition::GetHandler());
    createParams.m_classId = classId;
    if (!createParams.IsValid())
        return nullptr;

    InformationPartitionElementPtr partition = new InformationPartitionElement(createParams);
    if (description && *description)
        partition->SetDescription(description);

    return partition;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InformationPartitionElementCPtr InformationPartitionElement::CreateAndInsert(DgnClassId classId, SubjectCR parentSubject, Utf8StringCR name, Utf8CP description)
    {
    InformationPartitionElementPtr partition = Create(classId, parentSubject, name, description);
    return partition.IsValid() ? parentSubject.GetDgnDb().Elements().Insert<InformationPartitionElement>(*partition) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode InformationPartitionElement::CreateCode(SubjectCR parentSubject, Utf8StringCR name)
    {
    return CodeSpec::CreateCode(BIS_CODESPEC_InformationPartitionElement, parentSubject, name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode InformationPartitionElement::CreateUniqueCode(SubjectCR parentSubject, Utf8CP baseName)
    {
    DgnDbR db = parentSubject.GetDgnDb();
    DgnCode code = CreateCode(parentSubject, baseName);
    if (!db.Elements().QueryElementIdByCode(code).IsValid())
        return code;

    int counter=1;
    do  {
        Utf8PrintfString name("%s-%d", baseName, counter);
        code = CreateCode(parentSubject, name.c_str());
        counter++;
        } while (db.Elements().QueryElementIdByCode(code).IsValid());

    return code;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus InformationPartitionElement::_OnInsert()
    {
    if (!GetParentId().IsValid() || !GetDgnDb().Elements().Get<Subject>(GetParentId()).IsValid())
        {
        BeAssert(false && "InformationPartitionElements must be parented to a Subject");
        return DgnDbStatus::InvalidParent;
        }

    // InformationPartitionElements can only reside in the RepositoryModel
    return DgnModel::RepositoryModelId() == GetModel()->GetModelId() ? T_Super::_OnInsert() : DgnDbStatus::WrongModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus InformationPartitionElement::_OnUpdate(DgnElementCR original)
    {
    if (!GetParentId().IsValid() || !GetDgnDb().Elements().Get<Subject>(GetParentId()).IsValid())
        return DgnDbStatus::InvalidParent;

    return T_Super::_OnUpdate(original);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DefinitionPartition::_OnSubModelInsert(DgnModelCR model) const
    {
    // A DefinitionPartition can only be modeled by an DefinitionModel
    return model.IsDefinitionModel() ? T_Super::_OnSubModelInsert(model) : DgnDbStatus::ElementBlockedChange;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DefinitionPartitionPtr DefinitionPartition::Create(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description)
    {
    CreateParams createParams = InitCreateParams(parentSubject, name, dgn_ElementHandler::DefinitionPartition::GetHandler());
    if (!createParams.IsValid())
        return nullptr;

    DefinitionPartitionPtr partition = new DefinitionPartition(createParams);
    if (description && *description)
        partition->SetDescription(description);

    return partition;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DefinitionPartitionCPtr DefinitionPartition::CreateAndInsert(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description)
    {
    DefinitionPartitionPtr partition = Create(parentSubject, name, description);
    return partition.IsValid() ? parentSubject.GetDgnDb().Elements().Insert<DefinitionPartition>(*partition) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DocumentPartition::_OnSubModelInsert(DgnModelCR model) const
    {
    // A DocumentPartition can only be modeled by an InformationModel
    return model.IsInformationModel() ? T_Super::_OnSubModelInsert(model) : DgnDbStatus::ElementBlockedChange;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DocumentPartitionPtr DocumentPartition::Create(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description)
    {
    CreateParams createParams = InitCreateParams(parentSubject, name, dgn_ElementHandler::DocumentPartition::GetHandler());
    if (!createParams.IsValid())
        return nullptr;

    DocumentPartitionPtr partition = new DocumentPartition(createParams);
    if (description && *description)
        partition->SetDescription(description);

    return partition;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DocumentPartitionCPtr DocumentPartition::CreateAndInsert(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description)
    {
    DocumentPartitionPtr partition = Create(parentSubject, name, description);
    return partition.IsValid() ? parentSubject.GetDgnDb().Elements().Insert<DocumentPartition>(*partition) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GroupInformationPartition::_OnSubModelInsert(DgnModelCR model) const
    {
    if (nullptr == dynamic_cast<GroupInformationModelCP>(&model))
        {
        BeAssert(false && "A GroupInformationPartition can only be modeled by a GroupInformationModel");
        return DgnDbStatus::ElementBlockedChange;
        }

    return T_Super::_OnSubModelInsert(model);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GroupInformationPartitionPtr GroupInformationPartition::Create(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description)
    {
    CreateParams createParams = InitCreateParams(parentSubject, name, dgn_ElementHandler::GroupInformationPartition::GetHandler());
    if (!createParams.IsValid())
        return nullptr;

    GroupInformationPartitionPtr partition = new GroupInformationPartition(createParams);
    if (description && *description)
        partition->SetDescription(description);

    return partition;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GroupInformationPartitionCPtr GroupInformationPartition::CreateAndInsert(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description)
    {
    GroupInformationPartitionPtr partition = Create(parentSubject, name, description);
    return partition.IsValid() ? parentSubject.GetDgnDb().Elements().Insert<GroupInformationPartition>(*partition) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus InformationRecordPartition::_OnSubModelInsert(DgnModelCR model) const
    {
    // An InformationRecordPartition can only be modeled by an InformationRecordModel
    return model.IsInformationRecordModel() ? T_Super::_OnSubModelInsert(model) : DgnDbStatus::ElementBlockedChange;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InformationRecordPartitionPtr InformationRecordPartition::Create(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description)
    {
    CreateParams createParams = InitCreateParams(parentSubject, name, dgn_ElementHandler::InformationRecordPartition::GetHandler());
    if (!createParams.IsValid())
        return nullptr;

    InformationRecordPartitionPtr partition = new InformationRecordPartition(createParams);
    if (description && *description)
        partition->SetDescription(description);

    return partition;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InformationRecordPartitionCPtr InformationRecordPartition::CreateAndInsert(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description)
    {
    InformationRecordPartitionPtr partition = Create(parentSubject, name, description);
    return partition.IsValid() ? parentSubject.GetDgnDb().Elements().Insert<InformationRecordPartition>(*partition) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus SpatialLocationPartition::_OnSubModelInsert(DgnModelCR model) const
    {
    // Only SpatialLocationModels can model a SpatialLocationPartition
    return model.IsSpatialLocationModel() ? T_Super::_OnSubModelInsert(model) : DgnDbStatus::ElementBlockedChange;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialLocationPartitionPtr SpatialLocationPartition::Create(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description)
    {
    CreateParams createParams = InitCreateParams(parentSubject, name, dgn_ElementHandler::SpatialLocationPartition::GetHandler());
    if (!createParams.IsValid())
        return nullptr;

    SpatialLocationPartitionPtr partition = new SpatialLocationPartition(createParams);
    if (description && *description)
        partition->SetDescription(description);

    return partition;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialLocationPartitionCPtr SpatialLocationPartition::CreateAndInsert(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description)
    {
    SpatialLocationPartitionPtr partition = Create(parentSubject, name, description);
    return partition.IsValid() ? parentSubject.GetDgnDb().Elements().Insert<SpatialLocationPartition>(*partition) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus PhysicalPartition::_OnSubModelInsert(DgnModelCR model) const
    {
    return model.IsSpatialModel() ? T_Super::_OnSubModelInsert(model) : DgnDbStatus::ElementBlockedChange;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalPartitionPtr PhysicalPartition::Create(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description)
    {
    CreateParams createParams = InitCreateParams(parentSubject, name, dgn_ElementHandler::PhysicalPartition::GetHandler());
    if (!createParams.IsValid())
        return nullptr;

    PhysicalPartitionPtr partition = new PhysicalPartition(createParams);
    if (description && *description)
        partition->SetDescription(description);

    return partition;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalPartitionCPtr PhysicalPartition::CreateAndInsert(SubjectCR parentSubject, Utf8StringCR name, Utf8CP description)
    {
    PhysicalPartitionPtr partition = Create(parentSubject, name, description);
    return partition.IsValid() ? parentSubject.GetDgnDb().Elements().Insert<PhysicalPartition>(*partition) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus RoleElement::_OnInsert()
    {
    // RoleElements can only reside in a RoleModel
    return GetModel()->IsRoleModel() ? T_Super::_OnInsert() : DgnDbStatus::WrongModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode Drawing::CreateCode(DocumentListModelCR model, Utf8StringCR name)
    {
    return CodeSpec::CreateCode(BIS_CODESPEC_Drawing, model, name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode Drawing::CreateUniqueCode(DocumentListModelCR model, Utf8CP baseName)
    {
    DgnDbR db = model.GetDgnDb();
    DgnCode code = CreateCode(model, baseName);
    if (!db.Elements().QueryElementIdByCode(code).IsValid())
        return code;

    int counter=1;
    do  {
        Utf8PrintfString name("%s-%d", baseName, counter);
        code = CreateCode(model, name.c_str());
        counter++;
        } while (db.Elements().QueryElementIdByCode(code).IsValid());

    return code;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DrawingPtr Drawing::Create(DocumentListModelCR model, Utf8StringCR name)
    {
    DgnDbR db = model.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(dgn_ElementHandler::Drawing::GetHandler());

    if (!model.GetModelId().IsValid() || !classId.IsValid()) // || (name.empty() && !subModel->IsPrivate()))    A model element can have no name if the model is private. No way of checking the model-is-private condition here.
        {
        BeAssert(false);
        return nullptr;
        }

    return new Drawing(CreateParams(db, model.GetModelId(), classId, CreateCode(model, name)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SectionDrawingPtr SectionDrawing::Create(DocumentListModelCR model, Utf8StringCR name)
    {
    DgnDbR db = model.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(dgn_ElementHandler::SectionDrawing::GetHandler());

    if (!model.GetModelId().IsValid() || !classId.IsValid()) // || (name.empty() && !subModel->IsPrivate()))    A model element can have no name if the model is private. No way of checking the model-is-private condition here.
        {
        BeAssert(false);
        return nullptr;
        }

    return new SectionDrawing(CreateParams(db, model.GetModelId(), classId, CreateCode(model, name)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DrawingGraphicPtr DrawingGraphic::Create(GraphicalModel2dCR model, DgnCategoryId categoryId)
    {
    DgnDbR db = model.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(dgn_ElementHandler::DrawingGraphic::GetHandler());

    if (!model.GetModelId().IsValid() || !classId.IsValid() || !categoryId.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    return new DrawingGraphic(CreateParams(db, model.GetModelId(), classId, categoryId));
    }

//=======================================================================================
// @bsiclass
//=======================================================================================
struct OnInsertedCaller
    {
    DgnElementCR m_newEl;
    OnInsertedCaller(DgnElementCR newEl) : m_newEl(newEl){}
    DgnElement::AppData::DropMe operator()(DgnElement::AppData& app, DgnElementCR el) const {return app._OnInserted(m_newEl);}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::_OnInserted(DgnElementP copiedFrom) const {
    UnloadAutoHandledProperties(); // See "avoid incomplete NavigationProperty values"

    if (copiedFrom)
        copiedFrom->CallAppData(OnInsertedCaller(*this));

    CallJsPostHandler("onInserted");

    GetModel()->_OnInsertedElement(*this);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_SetParentId(DgnElementId parentId, DgnClassId parentRelClassId)
    {
    // Check for direct cycle...will check indirect cycles on update.
    if (parentId.IsValid() && parentId == GetElementId())
        return DgnDbStatus::InvalidParent;

    if (parentId.IsValid() && !parentRelClassId.IsValid())
        return DgnDbStatus::InvalidId;

    ECClassCP relClass = GetDgnDb().Schemas().GetClass(parentRelClassId);
    if (relClass == nullptr || !relClass->IsRelationshipClass())
        return DgnDbStatus::WrongClass;

    m_parent.m_id = parentId;
    m_parent.m_relClassId = parentRelClassId;
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool parentCycleExists(DgnElementId parentId, DgnElementId elemId, DgnDbR db)
    {
    // simple checks first...
    if (!parentId.IsValid() || !elemId.IsValid())
        return false;

    if (parentId == elemId)
        return true;

    CachedStatementPtr stmt = db.Elements().GetStatement("SELECT ParentId FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE Id=?");
    do
        {
        stmt->BindId(1, parentId);
        if (BE_SQLITE_ROW != stmt->Step())
            return false;

        parentId = stmt->GetValueId<DgnElementId>(0);
        if (parentId == elemId)
            return true;

        stmt->Reset();
        }
    while (parentId.IsValid());

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_OnUpdate(DgnElementCR original)
    {
    if (m_classId != original.m_classId)
        return DgnDbStatus::WrongClass; // cannot change class of element

    ElementHandlerR elementHandler = GetElementHandler();
    if (elementHandler.GetDomain().IsReadonly())
        return DgnDbStatus::ReadOnlyDomain;

    auto parentId = GetParentId();
    if (parentId.IsValid() && parentId != original.GetParentId() && parentCycleExists(parentId, GetElementId(), GetDgnDb()))
        return DgnDbStatus::InvalidParent;

    auto existingElemWithCode = GetDgnDb().Elements().QueryElementIdByCode(m_code);
    if ((existingElemWithCode.IsValid() && existingElemWithCode != GetElementId()))
        return DgnDbStatus::DuplicateCode;

    {
    BeMutexHolder lock(GetElementsMutex());
    for (auto entry=m_appData.begin(); entry!=m_appData.end(); ++entry)
        {
        DgnDbStatus stat = entry->second->_OnUpdate(*this, original);
        if (DgnDbStatus::Success != stat)
            return stat;
        }
    }

    return GetModel()->_OnUpdateElement(*this, original);
    }

struct OnUpdatedCaller
    {
    DgnElementCR m_updated, m_original;
    bool m_isOriginal;
    OnUpdatedCaller(DgnElementCR updated, DgnElementCR original, bool isOriginal) : m_updated(updated), m_original(original), m_isOriginal(isOriginal){}
    DgnElement::AppData::DropMe operator()(DgnElement::AppData& app, DgnElementCR el) const {return app._OnUpdated(m_updated, m_original, m_isOriginal);}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::_OnUpdated(DgnElementCR original) const
    {
    CallJsPostHandler("onUpdated");

    // We need to call the events on both sets of AppData. Start by calling the appdata on this (the replacement)
    // element. NOTE: This is where Aspects, etc. actually update the database.
    CallAppData(OnUpdatedCaller(*this, original, false));

    // All done. This gives appdata on the *original* element a notification that the update has happened
    original.CallAppData(OnUpdatedCaller(*this, original, true));

    // now tell the model that one of its elements has been changed.
    GetModel()->_OnUpdatedElement(*this, original);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_OnDelete() const
    {
    ElementHandlerR elementHandler = GetElementHandler();
    if (elementHandler.GetDomain().IsReadonly())
        return DgnDbStatus::ReadOnlyDomain;

    {
    BeMutexHolder lock(GetElementsMutex());
    for (auto entry=m_appData.begin(); entry!=m_appData.end(); ++entry)
        {
        DgnDbStatus stat = entry->second->_OnDelete(*this);
        if (DgnDbStatus::Success != stat)
            return stat;
        }
    }

    CallJsPostHandler("onDelete");
    return GetModel()->_OnDeleteElement(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void deleteLinkTableRelationships(DgnDbR db, Utf8CP tableName, DgnElementId elementId)
    {
    BeAssert(db.TableExists(tableName));
    Utf8PrintfString sql("DELETE FROM %s WHERE SourceId=? OR TargetId=?", tableName);
    CachedStatementPtr statement = db.Elements().GetStatement(sql.c_str());
    statement->BindId(1, elementId);
    statement->BindId(2, elementId);
    statement->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void deleteLinkTableRelationships(DgnDbR db, DgnElementId elementId)
    {
    // provide clean up behavior previously handled by foreign keys (the following link tables use "logical" foreign keys now)
    deleteLinkTableRelationships(db, BIS_TABLE(BIS_REL_ElementRefersToElements), elementId);
    deleteLinkTableRelationships(db, BIS_TABLE(BIS_REL_ElementDrivesElement), elementId);
    }

struct OnDeletedCaller  {DgnElement::AppData::DropMe operator()(DgnElement::AppData& app, DgnElementCR el) const {return app._OnDeleted(el);}};
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::_OnDeleted() const
    {
    CallJsPostHandler("onDeleted");
    CallAppData(OnDeletedCaller());
    GetDgnDb().Elements().DropFromPool(*this);
    deleteLinkTableRelationships(GetDgnDb(), GetElementId());
    DgnModelPtr model = GetModel();
    if (model.IsValid())
        model->_OnDeletedElement(m_elementId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnElement::ToJsonPropString() const
    {
    auto& ncThis = const_cast<DgnElement&>(*this);
    ncThis._OnSaveJsonProperties();
    return m_jsonProps.Stringify();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
void DgnElement::_BindWriteParams(ECSqlStatement& statement, ForInsert forInsert)
    {
    if (!m_code.IsValid())
        {
        BeAssert(false && "Code is missing");
        return;
        }

    if (m_code.IsEmpty())
        statement.BindNull(statement.GetParameterIndex(BIS_ELEMENT_PROP_CodeValue));
    else
        statement.BindText(statement.GetParameterIndex(BIS_ELEMENT_PROP_CodeValue), m_code.GetValue().GetUtf8().c_str(), IECSqlBinder::MakeCopy::No);

    statement.BindNavigationValue(statement.GetParameterIndex(BIS_ELEMENT_PROP_CodeSpec), m_code.GetCodeSpecId());
    statement.BindNavigationValue(statement.GetParameterIndex(BIS_ELEMENT_PROP_CodeScope), m_code.GetScopeElementId(GetDgnDb()));

    if (HasUserLabel())
        statement.BindText(statement.GetParameterIndex(BIS_ELEMENT_PROP_UserLabel), GetUserLabel(), IECSqlBinder::MakeCopy::No);
    else
        statement.BindNull(statement.GetParameterIndex(BIS_ELEMENT_PROP_UserLabel));

    statement.BindNavigationValue(statement.GetParameterIndex(BIS_ELEMENT_PROP_Parent), GetParentId(), GetParentRelClassId());

    if (m_federationGuid.IsValid())
        statement.BindBlob(statement.GetParameterIndex(BIS_ELEMENT_PROP_FederationGuid), &m_federationGuid, sizeof(m_federationGuid), IECSqlBinder::MakeCopy::No);
    else
        statement.BindNull(statement.GetParameterIndex(BIS_ELEMENT_PROP_FederationGuid));

    Utf8String jsonProps = ToJsonPropString();
    if (!jsonProps.empty())
        statement.BindText(statement.GetParameterIndex(BIS_ELEMENT_PROP_JsonProperties), jsonProps.c_str(), IECSqlBinder::MakeCopy::Yes);

    if (forInsert != ForInsert::Yes)
        return;

    statement.BindId(statement.GetParameterIndex(BIS_ELEMENT_PROP_ECInstanceId), m_elementId);
    statement.BindNavigationValue(statement.GetParameterIndex(BIS_ELEMENT_PROP_Model), m_modelId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus DgnElement::_InsertInDb()
    {
    CachedECSqlStatementPtr statement = GetDgnDb().Elements().GetPreparedInsertStatement(*this);
    if (statement.IsNull())
        return DgnDbStatus::WriteError;

    _BindWriteParams(*statement, ForInsert::Yes);

    auto stepResult = statement->Step();
    switch (stepResult)
        {
        case BE_SQLITE_DONE:
            break;

        case BE_SQLITE_CONSTRAINT_FOREIGNKEY:
            return DgnDbStatus::ForeignKeyConstraint;

        case BE_SQLITE_CONSTRAINT_UNIQUE:
            return DgnDbStatus::ConstraintNotUnique;

        case BE_SQLITE_CONSTRAINT_NOTNULL:
            return DgnDbStatus::InvalidCode;

        default:
            {
            LOG.errorv("Error inserting element into db. Status: %d", stepResult);
            return DgnDbStatus::WriteError;
            }
        }

    if (PropState::Dirty == m_flags.m_propState)
        {
        ElementAutoHandledPropertiesECInstanceAdapterPtr ec = ElementAutoHandledPropertiesECInstanceAdapter::Create(*this, false);
        if (ec.IsNull())
            return DgnDbStatus::WrongClass;
        auto status = ec->UpdateProperties();
        if (DgnDbStatus::Success != status)
            {
            BeAssert(false && "Auto-handled properties update failed - see log for sql constraint errors, etc.");
            return status;
            }
        }

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus DgnElement::_UpdateInDb()
    {
    CachedECSqlStatementPtr stmt = GetDgnDb().Elements().GetPreparedUpdateStatement(*this);
    if (stmt.IsNull())
        return DgnDbStatus::WriteError;

    _BindWriteParams(*stmt, ForInsert::No);

    auto stmtResult = stmt->Step();
    if (BE_SQLITE_DONE != stmtResult)
        {
        // SQLite doesn't tell us which constraint failed - check if it's the Code. (NOTE: We should catch this in _OnInsert())
        auto existingElemWithCode = GetDgnDb().Elements().QueryElementIdByCode(m_code);
        if (existingElemWithCode.IsValid() && existingElemWithCode != GetElementId())
            return DgnDbStatus::DuplicateCode;

        return DgnDbStatus::WriteError;
        }

    if (PropState::Dirty == m_flags.m_propState)
        {
        ElementAutoHandledPropertiesECInstanceAdapterPtr ec = ElementAutoHandledPropertiesECInstanceAdapter::Create(*this, false);
        if (ec.IsNull())
            return DgnDbStatus::WrongClass;
        auto status = ec->UpdateProperties();
        if (DgnDbStatus::Success != status)
            {
            BeAssert(false && "Auto-handled properties update failed - see log for sql constraint errors, etc.");
            return status;
            }
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_LoadFromDb()
    {
    DgnElements::ElementSelectStatement select = GetDgnDb().Elements().GetPreparedSelectStatement(*this);
    if (select.m_statement.IsNull())
        return DgnDbStatus::Success;

    if (BE_SQLITE_ROW != select.m_statement->Step())
        return DgnDbStatus::ReadError;

    return _ReadSelectParams(*select.m_statement, select.m_params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::RelatedElement::ToJson(DgnDbR db, BeJsValue val) const
    {
    val.SetEmptyObject();
    val[ECJsonUtilities::json_navId()] = m_id;
    auto relClass = db.Schemas().GetClass(m_relClassId);
    if (relClass != nullptr)
        {
        Utf8String fullName(relClass->GetSchema().GetName());
        fullName.append(":");
        fullName.append(relClass->GetName().c_str());
        val[ECJsonUtilities::json_navRelClassName()] = fullName;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::RelatedElement::FromJson(DgnDbR db, BeJsConst val)
    {
    if (val.isNull())
        {
        *this = RelatedElement();
        return;
        }

    m_id.FromJson(val[ECJsonUtilities::json_navId()]);
    if (m_id.IsValid())
        m_relClassId = ECJsonUtilities::GetClassIdFromClassNameJson(val[ECJsonUtilities::json_navRelClassName()], db.GetClassLocater());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static void autoHandlePropertiesToJsonFromECSql(BeJsValue elementJson, DgnElementCR elem)
    {
    BeAssert(elem.GetElementId().IsValid()); // element must be persistent to use ECSql
    ECClassCP eclass = elem.GetElementClass();

    Utf8StringCR autoHandledProps = elem.GetDgnDb().Elements().GetAutoHandledPropertiesSelectECSql(*eclass);
    if (autoHandledProps.empty())
        return;

    CachedECSqlStatementPtr stmt = elem.GetDgnDb().GetPreparedECSqlStatement(autoHandledProps.c_str());
    if (!stmt.IsValid())
        {
        BeAssert(false);
        return;
        }

    stmt->BindId(1, elem.GetElementId());
    if (BE_SQLITE_ROW != stmt->Step())
        {
        BeAssert(false);
        return;
        }

    JsonECSqlSelectAdapter const& adapter = elem.GetDgnDb().Elements().GetJsonSelectAdapter(*stmt);
    adapter.GetRow(elementJson, *stmt, true);
    }

/** Fill the "base" properties of this element into the JSON value */
void DgnElement::ToBaseJson(BeJsValue val) const {
    val[json_id()] = m_elementId;
    auto ecClass = GetElementClass();
    BeAssert(ecClass != nullptr);
    val[json_classFullName()] = ecClass->GetFullName();

    val[json_model()] = m_modelId;
    m_code.ToJson(val[json_code()]);

    if (m_parent.IsValid())
       m_parent.ToJson(GetDgnDb(), val[json_parent()]);

    if (m_federationGuid.IsValid())
        val[json_federationGuid()] = m_federationGuid.ToString();

    if (!m_userLabel.empty())
        val[json_userLabel()] = m_userLabel;

    if (!m_jsonProps.empty())
        val[json_jsonProperties()].From(m_jsonProps);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::_ToJson(BeJsValue val, BeJsConst opts) const {

    // all the base properties
    ToBaseJson(val);

    // if auto-handled properties are already in memory
    if (nullptr != m_ecPropertyData) {
        ElementAutoHandledPropertiesECInstanceAdapterPtr autoHandledAdapter = ElementAutoHandledPropertiesECInstanceAdapter::Create(*this, true);
        if (autoHandledAdapter.IsNull())
            return;

        std::function<bool(Utf8CP)> shouldWriteProperty = [this](Utf8CP propName) {
            if (ECJsonSystemNames::IsTopLevelSystemMember(propName))
                return false;

            ElementECPropertyAccessor const accessor(*this, propName);
            return accessor.IsValid() && accessor.IsAutoHandled();
        };
        JsonEcInstanceWriter::WritePartialInstanceToJson(val, *autoHandledAdapter, JsonEcInstanceWriter::MemberNameCasing::LowerFirstChar, shouldWriteProperty, &GetDgnDb().GetClassLocater());
    } else if (GetElementId().IsValid()) {
        // element must be persistent to use ECSql to load properties
        autoHandlePropertiesToJsonFromECSql(val, *this); // auto-handled properties were not loaded, query for them
    }
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::_FromJson(BeJsConst props) {
    auto model = props[json_model()];
    if (!model.isNull())
        m_modelId.FromJson(model);

    auto code = props[json_code()];
    if (!code.isNull())
        m_code = DgnCode::FromJson(code, m_dgndb, true);

    // support partial update, only update m_federationGuid if props has member
    if (props.hasMember(json_federationGuid())) {
        auto federationGuid = props[json_federationGuid()];
        if (federationGuid.isString())
            m_federationGuid.FromString(federationGuid.asString().c_str());
        else
            m_federationGuid.Invalidate(); // allow undefined to clear an existing value
    }

    // support partial update, only update m_userLabel if props has member
    if (props.hasMember(json_userLabel())) {
        auto label = props[json_userLabel()];
        if (label.isString())
            m_userLabel = label.asString();
        else
            m_userLabel.clear(); // allow undefined to clear an existing value
    }

    // support partial update, only update m_parent if props has member
    if (props.hasMember(json_parent())) {
        auto parent = props[json_parent()];
        m_parent.FromJson(m_dgndb, parent); // RelatedElement::FromJson also clears for undefined
    }

    auto jsonProps = props[json_jsonProperties()];
    if (!jsonProps.isNull()) {
        jsonProps.SaveTo(m_jsonProps);
        _OnLoadedJsonProperties();
    }

    static auto const isDefinitelyNotAutoHandled = [](Utf8CP propName) -> bool {
        switch (propName[0]) {
            case 'i': return 0==strcmp(propName, "id");
            case 'c': return 0==strcmp(propName, "classFullName") || 0==strcmp(propName, "code");
            case 'p': return 0==strcmp(propName, "placement");
        }
        return false;
    };

    ElementAutoHandledPropertiesECInstanceAdapterPtr ec = ElementAutoHandledPropertiesECInstanceAdapter::Create(*this, true);
    if (ec.IsNull())
        return;

    std::function<bool(Utf8CP)> shouldSerializeProperty = [this](Utf8CP propName) {
        if (isDefinitelyNotAutoHandled(propName))
            return false;

        if (ECJsonSystemNames::IsTopLevelSystemMember(propName))
            return false;

        ElementECPropertyAccessor const accessor(*this, propName);
        return accessor.IsValid() && accessor.IsAutoHandled();
    };
    ECN::JsonECInstanceConverter::JsonToECInstance(*ec, props, GetDgnDb().GetClassLocater(), shouldSerializeProperty);
}

/*---------------------------------------------------------------------------------**/ /**
 * @bsimethod
 +---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::CreateParams::CreateParams(DgnDbR db, BeJsConst val) : m_dgndb(db) {
    m_classId = ECJsonUtilities::GetClassIdFromClassNameJson(val[DgnElement::json_classFullName()], db.GetClassLocater());
    m_modelId.FromJson(val[DgnElement::json_model()]);
    m_federationGuid.FromString(val[DgnElement::json_federationGuid()].asString().c_str());
    m_userLabel = val[DgnElement::json_userLabel()].asString();

    DgnElement::RelatedElement parent;
    parent.FromJson(db, val[json_parent()]);
    if (!parent.IsValid())
        return;

    m_parentId = parent.m_id;
    m_parentRelClassId = parent.m_relClassId;
}

/*---------------------------------------------------------------------------------**/ /**
 * @bsimethod
 +---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIdSet DgnElement::QueryChildren() const
    {
    CachedStatementPtr stmt=GetDgnDb().Elements().GetStatement("SELECT Id FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE ParentId=?");
    stmt->BindId(1, GetElementId());

    DgnElementIdSet elementIdSet;
    while (BE_SQLITE_ROW == stmt->Step())
        elementIdSet.insert(stmt->GetValueId<DgnElementId>(0));

    return elementIdSet;
    }

//=======================================================================================
// @bsiclass
//=======================================================================================
struct GeomBlobHeader
{
    enum {Signature = 0x0600,}; // DgnDb06

    uint32_t m_signature;    // write this so we can detect errors on read
    uint32_t m_size;
    GeomBlobHeader(GeometryStream const& geom) {m_signature = Signature; m_size=geom.GetSize();}
    GeomBlobHeader(SnappyReader& in) {uint32_t actuallyRead; in._Read((Byte*) this, sizeof(*this), actuallyRead);}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometrySource2dCP DgnElement::ToGeometrySource2d() const
    {
    GeometrySourceCP source = _ToGeometrySource();
    return nullptr == source ? nullptr : source->GetAsGeometrySource2d();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GeometrySource3dCP DgnElement::ToGeometrySource3d() const
    {
    GeometrySourceCP source = _ToGeometrySource();
    return nullptr == source ? nullptr : source->GetAsGeometrySource3d();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometryStream::ReadGeometryStream(SnappyFromMemory& snappy, DgnDbR dgnDb, void const* blob, int blobSize)
    {
    if (0 == blobSize && nullptr == blob)
        return DgnDbStatus::Success;

    snappy.Init(const_cast<void*>(blob), static_cast<uint32_t>(blobSize));
    GeomBlobHeader header(snappy);
    if ((GeomBlobHeader::Signature != header.m_signature) || 0 == header.m_size)
        {
        BeAssert(false);
        return DgnDbStatus::ReadError;
        }

    Resize(header.m_size);

    uint32_t actuallyRead;
    auto readStatus = snappy._Read(GetDataP(), GetSize(), actuallyRead);

    if (ZIP_SUCCESS != readStatus || actuallyRead != GetSize())
        {
        BeAssert(false);
        return DgnDbStatus::ReadError;
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryStream::IsViewIndependent() const
    {
    GeometryStreamIO::Collection geom(data(), size());
    auto header = geom.GetHeader();
    return GeometryStreamIO::Header::Flags::None != (header.m_flags & GeometryStreamIO::Header::Flags::ViewIndependent);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Transform GeometrySource::GetPlacementTransform() const
    {
    GeometrySource3dCP source3d = _GetAsGeometrySource3d();
    return nullptr != source3d ? source3d->GetPlacement().GetTransform() : _GetAsGeometrySource2d()->GetPlacement().GetTransform();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement::Validate() const
    {
    if (!m_categoryId.IsValid())
        return DgnDbStatus::InvalidCategory;

    if (m_geom.HasGeometry() && !_IsPlacementValid())
        return DgnDbStatus::BadElement;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::CreateParams::RelocateToDestinationDb(DgnImportContext& importer)
    {
    m_modelId = importer.FindModelId(m_modelId);
    m_classId = importer.RemapClassId(m_classId);
    }

/*---------------------------------------------------------------------------------**//**
* NEEDSWORK: Not clear in what contexts an element's code should be copied, or not.
* GetCreateParamsForImport() only copies the code if we're copying between DBs.
* But _CopyFrom() always copies it. Which is what we want when called from CopyForEdit(),
* but not what we want from _CloneForImport() or Clone(), which both use their own CreateParams
* to specify the desired code.
* So - Clone-like methods use this to ensure the code is copied or not copied appropriately.
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::CopyForCloneFrom(DgnElementCR src)
    {
    DgnCode code = GetCode();
    _CopyFrom(src, CopyFromOptions());
    m_code = code;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr DgnElement::_Clone(DgnDbStatus* inStat, DgnElement::CreateParams const* params) const
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(stat, inStat);

    // Perform input params validation. Code must be different and element id should be invalid...
    if (nullptr != params)
        {
        if (params->m_id.IsValid())
            {
            stat = DgnDbStatus::InvalidId;
            return nullptr;
            }

        if (params->m_code == GetCode())
            {
            stat = DgnDbStatus::InvalidName;
            return nullptr;
            }
        }

    DgnElementPtr cloneElem = GetElementHandler().Create(nullptr != params ? *params : DgnElement::CreateParams(GetDgnDb(), GetModelId(), GetElementClassId(), DgnCode(), GetUserLabel()));
    if (!cloneElem.IsValid())
        {
        stat = DgnDbStatus::BadRequest;
        return nullptr;
        }

    cloneElem->CopyForCloneFrom(*this);

    stat = DgnDbStatus::Success;
    return cloneElem;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr DgnElement::_CloneForImport(DgnDbStatus* inStat, DgnModelR destModel, DgnImportContext& importer) const
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(stat, inStat);

    DgnElement::CreateParams params = GetCreateParamsForImport(destModel, importer);
    params.m_modelId = destModel.GetModelId();

    if (!params.IsValid())
        {
        BeNapi::ThrowJsException(
            m_dgndb.GetJsIModelDb()->Env(),
            params.m_classId.IsValid() ? "invalid create params" : "attempt to clone with unknown class",
            (int) (params.m_classId.IsValid() ? DgnDbStatus::BadRequest : DgnDbStatus::WrongClass)
        );
        }

    DgnElementPtr cloneElem = GetElementHandler().Create(params);

    if (!cloneElem.IsValid())
        {
        stat = DgnDbStatus::BadRequest;
        return nullptr;
        }

    cloneElem->CopyForCloneFrom(*this);

    if (importer.IsBetweenDbs())
        {
        cloneElem->_RemapIds(importer);
        cloneElem->_AdjustPlacementForImport(importer);
        }
    else if (GetParentId().IsValid())
        {
        cloneElem->SetParentId(importer.FindElementId(GetParentId()), GetParentRelClassId());
        }

    stat = DgnDbStatus::Success;
    return cloneElem;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::_CopyFrom(DgnElementCR other, CopyFromOptions const& opts)
    {
    if (&other == this)
        return;

    // Copying between DgnDbs is not allowed. Caller must do Id remapping.
    m_code = other.m_code;
    m_userLabel = other.m_userLabel;
    m_parent.m_id = other.m_parent.m_id;
    m_parent.m_relClassId = other.m_parent.m_relClassId;
    m_jsonProps.From(other.m_jsonProps);
    // don't copy FederationGuid

    if (opts.copyEcPropertyData)
        {
        // Copy the auto-handled EC properties
        ElementAutoHandledPropertiesECInstanceAdapterPtr ecOther = ElementAutoHandledPropertiesECInstanceAdapter::Create(other, true);
        const auto ecOtherPtrIsValid = ecOther.IsValid();
        if (!ecOtherPtrIsValid)
            m_dgndb.ThrowException("failed to copy auto handled EC properties", (int)DgnDbStatus::BadArg);

        const auto ecOtherInstanceIsValid = ecOther->IsValid();
        if (ecOtherInstanceIsValid)
            {
            bool sameClass = (GetElementClassId() == other.GetElementClassId());
            // Note that we are NOT necessarily going to call _SetPropertyValue on each property.
            // If the subclass needs to validate specific auto-handled properties even during copying, then it
            // must override _CopyFrom and validate after the copy is done.
            // TRICKY: If copying between elements of the same class, don't load my auto-handled properties at the outset. That will typically lead me to allocate a smaller
            //          buffer for what I have now (if anything) and then have to realloc it to accommodate the other element's buffer.
            //          Instead, wait and let CopyFromBuffer tell me the *exact* size to allocate.
            ElementAutoHandledPropertiesECInstanceAdapterPtr ecThis = ElementAutoHandledPropertiesECInstanceAdapter::Create(*this, !sameClass, ecOther->CalculateBytesUsed());
            const auto ecThisPtrIsValid = ecThis.IsValid();
            if (!ecThisPtrIsValid)
                // ElementAutoHandledPropertiesECInstanceAdapter::Create returns null if its class was invalid
                m_dgndb.ThrowException("failed to copy auto handled EC properties", (int)DgnDbStatus::WrongClass);

            const auto ecThisInstanceIsValid = ecThis->IsValid();
            if (ecThisInstanceIsValid) // this might not have auto-handled props if this and other are instances of different classes
                {
                if (!sameClass)
                    ecThis->CopyDataBuffer(*ecOther, true);
                else
                    ecThis->CopyFromBuffer(*ecOther);

                if (nullptr != m_ecPropertyData)
                    m_flags.m_propState = DgnElement::PropState::Dirty;
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::CopyAppDataFrom(DgnElementCR source, bool bindAspects) const
    {
    BeMutexHolder lock(GetElementsMutex());
    for (auto a : source.m_appData)
        {
        AddAppData(*a.first, a.second.get());

        if (bindAspects)
            {
            auto aspect = dynamic_cast<DgnElement::Aspect*>(a.second.get());
            if (nullptr != aspect)
                aspect->BindTo(*this);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::_RemapIds(DgnImportContext& importer)
    {
    BeAssert(importer.IsBetweenDbs());
    m_code.RelocateToDestinationDb(importer);
    m_parent.m_id = importer.FindElementId(m_parent.m_id);
    m_parent.m_relClassId = importer.RemapClassId(m_parent.m_relClassId);
    RemapAutoHandledNavigationproperties(importer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::CreateParams DgnElement::GetCreateParamsForImport(DgnModelR destModel, DgnImportContext& importer) const
    {
    CreateParams parms(importer.GetDestinationDb(), GetModelId(), GetElementClassId());
    CodeSpecCPtr codeSpec = GetCode().IsValid() ? GetDgnDb().CodeSpecs().GetCodeSpec(GetCode().GetCodeSpecId()) : nullptr;
    if (codeSpec.IsValid())
        codeSpec->CloneCodeForImport(parms.m_code, *this, destModel, importer);

    if (importer.IsBetweenDbs())
        parms.RelocateToDestinationDb(importer);

    return parms;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ElementImporter::ElementImporter(DgnImportContext& context) : m_context(context), m_copyChildren(true), m_copyGroups(false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr ElementImporter::ImportElement(DgnDbStatus* statusOut, DgnModelR destModel, DgnElementCR sourceElement)
    {
    auto destElementId = m_context.FindElementId(sourceElement.GetElementId());
    if (destElementId.IsValid()) // If source element was already copied, just return the existing copy. This happens, for example, when a parent deep-copies its children immediately.
        return m_context.GetDestinationDb().Elements().GetElement(destElementId);

    DgnElementCPtr destElement = sourceElement.Import(statusOut, destModel, m_context);
    if (!destElement.IsValid())
        return nullptr;

    if (m_copyChildren)
        {
        for (auto sourceChildid : sourceElement.QueryChildren())
            {
            DgnElementCPtr sourceChildElement = sourceElement.GetDgnDb().Elements().GetElement(sourceChildid);
            if (!sourceChildElement.IsValid())
                continue;

            ImportElement(statusOut, destModel, *sourceChildElement);
            }
        }

    IElementGroupCP sourceGroup;
    if (m_copyGroups && nullptr != (sourceGroup = sourceElement.ToIElementGroup()))
        {
        for (DgnElementId sourceMemberId : sourceGroup->QueryMembers())
            {
            DgnElementCPtr sourceMemberElement = sourceElement.GetDgnDb().Elements().GetElement(sourceMemberId);
            if (!sourceMemberElement.IsValid())
                continue;
            DgnModelId destMemberModelId = m_context.FindModelId(sourceMemberElement->GetModel()->GetModelId());
            DgnModelPtr destMemberModel = m_context.GetDestinationDb().Models().GetModel(destMemberModelId);
            if (!destMemberModel.IsValid())
                destMemberModel = &destModel;
            DgnElementCPtr destMemberElement = ImportElement(nullptr, *destMemberModel, *sourceMemberElement);
            if (destMemberElement.IsValid())
                ElementGroupsMembers::Insert(*destElement, *destMemberElement, 0); // *** WIP_GROUPS - is this the right way to re-create the member-of relationship? What about the _OnMemberAdded callbacks?  Preserve MemberPriority?
            }
        }

    return destElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnElement::Import(DgnDbStatus* stat, DgnModelR destModel, DgnImportContext& importer) const
    {
    if (nullptr != stat)
        *stat = DgnDbStatus::Success;

    auto parent = GetDgnDb().Elements().GetElement(m_parent.m_id);
    DgnDbStatus parentStatus = DgnDbStatus::Success;
    if (parent.IsValid() && DgnDbStatus::Success != (parentStatus = parent->_OnChildImport(*this, destModel, importer)))
        {
        if (nullptr != stat)
            *stat = parentStatus;

        return nullptr;
        }

    DgnElementPtr cc = _CloneForImport(stat, destModel, importer); // (also calls _CopyFrom and _RemapIds)
    if (!cc.IsValid())
        return DgnElementCPtr();

    DgnElementCPtr ccp = cc->Insert(stat);
    if (!ccp.IsValid())
        return ccp;

    importer.AddElementId(GetElementId(), ccp->GetElementId());

    parent = ccp->GetDgnDb().Elements().GetElement(ccp->GetParentId());
    if (parent.IsValid())
        parent->_OnChildImported(*ccp, *this, importer);

    ccp->_OnImported(*this, importer);

    return ccp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr DgnElement::Clone(DgnDbStatus* stat, DgnElement::CreateParams const* params) const
    {
    return _Clone(stat, params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement2d::_AdjustPlacementForImport(DgnImportContext const& importer)
    {
    m_placement.GetOriginR().Add(DPoint2d::From(importer.GetOriginOffset()));
    m_placement.GetAngleR() = (m_placement.GetAngle() + importer.GetYawAdjustment());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement3d::_AdjustPlacementForImport(DgnImportContext const& importer)
    {
    m_placement.GetOriginR().Add(importer.GetOriginOffset());
    m_placement.GetAnglesR().AddYaw(importer.GetYawAdjustment());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement::CopyFromGeometrySource(GeometrySourceCR src)
    {
    m_categoryId = src.GetCategoryId();
    m_geom = src.GetGeometryStream();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement2d::_CopyFrom(DgnElementCR el, CopyFromOptions const& opts)
    {
    T_Super::_CopyFrom(el, opts);
    auto src = el.ToGeometrySource2d();
    if (nullptr != src)
        {
        CopyFromGeometrySource(*src);
        m_placement = src->GetPlacement();
        }

    GeometricElement2dCR other = static_cast<GeometricElement2dCR>(el);
    m_typeDefinition.m_id = other.m_typeDefinition.m_id;
    m_typeDefinition.m_relClassId = other.m_typeDefinition.m_relClassId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement3d::_CopyFrom(DgnElementCR el, CopyFromOptions const& opts)
    {
    T_Super::_CopyFrom(el, opts);
    auto src = el.ToGeometrySource3d();
    if (nullptr != src)
        {
        CopyFromGeometrySource(*src);
        m_placement = src->GetPlacement();
        }

    GeometricElement3dCR other = static_cast<GeometricElement3dCR>(el);
    m_typeDefinition.m_id = other.m_typeDefinition.m_id;
    m_typeDefinition.m_relClassId = other.m_typeDefinition.m_relClassId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement::_RemapIds(DgnImportContext& importer)
    {
    T_Super::_RemapIds(importer);
    m_categoryId = importer.RemapCategory(m_categoryId);
    importer.RemapGeometryStreamIds(m_geom);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void remapIdsForTypeDefinition(DgnImportContext& importer, DgnElement::RelatedElement& typeDefinition)
    {
    if (typeDefinition.m_id.IsValid())
        {
        typeDefinition.m_id = importer.FindElementId(typeDefinition.m_id);
        if (typeDefinition.m_relClassId.IsValid())
            typeDefinition.m_relClassId = importer.RemapClassId(typeDefinition.m_relClassId);
        }
    else
        {
        typeDefinition.m_relClassId = DgnClassId();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement2d::_RemapIds(DgnImportContext& importer)
    {
    T_Super::_RemapIds(importer);
    remapIdsForTypeDefinition(importer, m_typeDefinition);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement3d::_RemapIds(DgnImportContext& importer)
    {
    T_Super::_RemapIds(importer);
    remapIdsForTypeDefinition(importer, m_typeDefinition);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandlerR DgnElement::GetElementHandler() const
    {
    return *dgn_ElementHandler::Element::FindHandler(GetDgnDb(), m_classId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr DgnElement::CopyForEditInternal(CopyFromOptions const& opts) const
    {
    DgnElement::CreateParams createParams(GetDgnDb(), m_modelId, m_classId, GetCode(), GetUserLabel(), m_parent.m_id, m_parent.m_relClassId, GetFederationGuid());
    createParams.SetElementId(GetElementId());
    createParams.SetIsLoadingElement(true);

    DgnElementPtr newEl = GetElementHandler()._CreateInstance(createParams);
#ifdef __clang__
    BeAssert(0 == strcmp(typeid(*newEl).name(), typeid(*this).name()));
#else
    BeAssert(typeid(*newEl) == typeid(*this)); // this means the ClassId of the element does not match the type of the element. Caller should find out why.
#endif
    newEl->_CopyFrom(*this, opts);
    return newEl;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr DgnElement::CopyForEdit() const
    {
    return CopyForEditInternal(CopyFromOptions());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::Update() {return GetDgnDb().Elements().Update(*this);}
DgnElementCPtr DgnElement::UpdateAndGet(DgnDbStatus* stat) {
    DgnDbStatus temp;
    if (nullptr == stat)
        stat = &temp;
    *stat = Update();
    return (*stat != DgnDbStatus::Success) ?  nullptr :  GetDgnDb().Elements().Get<DgnElement>(GetElementId());
}

DgnElementCPtr DgnElement::Insert(DgnDbStatus* stat) {return GetDgnDb().Elements().Insert(*this, stat);}
DgnDbStatus DgnElement::Delete() const {return GetDgnDb().Elements().Delete(*this);}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnDbStatus ElementGroupsMembers::Insert(DgnElementCR group, DgnElementCR member, int priority)
    {
    if (!group.GetElementId().IsValid() || !member.GetElementId().IsValid())
        return DgnDbStatus::InvalidId; // elements must be inserted to form link table relationship

    CachedECSqlStatementPtr statement = group.GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "INSERT INTO " BIS_SCHEMA(BIS_REL_ElementGroupsMembers)
        " (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId,MemberPriority) VALUES(?,?,?,?,?)", group.GetDgnDb().GetECCrudWriteToken());

    if (!statement.IsValid())
        return DgnDbStatus::BadRequest;

    statement->BindId(1, group.GetElementClassId());
    statement->BindId(2, group.GetElementId());
    statement->BindId(3, member.GetElementClassId());
    statement->BindId(4, member.GetElementId());
    statement->BindInt(5, priority);
    DbResult result = statement->Step();
    if (BE_SQLITE_DONE == result)
        return DgnDbStatus::Success;
    if (BE_SQLITE_CONSTRAINT_UNIQUE == result)
        return DgnDbStatus::ConstraintNotUnique;
    return DgnDbStatus::BadRequest;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnDbStatus ElementGroupsMembers::Delete(DgnElementCR group, DgnElementCR member)
    {
    CachedECSqlStatementPtr statement = group.GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "DELETE FROM " BIS_SCHEMA(BIS_REL_ElementGroupsMembers) " WHERE SourceECInstanceId=? AND TargetECInstanceId=?", group.GetDgnDb().GetECCrudWriteToken());

    if (!statement.IsValid())
        return DgnDbStatus::BadRequest;

    statement->BindId(1, group.GetElementId());
    statement->BindId(2, member.GetElementId());
    return (BE_SQLITE_DONE == statement->Step()) ? DgnDbStatus::Success : DgnDbStatus::BadRequest;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool ElementGroupsMembers::HasMember(DgnElementCR group, DgnElementCR member)
    {
    CachedECSqlStatementPtr statement = group.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT SourceECInstanceId FROM " BIS_SCHEMA(BIS_REL_ElementGroupsMembers) " WHERE SourceECInstanceId=? AND TargetECInstanceId=? LIMIT 1");

    if (!statement.IsValid())
        return false;

    statement->BindId(1, group.GetElementId());
    statement->BindId(2, member.GetElementId());
    return (BE_SQLITE_ROW == statement->Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnElementIdSet ElementGroupsMembers::QueryMembers(DgnElementCR group)
    {
    BeAssert(nullptr != group.ToIElementGroup());

    CachedECSqlStatementPtr statement = group.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT TargetECInstanceId FROM " BIS_SCHEMA(BIS_REL_ElementGroupsMembers) " WHERE SourceECInstanceId=?");

    if (!statement.IsValid())
        return DgnElementIdSet();

    statement->BindId(1, group.GetElementId());

    DgnElementIdSet elementIdSet;
    while (BE_SQLITE_ROW == statement->Step())
        elementIdSet.insert(statement->GetValueId<DgnElementId>(0));

    return elementIdSet;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
int ElementGroupsMembers::QueryMemberPriority(DgnElementCR group, DgnElementCR member)
    {
    CachedECSqlStatementPtr statement = group.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT MemberPriority FROM " BIS_SCHEMA(BIS_REL_ElementGroupsMembers) " WHERE SourceECInstanceId=? AND TargetECInstanceId=? LIMIT 1");

    if (!statement.IsValid())
        return -1;

    statement->BindId(1, group.GetElementId());
    statement->BindId(2, member.GetElementId());
    return (BE_SQLITE_ROW == statement->Step()) ? statement->GetValueInt(0) : -1;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnElementIdSet ElementGroupsMembers::QueryGroups(DgnElementCR member)
    {
    CachedECSqlStatementPtr statement = member.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT SourceECInstanceId FROM " BIS_SCHEMA(BIS_REL_ElementGroupsMembers) " WHERE TargetECInstanceId=?");

    if (!statement.IsValid())
        return DgnElementIdSet();

    statement->BindId(1, member.GetElementId());

    DgnElementIdSet elementIdSet;
    while (BE_SQLITE_ROW == statement->Step())
        elementIdSet.insert(statement->GetValueId<DgnElementId>(0));

    return elementIdSet;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::Aspect::ImportAspects(DgnElementR destEl, DgnElementCR srcEl, DgnImportContext& context)
    {
    auto& srcDb =  srcEl.GetDgnDb();

    auto multiAspects = srcDb.GetPreparedECSqlStatement("select ecinstanceid,ecclassid from " BIS_SCHEMA(BIS_CLASS_ElementMultiAspect) " where Element.Id=?");
    multiAspects->BindId(1, srcEl.GetElementId());
    while (BE_SQLITE_ROW == multiAspects->Step())
        {
        auto srcAspectInstanceId = multiAspects->GetValueId<ECInstanceId>(0);
        auto srcAspectClassId = multiAspects->GetValueId<ECClassId>(1);
        auto srcAspectClass = srcDb.Schemas().GetClass(srcAspectClassId);
        if (nullptr == srcAspectClass)
            {
            BeAssert(false);
            LOG.errorv("DgnElement::Aspect::ImportAspects - aspect class not found in source?!");
            continue;
            }
        auto srcAspect = DgnElement::MultiAspect::GetAspect(srcEl, *srcAspectClass, srcAspectInstanceId);
        if (nullptr == srcAspect)
            {
            BeAssert(false);
            LOG.errorv("DgnElement::Aspect::ImportAspects - failed to load source aspect");
            continue;
            }
        auto dstAspect = srcAspect->_CloneForImport(srcEl, context);
        if (!dstAspect.IsValid())
            {
            BeAssert(false);
            LOG.errorv("DgnElement::Aspect::ImportAspects - _CloneForImport failed");
            continue;
            }
        DgnElement::MultiAspect::AddAspect(destEl, static_cast<DgnElement::MultiAspect&>(*dstAspect));
        }


    auto uniqueAspects = srcDb.GetPreparedECSqlStatement("select ecclassid from " BIS_SCHEMA(BIS_CLASS_ElementUniqueAspect) " where ECInstanceId=?");
    uniqueAspects->BindId(1, srcEl.GetElementId());
    while (BE_SQLITE_ROW == uniqueAspects->Step())
        {
        auto srcAspectClassId = multiAspects->GetValueId<ECClassId>(0);
        auto srcAspectClass = srcDb.Schemas().GetClass(srcAspectClassId);
        if (nullptr == srcAspectClass)
            {
            BeAssert(false);
            LOG.errorv("DgnElement::Aspect::ImportAspects - unique aspect class not found in source?!");
            continue;
            }
        auto srcAspect = DgnElement::UniqueAspect::GetAspect(srcEl, *srcAspectClass);
        if (nullptr == srcAspect)
            {
            BeAssert(false);
            LOG.errorv("DgnElement::Aspect::ImportAspects - failed to load source unique aspect");
            continue;
            }
        auto dstAspect = srcAspect->_CloneForImport(srcEl, context);
        if (!dstAspect.IsValid())
            {
            BeAssert(false);
            LOG.errorv("DgnElement::Aspect::ImportAspects - _CloneForImport of unique aspect failed");
            continue;
            }
        DgnElement::UniqueAspect::SetAspect(destEl, static_cast<DgnElement::UniqueAspect&>(*dstAspect));
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
dgn_AspectHandler::Aspect* dgn_AspectHandler::Aspect::FindHandler(DgnDbR db, DgnClassId handlerId)
    {
    // quick check for a handler already known
    DgnDomain::Handler* handler = db.Domains().LookupHandler(handlerId);
    if (nullptr != handler)
        return dynamic_cast<dgn_AspectHandler::Aspect*>(handler);

    // not there, check via base classes
    handler = db.Domains().FindHandler(handlerId, db.Domains().GetClassId(GetHandler()));
    return handler ? dynamic_cast<dgn_AspectHandler::Aspect*>(handler) : nullptr;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::Aspect::Aspect()
    {
    m_changeType = ChangeType::None;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::Aspect::InsertThis(DgnElementCR el)
    {
    DgnDbStatus status = _InsertInstance(el, el.GetDgnDb().GetECCrudWriteToken());
    if (DgnDbStatus::Success != status)
        return status;
    return _UpdateProperties(el, el.GetDgnDb().GetECCrudWriteToken());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnClassId  DgnElement::Aspect::GetECClassId(DgnDbR db) const
    {
    return DgnClassId(db.Schemas().GetClassId(_GetECSchemaName(), _GetECClassName()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP DgnElement::Aspect::GetECClass(DgnDbR db) const
    {
    return db.Schemas().GetClass(_GetECSchemaName(), _GetECClassName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP DgnElement::Aspect::GetKeyECClass(DgnDbR db) const
    {
    return db.Schemas().GetClass(_GetKeyECSchemaName(), _GetKeyECClassName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassId DgnElement::Aspect::GetKeyECClassId(DgnDbR db) const
    {
    auto cls = GetKeyECClass(db);
    return cls ? cls->GetId() : ECClassId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::AppData::DropMe DgnElement::Aspect::_OnInserted(DgnElementCR el)
    {
    if (ChangeType::Delete != m_changeType) // (caller can cancel an add by calling Delete)
        {
        auto status = InsertThis(el);
        if (DgnDbStatus::Success != status)
            {
            LOG.errorv("Attempt to insert aspect %llu:%llu failed with status=%x", el.GetElementId().GetValueUnchecked(), GetAspectInstanceId().GetValueUnchecked(), status);
            }
        }

    m_changeType = ChangeType::None; // (Just in case)

    return DropMe::Yes;     // this scheduled change has been processed, so remove it.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::AppData::DropMe DgnElement::Aspect::_OnUpdated(DgnElementCR modified, DgnElementCR original, bool isOriginal)
    {
    if (ChangeType::None == m_changeType)
        return DropMe::Yes;     // Was just a cached instance? Drop it now, so that it does not become stale.

    if (ChangeType::Delete == m_changeType)
        {
        _DeleteInstance(modified, original.GetDgnDb().GetECCrudWriteToken());
        }
    else
        {
        DgnDbR db = modified.GetDgnDb();
        ECInstanceKey existing = _QueryExistingInstanceKey(modified);
        if (existing.IsValid() && (existing.GetClassId() != GetECClassId(db)))
            {
            _DeleteInstance(modified, original.GetDgnDb().GetECCrudWriteToken());
            existing = ECInstanceKey();  //  trigger an insert below
            }

        if (!existing.IsValid())
            InsertThis(modified);
        else
            _UpdateProperties(modified, original.GetDgnDb().GetECCrudWriteToken());
        }

    m_changeType = ChangeType::None; // (Just in case)

    return DropMe::Yes; // this scheduled change has been processed, so remove it.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void remapAutoHandledNavigationproperties(ECN::IECInstanceR instance, DgnImportContext& importer)
    {
    auto& db = importer.GetDestinationDb();
    auto const& ecClass = instance.GetClass();
    AutoHandledPropertiesCollection propCollection(ecClass, db, ECSqlClassParams::StatementType::All, false);
    for (auto prop : propCollection)
        {
        if (!prop->GetIsNavigation())
            continue;

        ECValue v;
        instance.GetValue(v, prop->GetName().c_str());
        if (v.IsNull())
            continue;

        DgnElementId sourceElementId = v.GetNavigationInfo().GetId<DgnElementId>();
        DgnElementId targetElementId = importer.FindElementId(sourceElementId);
        if (targetElementId.IsValid())
            v.SetNavigationInfo(targetElementId, importer.RemapClassId(v.GetNavigationInfo().GetRelationshipClassId()));
        else
            v = ECN::ECValue();
        instance.SetValue(prop->GetName().c_str(), v);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static IECInstancePtr cloneInstanceForImport(IECInstanceCR instance, DgnImportContext& importer)
    {
    auto schemaName = instance.GetEnabler().GetClass().GetSchema().GetName();
    auto className = instance.GetEnabler().GetClass().GetName();
    auto dstClass = importer.GetDestinationDb().Schemas().GetClass(schemaName.c_str(), className.c_str());
    if (nullptr == dstClass)
        {
        LOG.errorv("cloneInstanceForImport - class %s.%s not in destination", schemaName.c_str(), className.c_str());
        return nullptr;
        }
    auto newInstance = dstClass->GetDefaultStandaloneEnabler()->CreateInstance();
    newInstance->CopyValues(instance);
    remapAutoHandledNavigationproperties(*newInstance, importer);
    return newInstance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<DgnElement::Aspect> DgnElement::Aspect::_CloneForImport(DgnElementCR el, DgnImportContext& importer) const
    {
    DgnClassId classid = GetECClassId(el.GetDgnDb());
    if (!el.GetElementId().IsValid() || !classid.IsValid())
        return nullptr;

    dgn_AspectHandler::Aspect* handler = dgn_AspectHandler::Aspect::FindHandler(el.GetDgnDb(), classid);
    if ((nullptr == handler) || (handler == &dgn_AspectHandler::Aspect::GetHandler()))
        {
        auto genericUnique = dynamic_cast<GenericUniqueAspect const*>(this);
        if (nullptr != genericUnique)
            {
            if (!genericUnique->m_instance.IsValid())
                return nullptr;
            return new GenericUniqueAspect(*cloneInstanceForImport(*genericUnique->m_instance, importer));
            }

        auto genericMulti = dynamic_cast<GenericMultiAspect const*>(this);
        if (nullptr != genericMulti)
            {
            if (!genericMulti->m_instance.IsValid())
                return nullptr;
            return new GenericMultiAspect(*cloneInstanceForImport(*genericMulti->m_instance, importer), ECInstanceId());
            }

        BeAssert(false);
        return nullptr;
        }

    RefCountedPtr<DgnElement::Aspect> aspect = handler->_CreateInstance().get();
    if (!aspect.IsValid())
        return nullptr;

    // *** WIP_IMPORTER - we read the *persistent* properties -- we don't copy the in-memory copies -- pending changes are ignored!

    if (DgnDbStatus::Success != aspect->_LoadProperties(el))
        return nullptr;

    // Assume that there are no IDs to be remapped.

    return aspect;
    }

/*=================================================================================**//**
* @bsimethod
+===============+===============+===============+===============+===============+======*/
BEGIN_BENTLEY_DGN_NAMESPACE
struct MultiAspectMux : DgnElement::AppData
{
    ECClassCR m_ecclass;
    bvector<RefCountedPtr<DgnElement::MultiAspect>> m_instances;

    static Key& GetKey(ECClassCR cls) {return *(Key*)const_cast<ECClassP>(&cls);}
    Key& GetKey(DgnDbR db) {return GetKey(m_ecclass);}

    static MultiAspectMux* Find(DgnElementCR, ECClassCR);
    static MultiAspectMux& Get(DgnElementCR, ECClassCR);

    MultiAspectMux(ECClassCR cls) : m_ecclass(cls) {;}
    DropMe _OnInserted(DgnElementCR el) override;
    DropMe _OnUpdated(DgnElementCR modified, DgnElementCR original, bool isOriginal) override;
};

END_BENTLEY_DGN_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MultiAspectMux* MultiAspectMux::Find(DgnElementCR el, ECClassCR cls)
    {
    auto appData = el.FindAppData(GetKey(cls));
    if (appData.IsNull())
        return nullptr;

    MultiAspectMux* mux = dynamic_cast<MultiAspectMux*>(appData.get());
    BeAssert(nullptr != mux && "The same ECClass cannot have both Unique and MultiAspects");
    return mux;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MultiAspectMux& MultiAspectMux::Get(DgnElementCR el, ECClassCR cls)
    {
    return *el.ObtainAppData(GetKey(cls), [&]() { return new MultiAspectMux(cls); });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::AppData::DropMe MultiAspectMux::_OnInserted(DgnElementCR el)
    {
    for (auto aspect : m_instances)
        aspect->_OnInserted(el);

    return DropMe::Yes; // all scheduled changes have been processed, so remove them.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::AppData::DropMe MultiAspectMux::_OnUpdated(DgnElementCR modified, DgnElementCR original, bool isOriginal)
    {
    for (auto aspect : m_instances)
        aspect->_OnUpdated(modified, original, isOriginal);

    return DropMe::Yes; // all scheduled changes have been processed, so remove them.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::MultiAspect::_DeleteInstance(DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    // I am assuming that the ElementOwnsAspects ECRelationship is either just a foreign key column on the aspect or that ECSql somehow deletes the relationship instance automatically.
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(Utf8PrintfString("DELETE FROM %s WHERE ECInstanceId=?", GetFullEcSqlKeyClassName().c_str()).c_str(), writeToken);
    stmt->BindId(1, m_instanceId);
    BeSQLite::DbResult status = stmt->Step();
    return (BeSQLite::BE_SQLITE_DONE == status) ? DgnDbStatus::Success : DgnDbStatus::WriteError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::MultiAspect::_InsertInstance(DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(Utf8PrintfString("INSERT INTO %s (Element.Id,Element.RelECClassId) VALUES (?,?)", GetFullEcSqlClassName().c_str()).c_str(), writeToken);
    if (stmt == nullptr)
        {
        BeAssert(false);
        return DgnDbStatus::WriteError;
        }

    ECN::ECValue v;
    GetPropertyValue(v, "Element");
    DgnClassId relClassId;
    if (!v.IsNull())
        relClassId = DgnClassId(v.GetNavigationInfo().GetRelationshipClassId().GetValueUnchecked());
    if (!relClassId.IsValid())
        relClassId = el.GetDgnDb().Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsMultiAspects);

    if ((ECSqlStatus::Success != stmt->BindId(1, el.GetElementId())) ||
        (ECSqlStatus::Success != stmt->BindId(2, relClassId)))
        return DgnDbStatus::WriteError;

    ECInstanceKey key;
    if (BeSQLite::BE_SQLITE_DONE != stmt->Step(key))
        return DgnDbStatus::WriteError;

    m_instanceId = key.GetInstanceId();
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<DgnElement::MultiAspect> DgnElement::MultiAspect::CreateAspect(DgnDbR db, ECClassCR aspectClass)
    {
    if (!aspectClass.Is(BIS_ECSCHEMA_NAME, BIS_CLASS_ElementMultiAspect))
        return nullptr;

    dgn_AspectHandler::Aspect* aspectHandler = dgn_AspectHandler::Aspect::FindHandler(db, aspectClass.GetId());
    if ((nullptr == aspectHandler) || (aspectHandler == &dgn_AspectHandler::Aspect::GetHandler()))
        {
        StandaloneECInstancePtr aspectInstance = aspectClass.GetDefaultStandaloneEnabler()->CreateInstance();
        return aspectInstance.IsValid() ? new GenericMultiAspect(*aspectInstance, ECInstanceId()) : nullptr;
        }

    return dynamic_cast<DgnElement::MultiAspect*>(aspectHandler->_CreateInstance().get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::MultiAspect const* DgnElement::MultiAspect::GetAspect(DgnElementCR el, ECClassCR cls, ECInstanceId id)
    {
    //  First, see if we already have this particular MultiAspect cached
    MultiAspectMux* mux = MultiAspectMux::Find(el,cls);
    if (nullptr != mux)
        {
        for (RefCountedPtr<MultiAspect> aspect : mux->m_instances)
            {
            if (aspect->GetAspectInstanceId() != id)
                continue;
            if (aspect->m_changeType == ChangeType::Delete)
                return nullptr;
            return aspect.get();
            }
        }

    //  First time we've been asked for this particular aspect. Cache it.

    RefCountedPtr<MultiAspect> aspect;

    dgn_AspectHandler::Aspect* handler = dgn_AspectHandler::Aspect::FindHandler(el.GetDgnDb(), DgnClassId(cls.GetId()));
    if ((nullptr == handler) || handler == &dgn_AspectHandler::Aspect::GetHandler())
        aspect = new GenericMultiAspect(cls, id);
    else
        aspect = dynamic_cast<MultiAspect*>(handler->_CreateInstance().get());

    if (!aspect.IsValid())
        return nullptr;

    aspect->m_instanceId = id;

    if (DgnDbStatus::Success != aspect->_LoadProperties(el))
        return nullptr;

    MultiAspectMux::Get(el,cls).m_instances.push_back(aspect);

    return aspect.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::MultiAspect* DgnElement::MultiAspect::GetAspectP(DgnElementR el, ECClassCR cls, ECInstanceId id)
    {
    auto* aspect = const_cast<MultiAspect*>(GetAspect(el,cls,id));
    if (nullptr == aspect)
        return aspect;
    aspect->m_changeType = ChangeType::Write;
    return aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::MultiAspect::AddAspect(DgnElementR el, MultiAspect& aspect)
    {
    ECClassCP cls = aspect.GetECClass(el.GetDgnDb());
    if (nullptr == cls)
        {
        BeAssert(false && "aspect must know its class");
        return;
        }
    MultiAspectMux::Get(el,*cls).m_instances.push_back(&aspect);
    aspect.m_changeType = ChangeType::Write;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECInstanceKey DgnElement::MultiAspect::_QueryExistingInstanceKey(DgnElementCR el)
    {
    // My m_instanceId field is valid if and only if I was just inserted or was loaded from an existing instance.
    return ECInstanceKey(GetECClassId(el.GetDgnDb()), m_instanceId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<DgnElement::UniqueAspect> DgnElement::UniqueAspect::CreateAspect(DgnDbR db, ECClassCR aspectClass)
    {
    if (!aspectClass.Is(BIS_ECSCHEMA_NAME, BIS_CLASS_ElementUniqueAspect))
        return nullptr;

    dgn_AspectHandler::Aspect* aspectHandler = dgn_AspectHandler::Aspect::FindHandler(db, aspectClass.GetId());
    if ((nullptr == aspectHandler) || (aspectHandler == &dgn_AspectHandler::Aspect::GetHandler()))
        {
        StandaloneECInstancePtr aspectInstance = aspectClass.GetDefaultStandaloneEnabler()->CreateInstance();
        return aspectInstance.IsValid() ? new GenericUniqueAspect(*aspectInstance) : nullptr;
        }

    return dynamic_cast<DgnElement::UniqueAspect*>(aspectHandler->_CreateInstance().get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::UniqueAspect* DgnElement::UniqueAspect::Find(DgnElementCR el, ECClassCR cls)
    {
    AppDataPtr appData = el.FindAppData(GetKey(cls));
    if (appData.IsNull())
        return nullptr;

    UniqueAspect* aspect = dynamic_cast<UniqueAspect*>(appData.get());
    BeAssert(nullptr != aspect && "The same ECClass cannot have both Unique and MultiAspects");
    return aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::UniqueAspect::SetAspect(DgnElementR el, UniqueAspect& newAspect)
    {
    SetAspect0(el, newAspect);
    BeAssert(nullptr != Find(el, *newAspect.GetKeyECClass(el.GetDgnDb())));
    newAspect.m_changeType = ChangeType::Write;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::UniqueAspect const* DgnElement::UniqueAspect::GetAspect(DgnElementCR el, ECClassCR cls)
    {
    UniqueAspect const* aspect = Find(el,cls);
    if (nullptr == aspect)
        {
        aspect = Load(el,DgnClassId(cls.GetId()));
        }
    else
        {
        if (aspect->m_changeType == ChangeType::Delete)
            aspect = nullptr;
        }

    return aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::UniqueAspect* DgnElement::UniqueAspect::GetAspectP(DgnElementR el, ECClassCR cls)
    {
    UniqueAspect* aspect = const_cast<UniqueAspect*>(GetAspect(el,cls));
    if (nullptr == aspect)
        return aspect;
    aspect->m_changeType = ChangeType::Write;
    return aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::UniqueAspect::SetAspect0(DgnElementCR el, UniqueAspect& newItem)
    {
    Key& key = newItem.GetKey(el.GetDgnDb());
    el.ReplaceAppData(key, &newItem);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<DgnElement::UniqueAspect> DgnElement::UniqueAspect::Load0(DgnElementCR el, DgnClassId classid)
    {
    if (!el.GetElementId().IsValid() || !classid.IsValid())
        return nullptr;

    RefCountedPtr<DgnElement::UniqueAspect> aspect;

    dgn_AspectHandler::Aspect* handler = dgn_AspectHandler::Aspect::FindHandler(el.GetDgnDb(), classid);
    if ((nullptr == handler) || handler == &dgn_AspectHandler::Aspect::GetHandler())
        {
        auto eclass = el.GetDgnDb().Schemas().GetClass(classid);
        if (nullptr == eclass)
            return nullptr;
        aspect = new GenericUniqueAspect(*eclass);
        }
    else
        {
        aspect = dynamic_cast<DgnElement::UniqueAspect*>(handler->_CreateInstance().get());
        }

    if (!aspect.IsValid())
        return nullptr;

    if (DgnDbStatus::Success != aspect->_LoadProperties(el))
        return nullptr;

    return aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::UniqueAspect* DgnElement::UniqueAspect::Load(DgnElementCR el, DgnClassId classid)
    {
    RefCountedPtr<DgnElement::UniqueAspect> aspect = Load0(el, classid);
    if (!aspect.IsValid())
        return nullptr;

    SetAspect0(el, *aspect);
    aspect->m_changeType = ChangeType::None; // aspect starts out clean
    return aspect.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::UniqueAspect::_InsertInstance(DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    // Note that we use the exact class of this when we insert, not the key class, so that the relationship identifies the aspect accurately.
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(Utf8PrintfString("INSERT INTO %s (Element.Id,Element.RelECClassId) VALUES (?,?)", GetFullEcSqlClassName().c_str()).c_str(), writeToken);
    if (!stmt.IsValid())
        return DgnDbStatus::WriteError;

    ECN::ECValue v;
    GetPropertyValue(v, "Element");
    DgnClassId relClassId;
    if (!v.IsNull())
        relClassId = DgnClassId(v.GetNavigationInfo().GetRelationshipClassId().GetValueUnchecked());
    if (!relClassId.IsValid())
        relClassId = el.GetDgnDb().Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsUniqueAspect);

    if ((ECSqlStatus::Success != stmt->BindId(1, el.GetElementId())) ||
        (ECSqlStatus::Success != stmt->BindId(2, relClassId)))
        return DgnDbStatus::WriteError;

    ECInstanceKey key;
    if (BeSQLite::BE_SQLITE_DONE != stmt->Step(key))
        return DgnDbStatus::WriteError;

    m_instanceId = key.GetInstanceId();
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::UniqueAspect::_DeleteInstance(DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const* writeToken)
    {
    // I am assuming that the ElementOwnsAspects ECRelationship is either just a foreign key column on the aspect or that ECSql somehow deletes the relationship instance automatically.
    // Tricky: We want to delete whatever aspect is stored in the db. It may be a different subclass of the key class than this (which we might be about to write in its place).
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(Utf8PrintfString("DELETE FROM %s WHERE Element.Id=?", GetFullEcSqlKeyClassName().c_str()).c_str(), writeToken);

    if (ECSqlStatus::Success != stmt->BindId(1, el.GetElementId()))
        return DgnDbStatus::WriteError;

    DbResult status = stmt->Step();
    return (BE_SQLITE_DONE == status) ? DgnDbStatus::Success : DgnDbStatus::WriteError;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECInstanceKey DgnElement::UniqueAspect::_QueryExistingInstanceKey(DgnElementCR el)
    {
    // We know the key class and the ID of an instance, if it exists. See if such an instance actually exists.
    // Note that we must use the key class, not the actual class of this (which we might be about to write and which be a different subclass from the existing stored aspect).
    //CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("SELECT ECInstanceId, ECClassId FROM %s WHERE Element.Id=?", GetFullEcSqlKeyClassName().c_str()).c_str());
    CachedStatementPtr stmt = el.GetDgnDb().GetCachedStatement(Utf8PrintfString("SELECT Id, ECClassId from bis_ElementUniqueAspect WHERE ECClassId in ( SELECT ClassId from ec_cache_ClassHierarchy WHERE BaseClassId = ?) AND ElementId =? ").c_str());
    if (stmt == nullptr)
        {
        BeAssert(stmt != nullptr);
        return ECInstanceKey();
        }

    if (DbResult::BE_SQLITE_OK != stmt->BindId(1, GetKeyECClassId(el.GetDgnDb())))
        return ECInstanceKey();

    //if (ECSqlStatus::Success != stmt->BindId(1, el.GetElementId()))
    if (DbResult::BE_SQLITE_OK != stmt->BindId(2, el.GetElementId()))
        return ECInstanceKey();

    if (BE_SQLITE_ROW != stmt->Step())
        return ECInstanceKey();

    // And we know the ID. See if such an instance actually exists.
    return ECInstanceKey(stmt->GetValueId<ECClassId>(1), stmt->GetValueId<ECInstanceId>(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_ElementHandler::Element::_RegisterPropertyAccessors(ECSqlClassInfo& params, ECN::ClassLayoutCR layout)
    {
    params.RegisterPropertyAccessors(layout, BIS_ELEMENT_PROP_FederationGuid,
        [](ECValueR value, DgnElementCR el)
            {
            value.SetBinary((Byte*)const_cast<BeGuid*>(&el.m_federationGuid), sizeof(el.m_federationGuid));
            return DgnDbStatus::Success;
            },

        [](DgnElementR el, ECValueCR value)
            {
            if (!value.IsBinary())
                return DgnDbStatus::BadArg;
            size_t sz;
            Byte const* p = value.GetBinary(sz);
            if (sz != sizeof(BeGuid))
                return DgnDbStatus::BadArg;
            BeGuid guid;
            memcpy(&guid, p, sz);
            el.SetFederationGuid(guid);
            return DgnDbStatus::Success;
            });

    params.RegisterPropertyAccessors(layout, BIS_ELEMENT_PROP_CodeValue,
        [](ECValueR value, DgnElementCR el)
            {
            value.SetUtf8CP(el.GetCode().GetValueUtf8().c_str());
            return DgnDbStatus::Success;
            },

        [](DgnElementR el, ECValueCR value)
            {
            return DgnDbStatus::ReadOnly; // must set CodeSpec, CodeScope, and CodeValue together
            });

    params.RegisterPropertyAccessors(layout, BIS_ELEMENT_PROP_CodeScope,
        [](ECValueR value, DgnElementCR el)
            {
            value.SetNavigationInfo(el.GetCode().GetScopeElementId(el.GetDgnDb()));
            return DgnDbStatus::Success;
            },

        [](DgnElementR el, ECValueCR value)
            {
            return DgnDbStatus::ReadOnly; // must set CodeSpec, CodeScope, and CodeValue together
            });

    params.RegisterPropertyAccessors(layout, BIS_ELEMENT_PROP_CodeSpec,
        [](ECValueR value, DgnElementCR el)
            {
            value.SetNavigationInfo(el.GetCode().GetCodeSpecId());
            return DgnDbStatus::Success;
            },

        [](DgnElementR el, ECValueCR value)
            {
            return DgnDbStatus::ReadOnly; // must set CodeSpec, CodeScope, and CodeValue together
            });

    params.RegisterPropertyAccessors(layout, BIS_ELEMENT_PROP_Model,
        [](ECValueR value, DgnElementCR el)
            {
            value.SetNavigationInfo(el.GetModelId());
            return DgnDbStatus::Success;
            },

        [](DgnElementR el, ECValueCR value)
            {
            return DgnDbStatus::ReadOnly;
            });

    params.RegisterPropertyAccessors(layout, BIS_ELEMENT_PROP_Parent,
        [](ECValueR value, DgnElementCR el)
            {
            value.SetNavigationInfo(el.GetParentId(), el.GetParentRelClassId());
            return DgnDbStatus::Success;
            },

        [](DgnElementR el, ECValueCR value)
            {
            if (!value.IsNavigation())
                return DgnDbStatus::BadArg;
            return el.SetParentId(value.GetNavigationInfo().GetId<DgnElementId>(), value.GetNavigationInfo().GetRelationshipClassId());
            });

    params.RegisterPropertyAccessors(layout, BIS_ELEMENT_PROP_UserLabel,
        [](ECValueR value, DgnElementCR el)
            {
            value.SetUtf8CP(el.GetUserLabel());
            return DgnDbStatus::Success;
            },

        [](DgnElementR el, ECValueCR value)
            {
            if (!value.IsString())
                return DgnDbStatus::BadArg;
            el.SetUserLabel(value.ToString().c_str());
            return DgnDbStatus::Success;
            });

    params.RegisterPropertyAccessors(layout, BIS_ELEMENT_PROP_JsonProperties,
        [](ECValueR value, DgnElementCR el)
            {
            value.SetUtf8CP(el.ToJsonPropString().c_str());
            return DgnDbStatus::Success;
            },

        [](DgnElementR el, ECValueCR value)
            {
            if (!value.IsUtf8())
                return DgnDbStatus::BadArg;

            el.m_jsonProps.Parse(value.GetUtf8CP());
            el._OnLoadedJsonProperties();
            return DgnDbStatus::Success;
            });

    params.RegisterPropertyAccessors(layout, BIS_ELEMENT_PROP_LastMod,
        [](ECValueR value, DgnElementCR el)
            {
            value.SetDateTime(el.QueryLastModifyTime());
            return DgnDbStatus::Success;
            },

        [](DgnElementR el, ECValueCR value)
            {
            return DgnDbStatus::ReadOnly;
            });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement::RegisterGeometricPropertyAccessors(ECSqlClassInfo& params, ECN::ClassLayoutCR layout)
    {
    params.RegisterPropertyAccessors(layout, prop_GeometryStream(),
        [](ECValueR, DgnElementCR)
            {
            return DgnDbStatus::BadRequest;//  => Use GeometryCollection interface
            },

        [](DgnElementR, ECValueCR)
            {
            return DgnDbStatus::BadRequest;//  => Use GeometryBuilder
            });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_ElementHandler::Geometric3d::_RegisterPropertyAccessors(ECSqlClassInfo& params, ECN::ClassLayoutCR layout)
    {
    T_Super::_RegisterPropertyAccessors(params, layout);

#define GETGEOMPLCPROPDBL(EXPR) [](ECValueR value, DgnElementCR elIn){GeometricElement3d const& el = (GeometricElement3d const&)elIn; Placement3dCR plc = el.GetPlacement(); value.SetDouble(EXPR); return DgnDbStatus::Success;}
#define GETGEOMPLCPROPPT3(EXPR) [](ECValueR value, DgnElementCR elIn){GeometricElement3d const& el = (GeometricElement3d const&)elIn; Placement3dCR plc = el.GetPlacement(); value.SetPoint3d(EXPR); return DgnDbStatus::Success;}
#define SETGEOMPLCPROP(PTYPE, EXPR) [](DgnElement& elIn, ECN::ECValueCR valueIn)\
            {                                                                          \
            if (valueIn.IsNull() || valueIn.IsBoolean() || !valueIn.IsPrimitive())       \
                return DgnDbStatus::BadArg;                                              \
            ECN::ECValue value(valueIn);                                                 \
            if (!value.ConvertToPrimitiveType(PTYPE))                                    \
                return DgnDbStatus::BadArg;                                              \
            GeometricElement3d& el = (GeometricElement3d&)elIn;                          \
            Placement3d plc = el.GetPlacement();                                         \
            EXPR;                                                                        \
            return el.SetPlacement(plc);                                                 \
            }


    params.RegisterPropertyAccessors(layout, GeometricElement3d::prop_Yaw(),
        GETGEOMPLCPROPDBL(plc.GetAngles().GetYaw().Degrees()),
        SETGEOMPLCPROP(ECN::PRIMITIVETYPE_Double, plc.GetAnglesR().SetYaw(AngleInDegrees::FromDegrees(value.GetDouble()))));

    params.RegisterPropertyAccessors(layout, GeometricElement3d::prop_Pitch(),
        GETGEOMPLCPROPDBL(plc.GetAngles().GetPitch().Degrees()),
        SETGEOMPLCPROP(ECN::PRIMITIVETYPE_Double, plc.GetAnglesR().SetPitch(AngleInDegrees::FromDegrees(value.GetDouble()))));

    params.RegisterPropertyAccessors(layout, GeometricElement3d::prop_Roll(),
        GETGEOMPLCPROPDBL(plc.GetAngles().GetRoll().Degrees()),
        SETGEOMPLCPROP(ECN::PRIMITIVETYPE_Double, plc.GetAnglesR().SetRoll(AngleInDegrees::FromDegrees(value.GetDouble()))));

    params.RegisterPropertyAccessors(layout, GeometricElement3d::prop_Origin(),
        GETGEOMPLCPROPPT3(plc.GetOrigin()),
        SETGEOMPLCPROP(ECN::PRIMITIVETYPE_Point3d, plc.GetOriginR() = value.GetPoint3d()));

    params.RegisterPropertyAccessors(layout, GeometricElement3d::prop_BBoxLow(),
        GETGEOMPLCPROPPT3(plc.GetElementBox().low),
        SETGEOMPLCPROP(ECN::PRIMITIVETYPE_Point3d, plc.GetElementBoxR().low = value.GetPoint3d()));

    params.RegisterPropertyAccessors(layout, GeometricElement3d::prop_BBoxHigh(),
        GETGEOMPLCPROPPT3(plc.GetElementBox().high),
        SETGEOMPLCPROP(ECN::PRIMITIVETYPE_Point3d, plc.GetElementBoxR().high = value.GetPoint3d()));

#undef GETGEOMPLCPROPDBL
#undef GETGEOMPLCPROPPT3
#undef SETGEOMPLCPROP

    params.RegisterPropertyAccessors(layout, GeometricElement::prop_Category(),
        [](ECValueR value, DgnElementCR elIn)
            {
            GeometricElement3d const& el = (GeometricElement3d const&)elIn;
            value.SetNavigationInfo(el.GetCategoryId());
            return DgnDbStatus::Success;
            },
        [](DgnElementR elIn, ECValueCR value)
            {
            if (value.IsNull() || !value.IsNavigation())
                {
                BeAssert(false);
                return DgnDbStatus::BadArg;
                }
            GeometricElement3d& el = (GeometricElement3d&)elIn;
            return el.SetCategoryId(value.GetNavigationInfo().GetId<DgnCategoryId>());
            });

    params.RegisterPropertyAccessors(layout, GeometricElement::prop_TypeDefinition(),
        [](ECValueR value, DgnElementCR elIn)
            {
            GeometricElement3dCR el = static_cast<GeometricElement3dCR>(elIn);
            value.SetNavigationInfo(el.m_typeDefinition.m_id, el.m_typeDefinition.m_relClassId);
            return DgnDbStatus::Success;
            },
        [](DgnElementR elIn, ECValueCR value)
            {
            if (value.IsNull() || !value.IsNavigation())
                {
                BeAssert(false);
                return DgnDbStatus::BadArg;
                }
            GeometricElement3dR el = static_cast<GeometricElement3dR>(elIn);
            return el.SetTypeDefinition(value.GetNavigationInfo().GetId<DgnElementId>(), value.GetNavigationInfo().GetRelationshipClassId());
            });

    params.RegisterPropertyAccessors(layout, GeometricElement3d::prop_InSpatialIndex(),
        [](ECValueR value, DgnElementCR el)
            {
            value.SetBoolean(!el.GetModel()->IsTemplate());
            return DgnDbStatus::Success;
            },
        [](DgnElementR, ECValueCR)
            {
            return DgnDbStatus::ReadOnly;
            });

    GeometricElement::RegisterGeometricPropertyAccessors(params, layout);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_ElementHandler::Geometric2d::_RegisterPropertyAccessors(ECSqlClassInfo& params, ECN::ClassLayoutCR layout)
    {
    T_Super::_RegisterPropertyAccessors(params, layout);

#define GETGEOMPLCPROPDBL(EXPR) [](ECValueR value, DgnElementCR elIn)\
            {                                                                          \
            GeometricElement2d const& el = (GeometricElement2d const&)elIn;                          \
            Placement2dCR plc = el.GetPlacement();                                       \
            value.SetDouble(EXPR);                                                       \
            return DgnDbStatus::Success;                                                 \
            }
#define GETGEOMPLCPROPPT2(EXPR) [](ECValueR value, DgnElementCR elIn)\
            {                                                                          \
            GeometricElement2d const& el = (GeometricElement2d const&)elIn;                          \
            Placement2dCR plc = el.GetPlacement();                                       \
            value.SetPoint2d(EXPR);                                                      \
            return DgnDbStatus::Success;                                                 \
            }
#define SETGEOMPLCPROP(PTYPE, EXPR) [](DgnElement& elIn, ECN::ECValueCR valueIn)\
            {                                                                          \
            if (valueIn.IsNull() || valueIn.IsBoolean() || !valueIn.IsPrimitive())       \
                return DgnDbStatus::BadArg;                                              \
            ECN::ECValue value(valueIn);                                                 \
            if (!value.ConvertToPrimitiveType(PTYPE))                                    \
                return DgnDbStatus::BadArg;                                              \
            GeometricElement2d& el = (GeometricElement2d&)elIn;                          \
            Placement2d plc = el.GetPlacement();                                         \
            EXPR;                                                                        \
            return el.SetPlacement(plc);                                                 \
            }

    params.RegisterPropertyAccessors(layout, GeometricElement::prop_Origin(),
        GETGEOMPLCPROPPT2(plc.GetOrigin()),
        SETGEOMPLCPROP(ECN::PRIMITIVETYPE_Point2d, plc.GetOriginR() = value.GetPoint2d()));

    params.RegisterPropertyAccessors(layout, GeometricElement2d::prop_Rotation(),
        GETGEOMPLCPROPDBL(plc.GetAngle().Degrees()),
        SETGEOMPLCPROP(ECN::PRIMITIVETYPE_Double, plc.GetAngleR() = AngleInDegrees::FromDegrees(value.GetDouble())));

    params.RegisterPropertyAccessors(layout, GeometricElement::prop_BBoxLow(),
        GETGEOMPLCPROPPT2(plc.GetElementBox().low),
        SETGEOMPLCPROP(ECN::PRIMITIVETYPE_Point2d, plc.GetElementBoxR().low = value.GetPoint2d()));

    params.RegisterPropertyAccessors(layout, GeometricElement::prop_BBoxHigh(),
        GETGEOMPLCPROPPT2(plc.GetElementBox().high),
        SETGEOMPLCPROP(ECN::PRIMITIVETYPE_Point2d, plc.GetElementBoxR().high = value.GetPoint2d()));

    params.RegisterPropertyAccessors(layout, GeometricElement::prop_Category(),
        [](ECValueR value, DgnElementCR elIn)
            {
            GeometricElement2d const& el = (GeometricElement2d const&)elIn;
            value.SetNavigationInfo(el.GetCategoryId());
            return DgnDbStatus::Success;
            },
        [](DgnElementR elIn, ECValueCR value)
            {
            if (value.IsNull() || !value.IsNavigation())
                {
                BeAssert(false);
                return DgnDbStatus::BadArg;
                }
            GeometricElement2d& el = (GeometricElement2d&)elIn;
            return el.SetCategoryId(value.GetNavigationInfo().GetId<DgnCategoryId>());
            });

    params.RegisterPropertyAccessors(layout, GeometricElement::prop_TypeDefinition(),
        [](ECValueR value, DgnElementCR elIn)
            {
            GeometricElement2dCR el = static_cast<GeometricElement2dCR>(elIn);
            value.SetNavigationInfo(el.m_typeDefinition.m_id, el.m_typeDefinition.m_relClassId);
            return DgnDbStatus::Success;
            },
        [](DgnElementR elIn, ECValueCR value)
            {
            if (value.IsNull() || !value.IsNavigation())
                {
                BeAssert(false);
                return DgnDbStatus::BadArg;
                }
            GeometricElement2dR el = static_cast<GeometricElement2dR>(elIn);
            return el.SetTypeDefinition(value.GetNavigationInfo().GetId<DgnElementId>(), value.GetNavigationInfo().GetRelationshipClassId());
            });

    GeometricElement::RegisterGeometricPropertyAccessors(params, layout);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_OnChildInsert(DgnElementCR child) const
    {
    if (GetModelId() != child.GetModelId())
        {
        BeAssert(false);
        return DgnDbStatus::WrongModel; // parent and child must be in same model
        }

    CallJsChildPreHandler(child, "onChildInsert");
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_OnChildUpdate(DgnElementCR, DgnElementCR child) const
    {
    CallJsChildPreHandler(child, "onChildUpdate");
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_OnChildDelete(DgnElementCR child) const
    {
    CallJsChildPostHandler(child, "onChildDelete");
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_OnChildAdd(DgnElementCR child) const
    {
    if (GetModelId() != child.GetModelId())
        {
        BeAssert(false);
        return DgnDbStatus::WrongModel; // parent and child must be in same model
        }
    CallJsChildPreHandler(child, "onChildAdd");
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_OnChildDrop(DgnElementCR child) const
    {
    CallJsChildPostHandler(child, "onChildDrop");
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::SetCode(DgnCodeCR newCode)
    {
    DgnCode oldCode = GetCode();
    if (oldCode == newCode)
        return DgnDbStatus::Success;

    m_code = newCode;
    return DgnDbStatus::Success; // WIP: Validation?
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpecCPtr DgnElement::GetCodeSpec() const
    {
    return GetDgnDb().CodeSpecs().GetCodeSpec(GetCode().GetCodeSpecId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement::DoSetCategoryId(DgnCategoryId catId)
    {
    m_categoryId = catId;
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement2d::_SetCategoryId(DgnCategoryId categoryId)
    {
    if (!DrawingCategory::Get(GetDgnDb(), categoryId).IsValid())
        return DgnDbStatus::InvalidCategory; // A GeometricElement2d requires an existing DrawingCategory

    return DoSetCategoryId(categoryId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement3d::_SetCategoryId(DgnCategoryId categoryId)
    {
    if (!SpatialCategory::Get(GetDgnDb(), categoryId).IsValid())
        return DgnDbStatus::InvalidCategory; // A GeometricElement3d requires an existing SpatialCategory

    return DoSetCategoryId(categoryId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement2d::_SetPlacement(Placement2dCR placement)
    {
    m_placement = placement;
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement3d::_SetPlacement(Placement3dCR placement)
    {
    m_placement = placement;
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement2d::SetTypeDefinition(DgnElementId typeDefinitionId, DgnClassId typeDefinitionRelClassId)
    {
    // WIP: Validation
    m_typeDefinition.m_id = typeDefinitionId;
    m_typeDefinition.m_relClassId = typeDefinitionId.IsValid() ? typeDefinitionRelClassId : DgnClassId();
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement3d::SetTypeDefinition(DgnElementId typeDefinitionId, DgnClassId typeDefinitionRelClassId)
    {
    // WIP: Validation
    m_typeDefinition.m_id = typeDefinitionId;
    m_typeDefinition.m_relClassId = typeDefinitionId.IsValid() ? typeDefinitionRelClassId : DgnClassId();
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RecipeDefinitionElementCPtr TypeDefinitionElement::GetRecipe() const
    {
    return GetDgnDb().Elements().Get<RecipeDefinitionElement>(GetRecipeId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalTypeCPtr PhysicalElement::GetPhysicalType() const
    {
    return GetDgnDb().Elements().Get<PhysicalType>(GetTypeDefinitionId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode PhysicalType::CreateCode(DefinitionModelCR model, Utf8StringCR name)
    {
    return CodeSpec::CreateCode(BIS_CODESPEC_PhysicalType, model, name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialLocationTypeCPtr SpatialLocationElement::GetSpatialLocationType() const
    {
    return GetDgnDb().Elements().Get<SpatialLocationType>(GetTypeDefinitionId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode SpatialLocationType::CreateCode(DefinitionModelCR model, Utf8StringCR name)
    {
    return CodeSpec::CreateCode(BIS_CODESPEC_SpatialLocationType, model, name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode TemplateRecipe3d::CreateCode(DefinitionModelCR model, Utf8StringCR name)
    {
    return CodeSpec::CreateCode(BIS_CODESPEC_TemplateRecipe3d, model, name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TemplateRecipe3dPtr TemplateRecipe3d::Create(DefinitionModelCR model, Utf8StringCR name)
    {
    DgnDbR db = model.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(dgn_ElementHandler::TemplateRecipe3d::GetHandler());
    DgnCode code = CreateCode(model, name);
    return new TemplateRecipe3d(CreateParams(db, model.GetModelId(), classId, code));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicalType2dCPtr GraphicalElement2d::GetGraphicalType() const
    {
    return GetDgnDb().Elements().Get<GraphicalType2d>(GetTypeDefinitionId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode GraphicalType2d::CreateCode(DefinitionModelCR model, Utf8StringCR name)
    {
    return CodeSpec::CreateCode(BIS_CODESPEC_GraphicalType2d, model, name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode TemplateRecipe2d::CreateCode(DefinitionModelCR model, Utf8StringCR name)
    {
    return CodeSpec::CreateCode(BIS_CODESPEC_TemplateRecipe2d, model, name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TemplateRecipe2dPtr TemplateRecipe2d::Create(DefinitionModelCR model, Utf8StringCR name)
    {
    DgnDbR db = model.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(dgn_ElementHandler::TemplateRecipe2d::GetHandler());
    DgnCode code = CreateCode(model, name);
    return new TemplateRecipe2d(CreateParams(db, model.GetModelId(), classId, code));
    }

BEGIN_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename MapType>
static DbResult BulkInsertMappingsToDb(Db& db, const char* tableName, const MapType& NativeMap)
    {
    CachedStatementPtr stmt = db.GetCachedStatement(Utf8PrintfString("INSERT INTO %s VALUES (?, ?)", tableName).c_str());
    if (!stmt.IsValid())
        return BE_SQLITE_ERROR;
    for (const auto& Remapping : NativeMap)
        {
        DbResult result;
        stmt->Reset();
        if (BE_SQLITE_OK != (result = stmt->BindId(1, Remapping.first))
         || BE_SQLITE_OK != (result = stmt->BindId(2, Remapping.second))
         || BE_SQLITE_DONE != (result = stmt->Step()))
            return result;
        }
    return db.SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename MapType>
static DbResult BulkReadMappingsFromDb(const Db& db, const char* tableName, MapType& NativeMap)
    {
    DbResult result;
    CachedStatementPtr stmt = db.GetCachedStatement(Utf8PrintfString("SELECT Source, Target FROM %s", tableName).c_str());
    if (!stmt.IsValid())
        return BE_SQLITE_ERROR;
    while (BE_SQLITE_ROW == (result = stmt->Step()))
        {
        auto&& key = stmt->GetValueId<typename MapType::key_type>(0);
        auto&& val = stmt->GetValueId<typename MapType::data_type>(1);
        NativeMap.Insert(key, val);
        }
    if (result == BE_SQLITE_DONE)
        result = BE_SQLITE_OK;
    return result;
    }

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnImportContext::SaveStateToDb(BeSQLite::Db& db) const
    {
    DbResult result = BE_SQLITE_OK;
    if (BE_SQLITE_OK != (result = db.CreateTable("InstanceMembers", R"sql(
            xOffset REAL,
            yOffset REAL,
            zOffset REAL,
            yawAdjDegrees REAL,
            areCompatibleDbs BOOLEAN
        )sql"))
     || BE_SQLITE_OK != (result = db.CreateTable("CodeSpecIdRemaps", "Source INTEGER, Target INTEGER"))
     || BE_SQLITE_OK != (result = db.CreateTable("ElementIdRemaps", "Source INTEGER, Target INTEGER"))
     || BE_SQLITE_OK != (result = db.CreateTable("ClassIdRemaps", "Source INTEGER, Target INTEGER"))
     || BE_SQLITE_OK != (result = db.CreateTable("FontIdRemaps", "Source INTEGER, Target INTEGER"))
     || BE_SQLITE_OK != (result = db.CreateTable("FilteredSubCategoryIds", "Id INTEGER"))
     || BE_SQLITE_OK != (result = BulkInsertMappingsToDb(db, "CodeSpecIdRemaps", m_remap.m_codeSpecId))
     || BE_SQLITE_OK != (result = BulkInsertMappingsToDb(db, "ElementIdRemaps", m_remap.m_elementId))
     || BE_SQLITE_OK != (result = BulkInsertMappingsToDb(db, "ClassIdRemaps", m_remap.m_classId))
     || BE_SQLITE_OK != (result = BulkInsertMappingsToDb(db, "FontIdRemaps", m_remap.m_fontId)))
        return result;
    CachedStatementPtr insertFilteredSubCatsStmt = db.GetCachedStatement(R"sql( INSERT INTO FilteredSubCategoryIds VALUES (?) )sql");
    if (!insertFilteredSubCatsStmt.IsValid())
        return BE_SQLITE_ERROR;
    for (const auto& subCatId : m_filteredSubCategoryIds)
        {
        insertFilteredSubCatsStmt->BindId(1, subCatId);
        if (BE_SQLITE_DONE != (result = insertFilteredSubCatsStmt->Step()))
            return result;
        }
    CachedStatementPtr insertInstanceMembersStmt = db.GetCachedStatement(R"sql( INSERT INTO InstanceMembers VALUES (?,?,?,?,?) )sql");
    if (!insertInstanceMembersStmt.IsValid())
        return BE_SQLITE_ERROR;
    insertInstanceMembersStmt->BindDouble(1, m_xyzOffset.x);
    insertInstanceMembersStmt->BindDouble(2, m_xyzOffset.y);
    insertInstanceMembersStmt->BindDouble(3, m_xyzOffset.z);
    insertInstanceMembersStmt->BindDouble(4, m_yawAdj.Degrees());
    insertInstanceMembersStmt->BindBoolean(5, m_areCompatibleDbs);
    if (BE_SQLITE_DONE != (result = insertInstanceMembersStmt->Step()))
        return result;
    return db.SaveChanges();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnImportContext::LoadStateFromDb(const BeSQLite::Db& db)
    {
    DbResult result = BE_SQLITE_OK;
    if (BE_SQLITE_OK != (result = BulkReadMappingsFromDb(db, "CodeSpecIdRemaps", m_remap.m_codeSpecId))
     || BE_SQLITE_OK != (result = BulkReadMappingsFromDb(db, "ElementIdRemaps", m_remap.m_elementId))
     || BE_SQLITE_OK != (result = BulkReadMappingsFromDb(db, "ClassIdRemaps", m_remap.m_classId))
     || BE_SQLITE_OK != (result = BulkReadMappingsFromDb(db, "FontIdRemaps", m_remap.m_fontId)))
        return result;
    CachedStatementPtr selectFilteredSubCatsStmt = db.GetCachedStatement(R"sql( SELECT Id FROM FilteredSubCategoryIds )sql");
    if (!selectFilteredSubCatsStmt.IsValid())
        return BE_SQLITE_ERROR;
    while (BE_SQLITE_ROW == (result = selectFilteredSubCatsStmt->Step()))
        {
        auto&& id = selectFilteredSubCatsStmt->GetValueId<DgnSubCategoryId>(0);
        m_filteredSubCategoryIds.insert(id);
        }
    if (BE_SQLITE_DONE != result)
        return result;
    CachedStatementPtr selectInstanceMembersStmt = db.GetCachedStatement(R"sql( SELECT xOffset, yOffset, zOffset, yawAdjDegrees, areCompatibleDbs FROM InstanceMembers )sql");
    if (!selectInstanceMembersStmt.IsValid())
        return BE_SQLITE_ERROR;
    if (BE_SQLITE_ROW != (result = selectInstanceMembersStmt->Step()))
        return result;
    m_xyzOffset.x = selectInstanceMembersStmt->GetValueDouble(0);
    m_xyzOffset.y = selectInstanceMembersStmt->GetValueDouble(1);
    m_xyzOffset.z = selectInstanceMembersStmt->GetValueDouble(2);
    m_yawAdj.FromDegrees(selectInstanceMembersStmt->GetValueDouble(3));
    m_areCompatibleDbs = selectInstanceMembersStmt->GetValueBoolean(4);
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ElementCopier::ElementCopier(DgnCloneContext& c) : m_context(c), m_copyChildren(true), m_copyGroups(false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr ElementCopier::MakeCopy(DgnDbStatus* statusOut, DgnModelR targetModel, DgnElementCR sourceElement, DgnCodeCR icode, DgnElementId newParentId)
    {
    DgnElementId alreadyCopied = m_context.FindElementId(sourceElement.GetElementId());
    if (alreadyCopied.IsValid())
        return targetModel.GetDgnDb().Elements().Get<PhysicalElement>(alreadyCopied);

    DgnDbStatus ALLOW_NULL_OUTPUT(status, statusOut);

    DgnElement::CreateParams iparams(targetModel.GetDgnDb(), targetModel.GetModelId(), sourceElement.GetElementClassId(), icode);

    DgnElementPtr outputEditElement = sourceElement.Clone(&status, &iparams);
    if (!outputEditElement.IsValid())
        return nullptr;

    if (!newParentId.IsValid())
        {
        DgnElementId remappedParentId = m_context.FindElementId(outputEditElement->GetParentId());
        if (remappedParentId.IsValid())
            newParentId = remappedParentId;
        }

    if (newParentId.IsValid())
        outputEditElement->SetParentId(newParentId, sourceElement.GetParentRelClassId());

    DgnElementCPtr outputElement = outputEditElement->Insert(&status);
    if (!outputElement.IsValid())
        return nullptr;

    m_context.AddElementId(sourceElement.GetElementId(), outputElement->GetElementId());

    if (m_copyChildren)
        {
        for (auto sourceChildid : sourceElement.QueryChildren())
            {
            DgnElementCPtr sourceChildElement = sourceElement.GetDgnDb().Elements().GetElement(sourceChildid);
            if (!sourceChildElement.IsValid())
                continue;

            MakeCopy(nullptr, targetModel, *sourceChildElement, DgnCode(), outputElement->GetElementId());
            }
        }

    IElementGroupCP sourceGroup;
    if (m_copyGroups && nullptr != (sourceGroup = sourceElement.ToIElementGroup()))
        {
        for (auto sourceMemberId : sourceGroup->QueryMembers())
            {
            DgnElementCPtr sourceMemberElement = sourceElement.GetDgnDb().Elements().GetElement(sourceMemberId);
            if (!sourceMemberElement.IsValid())
                continue;
            DgnElementCPtr destMemberElement = MakeCopy(nullptr, *sourceMemberElement->GetModel(), *sourceMemberElement, DgnCode());
            if (destMemberElement.IsValid())
                ElementGroupsMembers::Insert(*outputElement, *destMemberElement, 0); // *** WIP_GROUPS - is this the right way to re-create the member-of relationship? What about the _OnMemberAdded callbacks? Preserve MemberPriority?
            }
        }

    return outputElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElementTransformer::ApplyTransformTo(DgnElementR el, Transform const& transformIn)
    {
    if (transformIn.IsIdentity())
        return DgnDbStatus::Success;

    Transform   placementTrans;

    auto geom = el.ToGeometrySourceP();
    if (nullptr == geom)
        return DgnDbStatus::BadElement;

    if (geom->Is3d())
        placementTrans = geom->GetAsGeometrySource3d()->GetPlacement().GetTransform();
    else
        placementTrans = geom->GetAsGeometrySource2d()->GetPlacement().GetTransform();

    DPoint3d    originPt;
    RotMatrix   rMatrix;

    Transform transform;
    transform.InitProduct(transformIn, placementTrans);
    transform.GetTranslation(originPt);
    transform.GetMatrix(rMatrix);

    YawPitchRollAngles  angles;

    if (!YawPitchRollAngles::TryFromRotMatrix(angles, rMatrix))
        return DgnDbStatus::BadArg;

    if (geom->Is3d())
        {
        Placement3d placement = geom->GetAsGeometrySource3d()->GetPlacement();

        placement.GetOriginR() = originPt;
        placement.GetAnglesR() = angles;

        return geom->GetAsGeometrySource3dP()->SetPlacement(placement);
        }

    Placement2d placement = geom->GetAsGeometrySource2d()->GetPlacement();

    placement.GetOriginR() = DPoint2d::From(originPt);
    placement.GetAngleR() = angles.GetYaw();

    return geom->GetAsGeometrySource2dP()->SetPlacement(placement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ReadSelectParams(stmt, params);
    if (DgnDbStatus::Success != status)
        return status;

    m_categoryId = stmt.GetValueNavigation<DgnCategoryId>(params.GetSelectIndex(prop_Category()));

    // Read GeomStream
    auto geomIndex = params.GetSelectIndex(prop_GeometryStream());
    if (stmt.IsValueNull(geomIndex))
        return DgnDbStatus::Success;    // no geometry...

    int blobSize;
    void const* blob = stmt.GetValueBlob(geomIndex, &blobSize);
    return m_geom.ReadGeometryStream(GetDgnDb().Elements().GetSnappyFrom(), GetDgnDb(), blob, blobSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement::_ToJson(BeJsValue val, BeJsConst opts) const
    {
    T_Super::_ToJson(val, opts);
    val[json_category()] = m_categoryId;

    if (!opts["wantGeometry"].asBool())
        return;

    // load geometry
    GeometryCollection collection(*ToGeometrySource());
    collection.ToJson(val[json_geom()], opts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement::_FromJson(BeJsConst props)
    {
    T_Super::_FromJson(props);
    auto catJson = props[json_category()];
    if (!catJson.isNull())
        m_categoryId.FromJson(catJson);

    auto elementGeometryBuilderParams = props[json_elementGeometryBuilderParams()];
    if (!elementGeometryBuilderParams.isNull()) {
        auto napiValue = elementGeometryBuilderParams.AsNapiValueRef();
        if (nullptr == napiValue) {
            throw std::invalid_argument("only supported from JavaScript");
        }
        auto napiObj = napiValue->m_napiVal.As<Napi::Object>();

        if (napiObj.Has("is2dPart")) { // make sure the caller is not confused about what kind of element this is for
            BeNapi::ThrowJsException(m_dgndb.GetJsIModelDb()->Env(), "BuildGeometryStream failed - invalid builder parameter", (int)DgnDbStatus::BadArg);
            return;
        }

        auto viewIndependentVal = napiObj.Get("viewIndependent");
        auto entryArrayObj = napiObj.Get("entryArray");
        BeAssert(viewIndependentVal.IsUndefined() || viewIndependentVal.IsBoolean());
        BeAssert(entryArrayObj.IsArray());

        GeometryBuilderParams bparams;
        bparams.viewIndependent = viewIndependentVal.IsBoolean() && viewIndependentVal.As<Napi::Boolean>().Value();

        auto status = GeometryStreamIO::BuildGeometryStream(*this, bparams, entryArrayObj.As<Napi::Array>());
        if (DgnDbStatus::Success != status) {
            // throw std::runtime_error("BuildGeometryStream failed");
            BeNapi::ThrowJsException(m_dgndb.GetJsIModelDb()->Env(), "BuildGeometryStream failed", (int)status);
        }
        return;
    }

    auto geomJson = props[json_geom()];
    if (!geomJson.isNull()) {
        GeometryBuilder::UpdateFromJson(*ToGeometrySourceP(), geomJson);
        return;
    }

    auto binJson = props[json_geomBinary()];
    if (!binJson.isNull())
        binJson.GetBinary(m_geom);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement::_BindWriteParams(ECSqlStatement& stmt, ForInsert forInsert)
    {
    T_Super::_BindWriteParams(stmt, forInsert);

    stmt.BindNavigationValue(stmt.GetParameterIndex(prop_Category()), m_categoryId);
    m_geom.BindGeometryStream(m_multiChunkGeomStream, GetDgnDb().Elements().GetSnappyTo(), stmt, prop_GeometryStream());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometryStream::BindGeometryStream(bool& multiChunkGeometryStream, SnappyToBlob& snappyTo, ECSqlStatement& stmt, Utf8CP parameterName) const
    {
    // Compress the serialized GeomStream
    multiChunkGeometryStream = false;
    snappyTo.Init();

    if (0 < GetSize())
        {
        GeomBlobHeader header(*this);
        snappyTo.Write((Byte const*)&header, sizeof(header));
        snappyTo.Write(GetData(), GetSize());
        }

    auto geomIndex = stmt.GetParameterIndex(parameterName);
    uint32_t zipSize = snappyTo.GetCompressedSize();
    if (0 < zipSize)
        {
        if (1 == snappyTo.GetCurrChunk())
            {
            // Common case - only one chunk in geom stream. Bind it directly.
            // NB: This requires that no other code uses DgnElements::SnappyToBlob() until our ECSqlStatement is executed...
            stmt.BindBlob(geomIndex, snappyTo.GetChunkData(0), zipSize, IECSqlBinder::MakeCopy::No);
            }
        else
            {
            // More than one chunk in geom stream. Avoid expensive alloc+copy by deferring writing geom stream until ECSqlStatement executes.
            multiChunkGeometryStream = true;
            stmt.BindZeroBlob(geomIndex, snappyTo.GetCompressedSize());
            }
        }
    else
        {
        // No geometry
        stmt.BindNull(geomIndex);
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometricElement::_EqualProperty(ECN::ECPropertyValueCR expected, DgnElementCR other) const
    {
    if (!expected.GetValueAccessor().GetECProperty()->GetName().Equals(prop_GeometryStream()))
        {
        return T_Super::_EqualProperty(expected, other);
        }

    // We need custom logic to compare GeometryStreams because the base class implementation of _EqualProperty cannot get this property's value as an ECN::ECValue.
    // We DON'T need custom logic to compare any of the other common GeometricElement properties, such as CategoryId,
    // because the base class implementation CAN get those property values as ECN::ECValues and can therefore compare them using generic logic.

    GeometricElement const* othergeom = static_cast<GeometricElement const*>(&other);

    uint32_t size = m_geom.GetSize();
    if (othergeom->m_geom.GetSize() != size)
        return false;

    auto data = m_geom.GetData();
    auto otherdata = othergeom->m_geom.GetData();
    return 0 == memcmp(data, otherdata, size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement::_InsertInDb()
    {
    auto stat = T_Super::_InsertInDb();
    return DgnDbStatus::Success == stat ? InsertGeomStream() : stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement::_UpdateInDb()
    {
    auto stat = T_Super::_UpdateInDb();
    return DgnDbStatus::Success == stat ? UpdateGeomStream() : stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement::_OnInsert()
    {
    auto stat = Validate();
    return DgnDbStatus::Success == stat ? T_Super::_OnInsert() : stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement::_OnUpdate(DgnElementCR el)
    {
    auto stat = Validate();
    return DgnDbStatus::Success == stat ? T_Super::_OnUpdate(el) : stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement2d::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ReadSelectParams(stmt, params);
    if (DgnDbStatus::Success != status)
        return status;

    m_typeDefinition.m_id = stmt.GetValueNavigation<DgnElementId>(params.GetSelectIndex(prop_TypeDefinition()), &m_typeDefinition.m_relClassId);
    m_placement = Placement2d();

    auto originIndex = params.GetSelectIndex(prop_Origin());
    if (stmt.IsValueNull(originIndex))
        return DgnDbStatus::Success;    // null placement

    DPoint2d boxLow = stmt.GetValuePoint2d(params.GetSelectIndex(prop_BBoxLow())),
             boxHi  = stmt.GetValuePoint2d(params.GetSelectIndex(prop_BBoxHigh()));

    m_placement = Placement2d(stmt.GetValuePoint2d(originIndex),
                              AngleInDegrees::FromDegrees(stmt.GetValueDouble(params.GetSelectIndex(prop_Rotation()))),
                              ElementAlignedBox2d(boxLow.x, boxLow.y, boxHi.x, boxHi.y));

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement2d::_ToJson(BeJsValue val, BeJsConst opts) const
    {
    T_Super::_ToJson(val, opts);
    m_placement.ToJson(val[json_placement()]);

    if (m_typeDefinition.IsValid())
        m_typeDefinition.ToJson(GetDgnDb(), val[json_typeDefinition()]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement2d::_FromJson(BeJsConst props)
    {
    T_Super::_FromJson(props);

    auto placementJson = props[json_placement()];
    if (!placementJson.isNull())
        {
        if (props.isMember(json_geomBinary()) || (props[json_geom()].isNull() && props[json_elementGeometryBuilderParams()].isNull()))
            {
            // NOTE: Use the existing bounding box when the GeometryStream is cloned as binary or no geometry exists to calculate from
            m_placement.FromJson(placementJson);
            }
        else
            {
            // NOTE: Bounding box will be updated from the supplied geometry
            Placement2d newPlacement;
            newPlacement.FromJson(placementJson);
            m_placement.GetOriginR() = newPlacement.GetOrigin();
            m_placement.GetAngleR()  = newPlacement.GetAngle();
            }
        }

    if (props.hasMember(json_typeDefinition())) // support partial update, only update m_typeDefinition if props has member
        {
        auto typeJson = props[json_typeDefinition()];
        m_typeDefinition.FromJson(GetDgnDb(), typeJson); // RelatedElement::FromJson also clears for undefined
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement3d::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ReadSelectParams(stmt, params);
    if (DgnDbStatus::Success != status)
        return status;

    m_typeDefinition.m_id = stmt.GetValueNavigation<DgnElementId>(params.GetSelectIndex(prop_TypeDefinition()), &m_typeDefinition.m_relClassId);
    m_placement = Placement3d();

    auto originIndex = params.GetSelectIndex(prop_Origin());
    if (stmt.IsValueNull(originIndex))
        return DgnDbStatus::Success;    // null placement

    DPoint3d boxLow = stmt.GetValuePoint3d(params.GetSelectIndex(prop_BBoxLow())),
             boxHi  = stmt.GetValuePoint3d(params.GetSelectIndex(prop_BBoxHigh()));

    double yaw      = stmt.GetValueDouble(params.GetSelectIndex(prop_Yaw())),
           pitch    = stmt.GetValueDouble(params.GetSelectIndex(prop_Pitch())),
           roll     = stmt.GetValueDouble(params.GetSelectIndex(prop_Roll()));

    m_placement = Placement3d(stmt.GetValuePoint3d(originIndex),
                              YawPitchRollAngles(Angle::FromDegrees(yaw), Angle::FromDegrees(pitch), Angle::FromDegrees(roll)),
                              ElementAlignedBox3d(boxLow.x, boxLow.y, boxLow.z, boxHi.x, boxHi.y, boxHi.z));
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement3d::_ToJson(BeJsValue val, BeJsConst opts) const
    {
    T_Super::_ToJson(val, opts);
    m_placement.ToJson(val[json_placement()]);

    if (m_typeDefinition.IsValid())
         m_typeDefinition.ToJson(GetDgnDb(), val[json_typeDefinition()]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement3d::_FromJson(BeJsConst props)
    {
    T_Super::_FromJson(props);

    auto placementJson = props[json_placement()];
    if (!placementJson.isNull())
        {
        if (props.isMember(json_geomBinary()) || (props[json_geom()].isNull() && props[json_elementGeometryBuilderParams()].isNull()))
            {
            // NOTE: Use the existing bounding box when the GeometryStream is cloned as binary or no geometry exists to calculate from
            m_placement.FromJson(placementJson);
            }
        else
            {
            // NOTE: Bounding box will be updated from the supplied geometry
            Placement3d newPlacement;
            newPlacement.FromJson(placementJson);
            m_placement.GetOriginR() = newPlacement.GetOrigin();
            m_placement.GetAnglesR() = newPlacement.GetAngles();
            }
        }

    if (props.hasMember(json_typeDefinition())) // support partial update, only update m_typeDefinition if props has member
        {
        auto typeJson = props[json_typeDefinition()];
        m_typeDefinition.FromJson(GetDgnDb(), typeJson); // RelatedElement::FromJson also clears for undefined
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement2d::_BindWriteParams(ECSqlStatement& stmt, ForInsert forInsert)
    {
    T_Super::_BindWriteParams(stmt, forInsert);
    stmt.BindNavigationValue(stmt.GetParameterIndex(prop_TypeDefinition()), m_typeDefinition.m_id, m_typeDefinition.m_relClassId);

    if (!m_placement.IsValid())
        {
        stmt.BindNull(stmt.GetParameterIndex(prop_Origin()));
        stmt.BindNull(stmt.GetParameterIndex(prop_BBoxLow()));
        stmt.BindNull(stmt.GetParameterIndex(prop_BBoxHigh()));
        stmt.BindNull(stmt.GetParameterIndex(prop_Rotation()));
        }
    else
        {
        stmt.BindPoint2d(stmt.GetParameterIndex(prop_Origin()), m_placement.GetOrigin());
        stmt.BindDouble(stmt.GetParameterIndex(prop_Rotation()), m_placement.GetAngle().Degrees());
        stmt.BindPoint2d(stmt.GetParameterIndex(prop_BBoxLow()), m_placement.GetElementBox().low);
        stmt.BindPoint2d(stmt.GetParameterIndex(prop_BBoxHigh()), m_placement.GetElementBox().high);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement2d::_OnInsert()
    {
    if (!GetModel()->Is2dModel())
        return DgnDbStatus::WrongModel; // A GeometricElement2d can only reside in a 2D model

    if (!DrawingCategory::Get(GetDgnDb(), GetCategoryId()).IsValid())
        return DgnDbStatus::InvalidCategory; // A GeometricElement2d requires an existing DrawingCategory

    return T_Super::_OnInsert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement3d::_BindWriteParams(ECSqlStatement& stmt, ForInsert forInsert)
    {
    T_Super::_BindWriteParams(stmt, forInsert);

    GeometricModel3dCP model3d = GetModel()->ToGeometricModel3d();
    if (nullptr == model3d)
        {
        BeAssert(false && "GeometricElement3d elements must reside in a GeometricModel3d."); // should have been avoided by checks in GeometricElement3d::_OnInsert
        return;
        }

    stmt.BindInt(stmt.GetParameterIndex(prop_InSpatialIndex()), model3d->IsSpatiallyLocated() ? 1 : 0);
    stmt.BindNavigationValue(stmt.GetParameterIndex(prop_TypeDefinition()), m_typeDefinition.m_id, m_typeDefinition.m_relClassId);

    if (!m_placement.IsValid())
        {
        stmt.BindNull(stmt.GetParameterIndex(prop_Origin()));
        stmt.BindNull(stmt.GetParameterIndex(prop_Yaw()));
        stmt.BindNull(stmt.GetParameterIndex(prop_Pitch()));
        stmt.BindNull(stmt.GetParameterIndex(prop_Roll()));
        stmt.BindNull(stmt.GetParameterIndex(prop_BBoxLow()));
        stmt.BindNull(stmt.GetParameterIndex(prop_BBoxHigh()));
        }
    else
        {
        stmt.BindPoint3d(stmt.GetParameterIndex(prop_Origin()), m_placement.GetOrigin());
        stmt.BindDouble(stmt.GetParameterIndex(prop_Yaw()), m_placement.GetAngles().GetYaw().Degrees());
        stmt.BindDouble(stmt.GetParameterIndex(prop_Pitch()), m_placement.GetAngles().GetPitch().Degrees());
        stmt.BindDouble(stmt.GetParameterIndex(prop_Roll()), m_placement.GetAngles().GetRoll().Degrees());
        stmt.BindPoint3d(stmt.GetParameterIndex(prop_BBoxLow()), m_placement.GetElementBox().low);
        stmt.BindPoint3d(stmt.GetParameterIndex(prop_BBoxHigh()), m_placement.GetElementBox().high);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement3d::_OnInsert()
    {
    if (!GetModel()->Is3dModel())
        return DgnDbStatus::WrongModel; // A GeometricElement3d can only reside in a 3D model

    if (!SpatialCategory::Get(GetDgnDb(), GetCategoryId()).IsValid())
        return DgnDbStatus::InvalidCategory; // A GeometricElement3d requires an existing SpatialCategory

    return T_Super::_OnInsert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement3d::_OnUpdate(DgnElementCR el) {return T_Super::_OnUpdate(el);}
DgnDbStatus GeometricElement3d::_OnDelete() const {return T_Super::_OnDelete();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement::WriteGeomStream() const
    {
    if (!m_multiChunkGeomStream)
        return DgnDbStatus::Success;

    m_multiChunkGeomStream = false;
    DgnDbR db = GetDgnDb();
    return GeometryStream::WriteGeometryStream(db.Elements().GetSnappyTo(), db, GetElementId(), _GetGeometryColumnClassName(), prop_GeometryStream());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometryStream::WriteGeometryStream(SnappyToBlob& snappyTo, DgnDbR db, DgnElementId elementId, Utf8CP className, Utf8CP propertyName)
    {
    if (1 >= snappyTo.GetCurrChunk())
        {
        BeAssert(false);    // Somebody overwrote our data.
        return DgnDbStatus::WriteError;
        }

    ECClassCP ecClass = db.Schemas().GetClass(BIS_ECSCHEMA_NAME, className);
    BeAssert(nullptr != ecClass);
    if (nullptr == ecClass)
        return DgnDbStatus::BadArg;

    BlobIO blobIO;
    if (SUCCESS != db.OpenBlobIO(blobIO, *ecClass, propertyName, elementId, true, db.GetECCrudWriteToken()))
        return DgnDbStatus::WriteError;

    if (BE_SQLITE_OK != snappyTo.SaveToRow(blobIO))
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement::InsertGeomStream() const
    {
    return WriteGeomStream();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement::UpdateGeomStream() const
    {
    return WriteGeomStream();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::GenericUniqueAspect::SetKeyClass(ECN::ECClassCP keyClass)
    {
    if (keyClass != nullptr)
        {
        m_key_ecclassName = keyClass->GetName();
        m_key_ecschemaName = keyClass->GetSchema().GetName();
        }
    else
        {
        m_key_ecclassName = m_ecclassName;
        m_key_ecschemaName = m_ecschemaName;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::GenericUniqueAspect::QueryAndSetActualClass(DgnElementCR el)
    {
    ECClassId actualClassId;
    if (DgnDbStatus::Success != QueryActualClass(actualClassId, el, m_ecschemaName.c_str(), m_ecclassName.c_str()))
        return DgnDbStatus::NotFound;
    auto actualClass = el.GetDgnDb().Schemas().GetClass(actualClassId);
    if (nullptr == actualClass)
        return DgnDbStatus::NotFound;
    m_key_ecschemaName = m_ecschemaName;
    m_key_ecclassName = m_ecclassName;
    m_ecschemaName = actualClass->GetSchema().GetName();
    m_ecclassName = actualClass->GetName();
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<DgnElement::UniqueAspect> DgnElement::GenericUniqueAspect::SetAspect(DgnElementR el, ECN::IECInstanceR instance, ECClassCP keyClass, DgnDbStatus* outStatus)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(status, outStatus);
    RefCountedPtr<GenericUniqueAspect> newAspect = new GenericUniqueAspect(instance);
    newAspect->SetKeyClass(keyClass);
    auto& db = el.GetDgnDb();
    GenericUniqueAspect* currentAspect = dynamic_cast<GenericUniqueAspect*>(T_Super::GetAspectP(el, *newAspect->GetKeyECClass(db)));
    if ((nullptr != currentAspect) && (currentAspect->GetECClass(db) == &instance.GetClass()))
        newAspect->m_instanceId = currentAspect->m_instanceId;
    T_Super::SetAspect(el, *newAspect);
    status = DgnDbStatus::Success;
    return newAspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::UniqueAspect::_BindTo(DgnElementCR el)
    {
    auto existing = _QueryExistingInstanceKey(el);
    if (existing.GetClassId() == GetECClassId(el.GetDgnDb()))
        m_instanceId = existing.GetInstanceId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::UniqueAspect::QueryActualClass(ECClassId& classId, Dgn::DgnElementCR el, Utf8CP schemaName, Utf8CP className)
    {
    //auto stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("SELECT ECClassId FROM [%s].[%s] WHERE Element.Id=?", schemaName, className).c_str());
    CachedStatementPtr stmt = el.GetDgnDb().GetCachedStatement(Utf8PrintfString("SELECT ECClassId from bis_ElementUniqueAspect WHERE ECClassId in ( SELECT ClassId from ec_cache_ClassHierarchy WHERE BaseClassId = ?) AND ElementId =? ").c_str());

    if (!stmt.IsValid())
        return DgnDbStatus::BadSchema;
    stmt->BindId(1, el.GetDgnDb().Schemas().GetClassId(schemaName, className));
    stmt->BindId(2, el.GetElementId());
    if (BE_SQLITE_ROW != stmt->Step())
        return DgnDbStatus::NotFound;
    classId = stmt->GetValueId<ECClassId>(0);
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::GenericUniqueAspect::_LoadProperties(Dgn::DgnElementCR el)
    {
    DgnDbStatus status = QueryAndSetActualClass(el);
    if (DgnDbStatus::Success != status)
        return status;

    auto stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("SELECT * FROM [%s].[%s] WHERE Element.Id=?", m_ecschemaName.c_str(), m_ecclassName.c_str()).c_str());
    if (!stmt.IsValid())
        return DgnDbStatus::BadSchema;
    stmt->BindId(1, el.GetElementId());
    if (BE_SQLITE_ROW != stmt->Step())
        return DgnDbStatus::NotFound;
    ECInstanceECSqlSelectAdapter adapter(*stmt);
    m_instance = adapter.GetInstance();
    adapter.GetInstanceId(m_instanceId);
    return m_instance.IsValid()? DgnDbStatus::Success: DgnDbStatus::ReadError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::GenericUniqueAspect::_UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const*)
    {
    if (!m_instance.IsValid() || !m_instanceId.IsValid())
        {
        BeAssert(false);
        return DgnDbStatus::BadArg;
        }

    auto updater = el.GetDgnDb().Elements().m_updaterCache.GetUpdater(el.GetDgnDb(), m_instance->GetClass());
    if (nullptr == updater)
        return DgnDbStatus::BadSchema;

    // Set the aspect's own ECInstanceId. This is what enables the updater to find and update an existing aspect.
    Utf8Char ecinstidstr[32];
    BeStringUtilities::FormatUInt64(ecinstidstr, m_instanceId.GetValue());
    m_instance->SetInstanceId(ecinstidstr);

    // Set the UniqueAspect's "Element" navigation property. This is what links the aspect to its host element. The IDs are not the same.
    ECValue v;
    m_instance->GetValue(v, "Element");
    if (v.IsNull() || !v.IsNavigation() || !v.GetNavigationInfo().GetRelationshipClassId().IsValid())
        m_instance->SetValue("Element", ECN::ECValue(el.GetElementId(), el.GetDgnDb().Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsUniqueAspect)));

    return (BE_SQLITE_OK == updater->Update(*m_instance))? DgnDbStatus::Success: DgnDbStatus::WriteError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::GenericUniqueAspect::_GetPropertyValue(ECN::ECValueR value, Utf8CP propertyName, PropertyArrayIndex const& arrayIndex) const
    {
    if (!m_instance.IsValid())
        return DgnDbStatus::BadRequest;

    if (arrayIndex.HasIndex())
        return ECObjectsStatus::Success == m_instance->GetValue(value, propertyName, arrayIndex.GetIndex()) ? DgnDbStatus::Success : DgnDbStatus::BadArg;

    return ECObjectsStatus::Success == m_instance->GetValue(value, propertyName) ? DgnDbStatus::Success : DgnDbStatus::BadArg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::GenericUniqueAspect::_SetPropertyValue(Utf8CP propertyName, ECN::ECValueCR value, PropertyArrayIndex const& arrayIndex)
    {
    if (!m_instance.IsValid())
        return DgnDbStatus::BadRequest;

    if (arrayIndex.HasIndex())
        return ECObjectsStatus::Success == m_instance->SetValue(propertyName, value, arrayIndex.GetIndex()) ? DgnDbStatus::Success : DgnDbStatus::BadArg;

    return ECObjectsStatus::Success == m_instance->SetValue(propertyName, value) ? DgnDbStatus::Success : DgnDbStatus::BadArg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::IECInstanceP DgnElement::GenericUniqueAspect::GetAspectP(DgnElementR el, ECN::ECClassCR cls)
    {
    GenericUniqueAspect* aspect = dynamic_cast<GenericUniqueAspect*>(T_Super::GetAspectP(el,cls));
    if (nullptr == aspect)
        return nullptr;
    if (hasHandler(cls))   // Don't allow caller to modify an aspect that has a handler (which is missing) by using a generic aspect
        return nullptr;
    return aspect->m_instance.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::IECInstanceCP DgnElement::GenericUniqueAspect::GetAspect(DgnElementCR el, ECN::ECClassCR cls)
    {
    GenericUniqueAspect const* aspect = dynamic_cast<GenericUniqueAspect const*>(T_Super::GetAspect(el,cls));
    if (nullptr == aspect)
        return nullptr;
    return aspect->m_instance.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::GenericMultiAspect::_LoadProperties(Dgn::DgnElementCR el)
    {
    CachedECSqlStatementPtr stmt;
    if (m_instanceId.IsValid())
        {
        Utf8PrintfString sql("SELECT * FROM [%s].[%s] WHERE ECInstanceId=?", m_ecschemaName.c_str(), m_ecclassName.c_str());
        stmt = el.GetDgnDb().GetPreparedECSqlStatement(sql.c_str());
        if (!stmt.IsValid())
            {
            LOG.debugv("DgnElement::GenericMultiAspect::_LoadProperties: Failed to get CachedECSqlStatementPtr for %s", sql.c_str());
            return DgnDbStatus::BadSchema;
            }
        stmt->BindId(1, m_instanceId);
        }
    else
        {
        Utf8PrintfString sql("SELECT * FROM [%s].[%s] WHERE Element.Id=?", m_ecschemaName.c_str(), m_ecclassName.c_str());
        // *** WIP_GenericMultiAspect - if no instance is specified, load the first one...?
        stmt = el.GetDgnDb().GetPreparedECSqlStatement(sql.c_str());
        if (!stmt.IsValid())
            {
            LOG.debugv("DgnElement::GenericMultiAspect::_LoadProperties: Failed to get CachedECSqlStatementPtr for %s", sql.c_str());
            return DgnDbStatus::BadSchema;
            }
        stmt->BindId(1, el.GetElementId());
        }
    if (BE_SQLITE_ROW != stmt->Step())
        return DgnDbStatus::NotFound;
    ECInstanceECSqlSelectAdapter adapter(*stmt);
    m_instance = adapter.GetInstance();
    adapter.GetInstanceId(m_instanceId);
    return m_instance.IsValid()? DgnDbStatus::Success: DgnDbStatus::ReadError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::GenericMultiAspect::_UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECCrudWriteToken const*)
    {
    if (!m_instance.IsValid() || !m_instanceId.IsValid())
        {
        BeAssert(false);
        return DgnDbStatus::BadArg;
        }

    auto updater = el.GetDgnDb().Elements().m_updaterCache.GetUpdater(el.GetDgnDb(), m_instance->GetClass());
    if (nullptr == updater)
        return DgnDbStatus::BadSchema;

    if (m_instanceId.IsValid())
        {
        // Set the aspect's own ECInstanceId. This is what enables the updater to find and update an existing aspect.
        Utf8Char ecinstidstr[32];
        BeStringUtilities::FormatUInt64(ecinstidstr, m_instanceId.GetValue());
        m_instance->SetInstanceId(ecinstidstr);
        }

    // Set the MultiAspect's "Element" navigation property. This is what links the aspect to its host element.
    ECN::ECValue v;
    m_instance->GetValue(v, "Element");
    if (v.IsNull() || !v.IsNavigation() || !v.GetNavigationInfo().GetRelationshipClassId().IsValid())
        m_instance->SetValue("Element", ECN::ECValue(el.GetElementId(), ECN::ECClassId(el.GetDgnDb().Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsMultiAspects).GetValue())));

    return (BE_SQLITE_OK == updater->Update(*m_instance))? DgnDbStatus::Success: DgnDbStatus::WriteError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::GenericMultiAspect::_GetPropertyValue(ECN::ECValueR value, Utf8CP propertyName, PropertyArrayIndex const& arrayIndex) const
    {
    if (!m_instance.IsValid())
        return DgnDbStatus::BadRequest;

    if (arrayIndex.HasIndex())
        return ECObjectsStatus::Success == m_instance->GetValue(value, propertyName, arrayIndex.GetIndex()) ? DgnDbStatus::Success : DgnDbStatus::BadArg;

    return ECObjectsStatus::Success == m_instance->GetValue(value, propertyName) ? DgnDbStatus::Success : DgnDbStatus::BadArg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::GenericMultiAspect::_SetPropertyValue(Utf8CP propertyName, ECN::ECValueCR value, PropertyArrayIndex const& arrayIndex)
    {
    if (!m_instance.IsValid())
        return DgnDbStatus::BadRequest;

    if (arrayIndex.HasIndex())
        return ECObjectsStatus::Success == m_instance->SetValue(propertyName, value, arrayIndex.GetIndex()) ? DgnDbStatus::Success : DgnDbStatus::BadArg;

    return ECObjectsStatus::Success == m_instance->SetValue(propertyName, value) ? DgnDbStatus::Success : DgnDbStatus::BadArg;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::IECInstanceP DgnElement::GenericMultiAspect::GetAspectP(DgnElementR el, ECN::ECClassCR cls, BeSQLite::EC::ECInstanceId id)
    {
    GenericMultiAspect* aspect = dynamic_cast<GenericMultiAspect*>(T_Super::GetAspectP(el,cls,id));
    if (nullptr == aspect)
        return nullptr;
    if (hasHandler(cls))   // Don't allow caller to modify an aspect that has a handler (which is missing) by using a generic aspect
        return nullptr;
    return aspect->m_instance.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::IECInstanceCP DgnElement::GenericMultiAspect::GetAspect(DgnElementCR el, ECN::ECClassCR cls, BeSQLite::EC::ECInstanceId id)
    {
    GenericMultiAspect const* aspect = dynamic_cast<GenericMultiAspect const*>(T_Super::GetAspect(el,cls,id));
    if (nullptr == aspect)
        return nullptr;
    return aspect->m_instance.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::GenericMultiAspect::GenericMultiAspect(ECN::ECClassCR cls, BeSQLite::EC::ECInstanceId id)
    : m_ecclassName(cls.GetName()), m_ecschemaName(cls.GetSchema().GetName())
    {
    m_instanceId = id;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::GenericMultiAspect::GenericMultiAspect(ECN::IECInstanceR inst, BeSQLite::EC::ECInstanceId id)
    : m_instance(&inst), m_ecclassName(inst.GetClass().GetName()), m_ecschemaName(inst.GetClass().GetSchema().GetName())
    {
    m_instanceId = id;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<DgnElement::MultiAspect> DgnElement::GenericMultiAspect::AddAspect(DgnElementR el, ECN::IECInstanceR properties, DgnDbStatus* outStatus)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(status, outStatus);
    RefCountedPtr<DgnElement::MultiAspect> aspect = new GenericMultiAspect(properties, BeSQLite::EC::ECInstanceId());
    T_Super::AddAspect(el, *aspect);
    status = DgnDbStatus::Success;
    return aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::GenericMultiAspect::SetAspect(DgnElementR el, ECN::IECInstanceR properties, BeSQLite::EC::ECInstanceId id)
    {
    if (hasHandler(properties.GetClass()))
        return DgnDbStatus::MissingHandler;

    auto existing = T_Super::GetAspectP(el, properties.GetClass(), id);
    if (nullptr == existing)
        return DgnDbStatus::NotFound;

    auto mexisting = dynamic_cast<GenericMultiAspect*>(existing);
    if (nullptr == mexisting)
        {
        BeAssert(false);
        return DgnDbStatus::MissingHandler;
        }

    mexisting->m_instance = &properties;
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeMutex& DgnElement::GetElementsMutex() const { return GetDgnDb().Elements().GetMutex(); }
void DgnElement::ClearAllAppData() { BeMutexHolder lock(GetElementsMutex()); m_appData.clear(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_ElementHandler::Definition::_RegisterPropertyAccessors(ECSqlClassInfo& params, ECN::ClassLayoutCR layout)
    {
    T_Super::_RegisterPropertyAccessors(params, layout);

    params.RegisterPropertyAccessors(layout, DefinitionElement::prop_IsPrivate(),
        [](ECValueR value, DgnElementCR el)
            {
            DefinitionElementCR def = (DefinitionElementCR)el;
            value.SetBoolean(def.IsPrivate());
            return DgnDbStatus::Success;
            },
        [](DgnElementR el, ECValueCR value)
            {
            if (!value.IsBoolean())
                return DgnDbStatus::BadArg;
            DefinitionElementR def = (DefinitionElementR)el;
            def.SetIsPrivate(value.GetBoolean());
            return DgnDbStatus::Success;
            });
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus JobSubjectUtils::GetTransform(TransformR trans, SubjectCR jobSubject, Utf8CP propertyName)
    {
    if (!IsJobSubject(jobSubject))
        {
        BeAssert(false);
        return BSIERROR;
        }

    if (!HasProperty(jobSubject, propertyName))
        return BSIERROR;

    auto json = GetProperty(jobSubject, propertyName);

    // A Transform is stored as 3 rows (each of which is an array of 3 doubles). Check it. // Change from isDouble to isNumeric, don't think typescript can store doubles equal to 0.
    if (!json.isArray() || json.size() != 3  || !json[0].isArray() || json[0].size() != 4 || !json[0][0].isNumeric())
        {
        BeAssert(false);
        LOG.errorv("JobSubjectUtils::GetTransform: %s.%s is invalid [%s]\n", jobSubject.GetCode().GetValue().GetUtf8CP(), propertyName, json.Stringify().c_str());
        return BSIERROR;
        }

    BeJsGeomUtils::TransformFromJson(trans, json);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JobSubjectUtils::SetTransform(SubjectR jobSubject, TransformCR trans)
    {
    if (!IsJobSubject(jobSubject))
        {
        BeAssert(false);
        return;
        }

    BeJsGeomUtils::TransformToJson(GetPropertiesR(jobSubject)[json_Transform()], trans);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void JobSubjectUtils::InitializeProperties(SubjectR jobSubject, Utf8StringCR bridgeRegSubKey, Utf8CP comments, JsonValueCP properties)
    {
    BeAssert(!Utf8String::IsNullOrEmpty(bridgeRegSubKey.c_str()));

    BeJsDocument jobProps;
    jobProps[json_Bridge()] = bridgeRegSubKey;

    if (!Utf8String::IsNullOrEmpty(comments))
        jobProps[json_Comments()] = comments;

    if (nullptr != properties)
        jobProps[json_Properties()].From(*properties);

    jobSubject.SetSubjectJsonProperties(Subject::json_Job(), jobProps);

    BeAssert(IsJobSubject(jobSubject));
    }

/*
    avoid incomplete NavigationProperty values

    When iModel.js does an insert and supplies JSON properties, we allow the caller to specify NavigationProperty values that contain only the
    target element ID and omit the relClassName. In that case, we would have to do an ECSql query in order to get the relClassName from the
    ec tables. To make sure that happens and to avoid having an incomplete NavigationProperty value in the cache, we discard the entire set
    of auto-handled property values from memory, so that the next request will be fulfilled by an ECSql query.

    This problem can also crop up in an update, so we discard cached properties after updating, too.
*/
