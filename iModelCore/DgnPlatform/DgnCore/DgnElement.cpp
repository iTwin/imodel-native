/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnElement.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include "BisCoreNames.h"
#include "ElementECInstanceAdapter.h"

/* GeometrySource properties... 
 *  GeometrySource
 *      Geometry : binary
 *      CategoryId : long
 *  GeometrySource2d : GeometrySource
 *      Origin : point2d
 *      Rotation : double
 *      BBoxLow : point2d
 *      BBoxHigh : point2d
 *  GeometrySource3d : GeometrySource
 *      InSpatialIndex : boolean
 *      Origin : point3d
 *      Yaw : double
 *      Pitch : double
 *      Roll : double
 *      BBoxLow : point3d
 *      BBoxHigh : point3d
 */

namespace ElementStrings
{
    static Utf8CP str_Variables() {return "Variables";}
};

using namespace ElementStrings;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::AddRef() const
    {
    if (1 == m_refCount.IncrementAtomicPre(std::memory_order_relaxed) && IsPersistent())
        GetDgnDb().Elements().OnReclaimed(*this); // someone just requested this previously unreferenced element
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::Release() const
    {
    uint32_t countWas = m_refCount.DecrementAtomicPost(std::memory_order_relaxed);
    BeAssert(0 != countWas);
    if (1 == countWas)
        {
        std::atomic_thread_fence(std::memory_order_acquire);
        if (IsPersistent()) // is this element in the pool?
            GetDgnDb().Elements().OnUnreferenced(*this); // yes, the last reference was just released, add to the unreferenced element count
        else
            delete this; // no, just delete it
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelPtr DgnElement::GetModel() const
    {
    return m_dgndb.Models().GetModel(m_modelId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelPtr DgnElement::GetSubModel() const
    {
    // The DgnModelId value for the SubModel is the same as this element's DgnElementId value
    return m_dgndb.Models().GetModel(DgnModelId(GetElementId().GetValue()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelId DgnElement::GetSubModelId() const
    {
    DgnModelPtr model = GetSubModel();
    return model.IsValid() ? model->GetModelId() : DgnModelId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_OnSubModelInsert(DgnModelCR model) const
    {
    bool isModellable = GetElementClass()->Is(BIS_ECSCHEMA_NAME, BIS_CLASS_IModellableElement);
    BeAssert(isModellable && "Only element ECClasses that implement bis:IModellableElement can have SubModels");
    return isModellable ? DgnDbStatus::Success : DgnDbStatus::WrongElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/06
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::AppData* DgnElement::FindAppData(AppData::Key const& key) const
    {
    auto entry = m_appData.find(&key);
    return entry==m_appData.end() ? nullptr : entry->second.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/06
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::AddAppData(AppData::Key const& key, AppData* obj) const
    {
    auto entry = m_appData.Insert(&key, obj);
    if (entry.second)
        return;

    // we already had appdata for this key. Clean up old and save new.
    entry.first->second = obj;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   09/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DgnElement::DropAppData(AppData::Key const& key) const
    {
    return 0==m_appData.erase(&key) ? ERROR : SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
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
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP DgnElement::GetElementClass() const
    {
    return GetDgnDb().Schemas().GetECClass(GetElementClassId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode DgnElement::_GenerateDefaultCode() const
    {
    return DgnCode::CreateEmpty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
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
* @bsimethod                                    Keith.Bentley                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
template<class T> void DgnElement::CallAppData(T const& caller) const
    {
    for (auto entry=m_appData.begin(); entry!=m_appData.end(); )
        {
        if (DgnElement::AppData::DropMe::Yes == caller(*entry->second, *this))
            entry = m_appData.erase(entry);
        else
            ++entry;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus DgnElement::PopulateRequest(IBriefcaseManager::Request& request, BeSQLite::DbOpcode opcode) const
    {
    DgnElementCPtr original = BeSQLite::DbOpcode::Update == opcode ? GetDgnDb().Elements().GetElement(GetElementId()) : nullptr;
    return _PopulateRequest(request, opcode, original.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/16
+---------------+---------------+---------------+---------------+---------------+------*/
RepositoryStatus DgnElement::_PopulateRequest(IBriefcaseManager::Request& request, BeSQLite::DbOpcode opcode, DgnElementCP original) const
    {
    DgnCode code;
    switch (opcode)
        {
        case BeSQLite::DbOpcode::Insert:
            {
            code = m_code.IsValid() ? m_code : _GenerateDefaultCode();
            request.Locks().Insert(*GetModel(), LockLevel::Shared);
            break;
            }
        case BeSQLite::DbOpcode::Delete:
            {
            request.Locks().Insert(*this, LockLevel::Exclusive);
            break;
            }
        case BeSQLite::DbOpcode::Update:
            {
            BeAssert(nullptr != original && original->IsPersistent() && original->GetElementId() == GetElementId());
            if (m_code != original->GetCode())
                code = m_code;

            request.Locks().Insert(*this, LockLevel::Exclusive);
            if (GetModelId() != original->GetModelId())
                request.Locks().Insert(*original->GetModel(), LockLevel::Shared);

            break;
            }
        }

    if (code.IsValid() && !code.IsEmpty())
        {
        // Avoid asking repository manager to reserve code if we know it's already in use...
        if (GetDgnDb().Elements().QueryElementIdByCode(code).IsValid())
            return RepositoryStatus::CodeUsed;

        request.Codes().insert(code);
        }

    return RepositoryStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_OnInsert()
    {
    if (GetElementHandler()._IsRestrictedAction(RestrictedAction::Insert))
        return DgnDbStatus::MissingHandler;

    if (!m_code.IsValid())
        {
        m_code = _GenerateDefaultCode();
        if (!m_code.IsValid())
            return DgnDbStatus::InvalidName;
        }

    if (GetDgnDb().Elements().QueryElementIdByCode(m_code).IsValid())
        return DgnDbStatus::DuplicateCode;

    for (auto entry=m_appData.begin(); entry!=m_appData.end(); ++entry)
        {
        DgnDbStatus stat = entry->second->_OnInsert(*this);
        if (DgnDbStatus::Success != stat)
            return stat;
        }

    // Ensure model not exclusively locked, and code reserved
    DgnDbStatus stat = GetDgnDb().BriefcaseManager().OnElementInsert(*this);
    if (DgnDbStatus::Success != stat)
        return stat;

    return GetModel()->_OnInsertElement(*this);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DefinitionElement::_OnInsert()
    {
    // DefinitionElements can reside *only* in a DefinitionModel
    DgnDbStatus status = GetModel()->IsDefinitionModel() ? T_Super::_OnInsert() : DgnDbStatus::WrongModel;
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode Session::CreateCode(DgnDbR db, Utf8StringCR name)
    {
    return DatabaseScopeAuthority::CreateCode(BIS_AUTHORITY_Session, db, name);
    }
                                                                                                                                                    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
SessionPtr Session::Create(DgnDbR db, Utf8CP name)
    {
    DgnModelId modelId = db.GetSessionModel()->GetModelId();
    DgnClassId classId = db.Domains().GetClassId(dgn_ElementHandler::Session::GetHandler());
    return new Session(CreateParams(db, modelId, classId, CreateCode(db, name)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
SessionCPtr Session::GetByName(DgnDbR db, Utf8StringCR name)
    {
    auto& elements = db.Elements(); 
    return elements.Get<Session>(elements.QueryElementIdByCode(CreateCode(db, name)));
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Session::_LoadFromDb() 
    {
    auto stat = T_Super::_LoadFromDb();
    if (DgnDbStatus::Success != stat)
        return stat;

    Json::Reader::Parse(GetPropertyValueString(str_Variables()), m_variables);
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Session::_CopyFrom(DgnElementCR el) 
    {
    auto& other = static_cast<SessionCR>(el);
    other.SaveVariables();
    T_Super::_CopyFrom(el);

    m_variables = other.m_variables;
    m_dirty = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void Session::SaveVariables() const
    {
    if (!m_dirty)
        return;

    auto& ncThis = const_cast<SessionR>(*this);
    ncThis.SetPropertyValue(str_Variables(), Json::FastWriter::ToString(m_variables).c_str());
    m_dirty = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
ElementIterator Session::MakeIterator(DgnDbR db, Utf8CP whereClause, Utf8CP orderByClause)
    {
    return db.Elements().MakeIterator(BIS_SCHEMA(BIS_CLASS_Session), whereClause, orderByClause);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Subject::_OnInsert()
    {
    // Subjects can only reside in the RepositoryModel
    return DgnModel::RepositoryModelId() == GetModel()->GetModelId() ? T_Super::_OnInsert() : DgnDbStatus::WrongModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    08/16
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
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Subject::_OnSubModelInsert(DgnModelCR model) const 
    {
    BeAssert(false && "A Subject is not directly modeled. Insert a child InformationPartitionElement to be modeled instead.");
    return DgnDbStatus::ElementBlockedChange;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
SubjectPtr Subject::Create(SubjectCR parentSubject, Utf8CP label, Utf8CP description)
    {
    DgnDbR db = parentSubject.GetDgnDb();
    DgnModelId modelId = DgnModel::RepositoryModelId();
    DgnClassId classId = db.Domains().GetClassId(dgn_ElementHandler::Subject::GetHandler());
    DgnElementId parentId = parentSubject.GetElementId();

    if (!parentId.IsValid() || !classId.IsValid() || !label || !*label)
        return nullptr;

    SubjectPtr subject = new Subject(CreateParams(db, modelId, classId, DgnCode(), label, parentId));
    if (description && *description)
        subject->SetDescription(description);

    return subject;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
SubjectCPtr Subject::CreateAndInsert(SubjectCR parentSubject, Utf8CP label, Utf8CP description)
    {
    SubjectPtr subject = Create(parentSubject, label, description);
    return subject.IsValid() ? parentSubject.GetDgnDb().Elements().Insert<Subject>(*subject) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::CreateParams InformationPartitionElement::InitCreateParams(SubjectCR parentSubject, Utf8CP name, DgnDomain::Handler& handler)
    {
    DgnDbR db = parentSubject.GetDgnDb();
    DgnModelId modelId = DgnModel::RepositoryModelId();
    DgnClassId classId = db.Domains().GetClassId(handler);
    DgnElementId parentId = parentSubject.GetElementId();
    DgnCode code = CreateCode(parentSubject, name);

    if (!parentId.IsValid() || !classId.IsValid() || !name || !*name)
        modelId.Invalidate(); // mark CreateParams as invalid

    return CreateParams(db, modelId, classId, code, nullptr, parentId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode InformationPartitionElement::CreateCode(SubjectCR parentSubject, Utf8CP name)
    {
    return ElementScopeAuthority::CreateCode(BIS_AUTHORITY_InformationPartitionElement, parentSubject, name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    10/16
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
* @bsimethod                                                    Shaun.Sewall    10/16
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
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DefinitionPartition::_OnSubModelInsert(DgnModelCR model) const 
    {
    // A DefinitionPartition can only be modeled by an DefinitionModel
    return model.IsDefinitionModel() ? T_Super::_OnSubModelInsert(model) : DgnDbStatus::ElementBlockedChange;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DefinitionPartitionPtr DefinitionPartition::Create(SubjectCR parentSubject, Utf8CP name, Utf8CP description)
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
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DefinitionPartitionCPtr DefinitionPartition::CreateAndInsert(SubjectCR parentSubject, Utf8CP name, Utf8CP description)
    {
    DefinitionPartitionPtr partition = Create(parentSubject, name, description);
    return partition.IsValid() ? parentSubject.GetDgnDb().Elements().Insert<DefinitionPartition>(*partition) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DocumentPartition::_OnSubModelInsert(DgnModelCR model) const 
    {
    // A DocumentPartition can only be modeled by an InformationModel
    return model.IsInformationModel() ? T_Super::_OnSubModelInsert(model) : DgnDbStatus::ElementBlockedChange;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DocumentPartitionPtr DocumentPartition::Create(SubjectCR parentSubject, Utf8CP name, Utf8CP description)
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
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DocumentPartitionCPtr DocumentPartition::CreateAndInsert(SubjectCR parentSubject, Utf8CP name, Utf8CP description)
    {
    DocumentPartitionPtr partition = Create(parentSubject, name, description);
    return partition.IsValid() ? parentSubject.GetDgnDb().Elements().Insert<DocumentPartition>(*partition) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    10/16
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
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
GroupInformationPartitionPtr GroupInformationPartition::Create(SubjectCR parentSubject, Utf8CP label, Utf8CP description)
    {
    CreateParams createParams = InitCreateParams(parentSubject, label, dgn_ElementHandler::GroupInformationPartition::GetHandler());
    if (!createParams.IsValid())
        return nullptr;

    GroupInformationPartitionPtr partition = new GroupInformationPartition(createParams);
    if (description && *description)
        partition->SetDescription(description);

    return partition;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
GroupInformationPartitionCPtr GroupInformationPartition::CreateAndInsert(SubjectCR parentSubject, Utf8CP label, Utf8CP description)
    {
    GroupInformationPartitionPtr partition = Create(parentSubject, label, description);
    return partition.IsValid() ? parentSubject.GetDgnDb().Elements().Insert<GroupInformationPartition>(*partition) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus SpatialLocationPartition::_OnSubModelInsert(DgnModelCR model) const 
    {
    // Only SpatialLocationModels can model a SpatialLocationPartition
    return model.IsSpatialLocationModel() ? T_Super::_OnSubModelInsert(model) : DgnDbStatus::ElementBlockedChange;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialLocationPartitionPtr SpatialLocationPartition::Create(SubjectCR parentSubject, Utf8CP name, Utf8CP description)
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
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
SpatialLocationPartitionCPtr SpatialLocationPartition::CreateAndInsert(SubjectCR parentSubject, Utf8CP name, Utf8CP description)
    {
    SpatialLocationPartitionPtr partition = Create(parentSubject, name, description);
    return partition.IsValid() ? parentSubject.GetDgnDb().Elements().Insert<SpatialLocationPartition>(*partition) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus PhysicalPartition::_OnSubModelInsert(DgnModelCR model) const 
    {
    return model.IsSpatialModel() ? T_Super::_OnSubModelInsert(model) : DgnDbStatus::ElementBlockedChange;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalPartitionPtr PhysicalPartition::Create(SubjectCR parentSubject, Utf8CP name, Utf8CP description)
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
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalPartitionCPtr PhysicalPartition::CreateAndInsert(SubjectCR parentSubject, Utf8CP name, Utf8CP description)
    {
    PhysicalPartitionPtr partition = Create(parentSubject, name, description);
    return partition.IsValid() ? parentSubject.GetDgnDb().Elements().Insert<PhysicalPartition>(*partition) : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GroupInformationElement::_OnInsert()
    {
    return GetModel()->IsInformationModel() ? T_Super::_OnInsert() : DgnDbStatus::WrongModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus RoleElement::_OnInsert()
    {
    // RoleElements can only reside in a RoleModel
    return GetModel()->IsRoleModel() ? T_Super::_OnInsert() : DgnDbStatus::WrongModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode Drawing::CreateCode(DocumentListModelCR model, Utf8CP name)
    {
    return ModelScopeAuthority::CreateCode(BIS_AUTHORITY_Drawing, model, name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    11/16
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
* @bsimethod                                                    Shaun.Sewall    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DrawingPtr Drawing::Create(DocumentListModelCR model, Utf8CP name)
    {
    DgnDbR db = model.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(dgn_ElementHandler::Drawing::GetHandler());

    if (!model.GetModelId().IsValid() || !classId.IsValid() || !name || !*name)
        {
        BeAssert(false);
        return nullptr;
        }

    return new Drawing(CreateParams(db, model.GetModelId(), classId, CreateCode(model, name)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
SectionDrawingPtr SectionDrawing::Create(DocumentListModelCR model, Utf8CP name)
    {
    DgnDbR db = model.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(dgn_ElementHandler::SectionDrawing::GetHandler());

    if (!model.GetModelId().IsValid() || !classId.IsValid() || !name || !*name)
        {
        BeAssert(false);
        return nullptr;
        }

    return new SectionDrawing(CreateParams(db, model.GetModelId(), classId, CreateCode(model, name)));
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
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::_OnInserted(DgnElementP copiedFrom) const
    {
    if (copiedFrom)
        copiedFrom->CallAppData(OnInsertedCaller(*this));

    GetModel()->_OnInsertedElement(*this);
    GetDgnDb().BriefcaseManager().OnElementInserted(GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::_OnReversedDelete() const
    {
    GetModel()->_OnReversedDeleteElement(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_SetParentId(DgnElementId parentId)
    {
    // Check for direct cycle...will check indirect cycles on update.
    if (parentId.IsValid() && parentId == GetElementId())
        return DgnDbStatus::InvalidParent;

    if (GetElementHandler()._IsRestrictedAction(RestrictedAction::SetParent))
        return DgnDbStatus::MissingHandler;

    m_parentId = parentId;
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
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
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_OnUpdate(DgnElementCR original)
    {
    if (m_classId != original.m_classId)
        return DgnDbStatus::WrongClass;

    if (GetElementHandler()._IsRestrictedAction(RestrictedAction::Update))
        return DgnDbStatus::MissingHandler;

    auto parentId = GetParentId();
    if (parentId.IsValid() && parentId != original.GetParentId() && parentCycleExists(parentId, GetElementId(), GetDgnDb()))
        return DgnDbStatus::InvalidParent;

    auto existingElemWithCode = GetDgnDb().Elements().QueryElementIdByCode(m_code);
    if ((existingElemWithCode.IsValid() && existingElemWithCode != GetElementId()))
        return DgnDbStatus::DuplicateCode;

    for (auto entry=m_appData.begin(); entry!=m_appData.end(); ++entry)
        {
        DgnDbStatus stat = entry->second->_OnUpdate(*this, original);
        if (DgnDbStatus::Success != stat)
            return stat;
        }

    // Ensure lock acquired and code reserved
    DgnDbStatus stat = GetDgnDb().BriefcaseManager().OnElementUpdate(*this);
    if (DgnDbStatus::Success != stat)
        return stat;

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
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::_OnUpdated(DgnElementCR original) const
    {
    // We need to call the events on both sets of AppData. Start by calling the appdata on this (the replacement)
    // element. NOTE: This is where Aspects, etc. actually update the database.
    CallAppData(OnUpdatedCaller(*this, original, false));

    // All done. This gives appdata on the *original* element a notification that the update has happened
    original.CallAppData(OnUpdatedCaller(*this, original, true));

    // now tell the model that one of its elements has been changed.
    GetModel()->_OnUpdatedElement(*this, original);
    }

struct OnUpdateReversedCaller
    {
    DgnElementCR m_updated, m_original;
    OnUpdateReversedCaller(DgnElementCR updated, DgnElementCR original) : m_updated(updated), m_original(original){}
    DgnElement::AppData::DropMe operator()(DgnElement::AppData& app, DgnElementCR el) const {return app._OnReversedUpdate(m_updated, m_original);}
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::_OnReversedUpdate(DgnElementCR original) const
    {
    // we need to call the events on BOTH sets of appdata
    original.CallAppData(OnUpdateReversedCaller(*this, original));
    CallAppData(OnUpdateReversedCaller(*this, original));

    GetModel()->_OnReversedUpdateElement(*this, original);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_OnDelete() const
    {
    if (GetElementHandler()._IsRestrictedAction(RestrictedAction::Delete))
        return DgnDbStatus::MissingHandler;

    for (auto entry=m_appData.begin(); entry!=m_appData.end(); ++entry)
        {
        DgnDbStatus stat = entry->second->_OnDelete(*this);
        if (DgnDbStatus::Success != stat)
            return stat;
        }

    // Ensure lock acquired
    DgnDbStatus stat = GetDgnDb().BriefcaseManager().OnElementDelete(*this);
    if (DgnDbStatus::Success != stat)
        return stat;

    return GetModel()->_OnDeleteElement(*this);
    }

struct OnDeletedCaller  {DgnElement::AppData::DropMe operator()(DgnElement::AppData& app, DgnElementCR el) const {return app._OnDeleted(el);}};
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::_OnDeleted() const
    {
    CallAppData(OnDeletedCaller());
    GetDgnDb().Elements().DropFromPool(*this);
    DgnModelPtr model = GetModel();
    if (model.IsValid())
        model->_OnDeletedElement(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::_OnReversedAdd() const
    {
    CallAppData(OnDeletedCaller());
    GetDgnDb().Elements().DropFromPool(*this);
    DgnModelPtr model = GetModel();
    if (model.IsValid())
        model->_OnReversedAddElement(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
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
        statement.BindText(statement.GetParameterIndex(BIS_ELEMENT_PROP_CodeValue), m_code.GetValue().c_str(), IECSqlBinder::MakeCopy::No);

    statement.BindId(statement.GetParameterIndex(BIS_ELEMENT_PROP_CodeAuthorityId), m_code.GetAuthority());
    statement.BindText(statement.GetParameterIndex(BIS_ELEMENT_PROP_CodeNamespace), m_code.GetNamespace().c_str(), IECSqlBinder::MakeCopy::No);

    if (HasUserLabel())
        statement.BindText(statement.GetParameterIndex(BIS_ELEMENT_PROP_UserLabel), GetUserLabel(), IECSqlBinder::MakeCopy::No);
    else
        statement.BindNull(statement.GetParameterIndex(BIS_ELEMENT_PROP_UserLabel));

    statement.BindId(statement.GetParameterIndex(BIS_ELEMENT_PROP_ParentId), m_parentId);

    if (m_federationGuid.IsValid())
        statement.BindBinary(statement.GetParameterIndex(BIS_ELEMENT_PROP_FederationGuid), &m_federationGuid, sizeof(m_federationGuid), IECSqlBinder::MakeCopy::No);
    else
        statement.BindNull(statement.GetParameterIndex(BIS_ELEMENT_PROP_FederationGuid));

    if (forInsert != ForInsert::Yes)
        return;

    statement.BindId(statement.GetParameterIndex(BIS_ELEMENT_PROP_ECInstanceId), m_elementId);
    statement.BindId(statement.GetParameterIndex(BIS_ELEMENT_PROP_ModelId), m_modelId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus DgnElement::_InsertInDb()
    {
    CachedECSqlStatementPtr statement = GetDgnDb().Elements().GetPreparedInsertStatement(*this);
    if (statement.IsNull())
        return DgnDbStatus::WriteError;

    _BindWriteParams(*statement, ForInsert::Yes);
 
    auto stmtResult = statement->Step();
    if (BE_SQLITE_DONE != stmtResult)
        {
        // SQLite doesn't tell us which constraint failed - check if it's the Code. (NOTE: We should catch this in _OnInsert())
        auto existingElemWithCode = GetDgnDb().Elements().QueryElementIdByCode(m_code);
        return existingElemWithCode.IsValid() ? DgnDbStatus::DuplicateCode : DgnDbStatus::WriteError;
        }

    if (PropState::Dirty == m_flags.m_propState)
        {
        ElementAutoHandledPropertiesECInstanceAdapter ec(*this, false);
        auto status = ec.UpdateProperties();
        if (DgnDbStatus::Success != status)
            {
            BeAssert(false && "Auto-handled properties update failed - see log for sql constraint errors, etc.");
            return status;
            }
        }

    return m_userProperties ? SaveUserProperties() : DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
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
        ElementAutoHandledPropertiesECInstanceAdapter ec(*this, false);
        auto status = ec.UpdateProperties();
        if (DgnDbStatus::Success != status)
            {
            BeAssert(false && "Auto-handled properties update failed - see log for sql constraint errors, etc.");
            return status;
            }
        }

    return m_userProperties ? SaveUserProperties() : DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
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
* @bsimethod                                    Keith.Bentley                   05/15
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
// @bsiclass                                                    Keith.Bentley   11/10
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
* @bsimethod                                    Brien.Bastings                  11/15
+---------------+---------------+---------------+---------------+---------------+------*/
GeometrySource2dCP DgnElement::ToGeometrySource2d() const
    {
    GeometrySourceCP source = _ToGeometrySource();
    return nullptr == source ? nullptr : source->GetAsGeometrySource2d();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  11/15
+---------------+---------------+---------------+---------------+---------------+------*/
GeometrySource3dCP DgnElement::ToGeometrySource3d() const
    {
    GeometrySourceCP source = _ToGeometrySource();
    return nullptr == source ? nullptr : source->GetAsGeometrySource3d();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
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
* @bsimethod                                    Brien.Bastings                  11/15
+---------------+---------------+---------------+---------------+---------------+------*/
Transform GeometrySource::GetPlacementTransform() const
    {
    GeometrySource3dCP source3d = _GetAsGeometrySource3d();
    return nullptr != source3d ? source3d->GetPlacement().GetTransform() : _GetAsGeometrySource2d()->GetPlacement().GetTransform();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometrySource::SetUndisplayed(bool yesNo) const
    {
    DgnElementP el = const_cast<DgnElementP>(_ToElement());

    if (nullptr == el)
        return;

    el->m_flags.m_undisplayed = yesNo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometrySource::_SetHilited(DgnElement::Hilited newState) const
    {
    DgnElementP el = const_cast<DgnElementP>(_ToElement());

    if (nullptr == el)
        return;

    el->m_flags.m_hilited = (uint8_t) newState;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  11/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometrySource::SetInSelectionSet(bool yesNo) const
    {
    DgnElementP el = const_cast<DgnElementP>(_ToElement());

    if (nullptr == el)
        return;

    el->m_flags.m_inSelectionSet = yesNo; 
    el->m_flags.m_hilited = (uint8_t) (yesNo ? DgnElement::Hilited::Normal : DgnElement::Hilited::None);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
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
* @bsimethod                                    Sam.Wilson                      07/15
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
* @bsistruct                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::CopyForCloneFrom(DgnElementCR src)
    {
    if (!src.m_userProperties)
        src.LoadUserProperties();

    DgnCode code = GetCode();
    _CopyFrom(src);
    m_code = code;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  07/15
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
* @bsimethod                                    Sam.Wilson                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr DgnElement::_CloneForImport(DgnDbStatus* inStat, DgnModelR destModel, DgnImportContext& importer) const
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(stat, inStat);

    DgnElement::CreateParams params = GetCreateParamsForImport(destModel, importer);
    params.m_modelId = destModel.GetModelId();

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

    stat = DgnDbStatus::Success;
    return cloneElem;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::_CopyFrom(DgnElementCR other)
    {
    if (&other == this)
        return;

    // Copying between DgnDbs is not allowed. Caller must do Id remapping.
    m_code      = other.m_code;
    m_userLabel = other.m_userLabel;
    m_parentId  = other.m_parentId;
    // don't copy FederationGuid

    // Copy the auto-handled EC properties
    ElementAutoHandledPropertiesECInstanceAdapter ecOther(other, true);
    if (ecOther.IsValid())
        {
        // Note that we are NOT necessarily going to call _SetPropertyValue on each property. 
        // If the subclass needs to validate specific auto-handled properties even during copying, then it
        // must override _CopyFrom and validate after the copy is done.
        // TRICKY: Don't load my auto-handled properties at the outset. That will typically lead me to allocate a smaller
        //          buffer for what I have now (if anything) and then have to realloc it to accommodate the other element's buffer.
        //          Instead, wait and let CopyFromBuffer tell me the *exact* size to allocate.
        ElementAutoHandledPropertiesECInstanceAdapter ecThis(*this, false, ecOther.CalculateBytesUsed());
        if (ecThis.IsValid()) // this might not have auto-handled props if this and other are instances of different classes
            {
            if (GetElementClassId() != other.GetElementClassId())
                ecThis.CopyDataBuffer(ecOther, true);
            else
                ecThis.CopyFromBuffer(ecOther);

            if (nullptr != m_ecPropertyData)
                m_flags.m_propState = DgnElement::PropState::Dirty;
            }
        }

    CopyUserProperties(other);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::CopyAppDataFrom(DgnElementCR source) const
    {
    for (auto a : source.m_appData)
        {
        AddAppData(*a.first, a.second.get());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnCode::RelocateToDestinationDb(DgnImportContext& importer)
    {
    m_authority = importer.RemapAuthorityId(m_authority);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::_RemapIds(DgnImportContext& importer)
    {
    BeAssert(importer.IsBetweenDbs());
    m_code.RelocateToDestinationDb(importer);
    m_parentId   = importer.FindElementId(m_parentId);
    RemapAutoHandledNavigationproperties(importer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::CreateParams DgnElement::GetCreateParamsForImport(DgnModelR destModel, DgnImportContext& importer) const
    {
    CreateParams parms(importer.GetDestinationDb(), GetModelId(), GetElementClassId());
    DgnAuthorityCPtr authority = GetCode().IsValid() ? GetDgnDb().Authorities().GetAuthority(GetCode().GetAuthority()) : nullptr;
    if (authority.IsValid())
        authority->CloneCodeForImport(parms.m_code, *this, destModel, importer);

    if (importer.IsBetweenDbs())
        parms.RelocateToDestinationDb(importer);

    return parms;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Ramanujam.Raman                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::LoadUserProperties() const
    {
    BeAssert(!m_userProperties);
    
    m_userProperties = new AdHocJsonContainer();

    if (!IsPersistent())
       return;
    BeAssert(GetElementId().IsValid());

    CachedECSqlStatementPtr stmt = GetDgnDb().GetPreparedECSqlStatement("SELECT UserProperties FROM " BIS_SCHEMA(BIS_CLASS_Element) " WHERE ECInstanceId=?");
    BeAssert(stmt.IsValid());

    stmt->BindId(1, GetElementId());

    DbResult result = stmt->Step();
    BeAssert(result == BE_SQLITE_ROW && "Expected user properties for element");
    UNUSED_VARIABLE(result);

    Utf8CP userPropertiesStr = stmt->GetValueText(0);
    if (!Utf8String::IsNullOrEmpty(userPropertiesStr))
       m_userProperties->FromString(userPropertiesStr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Ramanujam.Raman                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::SaveUserProperties() const
    {
    BeAssert(m_userProperties);

    CachedECSqlStatementPtr stmt = GetDgnDb().GetNonSelectPreparedECSqlStatement("UPDATE " BIS_SCHEMA(BIS_CLASS_Element) " SET UserProperties=? WHERE ECInstanceId=?", GetDgnDb().GetECSqlWriteToken());
    BeAssert(stmt.IsValid());

    Utf8String str;
    if (m_userProperties->IsEmpty())
        {
        stmt->BindNull(1);
        }
    else
        {
        str = m_userProperties->ToString();
        stmt->BindText(1, str.c_str(), IECSqlBinder::MakeCopy::No);
        }
 
    BeAssert(GetElementId().IsValid());
    stmt->BindId(2, GetElementId());

    DbResult result = stmt->Step();
    if (result != BE_SQLITE_DONE)
        {
        BeAssert(false && "Could not save user properties");
        return DgnDbStatus::WriteError;
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Ramanujam.Raman                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::UnloadUserProperties() const
    {
    BeAssert(m_userProperties);
    delete m_userProperties;
    m_userProperties = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Ramanujam.Raman                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::CopyUserProperties(DgnElementCR other)
    {
    /* We do not want to incur the expense of loading/saving the user properties unless 
     * necessary - these properties may be quite large, and in the majority of use-cases 
     * DgnElement-s don't need to access these properties (even if they contain them). 
     * 
     * We dynamically load these properties only in these scenarios - 
     * ... when access is required - see GetUserProperties(). 
     * ... when copies are made for cloning - see CopyForCloneFrom(). 
     * 
     * Note that we do NOT dynamically load these properties when copies are made for
     * edits (CopyForEdit()) and updates (UpdateElement()). Whereas clone-s can be done between 
     * different elements, edits/updates are done only for the same element. 
     *
     * We then save these properties only if they have been loaded (see SaveUserProperties()).
     *
     * Therefore, in this routine (which is called from _CopyFrom(), i.e., called in all scenarios 
     * where an in-memory copy is made), we can  reliably infer that we need to make a copy of the 
     * user properties only when they have been loaded in the source/other element.  
     */

    if (other.m_userProperties)
        {
        if (!m_userProperties)
            LoadUserProperties();
        *m_userProperties = *other.m_userProperties;
        }
    else
        {
        if (m_userProperties)
            UnloadUserProperties();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Ramanujam.Raman                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::AdHocJsonPropertyValue DgnElement::GetUserProperty(Utf8CP name) const
    {
    if (!m_userProperties)
        LoadUserProperties();
    return m_userProperties->Get(name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Ramanujam.Raman                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnElement::ContainsUserProperty(Utf8CP name) const
    {
    if (!m_userProperties)
        LoadUserProperties();
    return m_userProperties->Contains(name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Ramanujam.Raman                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::RemoveUserProperty(Utf8CP name)
    {
    if (!m_userProperties)
        LoadUserProperties();
    return m_userProperties->Remove(name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                Ramanujam.Raman                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::ClearUserProperties()
    {
    if (!m_userProperties)
        LoadUserProperties();
    return m_userProperties->Clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
ElementImporter::ElementImporter(DgnImportContext& c) : m_context(c), m_copyChildren(true), m_copyGroups(false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
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
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnElement::Import(DgnDbStatus* stat, DgnModelR destModel, DgnImportContext& importer) const
    {
    if (GetElementHandler()._IsRestrictedAction(RestrictedAction::Clone))
        {
        if (nullptr != stat)
            *stat = DgnDbStatus::MissingHandler;

        return nullptr;
        }

    if (nullptr != stat)
        *stat = DgnDbStatus::Success;

    auto parent = GetDgnDb().Elements().GetElement(m_parentId);
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
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr DgnElement::Clone(DgnDbStatus* stat, DgnElement::CreateParams const* params) const
    {
    if (GetElementHandler()._IsRestrictedAction(RestrictedAction::Clone))
        {
        if (nullptr != stat)
            *stat = DgnDbStatus::MissingHandler;

        return nullptr;
        }

    return _Clone(stat, params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement2d::_AdjustPlacementForImport(DgnImportContext const& importer)
    {
    m_placement.GetOriginR().Add(DPoint2d::From(importer.GetOriginOffset()));
    m_placement.GetAngleR() = (m_placement.GetAngle() + importer.GetYawAdjustment());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement3d::_AdjustPlacementForImport(DgnImportContext const& importer)
    {
    m_placement.GetOriginR().Add(importer.GetOriginOffset());
    m_placement.GetAnglesR().AddYaw(importer.GetYawAdjustment());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement::CopyFromGeometrySource(GeometrySourceCR src)
    {
    m_categoryId = src.GetCategoryId();
    m_geom = src.GetGeometryStream();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement2d::_CopyFrom(DgnElementCR el)
    {
    T_Super::_CopyFrom(el);
    auto src = el.ToGeometrySource2d();
    if (nullptr != src)
        {
        CopyFromGeometrySource(*src);
        m_placement = src->GetPlacement();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement3d::_CopyFrom(DgnElementCR el)
    {
    T_Super::_CopyFrom(el);
    auto src = el.ToGeometrySource3d();
    if (nullptr != src)
        {
        CopyFromGeometrySource(*src);
        m_placement = src->GetPlacement();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement::_RemapIds(DgnImportContext& importer)
    {
    T_Super::_RemapIds(importer);
    m_categoryId = importer.RemapCategory(m_categoryId);
    importer.RemapGeometryStreamIds(m_geom);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandlerR DgnElement::GetElementHandler() const
    {
    return *dgn_ElementHandler::Element::FindHandler(GetDgnDb(), m_classId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr DgnElement::CopyForEdit() const
    {
    DgnElement::CreateParams createParams(GetDgnDb(), m_modelId, m_classId, GetCode(), GetUserLabel(), m_parentId, GetFederationGuid());
    createParams.SetElementId(GetElementId());

    DgnElementPtr newEl = GetElementHandler()._CreateInstance(createParams);
#ifdef __clang__
    BeAssert(0 == strcmp(typeid(*newEl).name(), typeid(*this).name()));
#else
    BeAssert(typeid(*newEl) == typeid(*this)); // this means the ClassId of the element does not match the type of the element. Caller should find out why.
#endif
    newEl->_CopyFrom(*this);
    return newEl;
    }

BEGIN_UNNAMED_NAMESPACE
static const double halfMillimeter() {return .5 * DgnUnits::OneMillimeter();}
static void fixRange(double& low, double& high) {if (low==high) {low-=halfMillimeter(); high+=halfMillimeter();}}
END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d Placement3d::CalculateRange() const
    {
    if (!IsValid())
        return AxisAlignedBox3d();

    AxisAlignedBox3d range;
    GetTransform().Multiply(range, m_boundingBox);

    // low and high are not allowed to be equal
    fixRange(range.low.x, range.high.x);
    fixRange(range.low.y, range.high.y);
    fixRange(range.low.z, range.high.z);

    return range;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
AxisAlignedBox3d Placement2d::CalculateRange() const
    {
    if (!IsValid())
        return AxisAlignedBox3d();

    AxisAlignedBox3d range;
    GetTransform().Multiply(range, DRange3d::From(&m_boundingBox.low, 2, 0.0));

    // low and high are not allowed to be equal
    fixRange(range.low.x, range.high.x);
    fixRange(range.low.y, range.high.y);
    range.low.z = -halfMillimeter();
    range.high.z = halfMillimeter();

    return range;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   05/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnElement::Update(DgnDbStatus* stat) {return GetDgnDb().Elements().Update(*this, stat);}
DgnElementCPtr DgnElement::Insert(DgnDbStatus* stat) {return GetDgnDb().Elements().Insert(*this, stat);}
DgnDbStatus DgnElement::Delete() const {return GetDgnDb().Elements().Delete(*this);}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    10/2015
//---------------------------------------------------------------------------------------
DgnDbStatus ElementGroupsMembers::Insert(DgnElementCR group, DgnElementCR member, int priority)
    {
    CachedECSqlStatementPtr statement = group.GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "INSERT INTO " BIS_SCHEMA(BIS_REL_ElementGroupsMembers) 
        " (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId,MemberPriority) VALUES(?,?,?,?,?)", group.GetDgnDb().GetECSqlWriteToken());

    if (!statement.IsValid())
        return DgnDbStatus::BadRequest;

    statement->BindId(1, group.GetElementClassId());
    statement->BindId(2, group.GetElementId());
    statement->BindId(3, member.GetElementClassId());
    statement->BindId(4, member.GetElementId());
    statement->BindInt(5, priority);
    return (BE_SQLITE_DONE == statement->Step()) ? DgnDbStatus::Success : DgnDbStatus::BadRequest;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    10/2015
//---------------------------------------------------------------------------------------
DgnDbStatus ElementGroupsMembers::Delete(DgnElementCR group, DgnElementCR member)
    {
    CachedECSqlStatementPtr statement = group.GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "DELETE FROM " BIS_SCHEMA(BIS_REL_ElementGroupsMembers) " WHERE SourceECInstanceId=? AND TargetECInstanceId=?", group.GetDgnDb().GetECSqlWriteToken());

    if (!statement.IsValid())
        return DgnDbStatus::BadRequest;

    statement->BindId(1, group.GetElementId());
    statement->BindId(2, member.GetElementId());
    return (BE_SQLITE_DONE == statement->Step()) ? DgnDbStatus::Success : DgnDbStatus::BadRequest;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    10/2015
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
// @bsimethod                                   Shaun.Sewall                    10/2015
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
// @bsimethod                                   Shaun.Sewall                    12/2015
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
// @bsimethod                                   Shaun.Sewall                    10/2015
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
* @bsimethod                                    Sam.Wilson      06/15
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
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::Aspect::Aspect()
    {
    m_changeType = ChangeType::None;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::Aspect::InsertThis(DgnElementCR el)
    {
    DgnDbStatus status = _InsertInstance(el, el.GetDgnDb().GetECSqlWriteToken());
    if (DgnDbStatus::Success != status)
        return status;
    return _UpdateProperties(el, el.GetDgnDb().GetECSqlWriteToken()); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnClassId  DgnElement::Aspect::GetECClassId(DgnDbR db) const
    {
    return DgnClassId(db.Schemas().GetECClassId(_GetECSchemaName(), _GetECClassName()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP DgnElement::Aspect::GetECClass(DgnDbR db) const
    {
    return db.Schemas().GetECClass(_GetECSchemaName(), _GetECClassName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::AppData::DropMe DgnElement::Aspect::_OnInserted(DgnElementCR el)
    {
    if (ChangeType::Delete != m_changeType) // (caller can cancel an add by calling Delete)
        InsertThis(el);
    
    m_changeType = ChangeType::None; // (Just in case)

    return DropMe::Yes;     // this scheduled change has been processed, so remove it.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::AppData::DropMe DgnElement::Aspect::_OnUpdated(DgnElementCR modified, DgnElementCR original, bool isOriginal)
    {
    if (ChangeType::None == m_changeType)
        return DropMe::Yes;     // Was just a cached instance? Drop it now, so that it does not become stale.

    if (ChangeType::Delete == m_changeType)
        {
        _DeleteInstance(modified, original.GetDgnDb().GetECSqlWriteToken());
        }
    else
        {
        DgnDbR db = modified.GetDgnDb();
        ECInstanceKey existing = _QueryExistingInstanceKey(modified);
        if (existing.IsValid() && (existing.GetECClassId() != GetECClassId(db)))
            {
            _DeleteInstance(modified, original.GetDgnDb().GetECSqlWriteToken());
            existing = ECInstanceKey();  //  trigger an insert below
            }
            
        if (!existing.IsValid())
            InsertThis(modified);
        else
            _UpdateProperties(modified, original.GetDgnDb().GetECSqlWriteToken());
        }

    m_changeType = ChangeType::None; // (Just in case)

    return DropMe::Yes; // this scheduled change has been processed, so remove it.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      08/15
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<DgnElement::Aspect> DgnElement::Aspect::_CloneForImport(DgnElementCR el, DgnImportContext& importer) const
    {
    DgnClassId classid = GetECClassId(el.GetDgnDb());
    if (!el.GetElementId().IsValid() || !classid.IsValid())
        return nullptr;

    dgn_AspectHandler::Aspect* handler = dgn_AspectHandler::Aspect::FindHandler(el.GetDgnDb(), classid);
    if (nullptr == handler)
        return nullptr;

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
* @bsimethod                                    Sam.Wilson      06/15
+===============+===============+===============+===============+===============+======*/
BEGIN_BENTLEY_DGN_NAMESPACE
struct MultiAspectMux : DgnElement::AppData
{
    ECClassCR m_ecclass;
    bvector<RefCountedPtr<DgnElement::MultiAspect>> m_instances;

    static Key& GetKey(ECClassCR cls) {return *(Key*)&cls;}
    Key& GetKey(DgnDbR db) {return GetKey(m_ecclass);}

    static MultiAspectMux* Find(DgnElementCR, ECClassCR);
    static MultiAspectMux& Get(DgnElementCR, ECClassCR);

    MultiAspectMux(ECClassCR cls) : m_ecclass(cls) {;}
    DropMe _OnInserted(DgnElementCR el) override;
    DropMe _OnUpdated(DgnElementCR modified, DgnElementCR original, bool isOriginal) override;
};

END_BENTLEY_DGN_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
MultiAspectMux* MultiAspectMux::Find(DgnElementCR el, ECClassCR cls)
    {
    AppData* appData = el.FindAppData(GetKey(cls));
    if (nullptr == appData)
        return nullptr;
    MultiAspectMux* mux = dynamic_cast<MultiAspectMux*>(appData);
    BeAssert(nullptr != mux && "The same ECClass cannot have both Unique and MultiAspects");
    return mux;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
MultiAspectMux& MultiAspectMux::Get(DgnElementCR el, ECClassCR cls)
    {
    MultiAspectMux* mux = Find(el,cls);
    if (nullptr == mux)
        el.AddAppData(GetKey(cls), mux = new MultiAspectMux(cls));
    return *mux;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::AppData::DropMe MultiAspectMux::_OnInserted(DgnElementCR el)
    {
    for (auto aspect : m_instances)
        aspect->_OnInserted(el);

    return DropMe::Yes; // all scheduled changes have been processed, so remove them.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::AppData::DropMe MultiAspectMux::_OnUpdated(DgnElementCR modified, DgnElementCR original, bool isOriginal)
    {
    for (auto aspect : m_instances)
        aspect->_OnUpdated(modified, original, isOriginal);

    return DropMe::Yes; // all scheduled changes have been processed, so remove them.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::MultiAspect::_DeleteInstance(DgnElementCR el, BeSQLite::EC::ECSqlWriteToken const* writeToken)
    {
    // I am assuming that the ElementOwnsAspects ECRelationship is either just a foreign key column on the aspect or that ECSql somehow deletes the relationship instance automatically.
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(Utf8PrintfString("DELETE FROM %s WHERE ECInstanceId=?", GetFullEcSqlClassName().c_str()).c_str(), writeToken);
    stmt->BindId(1, m_instanceId);
    BeSQLite::DbResult status = stmt->Step();
    return (BeSQLite::BE_SQLITE_DONE == status) ? DgnDbStatus::Success : DgnDbStatus::WriteError;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::MultiAspect::_InsertInstance(DgnElementCR el, BeSQLite::EC::ECSqlWriteToken const* writeToken)
    {
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(Utf8PrintfString("INSERT INTO %s ([ElementId]) VALUES (?)", GetFullEcSqlClassName().c_str()).c_str(), writeToken);
    stmt->BindId(1, el.GetElementId());

    ECInstanceKey key;
    if (BeSQLite::BE_SQLITE_DONE != stmt->Step(key))
        return DgnDbStatus::WriteError;

    m_instanceId = key.GetECInstanceId();
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::MultiAspect* DgnElement::MultiAspect::GetAspectP(DgnElementR el, ECClassCR cls, ECInstanceId id)
    {
    //  First, see if we alrady have this particular MultiAspect cached
    MultiAspectMux* mux = MultiAspectMux::Find(el,cls);
    if (nullptr != mux)
        {
        for (RefCountedPtr<MultiAspect> aspect : mux->m_instances)
            {
            if (aspect->GetAspectInstanceId() != id)
                continue;
            if (aspect->m_changeType == ChangeType::Delete)
                return nullptr;
            aspect->m_changeType = ChangeType::Write; // caller intends to modify the aspect
            return aspect.get();
            }
        }

    //  First time we've been asked for this particular aspect. Cache it.
    dgn_AspectHandler::Aspect* handler = dgn_AspectHandler::Aspect::FindHandler(el.GetDgnDb(), DgnClassId(cls.GetId()));
    if (nullptr == handler)
        return nullptr;

    RefCountedPtr<MultiAspect> aspect = dynamic_cast<MultiAspect*>(handler->_CreateInstance().get());
    if (!aspect.IsValid())
        return nullptr;

    aspect->m_instanceId = id;

    if (DgnDbStatus::Success != aspect->_LoadProperties(el))
        return nullptr;

    MultiAspectMux::Get(el,cls).m_instances.push_back(aspect);

    aspect->m_changeType = ChangeType::Write; // caller intends to modify the aspect

    return aspect.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
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
// @bsimethod                                  Sam.Wilson                    03/2015
//---------------------------------------------------------------------------------------
ECInstanceKey DgnElement::MultiAspect::_QueryExistingInstanceKey(DgnElementCR el)
    {
    // My m_instanceId field is valid if and only if I was just inserted or was loaded from an existing instance.
    return ECInstanceKey(GetECClassId(el.GetDgnDb()), m_instanceId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::UniqueAspect* DgnElement::UniqueAspect::Find(DgnElementCR el, ECClassCR cls)
    {
    AppData* appData = el.FindAppData(GetKey(cls));
    if (nullptr == appData)
        return nullptr;
    UniqueAspect* aspect = dynamic_cast<UniqueAspect*>(appData);
    BeAssert(nullptr != aspect && "The same ECClass cannot have both Unique and MultiAspects");
    return aspect;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::UniqueAspect::SetAspect(DgnElementR el, UniqueAspect& newAspect)
    {
    SetAspect0(el, newAspect);
    BeAssert(nullptr != Find(el, *newAspect.GetECClass(el.GetDgnDb())));
    newAspect.m_changeType = ChangeType::Write;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
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
* @bsimethod                                    Sam.Wilson      06/15
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
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::UniqueAspect::SetAspect0(DgnElementCR el, UniqueAspect& newItem)
    {
    Key& key = newItem.GetKey(el.GetDgnDb());
    el.DropAppData(key);  // remove any existing cached aspect
    el.AddAppData(key, &newItem);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
RefCountedPtr<DgnElement::UniqueAspect> DgnElement::UniqueAspect::Load0(DgnElementCR el, DgnClassId classid)
    {
    if (!el.GetElementId().IsValid() || !classid.IsValid())
        return nullptr;

    RefCountedPtr<DgnElement::UniqueAspect> aspect;

    dgn_AspectHandler::Aspect* handler = dgn_AspectHandler::Aspect::FindHandler(el.GetDgnDb(), classid);
    if ((nullptr == handler) || handler->_IsMissingHandler()) 
        {
        auto eclass = el.GetDgnDb().Schemas().GetECClass(classid);
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
* @bsimethod                                    Sam.Wilson      06/15
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
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::UniqueAspect::_InsertInstance(DgnElementCR el, BeSQLite::EC::ECSqlWriteToken const* writeToken)
    {
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(Utf8PrintfString("INSERT INTO %s (ElementId) VALUES(?)", GetFullEcSqlClassName().c_str()).c_str(), writeToken);
    stmt->BindId(1, el.GetElementId());

    ECInstanceKey key;
    if (BeSQLite::BE_SQLITE_DONE != stmt->Step(key))
        return DgnDbStatus::WriteError;

    m_instanceId = key.GetECInstanceId();
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::UniqueAspect::_DeleteInstance(DgnElementCR el, BeSQLite::EC::ECSqlWriteToken const* writeToken)
    {
    // I am assuming that the ElementOwnsAspects ECRelationship is either just a foreign key column on the aspect or that ECSql somehow deletes the relationship instance automatically.
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(Utf8PrintfString("DELETE FROM %s WHERE [ElementId]=?", GetFullEcSqlClassName().c_str()).c_str(), writeToken);
    stmt->BindId(1, el.GetElementId());
    DbResult status = stmt->Step();
    return (BE_SQLITE_DONE == status) ? DgnDbStatus::Success : DgnDbStatus::WriteError;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                  Sam.Wilson                    03/2015
//---------------------------------------------------------------------------------------
ECInstanceKey DgnElement::UniqueAspect::_QueryExistingInstanceKey(DgnElementCR el)
    {
    // We know what the class and the ID of an instance *would be* if it exists. See if such an instance actually exists.
    DgnClassId classId = GetECClassId(el.GetDgnDb());

    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("SELECT ECInstanceId FROM %s WHERE [ElementId]=?", GetFullEcSqlClassName().c_str()).c_str());
    if (!stmt.IsValid())
        {
        BeAssert(false);
        return ECInstanceKey();
        }
    stmt->BindId(1, el.GetElementId());
    if (BE_SQLITE_ROW != stmt->Step())
        return ECInstanceKey();

    // And we know the ID. See if such an instance actually exists.
    return ECInstanceKey(classId, stmt->GetValueId<ECInstanceId>(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_ElementHandler::Element::_RegisterPropertyAccessors(ECSqlClassInfo& params, ECN::ClassLayoutCR layout)
    {
    params.RegisterPropertyAccessors(layout, BIS_ELEMENT_PROP_FederationGuid, 
        [](ECValueR value, DgnElementCR el)
            {
            value.SetBinary((Byte*)&el.m_federationGuid, sizeof(el.m_federationGuid));
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
            value.SetUtf8CP(el.GetCode().GetValue().c_str());
            return DgnDbStatus::Success;
            },
        
        [](DgnElementR el, ECValueCR value)
            {
            if (!value.IsString())
                return DgnDbStatus::BadArg;
            DgnCode existingCode = el.GetCode();
            DgnCode newCode(existingCode.GetAuthority(), value.ToString(), existingCode.GetNamespace());
            return el.SetCode(newCode);
            });

    params.RegisterPropertyAccessors(layout, BIS_ELEMENT_PROP_CodeNamespace,
        [](ECValueR value, DgnElementCR el)
            {
            value.SetUtf8CP(el.GetCode().GetNamespace().c_str());
            return DgnDbStatus::Success;
            },
        
        [](DgnElementR el, ECValueCR value)
            {
            if (!value.IsString())
                return DgnDbStatus::BadArg;
            DgnCode existingCode = el.GetCode();
            DgnCode newCode(existingCode.GetAuthority(), existingCode.GetValue(), value.ToString());
            return el.SetCode(newCode);
            });
        
    params.RegisterPropertyAccessors(layout, BIS_ELEMENT_PROP_CodeAuthorityId, 
        [](ECValueR value, DgnElementCR el)
            {
            value.SetLong(el.GetCode().GetAuthority().GetValueUnchecked());
            return DgnDbStatus::Success;
            },
        
        [](DgnElementR el, ECValueCR value)
            {
            if (!value.IsLong())
                return DgnDbStatus::BadArg;
            DgnCode existingCode = el.GetCode();
            DgnCode newCode(DgnAuthorityId((uint64_t)value.GetLong()), existingCode.GetValue(), existingCode.GetNamespace());
            return el.SetCode(newCode);
            });
        
    params.RegisterPropertyAccessors(layout, BIS_ELEMENT_PROP_ModelId, 
        [](ECValueR value, DgnElementCR el)
            {
            value.SetLong(el.GetModelId().GetValueUnchecked());
            return DgnDbStatus::Success;
            },
        
        [](DgnElementR el, ECValueCR value)
            {
            return DgnDbStatus::ReadOnly;
            });
        
    params.RegisterPropertyAccessors(layout, BIS_ELEMENT_PROP_ParentId, 
        [](ECValueR value, DgnElementCR el)
            {
            value.SetLong(el.GetParentId().GetValueUnchecked());
            return DgnDbStatus::Success;
            },
        
        [](DgnElementR el, ECValueCR value)
            {
            if (!value.IsLong())
                return DgnDbStatus::BadArg;
            return el.SetParentId(DgnElementId((uint64_t)value.GetLong()));
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
* @bsimethod                                    Sam.Wilson                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement::RegisterGeometricPropertyAccessors(ECSqlClassInfo& params, ECN::ClassLayoutCR layout)
    {
    params.RegisterPropertyAccessors(layout, GEOM_GeometryStream, 
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
* @bsimethod                                    Sam.Wilson                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_ElementHandler::Geometric3d::_RegisterPropertyAccessors(ECSqlClassInfo& params, ECN::ClassLayoutCR layout)
    {
    T_Super::_RegisterPropertyAccessors(params, layout);

#define GETGEOMPLCPROPDBL(EXPR) [](ECValueR value, DgnElementCR elIn){GeometricElement3d& el = (GeometricElement3d&)elIn; Placement3dCR plc = el.GetPlacement(); value.SetDouble(EXPR); return DgnDbStatus::Success;}
#define GETGEOMPLCPROPPT3(EXPR) [](ECValueR value, DgnElementCR elIn){GeometricElement3d& el = (GeometricElement3d&)elIn; Placement3dCR plc = el.GetPlacement(); value.SetPoint3d(EXPR); return DgnDbStatus::Success;}
#define SETGEOMPLCPROP(EXPR) [](DgnElement& elIn, ECN::ECValueCR value){GeometricElement3d& el = (GeometricElement3d&)elIn; Placement3d plc = el.GetPlacement(); EXPR; return el.SetPlacement(plc);}

    params.RegisterPropertyAccessors(layout, GEOM3_Yaw, 
        GETGEOMPLCPROPDBL(plc.GetAngles().GetYaw().Degrees()),
        SETGEOMPLCPROP(plc.GetAnglesR().SetYaw(AngleInDegrees::FromDegrees(value.GetDouble()))));

    params.RegisterPropertyAccessors(layout, GEOM3_Pitch,
        GETGEOMPLCPROPDBL(plc.GetAngles().GetPitch().Degrees()),
        SETGEOMPLCPROP(plc.GetAnglesR().SetPitch(AngleInDegrees::FromDegrees(value.GetDouble()))));

    params.RegisterPropertyAccessors(layout, GEOM3_Roll, 
        GETGEOMPLCPROPDBL(plc.GetAngles().GetRoll().Degrees()),
        SETGEOMPLCPROP(plc.GetAnglesR().SetRoll(AngleInDegrees::FromDegrees(value.GetDouble()))));

    params.RegisterPropertyAccessors(layout, GEOM_Origin, 
        GETGEOMPLCPROPPT3(plc.GetOrigin()),
        SETGEOMPLCPROP(plc.GetOriginR() = value.GetPoint3d()));

    params.RegisterPropertyAccessors(layout, GEOM_Box_Low, 
        GETGEOMPLCPROPPT3(plc.GetElementBox().low),
        SETGEOMPLCPROP(plc.GetElementBoxR().low = value.GetPoint3d()));

    params.RegisterPropertyAccessors(layout, GEOM_Box_High, 
        GETGEOMPLCPROPPT3(plc.GetElementBox().high),
        SETGEOMPLCPROP(plc.GetElementBoxR().high = value.GetPoint3d()));

#undef GETGEOMPLCPROPDBL
#undef GETGEOMPLCPROPPT3
#undef SETGEOMPLCPROP

    params.RegisterPropertyAccessors(layout, GEOM_Category, 
        [](ECValueR value, DgnElementCR elIn)
            {
            GeometricElement3d& el = (GeometricElement3d&)elIn;
            value.SetLong(el.GetCategoryId().GetValueUnchecked());
            return DgnDbStatus::Success;
            },

        [](DgnElementR elIn, ECValueCR value)
            {
            if (value.IsNull())
                {
                BeAssert(false);
                return DgnDbStatus::BadArg;
                }
            GeometricElement3d& el = (GeometricElement3d&)elIn;
            return el.SetCategoryId(DgnCategoryId((uint64_t)value.GetLong()));
            });

    params.RegisterPropertyAccessors(layout, GEOM3_InSpatialIndex, 
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
* @bsimethod                                    Sam.Wilson                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_ElementHandler::Geometric2d::_RegisterPropertyAccessors(ECSqlClassInfo& params, ECN::ClassLayoutCR layout)
    {
    T_Super::_RegisterPropertyAccessors(params, layout);

#define GETGEOMPLCPROPDBL(EXPR) [](ECValueR value, DgnElementCR elIn)\
            {                                                                            \
            GeometricElement2d& el = (GeometricElement2d&)elIn;                          \
            Placement2dCR plc = el.GetPlacement();                                       \
            value.SetDouble(EXPR);                                                       \
            return DgnDbStatus::Success;                                                 \
            }
#define GETGEOMPLCPROPPT2(EXPR) [](ECValueR value, DgnElementCR elIn)\
            {                                                                            \
            GeometricElement2d& el = (GeometricElement2d&)elIn;                          \
            Placement2dCR plc = el.GetPlacement();                                       \
            value.SetPoint2d(EXPR);                                                      \
            return DgnDbStatus::Success;                                                 \
            }
#define SETGEOMPLCPROP(EXPR) [](DgnElement& elIn, ECN::ECValueCR value)\
            {                                                                            \
            GeometricElement2d& el = (GeometricElement2d&)elIn;                          \
            Placement2d plc = el.GetPlacement();                                         \
            EXPR;                                                                        \
            return el.SetPlacement(plc);                                                 \
            }

    params.RegisterPropertyAccessors(layout, GEOM_Origin, 
        GETGEOMPLCPROPPT2(plc.GetOrigin()),
        SETGEOMPLCPROP(plc.GetOriginR() = value.GetPoint2d()));

    params.RegisterPropertyAccessors(layout, GEOM2_Rotation, 
        GETGEOMPLCPROPDBL(plc.GetAngle().Degrees()),
        SETGEOMPLCPROP(plc.GetAngleR() = AngleInDegrees::FromRadians(value.GetDouble())));

    params.RegisterPropertyAccessors(layout, GEOM_Box_Low, 
        GETGEOMPLCPROPPT2(plc.GetElementBox().low),
        SETGEOMPLCPROP(plc.GetElementBoxR().low = value.GetPoint2d()));

    params.RegisterPropertyAccessors(layout, GEOM_Box_High, 
        GETGEOMPLCPROPPT2(plc.GetElementBox().high),
        SETGEOMPLCPROP(plc.GetElementBoxR().high = value.GetPoint2d()));

    params.RegisterPropertyAccessors(layout, GEOM_Category, 
        [](ECValueR value, DgnElementCR elIn)
            {
            GeometricElement2d& el = (GeometricElement2d&)elIn;
            value.SetLong(el.GetCategoryId().GetValueUnchecked());
            return DgnDbStatus::Success;
            },
        [](DgnElementR elIn, ECValueCR value)
            {
            if (value.IsNull())
                {
                BeAssert(false);
                return DgnDbStatus::BadArg;
                }
            GeometricElement2d& el = (GeometricElement2d&)elIn;
            return el.SetCategoryId(DgnCategoryId((uint64_t)value.GetLong()));
            });

    GeometricElement::RegisterGeometricPropertyAccessors(params, layout);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::AppData::Key const& DgnElement::ExternalKeyAspect::GetAppDataKey()
    {
    static Key s_appDataKey;
    return s_appDataKey;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::ExternalKeyAspectPtr DgnElement::ExternalKeyAspect::Create(DgnAuthorityId authorityId, Utf8CP externalKey)
    {
    if (!authorityId.IsValid() || !externalKey || !*externalKey)
        {
        BeAssert(false);
        return nullptr;
        }

    return new DgnElement::ExternalKeyAspect(authorityId, externalKey);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::AppData::DropMe DgnElement::ExternalKeyAspect::_OnInserted(DgnElementCR element)
    {
    CachedECSqlStatementPtr statement = element.GetDgnDb().GetNonSelectPreparedECSqlStatement("INSERT INTO " BIS_SCHEMA(BIS_CLASS_ElementExternalKey) " ([ElementId],[AuthorityId],[ExternalKey]) VALUES (?,?,?)", element.GetDgnDb().GetECSqlWriteToken());
    if (!statement.IsValid())
        return DgnElement::AppData::DropMe::Yes;

    statement->BindId(1, element.GetElementId());
    statement->BindId(2, GetAuthorityId());
    statement->BindText(3, GetExternalKey(), IECSqlBinder::MakeCopy::No);

    ECInstanceKey key;
    if (BE_SQLITE_DONE != statement->Step(key))
        {
        BeAssert(false);
        }

    return DgnElement::AppData::DropMe::Yes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::ExternalKeyAspect::Query(Utf8StringR externalKey, DgnElementCR element, DgnAuthorityId authorityId)
    {
    CachedECSqlStatementPtr statement = element.GetDgnDb().GetPreparedECSqlStatement("SELECT ExternalKey FROM " BIS_SCHEMA(BIS_CLASS_ElementExternalKey) " WHERE ElementId=? AND AuthorityId=?");
    if (!statement.IsValid())
        return DgnDbStatus::ReadError;

    statement->BindId(1, element.GetElementId());
    statement->BindId(2, authorityId);

    if (BE_SQLITE_ROW != statement->Step())
        return DgnDbStatus::ReadError;

    externalKey.AssignOrClear(statement->GetValueText(0));
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    09/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::ExternalKeyAspect::Delete(DgnElementCR element, DgnAuthorityId authorityId)
    {
    CachedECSqlStatementPtr statement = element.GetDgnDb().GetNonSelectPreparedECSqlStatement("DELETE FROM " BIS_SCHEMA(BIS_CLASS_ElementExternalKey) " WHERE ElementId=? AND AuthorityId=?", element.GetDgnDb().GetECSqlWriteToken());
    if (!statement.IsValid())
        return DgnDbStatus::WriteError;

    statement->BindId(1, element.GetElementId());
    statement->BindId(2, authorityId);

    if (BE_SQLITE_DONE != statement->Step())
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t DgnElement::RestrictedAction::Parse(Utf8CP name)
    {
    struct Pair {Utf8CP name; uint64_t action;};

    static const Pair s_pairs[] = 
        {
            {"clone",          Clone },
            {"setparent",      SetParent },
            {"insertchild",    InsertChild },
            {"updatechild",    UpdateChild },
            {"deletechild",    DeleteChild },
            {"setcode",        SetCode },
            {"move",           Move },
            {"setcategory",    SetCategory },
            {"setgeometry",    SetGeometry },
        };

    for (auto const& pair : s_pairs)
        {
        if (0 == BeStringUtilities::Stricmp(name, pair.name))
            return pair.action;
        }

    return T_Super::Parse(name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_OnChildInsert(DgnElementCR child) const 
    {
    if (GetElementHandler()._IsRestrictedAction(RestrictedAction::InsertChild))
        return DgnDbStatus::ParentBlockedChange;

    if (GetModelId() != child.GetModelId())
        {
        BeAssert(false);
        return DgnDbStatus::WrongModel; // parent and child must be in same model
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_OnChildUpdate(DgnElementCR, DgnElementCR) const 
    {
    if (GetElementHandler()._IsRestrictedAction(RestrictedAction::UpdateChild))
        return DgnDbStatus::ParentBlockedChange;
    
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_OnChildDelete(DgnElementCR) const 
    {
    return GetElementHandler()._IsRestrictedAction(RestrictedAction::DeleteChild) ? DgnDbStatus::ParentBlockedChange : DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_OnChildAdd(DgnElementCR child) const 
    {
    if (GetElementHandler()._IsRestrictedAction(RestrictedAction::InsertChild))
        return DgnDbStatus::ParentBlockedChange;
    
    if (GetModelId() != child.GetModelId())
        {
        BeAssert(false);
        return DgnDbStatus::WrongModel; // parent and child must be in same model
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    05/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_OnChildDrop(DgnElementCR) const 
    {
    if (GetElementHandler()._IsRestrictedAction(RestrictedAction::DeleteChild))
        return DgnDbStatus::ParentBlockedChange;
    
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::SetCode(DgnCodeCR newCode)
    {
    DgnCode oldCode = GetCode();
    if (oldCode == newCode)
        return DgnDbStatus::Success;

    if (GetElementHandler()._IsRestrictedAction(RestrictedAction::SetCode))
        return DgnDbStatus::MissingHandler;

    m_code = newCode;

    DgnDbStatus status = ValidateCode();
    if (DgnDbStatus::Success != status)
        m_code = oldCode;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnAuthorityCPtr DgnElement::GetCodeAuthority() const
    {
    return GetDgnDb().Authorities().GetAuthority(GetCode().GetAuthority());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::ValidateCode() const
    {
    DgnAuthorityCPtr auth = GetCodeAuthority();
    if (auth.IsNull() || !SupportsCodeAuthority(*auth))
        return DgnDbStatus::InvalidCodeAuthority;

    return auth->ValidateCode(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement::DoSetCategoryId(DgnCategoryId catId)
    {
    if (GetElementHandler()._IsRestrictedAction(DgnElement::RestrictedAction::SetCategory))
        return DgnDbStatus::MissingHandler;

    m_categoryId = catId;
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement2d::_SetPlacement(Placement2dCR placement)
    {
    if (GetElementHandler()._IsRestrictedAction(DgnElement::RestrictedAction::Move))
        return DgnDbStatus::MissingHandler;

    m_placement = placement;
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement3d::_SetPlacement(Placement3dCR placement)
    {
    if (GetElementHandler()._IsRestrictedAction(DgnElement::RestrictedAction::Move))
        return DgnDbStatus::MissingHandler;

    m_placement = placement;
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
PhysicalTypeCPtr PhysicalElement::GetPhysicalType() const
    {
    return GetDgnDb().Elements().Get<PhysicalType>(GetPhysicalTypeId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
GraphicalType2dCPtr GraphicalElement2d::GetGraphicalType() const
    {
    return GetDgnDb().Elements().Get<GraphicalType2d>(GetGraphicalTypeId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
ElementCopier::ElementCopier(DgnCloneContext& c) : m_context(c), m_copyChildren(true), m_copyGroups(false), m_preserveOriginalModels(true)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr ElementCopier::MakeCopy(DgnDbStatus* statusOut, DgnModelR targetModel, DgnElementCR sourceElement, DgnCode const& icode, DgnElementId newParentId)
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
    outputEditElement->SetParentId(newParentId);

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

            MakeCopy(nullptr, m_preserveOriginalModels? *sourceChildElement->GetModel(): targetModel, *sourceChildElement, DgnCode(), outputElement->GetElementId());
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
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElementTransformer::ApplyTransformTo(DgnElementR el, Transform const& transformIn)
    {
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
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnEditElementCollector& DgnEditElementCollector::operator=(DgnEditElementCollector const& rhs)
    {
    if (this != &rhs)
        {
        EmptyAll();
        CopyFrom(rhs);
        }
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnEditElementCollector::EmptyAll()
    {
    for (auto el : m_elements)
        el->Release();
    m_elements.clear();
    m_ids.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnEditElementCollector::CopyFrom(DgnEditElementCollector const& rhs)
    {
    for (auto el : rhs.m_elements)
        EditElement(*el);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr DgnEditElementCollector::AddElement(DgnElementR el) 
    {
    DgnElementId eid = el.GetElementId();
    if (eid.IsValid())
        {
        auto existing = FindElementById(eid);   // If we already have a copy of this element, return that.
        if (existing.IsValid())
            return existing;
        }

    // If we already have this exact same pointer, then don't add it again.
    auto ifound = std::find(m_elements.begin(), m_elements.end(), &el);
    if (ifound != m_elements.end())
        return &el;
    
    // This element is new. Insert it into the collection.
    m_elements.push_back(&el);
    el.AddRef();

    if (eid.IsValid())
        m_ids[eid] = &el;

    return &el;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnEditElementCollector::Equals(DgnEditElementCollector const& other, DgnElement::ComparePropertyFilter const& filter) const
    {
    if (this == &other)
        return true;
    size_t n = m_elements.size();
    if (other.m_elements.size() != n)
        return false;
    for (size_t i=0; i<n; ++i)
        {
        auto el = m_elements.at(i);
        auto otherel = other.m_elements.at(i);
        if (nullptr == el || nullptr == otherel)
            {
            if (nullptr == el && nullptr == otherel)
                continue;
            return false;
            }
        if (!el->Equals(*otherel, filter))
            return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnEditElementCollector::Dump(Utf8StringR str, DgnElement::ComparePropertyFilter const& filter) const 
    {
    str.append("{\n");
    for (auto el : *this)
        {
        str.append(Utf8PrintfString("%llx ", (uint64_t)(intptr_t)el).c_str());
        el->Dump(str, filter);
        }
    str.append("}\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnEditElementCollector::DumpTwo(Utf8StringR str, DgnEditElementCollector const& other, DgnElement::ComparePropertyFilter const& filter) const
    {
    str.append("{\n");
    for (size_t i=0; i<m_elements.size(); ++i)
        {
        auto el = m_elements.at(i);
        auto otherel = other.m_elements.at(i);
        str.append(Utf8PrintfString("%llx ", (uint64_t)(intptr_t)el).c_str());
        el->Dump(str, filter);
        str.append(Utf8PrintfString("%llx ", (uint64_t)(intptr_t)otherel).c_str());
        otherel->Dump(str, filter);
        }
    str.append("}\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnEditElementCollector::AddChildren(DgnElementCR el, size_t maxDepth) 
    {
    if (0 ==maxDepth)
        return;

    for (auto childid : el.QueryChildren())
        {
        auto child = el.GetDgnDb().Elements().GetForEdit<DgnElement>(childid);
        if (child.IsValid())
            AddAssembly(*child, maxDepth-1);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr DgnEditElementCollector::FindElementById(DgnElementId eid)
    {
    auto i = m_ids.find(eid);
    return (i == m_ids.end())? nullptr: i->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr DgnEditElementCollector::FindElementByClass(ECN::ECClassCR ecclass)
    {
    for (auto el: m_elements)
        {
        if (el->GetElementClass()->Is(&ecclass))
            return el;
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnEditElementCollector::RemoveElement(DgnElementR el) 
    {
    auto ifound = std::find(m_elements.begin(), m_elements.end(), &el);
    if (ifound == m_elements.end())
        return;

    m_elements.erase(ifound);

    DgnElementId eid = el.GetElementId();
    if (eid.IsValid())
        m_ids.erase(eid);

    el.Release();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnEditElementCollector::RemoveChildren(DgnElementCR el, size_t maxDepth) 
    {
    if (0 ==maxDepth)
        return;

    for (auto childid : el.QueryChildren())
        {
        auto child = FindElementById(childid);
        if (child.IsValid())
            RemoveAssembly(*child, maxDepth-1);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnEditElementCollector::Write(bool* anyInserts)
    {
    if (anyInserts)
        *anyInserts = false;

    bmap<DgnElementP, DgnElementCPtr> inserted;

    for (auto el : m_elements)
        {
        DgnDbStatus status;
        bool needsInsert = !el->GetElementId().IsValid();
        if (anyInserts)
            *anyInserts |= needsInsert;
        DgnElementCPtr updatedEl = !needsInsert ? el->Update(&status) : el->Insert(&status);
        if (!updatedEl.IsValid())
            return status;
        if (needsInsert)
            inserted[el] = updatedEl;
        }

    for (auto const& insert : inserted)
        {
        RemoveElement(*insert.first);
        EditElement(*insert.second);
        }
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/16
+---------------+---------------+---------------+---------------+---------------+------*/
static void getChildElementIdSet(DgnElementIdSet& assemblyIdSet, DgnElementId parentId, DgnDbR dgnDb)
    {
    DgnElementCPtr  parentEl = dgnDb.Elements().GetElement(parentId);

    if (!parentEl.IsValid())
        return;

    DgnElementIdSet parentIdSet = parentEl->QueryChildren();

    assemblyIdSet.insert(parentIdSet.begin(), parentIdSet.end());

    for (DgnElementId childId : parentIdSet)
        getChildElementIdSet(assemblyIdSet, childId, dgnDb);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId ElementAssemblyUtil::GetAssemblyParentId(DgnElementCR el)
    {
    DgnElementId parentId = el.GetParentId();

    if (!parentId.IsValid())
        return DgnElementId();

    DgnElementId thisParentId;

    do
        {
        DgnElementCPtr parentEl = el.GetDgnDb().Elements().GetElement(parentId);

        if (!parentEl.IsValid() || nullptr == parentEl->ToGeometrySource())
            return DgnElementId(); // Missing or non-geometric parent...

        // NOTE: For plant applications, it might be a good idea to stop at the first non-geometric parent to avoid selecting the entire plant with assembly lock???
        thisParentId = parentEl->GetParentId();

        if (thisParentId.IsValid())
            parentId = thisParentId;

        } while (thisParentId.IsValid());

    return parentId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  04/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIdSet ElementAssemblyUtil::GetAssemblyElementIdSet(DgnElementCR el)
    {
    DgnElementId parentId = GetAssemblyParentId(el);

    if (!parentId.IsValid())
        parentId = el.GetElementId();

    DgnElementIdSet assemblyIdSet;

    assemblyIdSet.insert(parentId); // Include parent...
    getChildElementIdSet(assemblyIdSet, parentId, el.GetDgnDb());

    if (assemblyIdSet.size() < 2)
        assemblyIdSet.clear();

    return assemblyIdSet;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ReadSelectParams(stmt, params);
    if (DgnDbStatus::Success != status)
        return status;

    m_categoryId = stmt.GetValueId<DgnCategoryId>(params.GetSelectIndex(GEOM_Category));

    // Read GeomStream
    auto geomIndex = params.GetSelectIndex(GEOM_GeometryStream);
    if (stmt.IsValueNull(geomIndex))
        return DgnDbStatus::Success;    // no geometry...

    int blobSize;
    void const* blob = stmt.GetValueBinary(geomIndex, &blobSize);
    return m_geom.ReadGeometryStream(GetDgnDb().Elements().GetSnappyFrom(), GetDgnDb(), blob, blobSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement::_BindWriteParams(ECSqlStatement& stmt, ForInsert forInsert)
    {
    T_Super::_BindWriteParams(stmt, forInsert);

    stmt.BindId(stmt.GetParameterIndex(GEOM_Category), m_categoryId);
    m_geom.BindGeometryStream(m_multiChunkGeomStream, GetDgnDb().Elements().GetSnappyTo(), stmt, GEOM_GeometryStream);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
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
            stmt.BindBinary(geomIndex, snappyTo.GetChunkData(0), zipSize, IECSqlBinder::MakeCopy::No);
            }
        else
            {
            // More than one chunk in geom stream. Avoid expensive alloc+copy by deferring writing geom stream until ECSqlStatement executes.
            multiChunkGeometryStream = true;
            stmt.BindNull(geomIndex);
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
* @bsimethod                                                    Sam.Wilson      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometricElement::_EqualProperty(ECN::ECPropertyValueCR expected, DgnElementCR other) const
    {
    if (!expected.GetValueAccessor().GetECProperty()->GetName().Equals(GEOM_GeometryStream))
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
* @bsimethod                                                    Paul.Connelly   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement::_InsertInDb()
    {
    auto stat = T_Super::_InsertInDb();
    return DgnDbStatus::Success == stat ? InsertGeomStream() : stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement::_UpdateInDb()
    {
    auto stat = T_Super::_UpdateInDb();
    return DgnDbStatus::Success == stat ? UpdateGeomStream() : stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement::_OnInsert()
    {
    auto stat = Validate();
    return DgnDbStatus::Success == stat ? T_Super::_OnInsert() : stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement::_OnUpdate(DgnElementCR el)
    {
    auto stat = Validate();
    return DgnDbStatus::Success == stat ? T_Super::_OnUpdate(el) : stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement::_OnInserted(DgnElementP copiedFrom) const 
    {
    T_Super::_OnInserted(copiedFrom);
    T_HOST.GetTxnAdmin()._OnGraphicElementAdded(m_elementId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void  GeometricElement::_OnDeleted() const 
    {
    T_Super::_OnDeleted();
    if (m_graphics.IsEmpty())
        return;
        
    T_HOST.GetTxnAdmin()._OnGraphicsRemoved(m_graphics);
    m_graphics.Clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement::_OnReversedAdd() const 
    {
    T_Super::_OnReversedAdd();
    if (m_graphics.IsEmpty())
        return;
        
    T_HOST.GetTxnAdmin()._OnGraphicsRemoved(m_graphics);
    m_graphics.Clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement::_OnReversedDelete() const 
    {
    T_Super::_OnReversedDelete();
    T_HOST.GetTxnAdmin()._OnGraphicElementAdded(m_elementId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   03/16
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement::_OnUpdateFinished() const 
    {
    T_Super::_OnUpdateFinished(); 
    T_HOST.GetTxnAdmin()._OnGraphicElementAdded(m_elementId);

    if (m_graphics.IsEmpty())
        return;
    T_HOST.GetTxnAdmin()._OnGraphicsRemoved(m_graphics);
    m_graphics.Clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement2d::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ReadSelectParams(stmt, params);
    if (DgnDbStatus::Success != status)
        return status;

    m_placement = Placement2d();

    auto originIndex = params.GetSelectIndex(GEOM_Origin);
    if (stmt.IsValueNull(originIndex))
        return DgnDbStatus::Success;    // null placement

    DPoint2d boxLow = stmt.GetValuePoint2d(params.GetSelectIndex(GEOM_Box_Low)),
             boxHi  = stmt.GetValuePoint2d(params.GetSelectIndex(GEOM_Box_High));

    m_placement = Placement2d(stmt.GetValuePoint2d(originIndex),
                              AngleInDegrees::FromDegrees(stmt.GetValueDouble(params.GetSelectIndex(GEOM2_Rotation))),
                              ElementAlignedBox2d(boxLow.x, boxLow.y, boxHi.x, boxHi.y));

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement3d::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ReadSelectParams(stmt, params);
    if (DgnDbStatus::Success != status)
        return status;

    m_placement = Placement3d();

    auto originIndex = params.GetSelectIndex(GEOM_Origin);
    if (stmt.IsValueNull(originIndex))
        return DgnDbStatus::Success;    // null placement

    DPoint3d boxLow = stmt.GetValuePoint3d(params.GetSelectIndex(GEOM_Box_Low)),
             boxHi  = stmt.GetValuePoint3d(params.GetSelectIndex(GEOM_Box_High));

    double yaw      = stmt.GetValueDouble(params.GetSelectIndex(GEOM3_Yaw)),
           pitch    = stmt.GetValueDouble(params.GetSelectIndex(GEOM3_Pitch)),
           roll     = stmt.GetValueDouble(params.GetSelectIndex(GEOM3_Roll));

    m_placement = Placement3d(stmt.GetValuePoint3d(originIndex),
                              YawPitchRollAngles(Angle::FromDegrees(yaw), Angle::FromDegrees(pitch), Angle::FromDegrees(roll)),
                              ElementAlignedBox3d(boxLow.x, boxLow.y, boxLow.z, boxHi.x, boxHi.y, boxHi.z));

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement2d::_BindWriteParams(ECSqlStatement& stmt, ForInsert forInsert)
    {
    T_Super::_BindWriteParams(stmt, forInsert);

    if (!m_placement.IsValid())
        {
        stmt.BindNull(stmt.GetParameterIndex(GEOM_Origin));
        stmt.BindNull(stmt.GetParameterIndex(GEOM_Box_Low));
        stmt.BindNull(stmt.GetParameterIndex(GEOM_Box_High));
        stmt.BindNull(stmt.GetParameterIndex(GEOM2_Rotation));
        }
    else
        {
        stmt.BindPoint2d(stmt.GetParameterIndex(GEOM_Origin), m_placement.GetOrigin());
        stmt.BindDouble(stmt.GetParameterIndex(GEOM2_Rotation), m_placement.GetAngle().Degrees());
        stmt.BindPoint2d(stmt.GetParameterIndex(GEOM_Box_Low), m_placement.GetElementBox().low);
        stmt.BindPoint2d(stmt.GetParameterIndex(GEOM_Box_High), m_placement.GetElementBox().high);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement2d::_OnInsert()
    {
    // GeometricElement2ds can only reside in 2D models
    return GetModel()->Is2dModel() ? T_Super::_OnInsert() : DgnDbStatus::WrongModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
void GeometricElement3d::_BindWriteParams(ECSqlStatement& stmt, ForInsert forInsert)
    {
    T_Super::_BindWriteParams(stmt, forInsert);

    stmt.BindInt(stmt.GetParameterIndex(GEOM3_InSpatialIndex), GetModel()->IsTemplate() ? 0 : 1); // elements in a template model do not belong in the SpatialIndex

    if (!m_placement.IsValid())
        {
        stmt.BindNull(stmt.GetParameterIndex(GEOM_Origin));
        stmt.BindNull(stmt.GetParameterIndex(GEOM3_Yaw));
        stmt.BindNull(stmt.GetParameterIndex(GEOM3_Pitch));
        stmt.BindNull(stmt.GetParameterIndex(GEOM3_Roll));
        stmt.BindNull(stmt.GetParameterIndex(GEOM_Box_Low));
        stmt.BindNull(stmt.GetParameterIndex(GEOM_Box_High));
        }
    else
        {
        stmt.BindPoint3d(stmt.GetParameterIndex(GEOM_Origin), m_placement.GetOrigin());
        stmt.BindDouble(stmt.GetParameterIndex(GEOM3_Yaw), m_placement.GetAngles().GetYaw().Degrees());
        stmt.BindDouble(stmt.GetParameterIndex(GEOM3_Pitch), m_placement.GetAngles().GetPitch().Degrees());
        stmt.BindDouble(stmt.GetParameterIndex(GEOM3_Roll), m_placement.GetAngles().GetRoll().Degrees());
        stmt.BindPoint3d(stmt.GetParameterIndex(GEOM_Box_Low), m_placement.GetElementBox().low);
        stmt.BindPoint3d(stmt.GetParameterIndex(GEOM_Box_High), m_placement.GetElementBox().high);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement3d::_OnInsert()
    {
    // GeometricElement3ds can only reside in 3D models
    return GetModel()->Is3dModel() ? T_Super::_OnInsert() : DgnDbStatus::WrongModel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement::WriteGeomStream() const
    {
    if (!m_multiChunkGeomStream)
        return DgnDbStatus::Success;

    m_multiChunkGeomStream = false;
    DgnDbR db = GetDgnDb();
    return GeometryStream::WriteGeometryStream(db.Elements().GetSnappyTo(), db, GetElementId(), _GetGeometryColumnTableName(), GEOM_GeometryStream);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometryStream::WriteGeometryStream(SnappyToBlob& snappyTo, DgnDbR db, DgnElementId elementId, Utf8CP tableName, Utf8CP columnName)
    {
    if (1 >= snappyTo.GetCurrChunk())
        {
        BeAssert(false);    // Somebody overwrote our data.
        return DgnDbStatus::WriteError;
        }

    // SaveToRow() requires a blob of the required size has already been allocated in the blob column.
    // Ideally we would do this in BindTo(), but ECSql does not support binding a zero blob.
    Utf8String sql("UPDATE ");
    sql.append(tableName);
    sql.append(" SET ");
    sql.append(columnName);
    sql.append("=? WHERE ElementId=?");

    CachedStatementPtr stmt = db.Elements().GetStatement(sql.c_str());
    stmt->BindId(2, elementId);
    stmt->BindZeroBlob(1, snappyTo.GetCompressedSize());
    if (BE_SQLITE_DONE != stmt->Step())
        return DgnDbStatus::WriteError;

    StatusInt status = snappyTo.SaveToRow(db, tableName, columnName, elementId.GetValue());
    return SUCCESS == status ? DgnDbStatus::Success : DgnDbStatus::WriteError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement::InsertGeomStream() const
    {
    DgnDbStatus status = WriteGeomStream();
    if (DgnDbStatus::Success != status)
        return status;

#if defined (NOT_NOW_TOO_EXPENSIVE_FOR_BENEFIT)
    // Insert ElementUsesGeometryParts relationships for any GeometryPartIds in the GeomStream
    DgnDbR db = GetDgnDb();
    IdSet<DgnGeometryPartId> parts;
    GeometryStreamIO::Collection(m_geom.GetData(), m_geom.GetSize()).GetGeometryPartIds(parts, db);
    for (DgnGeometryPartId const& partId : parts)
        {
        if (BentleyStatus::SUCCESS != DgnGeometryPart::InsertElementUsesGeometryParts(db, GetElementId(), partId))
            status = DgnDbStatus::WriteError;
        }
#endif

    return status;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement::UpdateGeomStream() const
    {
    DgnDbStatus status = WriteGeomStream();
    if (DgnDbStatus::Success != status)
        return status;

#if defined (NOT_NOW_TOO_EXPENSIVE_FOR_BENEFIT)
    // Update ElementUsesGeometryParts relationships for any GeometryPartIds in the GeomStream
    DgnDbR db = GetDgnDb();
    DgnElementId elementId = GetElementId();
    CachedECSqlStatementPtr statement = db.GetPreparedECSqlStatement("SELECT TargetECInstanceId FROM " BIS_SCHEMA(BIS_REL_ElementUsesGeometryParts) " WHERE SourceECInstanceId=?");
    if (!statement.IsValid())
        return DgnDbStatus::ReadError;

    statement->BindId(1, elementId);

    IdSet<DgnGeometryPartId> partsOld;
    while (BE_SQLITE_ROW == statement->Step())
        partsOld.insert(statement->GetValueId<DgnGeometryPartId>(0));

    IdSet<DgnGeometryPartId> partsNew;
    GeometryStreamIO::Collection(m_geom.GetData(), m_geom.GetSize()).GetGeometryPartIds(partsNew, db);

    if (partsOld.empty() && partsNew.empty())
        return status;

    bset<DgnGeometryPartId> partsToRemove;
    std::set_difference(partsOld.begin(), partsOld.end(), partsNew.begin(), partsNew.end(), std::inserter(partsToRemove, partsToRemove.end()));

    if (!partsToRemove.empty())
        {
        CachedECSqlStatementPtr statement = db.GetNonSelectPreparedECSqlStatement("DELETE FROM " BIS_SCHEMA(BIS_REL_ElementUsesGeometryParts) " WHERE SourceECInstanceId=? AND TargetECInstanceId=?", db.GetECSqlWriteToken());
        if (!statement.IsValid())
            return DgnDbStatus::BadRequest;

        statement->BindId(1, elementId);

        for (DgnGeometryPartId const& partId : partsToRemove)
            {
            statement->BindId(2, partId);
            if (BE_SQLITE_DONE != statement->Step())
                status = DgnDbStatus::BadRequest;
            }
        }

    bset<DgnGeometryPartId> partsToAdd;
    std::set_difference(partsNew.begin(), partsNew.end(), partsOld.begin(), partsOld.end(), std::inserter(partsToAdd, partsToAdd.end()));

    for (DgnGeometryPartId const& partId : partsToAdd)
        {
        if (BentleyStatus::SUCCESS != DgnGeometryPart::InsertElementUsesGeometryParts(db, elementId, partId))
            status = DgnDbStatus::WriteError;
        }
#endif

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::GenericUniqueAspect::_LoadProperties(Dgn::DgnElementCR el)
    {
    auto stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("SELECT * FROM [%s].[%s] WHERE ElementId=?", m_ecschemaName.c_str(), m_ecclassName.c_str()).c_str());
    if (!stmt.IsValid())
        return DgnDbStatus::BadSchema;
    stmt->BindId(1, el.GetElementId());
    if (BE_SQLITE_ROW != stmt->Step())
        return DgnDbStatus::NotFound;
    ECInstanceECSqlSelectAdapter adapter(*stmt);
    m_instance = adapter.GetInstance();
    return m_instance.IsValid()? DgnDbStatus::Success: DgnDbStatus::ReadError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::GenericUniqueAspect::_UpdateProperties(Dgn::DgnElementCR el, BeSQLite::EC::ECSqlWriteToken const*)
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

    // The the UniqueAspect's "ElementId" property. This is what links the aspect to its host element. The IDs are not the same.
    m_instance->SetValue("ElementId", ECN::ECValue(el.GetElementId().GetValue()));

    return (BE_SQLITE_OK == updater->Update(*m_instance))? DgnDbStatus::Success: DgnDbStatus::WriteError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::IECInstanceP DgnElement::GenericUniqueAspect::GetAspectP(DgnElementR el, ECN::ECClassCR cls)
    {
    GenericUniqueAspect* aspect = dynamic_cast<GenericUniqueAspect*>(T_Super::GetAspectP(el,cls));
    if (nullptr == aspect)
        return nullptr;
    return aspect->m_instance.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::IECInstanceCP DgnElement::GenericUniqueAspect::GetAspect(DgnElementCR el, ECN::ECClassCR cls)
    {
    GenericUniqueAspect const* aspect = dynamic_cast<GenericUniqueAspect const*>(T_Super::GetAspect(el,cls));
    if (nullptr == aspect)
        return nullptr;
    return aspect->m_instance.get();
    }