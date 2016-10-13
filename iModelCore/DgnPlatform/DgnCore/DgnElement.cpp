/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnElement.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnScript.h>

#define BIS_ELEMENT_PROP_ECInstanceId "ECInstanceId"
#define BIS_ELEMENT_PROP_ModelId "ModelId"
#define BIS_ELEMENT_PROP_FederationGuid "FederationGuid"
#define BIS_ELEMENT_PROP_CodeAuthorityId "CodeAuthorityId"
#define BIS_ELEMENT_PROP_CodeNamespace "CodeNamespace"
#define BIS_ELEMENT_PROP_CodeValue "CodeValue"
#define BIS_ELEMENT_PROP_UserLabel "UserLabel"
#define BIS_ELEMENT_PROP_ParentId "ParentId"
#define BIS_ELEMENT_PROP_LastMode "LastMod"

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

#define GEOM_GeometryStream "GeometryStream"
#define GEOM_Category "CategoryId"
#define GEOM_Origin "Origin"
#define GEOM_Box_Low "BBoxLow"
#define GEOM_Box_High "BBoxHigh"
#define GEOM_FacetCount "FacetCount"
#define GEOM2_Rotation "Rotation"
#define GEOM3_InSpatialIndex "InSpatialIndex"
#define GEOM3_Yaw "Yaw"
#define GEOM3_Pitch "Pitch"
#define GEOM3_Roll "Roll"

BEGIN_BENTLEY_DGN_NAMESPACE
//=======================================================================================
// *** NB: This adapter provides access to AUTO-HANDLED PROPERTIES ONLY.
// ***     It cannot be used to present all of the properties of an element as an IECInstance. 
// @bsiclass                                                     Sam.Wilson        10/16
//=======================================================================================
struct ElementInstanceAdapter : ECN::ECDBuffer, ECN::IECInstance
{
    DgnElement& m_element;
    ECClassCP m_eclass;

    ElementInstanceAdapter(DgnElement const& el);
    
    bool IsValid() const {return nullptr != m_eclass;}

    bool IsValidValue(ECN::ECPropertyCR prop, ECN::ECValueCR value);
    bool IsValidForStatementType(ECN::ECPropertyCR prop, ECSqlClassParams::StatementType stypeNeeded);
    uint32_t GetBytesUsed() const;

    BentleyStatus LoadProperties();
    BeSQLite::EC::ECInstanceUpdater* GetUpdater();
    DgnDbStatus UpdateProperties();

    // ECDBuffer:
    ECN::ECObjectsStatus _SetStructArrayValueToMemory(ECN::ECValueCR v, ECN::PropertyLayoutCR propertyLayout, uint32_t index) override { BeAssert(false); return ECN::ECObjectsStatus::Error; }
    ECN::ECObjectsStatus _GetStructArrayValueFromMemory(ECN::ECValueR v, ECN::PropertyLayoutCR propertyLayout, uint32_t index) const override { BeAssert(false); return ECN::ECObjectsStatus::Error; }
    ECN::PrimitiveType _GetStructArrayPrimitiveType() const override {/*BeAssert(false);*/ return ECN::PrimitiveType::PRIMITIVETYPE_Integer; }
    ECN::ECObjectsStatus _RemoveStructArrayElementsFromMemory(ECN::PropertyLayoutCR propertyLayout, uint32_t removeIndex, uint32_t removeCount) override { BeAssert(false); return ECN::ECObjectsStatus::Error; }
    bool _IsStructValidForArray(ECN::IECInstanceCR structInstance, ECN::PropertyLayoutCR propLayout) const { BeAssert(false); return false; }
    void _SetPerPropertyFlag(ECN::PropertyLayoutCR propertyLayout, bool, uint32_t, int flagIndex, bool enable) override { BeAssert(false); }
    ECN::ECObjectsStatus _EvaluateCalculatedProperty(ECN::ECValueR evaluatedValue, ECN::ECValueCR existingValue, ECN::PropertyLayoutCR propLayout) const override { BeAssert(false); return ECN::ECObjectsStatus::Error; }
    ECN::ECObjectsStatus _UpdateCalculatedPropertyDependents(ECN::ECValueCR calculatedValue, ECN::PropertyLayoutCR propLayout) override { BeAssert(false); return ECN::ECObjectsStatus::Error; }

    bool _AcquireData(bool) const override { return DgnElement::PropState::InBuffer == m_element.m_flags.m_propState; }
    bool _ReleaseData() const override { return true; }
    bool _IsMemoryInitialized() const override { return nullptr != m_element.m_ecPropertyData; }
    Byte const * _GetData() const override { return m_element.m_ecPropertyData; }
    bool _AllowWritingDirectlyToInstanceMemory() const override { return true; }
    bool _AreAllPropertiesCalculated() const override { return true; }
    void _SetAllPropertiesCalculated(bool) override {}
    uint32_t _GetBytesAllocated() const override { return m_element.m_ecPropertyDataSize; }
    ECN::ECObjectsStatus _ModifyData(uint32_t offset, void const * newData, uint32_t dataLength) override;
    ECN::ECObjectsStatus _MoveData(uint32_t toOffset, uint32_t fromOffset, uint32_t dataLength) override;
    ECN::ECObjectsStatus _GrowAllocation(uint32_t additionalBytesNeeded) override;
    ECN::ECObjectsStatus _ShrinkAllocation() override;
    void _FreeAllocation() override;
    void _ClearValues() override;
    ECN::ECObjectsStatus _CopyFromBuffer(ECN::ECDBufferCR source) override;
    ECN::ClassLayoutCR _GetClassLayout() const override;

    // IECInstance
    Utf8String      _GetInstanceId() const override {return m_element.GetElementId().ToString();}
    ECObjectsStatus _GetIsPropertyNull (bool& isNull, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const override
        {return GetIsNullValueFromMemory (isNull, propertyIndex, useArrayIndex, arrayIndex);}
    ECObjectsStatus _GetValue (ECValueR v, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const override 
        {return GetValueFromMemory(v, propertyIndex, useArrayIndex, arrayIndex);}
    ECObjectsStatus _SetValue (uint32_t propertyIndex, ECValueCR v, bool useArrayIndex, uint32_t arrayIndex) override
        {return SetValueToMemory(propertyIndex, v, useArrayIndex, arrayIndex);}
    ECObjectsStatus _InsertArrayElements (uint32_t propertyIndex, uint32_t index, uint32_t size) override
        {return InsertNullArrayElementsAt(propertyIndex, index, size);}
    ECObjectsStatus _AddArrayElements (uint32_t propertyIndex, uint32_t size) override
        {return AddNullArrayElementsAt(propertyIndex, size);}
    ECObjectsStatus _RemoveArrayElement (uint32_t propertyIndex, uint32_t index) override
        {return RemoveArrayElementsAt(propertyIndex, index, 1);}
    ECObjectsStatus _ClearArray (uint32_t propIdx) override
        {
        PropertyLayoutCP pPropertyLayout = NULL;
        ECObjectsStatus status = GetClassLayout().GetPropertyLayoutByIndex (pPropertyLayout, propIdx);
        if (ECObjectsStatus::Success != status || NULL == pPropertyLayout)
            return ECObjectsStatus::PropertyNotFound;

        uint32_t arrayCount = GetReservedArrayCount (*pPropertyLayout);
        if (arrayCount > 0)
            {
            RemoveArrayElements (*pPropertyLayout, 0, arrayCount);
            }

        return ECObjectsStatus::Success;
        }

    ECEnablerCR     _GetEnabler() const override {return *(m_eclass->GetDefaultStandaloneEnabler());}
    bool            _IsReadOnly() const override {return false;}
    Utf8String      _ToString (Utf8CP indent) const override {return "";}
    size_t          _GetOffsetToIECInstance () const override {return 0;} // WIP_AUTO_HANDLED_PROPERTIES -- what is this??


};

//=======================================================================================
// @bsiclass                                                     Sam.Wilson        10/16
//=======================================================================================
struct ElementECPropertyAccessor : ElementInstanceAdapter
{
    ECPropertyCP m_ecprop;

    ElementECPropertyAccessor(DgnElement const& el, Utf8CP propName);

    bool IsValid() const { return ElementInstanceAdapter::IsValid() && (nullptr != m_ecprop); }

    DgnDbStatus SetPropertyValue(ECValueCR value, DgnElement::PropertyArrayIndex const& arrayIdx);
    DgnDbStatus GetPropertyValue(ECN::ECValueR value, DgnElement::PropertyArrayIndex const& arrayIdx);
};

END_BENTLEY_DGN_NAMESPACE

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
DateTime DgnElement::QueryTimeStamp() const
    {
    ECSqlStatement stmt;
    stmt.Prepare(GetDgnDb(), "SELECT " BIS_ELEMENT_PROP_LastMode " FROM " BIS_SCHEMA(BIS_CLASS_Element) " WHERE " BIS_ELEMENT_PROP_ECInstanceId "=?");
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

    if (GetDgnDb().Elements().QueryElementIdByCode(m_code).IsValid() || GetDgnDb().Models().QueryModelId(m_code).IsValid())
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
* @bsimethod                                                    Shaun.Sewall    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
SubjectPtr Subject::Create(SubjectCR parentSubject, Utf8CP label, Utf8CP description)
    {
    DgnDbR db = parentSubject.GetDgnDb();
    DgnElementId parentId = parentSubject.GetElementId();
    DgnClassId classId = db.Domains().GetClassId(dgn_ElementHandler::Subject::GetHandler());

    if (!parentId.IsValid() || !classId.IsValid() || !label || !*label)
        {
        BeAssert(false);
        return nullptr;
        }

    SubjectPtr subject = new Subject(CreateParams(db, DgnModel::RepositoryModelId(), classId, DgnCode(), label, parentId));

    if (description && *description)
        {
        if (DgnDbStatus::Success != subject->SetPropertyValue("Descr", ECValue(description)))
            return nullptr;
        }

    return subject;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
SubjectCPtr Subject::CreateAndInsert(SubjectCR parentSubject, Utf8CP label, Utf8CP description)
    {
    SubjectPtr subject = Create(parentSubject, label, description);
    if (!subject.IsValid())
        return nullptr;

    return parentSubject.GetDgnDb().Elements().Insert<Subject>(*subject);
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
* @bsimethod                                                    Shaun.Sewall    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
DrawingPtr Drawing::Create(DocumentListModelCR model, DgnCodeCR code, Utf8CP userLabel)
    {
    DgnDbR db = model.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(dgn_ElementHandler::Drawing::GetHandler());

    if (!model.GetModelId().IsValid() || !classId.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    return new Drawing(CreateParams(db, model.GetModelId(), classId, code, userLabel));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
SectionDrawingPtr SectionDrawing::Create(DocumentListModelCR model, DgnCodeCR code, Utf8CP userLabel)
    {
    DgnDbR db = model.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(dgn_ElementHandler::SectionDrawing::GetHandler());

    if (!model.GetModelId().IsValid() || !classId.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    return new SectionDrawing(CreateParams(db, model.GetModelId(), classId, code, userLabel));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Shaun.Sewall    09/16
+---------------+---------------+---------------+---------------+---------------+------*/
SheetPtr Sheet::Create(DocumentListModelCR model, DgnCodeCR code, Utf8CP userLabel)
    {
    DgnDbR db = model.GetDgnDb();
    DgnClassId classId = db.Domains().GetClassId(dgn_ElementHandler::Sheet::GetHandler());

    if (!model.GetModelId().IsValid() || !classId.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    return new Sheet(CreateParams(db, model.GetModelId(), classId, code, userLabel));
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
    while(parentId.IsValid());

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
    if ((existingElemWithCode.IsValid() && existingElemWithCode != GetElementId()) || GetDgnDb().Models().QueryModelId(m_code).IsValid())
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
DgnDbStatus DgnElement::BindParams(ECSqlStatement& statement, bool isForUpdate)
    {
    if (!m_code.IsValid())
        {
        BeAssert(false && "Code is missing");
        return DgnDbStatus::InvalidName;
        }

    if (m_code.IsEmpty() && (ECSqlStatus::Success != statement.BindNull(statement.GetParameterIndex(BIS_ELEMENT_PROP_CodeValue))))
        return DgnDbStatus::BadArg;
    if (!m_code.IsEmpty() && (ECSqlStatus::Success != statement.BindText(statement.GetParameterIndex(BIS_ELEMENT_PROP_CodeValue), m_code.GetValue().c_str(), IECSqlBinder::MakeCopy::No)))
        {
        BeAssert(false && "INSERT or UPDATE statement must include CodeValue property");
        return DgnDbStatus::BadArg;
        }

    if ((ECSqlStatus::Success != statement.BindId(statement.GetParameterIndex(BIS_ELEMENT_PROP_CodeAuthorityId), m_code.GetAuthority())) ||
        (ECSqlStatus::Success != statement.BindText(statement.GetParameterIndex(BIS_ELEMENT_PROP_CodeNamespace), m_code.GetNamespace().c_str(), IECSqlBinder::MakeCopy::No)))
        {
        BeAssert(false && "INSERT or UPDATE statement must include CodeAuthority and CodeNamespace properties");
        return DgnDbStatus::BadArg;
        }

    if (HasUserLabel())
        statement.BindText(statement.GetParameterIndex(BIS_ELEMENT_PROP_UserLabel), GetUserLabel(), IECSqlBinder::MakeCopy::No);
    else
        statement.BindNull(statement.GetParameterIndex(BIS_ELEMENT_PROP_UserLabel));

    if (ECSqlStatus::Success != statement.BindId(statement.GetParameterIndex(BIS_ELEMENT_PROP_ParentId), m_parentId))
        {
        BeAssert(false && "INSERT or UPDATE statement must include ParentId property");
        return DgnDbStatus::BadArg;
        }

    ECSqlStatus bindStatus;
    if (m_federationGuid.IsValid())
        bindStatus = statement.BindBinary(statement.GetParameterIndex(BIS_ELEMENT_PROP_FederationGuid), &m_federationGuid, sizeof(m_federationGuid), IECSqlBinder::MakeCopy::No);
    else
        bindStatus = statement.BindNull(statement.GetParameterIndex(BIS_ELEMENT_PROP_FederationGuid));

    if (ECSqlStatus::Success != bindStatus)
        {
        BeAssert(false && "INSERT or UPDATE statement must include FederationGuid property");
        return DgnDbStatus::BadArg;
        }

    if (!isForUpdate)
        {
        if (ECSqlStatus::Success != statement.BindId(statement.GetParameterIndex(BIS_ELEMENT_PROP_ECInstanceId), m_elementId) ||
            ECSqlStatus::Success != statement.BindId(statement.GetParameterIndex(BIS_ELEMENT_PROP_ModelId), m_modelId))
            {
            BeAssert(false && "INSERT statement must include ECInstanceId and ModelId properties");
            return DgnDbStatus::BadArg;
            }
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_BindInsertParams(ECSqlStatement& statement)
    {
    return BindParams(statement, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus DgnElement::_InsertInDb()
    {
    CachedECSqlStatementPtr statement = GetDgnDb().Elements().GetPreparedInsertStatement(*this);
    if (statement.IsNull())
        return DgnDbStatus::WriteError;

    auto status = _BindInsertParams(*statement);
    if (DgnDbStatus::Success != status)
        return status;
 
    auto stmtResult = statement->Step();
    if (BE_SQLITE_DONE != stmtResult)
        {
        // SQLite doesn't tell us which constraint failed - check if it's the Code. (NOTE: We should catch this in _OnInsert())
        auto existingElemWithCode = GetDgnDb().Elements().QueryElementIdByCode(m_code);
        return existingElemWithCode.IsValid() ? DgnDbStatus::DuplicateCode : DgnDbStatus::WriteError;
        }

    if (PropState::Dirty == m_flags.m_propState)
        {
        ElementInstanceAdapter ec(*this);
        status = ec.UpdateProperties();
        if (DgnDbStatus::Success != status)
            {
            BeAssert(false && "Auto-handled properties update failed - see log for sql constraint errors, etc.");
            return status;
            }
        }

    if (m_userProperties)
        status = SaveUserProperties();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   04/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_BindUpdateParams(ECSqlStatement& statement)
    {
    return BindParams(statement, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2015
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbStatus DgnElement::_UpdateInDb()
    {
    CachedECSqlStatementPtr stmt = GetDgnDb().Elements().GetPreparedUpdateStatement(*this);
    if (stmt.IsNull())
        return DgnDbStatus::WriteError;

    DgnDbStatus status = _BindUpdateParams(*stmt);
    if (DgnDbStatus::Success != status)
        return status;

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
        ElementInstanceAdapter ec(*this);
        status = ec.UpdateProperties();
        if (DgnDbStatus::Success != status)
            {
            BeAssert(false && "Auto-handled properties update failed - see log for sql constraint errors, etc.");
            return status;
            }
        }

    if (m_userProperties)
        status = SaveUserProperties();

    return status;
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
    return nullptr == source ? nullptr : source->ToGeometrySource2d();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Brien.Bastings                  11/15
+---------------+---------------+---------------+---------------+---------------+------*/
GeometrySource3dCP DgnElement::ToGeometrySource3d() const
    {
    GeometrySourceCP source = _ToGeometrySource();
    return nullptr == source ? nullptr : source->ToGeometrySource3d();
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
    GeometrySource3dCP source3d = _ToGeometrySource3d();
    return nullptr != source3d ? source3d->GetPlacement().GetTransform() : _ToGeometrySource2d()->GetPlacement().GetTransform();
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

    ElementInstanceAdapter ecThis(*this);
    ElementInstanceAdapter ecOther(other);
    if (ecThis.IsValid() && ecOther.IsValid())
        ecThis.CopyDataBuffer(ecOther, true);

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
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnElement::_EqualProperty(ECN::ECPropertyCR prop, DgnElementCR other, bset<Utf8String> const& ignore, PropertyArrayIndex const& arrayIdx) const
    {
    auto const& propName = prop.GetName();

    if (propName.Equals("UserProperties")) // _GetPropertyValue does not work for user props
        {
        if (nullptr == m_userProperties)
            LoadUserProperties();
        if (nullptr == other.m_userProperties)
            other.LoadUserProperties();
        return m_userProperties->ToString().Equals(other.m_userProperties->ToString());
        }

    ECN::ECValue value, othervalue;
    if (DgnDbStatus::Success != _GetPropertyValue(value, propName.c_str(), arrayIdx)
     || DgnDbStatus::Success != other._GetPropertyValue(othervalue, propName.c_str(), arrayIdx))
        return false;
    return value.Equals(othervalue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
bset<Utf8String> const& DgnElement::GetStandardPropertyIgnoreList()
    {
    static std::once_flag s_ignoreListOnceFlag;
    static bset<Utf8String>* s_ignoreList;
    std::call_once(s_ignoreListOnceFlag, []()
        {
        s_ignoreList = new bset<Utf8String>();
        s_ignoreList->insert("LastMod");
        });
    return *s_ignoreList;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnElement::_Equals(DgnElementCR other, bset<Utf8String> const& ignore) const
    {
    if (&other == this)
        return true;

    auto ecclass = GetElementClass();
    if (ecclass != other.GetElementClass())
        return false;

    // Note that ECInstanceId is not a normal property and will not be returned by the property collection below
    if (ignore.find("ECInstanceId") == ignore.end() && ignore.find("Id") == ignore.end())
        {
        if (GetElementId() != other.GetElementId())
            return false;
        }

    for (auto prop : ecclass->GetProperties())
        {
        auto const& propName = prop->GetName();
        if (ignore.find(propName) != ignore.end())
            continue;

        // *** WIP_AUTO_HANDLED_PROPERTIES Structs, Arrays
        if (!_EqualProperty(*prop, other, ignore, PropertyArrayIndex()))
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::_Dump(Utf8StringR str, bset<Utf8String> const& ignore) const
    {
    auto ecclass = GetElementClass();
    str.append(ecclass->GetName().c_str());
    str.append(Utf8PrintfString(" %lld {", GetElementId().GetValueUnchecked()).c_str());
    Utf8CP comma = "";
    for (auto prop : ecclass->GetProperties())
        {
        auto const& propName = prop->GetName();
        if (ignore.find(propName.c_str()) != ignore.end())
            continue;
        ECN::ECValue value;
        // *** WIP_AUTO_HANDLED_PROPERTIES Structs, Arrays
        if (DgnDbStatus::Success == _GetPropertyValue(value, propName.c_str(), PropertyArrayIndex()))
            {
            str.append(propName.c_str());
            str.append("=");
            str.append(value.ToString().c_str());
            str.append(comma);
            comma = ", ";
            }
        }
    str.append("}\n");
    }


void DgnElement::Dump(Utf8StringR str, bset<Utf8String> const& ignore) const {_Dump(str,ignore);}

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

    CachedECSqlStatementPtr stmt = GetDgnDb().GetPreparedECSqlStatement("UPDATE " BIS_SCHEMA(BIS_CLASS_Element) " SET UserProperties=? WHERE ECInstanceId=?");
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

//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            01/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct InstanceUpdater : DgnElement::AppData
    {
    private:
        static Key s_key;
        DgnImportContext& m_importer;
        DgnElementId m_sourceElementId;
        DropMe _OnInserted(DgnElementCR el) override;
        void Update(DgnElementCR el);

    public:
        explicit InstanceUpdater(DgnImportContext& importer, DgnElementId sourceId) : m_importer(importer), m_sourceElementId(sourceId) {}
        static Key& GetKey() { return s_key; }
    };

//static
InstanceUpdater::Key InstanceUpdater::s_key;

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2016
//---------------+---------------+---------------+---------------+---------------+-------
InstanceUpdater::DropMe InstanceUpdater::_OnInserted(DgnElementCR el)
    {
    if (m_sourceElementId.IsValid())
        Update(el);

    return DropMe::Yes;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2016
//---------------+---------------+---------------+---------------+---------------+-------
BeSQLite::EC::ECInstanceUpdater const& DgnImportContext::GetUpdater(ECN::ECClassCR ecClass) const
    {
    auto it = m_updaterCache.find(&ecClass);
    if (it != m_updaterCache.end())
        return *it->second;

    bvector<ECN::ECPropertyCP> propertiesToBind;
    for (ECN::ECPropertyCP ecProperty : ecClass.GetProperties(true))
        {
        // Don't bind any of the dgn derived properties
        if (ecProperty->GetClass().GetSchema().GetName().Equals(BIS_ECSCHEMA_NAME))
            continue;
        propertiesToBind.push_back(ecProperty);
        }

    auto updater = new BeSQLite::EC::ECInstanceUpdater(GetDestinationDb(), ecClass, propertiesToBind);

    //just log error, we will still return the invalid inserter, and the caller will check for validity again
    if (!updater->IsValid())
        {
        Utf8String error;
        error.Sprintf("Could not create ECInstanceUpdater for ECClass '%s'. No instances of that class will be updated in the DgnDb file. Please see ECDb entries in log file for details.",
                        Utf8String(ecClass.GetFullName()).c_str());
        //LOG(error.c_str());
        }

    m_updaterCache[&ecClass] = updater;
    return *updater;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2016
//---------------+---------------+---------------+---------------+---------------+-------
void InstanceUpdater::Update(DgnElementCR el)
    {
    ECN::ECClassCP targetClass = el.GetElementClass();
    ECInstanceUpdater const& updater = m_importer.GetUpdater(*targetClass);
    if (!updater.IsValid())
        return;

    Utf8PrintfString ecSql("SELECT * FROM [%s].[%s] WHERE ECInstanceId = ?", Utf8String(targetClass->GetSchema().GetName()).c_str(), Utf8String(targetClass->GetName()).c_str());
    CachedECSqlStatementPtr ecStatement = m_importer.GetSourceDb().GetPreparedECSqlStatement(ecSql.c_str());

    if (!ecStatement.IsValid())
        {
        // log error?
        return;
        }

    ecStatement->BindId(1, m_sourceElementId);
    ECInstanceECSqlSelectAdapter adapter(*ecStatement);
    while (BE_SQLITE_ROW == ecStatement->Step())
        {
        IECInstancePtr instance = adapter.GetInstance();
        BeAssert(instance.IsValid());
        Utf8Char idStrBuffer[BeInt64Id::ID_STRINGBUFFER_LENGTH];
        el.GetElementId().ToString(idStrBuffer);
        ECN::StandaloneECInstancePtr targetInstance = targetClass->GetDefaultStandaloneEnabler()->CreateInstance();
        targetInstance->SetInstanceId(idStrBuffer);
        targetInstance->CopyValues(*instance.get());
        updater.Update(*targetInstance.get());
        }
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

    InstanceUpdater* instanceUpdater = new InstanceUpdater(importer, this->GetElementId());
    cc->AddAppData(InstanceUpdater::GetKey(), instanceUpdater);
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
    m_facetCount = src._GetFacetCount();
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
    CachedECSqlStatementPtr statement = group.GetDgnDb().GetPreparedECSqlStatement(
        "INSERT INTO " BIS_SCHEMA(BIS_REL_ElementGroupsMembers) 
        " (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId,MemberPriority) VALUES(?,?,?,?,?)");

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
    CachedECSqlStatementPtr statement = group.GetDgnDb().GetPreparedECSqlStatement(
        "DELETE FROM " BIS_SCHEMA(BIS_REL_ElementGroupsMembers) " WHERE SourceECInstanceId=? AND TargetECInstanceId=?");

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
    DgnDbStatus status = _InsertInstance(el);
    if (DgnDbStatus::Success != status)
        return status;
    return _UpdateProperties(el); 
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
        _DeleteInstance(modified);
        }
    else
        {
        DgnDbR db = modified.GetDgnDb();
        ECInstanceKey existing = _QueryExistingInstanceKey(modified);
        if (existing.IsValid() && (existing.GetECClassId() != GetECClassId(db)))
            {
            _DeleteInstance(modified);
            existing = ECInstanceKey();  //  trigger an insert below
            }
            
        if (!existing.IsValid())
            InsertThis(modified);
        else
            _UpdateProperties(modified);
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
DgnDbStatus DgnElement::MultiAspect::_DeleteInstance(DgnElementCR el)
    {
    // I am assuming that the ElementOwnsAspects ECRelationship is either just a foreign key column on the aspect or that ECSql somehow deletes the relationship instance automatically.
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("DELETE FROM %s WHERE ECInstanceId=?", GetFullEcSqlClassName().c_str()).c_str());
    stmt->BindId(1, m_instanceId);
    BeSQLite::DbResult status = stmt->Step();
    return (BeSQLite::BE_SQLITE_DONE == status) ? DgnDbStatus::Success : DgnDbStatus::WriteError;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      06/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::MultiAspect::_InsertInstance(DgnElementCR el)
    {
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("INSERT INTO %s ([ElementId]) VALUES (?)", GetFullEcSqlClassName().c_str()).c_str());
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

    dgn_AspectHandler::Aspect* handler = dgn_AspectHandler::Aspect::FindHandler(el.GetDgnDb(), classid);
    if (nullptr == handler)
        return nullptr;

    RefCountedPtr<DgnElement::UniqueAspect> aspect = dynamic_cast<DgnElement::UniqueAspect*>(handler->_CreateInstance().get());
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
DgnDbStatus DgnElement::UniqueAspect::_InsertInstance(DgnElementCR el)
    {
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("INSERT INTO %s (ElementId) VALUES(?)", GetFullEcSqlClassName().c_str()).c_str());
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
DgnDbStatus DgnElement::UniqueAspect::_DeleteInstance(DgnElementCR el)
    {
    // I am assuming that the ElementOwnsAspects ECRelationship is either just a foreign key column on the aspect or that ECSql somehow deletes the relationship instance automatically.
    CachedECSqlStatementPtr stmt = el.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("DELETE FROM %s WHERE [ElementId]=?", GetFullEcSqlClassName().c_str()).c_str());
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
    stmt->BindId(1, el.GetElementId());
    if (BE_SQLITE_ROW != stmt->Step())
        return ECInstanceKey();

    // And we know the ID. See if such an instance actually exists.
    return ECInstanceKey(classId, stmt->GetValueId<ECInstanceId>(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_GetPropertyValue(ECN::ECValueR value, Utf8CP name, PropertyArrayIndex const& arrayIdx) const
    {
    if (PropState::NotFound != m_flags.m_propState) // Unless we know that this class does NOT have auto-handled properties
        {
        // Common case: check auto-handled properties first
        ElementECPropertyAccessor ecPropAccess(*this, name);
        if (ecPropAccess.IsValid())
            return ecPropAccess.GetPropertyValue(value, arrayIdx);
        }

    // Rare: custom-handled properties
    if (0 == strcmp(BIS_ELEMENT_PROP_FederationGuid, name))
        {
        value.SetBinary((Byte*)&m_federationGuid, sizeof(m_federationGuid));
        return DgnDbStatus::Success;
        }
    if (0 == strcmp(BIS_ELEMENT_PROP_CodeValue, name))
        {
        value.SetUtf8CP(GetCode().GetValue().c_str());
        return DgnDbStatus::Success;
        }
    if (0 == strcmp(BIS_ELEMENT_PROP_CodeNamespace, name))
        {
        value.SetUtf8CP(GetCode().GetNamespace().c_str());
        return DgnDbStatus::Success;
        }
    if (0 == strcmp(BIS_ELEMENT_PROP_CodeAuthorityId, name))
        {
        value.SetLong(GetCode().GetAuthority().GetValueUnchecked());
        return DgnDbStatus::Success;
        }
    if (0 == strcmp("Id", name) || 0 == strcmp(BIS_ELEMENT_PROP_ECInstanceId, name))
        {
        value.SetLong(GetElementId().GetValueUnchecked());
        return DgnDbStatus::Success;
        }
    if (0 == strcmp(BIS_ELEMENT_PROP_ModelId, name))
        {
        value.SetLong(GetModelId().GetValueUnchecked());
        return DgnDbStatus::Success;
        }
    if (0 == strcmp(BIS_ELEMENT_PROP_ParentId, name))
        {
        value.SetLong(GetParentId().GetValueUnchecked());
        return DgnDbStatus::Success;
        }
    if (0 == strcmp(BIS_ELEMENT_PROP_UserLabel, name))
        {
        value.SetUtf8CP(GetUserLabel());
        return DgnDbStatus::Success;
        }
    if (0 == strcmp(BIS_ELEMENT_PROP_LastMode, name))
        {
        value.SetDateTime(QueryTimeStamp());
        return DgnDbStatus::Success;
        }

    return DgnDbStatus::NotFound;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_SetPropertyValue(Utf8CP name, ECN::ECValueCR value, PropertyArrayIndex const& arrayIdx)
    {
    if (PropState::NotFound != m_flags.m_propState) // Unless we know that this class does NOT have auto-handled properties
        {
        // Common case: check auto-handled properties first
        ElementECPropertyAccessor ecPropAccess(*this, name);
        if (ecPropAccess.IsValid())
            return ecPropAccess.SetPropertyValue(value, arrayIdx);
        }

    if (0 == strcmp(BIS_ELEMENT_PROP_CodeValue, name)
     || 0 == strcmp(BIS_ELEMENT_PROP_CodeNamespace, name)
     || 0 == strcmp(BIS_ELEMENT_PROP_CodeAuthorityId, name))
        {
        // *** NEEDS WORK: I don't think we should change the parts of a code individually.
        return DgnDbStatus::ReadOnly;
        }
    if (0 == strcmp("Id", name) || 0 == strcmp(BIS_ELEMENT_PROP_ECInstanceId, name))
        {
        return DgnDbStatus::ReadOnly;
        }
    if (0 == strcmp(BIS_ELEMENT_PROP_ModelId, name))
        {
        return DgnDbStatus::ReadOnly;
        }
    if (0 == strcmp(BIS_ELEMENT_PROP_ParentId, name))
        {
        return SetParentId(DgnElementId((uint64_t)value.GetLong()));
        }
    if (0 == strcmp(BIS_ELEMENT_PROP_UserLabel, name))
        {
        SetUserLabel(value.ToString().c_str());
        return DgnDbStatus::Success;
        }
    if (0 == strcmp(BIS_ELEMENT_PROP_LastMode, name))
        {
        return DgnDbStatus::ReadOnly;
        }

    return DgnDbStatus::NotFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement::GetGeometricElementPropertyValue(ECN::ECValueR value, Utf8CP name) const
    {
    if (0 == strcmp(name, "GeometryStream"))
        {
        return DgnDbStatus::BadRequest;//  => Use GeometryCollection interface
        }
    if (0 == strcmp(name, GEOM_FacetCount))
        {
        value.SetLong(m_facetCount);
        return DgnDbStatus::Success;
        }
    return DgnDbStatus::NotFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement3d::_GetPropertyValue(ECN::ECValueR value, Utf8CP name, PropertyArrayIndex const& arrayIdx) const
    {
    if (0 == strcmp(name, "CategoryId"))
        {
        value.SetLong(GetCategoryId().GetValueUnchecked());
        return DgnDbStatus::Success;
        }

    if (0 == strcmp(name, "InSpatialIndex"))
        {
        value.SetBoolean(!GetModel()->IsTemplate());
        return DgnDbStatus::Success;
        }
    
    DgnDbStatus status;
    if (DgnDbStatus::NotFound != (status = GetPlacementProperty(value, name)))
        return status;

    if (DgnDbStatus::NotFound != (status = GetGeometricElementPropertyValue(value, name)))
        return status;

    return T_Super::_GetPropertyValue(value, name, arrayIdx);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement::SetGeometricElementPropertyValue(Utf8CP name, ECN::ECValueCR value)
    {
    if (0 == strcmp(name, "GeometryStream"))
        return DgnDbStatus::BadRequest;//  => Use ElementGeometryBuilder

    return DgnDbStatus::NotFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement3d::_SetPropertyValue(Utf8CP name, ECN::ECValueCR value, PropertyArrayIndex const& arrayIdx)
    {
    if (0 == strcmp(name, GEOM_FacetCount))
        {
        _RecordFacetCount((size_t)value.GetLong());
        return DgnDbStatus::Success;
        }
    
    if (0 == strcmp(name, "CategoryId"))
        return SetCategoryId(DgnCategoryId((uint64_t)value.GetLong()));

    if (0 == strcmp(name, "InSpatialIndex"))
        return DgnDbStatus::ReadOnly;
    
    DgnDbStatus status;
    if (DgnDbStatus::NotFound != (status = SetPlacementProperty(name, value)))
        return status;

    if (DgnDbStatus::NotFound != (status = SetGeometricElementPropertyValue(name, value)))
        return status;

    return T_Super::_SetPropertyValue(name, value, arrayIdx);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement3d::GetPlacementProperty(ECN::ECValueR value, Utf8CP name) const
    {
    bool isplcprop;
    if ((isplcprop = (0 == strcmp(name, "Origin"))))
        value.SetPoint3d(GetPlacement().GetOrigin());
    else if ((isplcprop = (0 == strcmp(name, "Yaw"))))
        value.SetDouble(GetPlacement().GetAngles().GetYaw().Degrees());
    else if ((isplcprop = (0 == strcmp(name, "Pitch"))))
        value.SetDouble(GetPlacement().GetAngles().GetPitch().Degrees());
    else if ((isplcprop = (0 == strcmp(name, "Roll"))))
        value.SetDouble(GetPlacement().GetAngles().GetRoll().Degrees());
    else if ((isplcprop = (0 == strcmp(name, "BBoxLow"))))
        value.SetPoint3d(GetPlacement().GetElementBox().low);
    else if ((isplcprop = (0 == strcmp(name, "BBoxHigh"))))
        value.SetPoint3d(GetPlacement().GetElementBox().high);
        
    return isplcprop? DgnDbStatus::Success: DgnDbStatus::NotFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement3d::SetPlacementProperty(Utf8CP name, ECN::ECValueCR value)
    {
    Placement3d plc;
    bool isplcprop;

    if ((isplcprop = (0 == strcmp(name, "Origin"))))
        (plc = GetPlacement()).GetOriginR() = value.GetPoint3d();
    else if ((isplcprop = (0 == strcmp(name, "Yaw"))))
        (plc = GetPlacement()).GetAnglesR().SetYaw(AngleInDegrees::FromRadians(value.GetDouble()));
    else if ((isplcprop = (0 == strcmp(name, "Pitch"))))
        (plc = GetPlacement()).GetAnglesR().SetPitch(AngleInDegrees::FromRadians(value.GetDouble()));
    else if ((isplcprop = (0 == strcmp(name, "Roll"))))
        (plc = GetPlacement()).GetAnglesR().SetRoll(AngleInDegrees::FromRadians(value.GetDouble()));
    else if ((isplcprop = (0 == strcmp(name, "BBoxLow"))))
        (plc = GetPlacement()).GetElementBoxR().low = value.GetPoint3d();
    else if ((isplcprop = (0 == strcmp(name, "BBoxHigh"))))
        (plc = GetPlacement()).GetElementBoxR().high = value.GetPoint3d();
        
   if (!isplcprop)
       return DgnDbStatus::NotFound;
   
   SetPlacement(plc);
   return DgnDbStatus::Success;
   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DateTime DgnElement::GetPropertyValueDateTime(Utf8CP propertyName, PropertyArrayIndex const& arrayIdx) const
    {
    ECN::ECValue value;
    DgnDbStatus status = GetPropertyValue(value, propertyName, arrayIdx);
    BeAssert(DgnDbStatus::Success == status);
    UNUSED_VARIABLE(status);
    return value.IsNull() ? DateTime() : value.GetDateTime();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d DgnElement::GetPropertyValueDPoint3d(Utf8CP propertyName, PropertyArrayIndex const& arrayIdx) const
    {
    ECN::ECValue value;
    DgnDbStatus status = GetPropertyValue(value, propertyName, arrayIdx);
    BeAssert(DgnDbStatus::Success == status);
    UNUSED_VARIABLE(status);
    return value.IsNull() ? DPoint3d::From(0,0,0) : value.GetPoint3d();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d DgnElement::GetPropertyValueDPoint2d(Utf8CP propertyName, PropertyArrayIndex const& arrayIdx) const
    {
    ECN::ECValue value;
    DgnDbStatus status = GetPropertyValue(value, propertyName, arrayIdx);
    BeAssert(DgnDbStatus::Success == status);
    UNUSED_VARIABLE(status);
    return value.IsNull() ? DPoint2d::From(0,0) : value.GetPoint2d();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnElement::GetPropertyValueBoolean(Utf8CP propertyName, PropertyArrayIndex const& arrayIdx) const
    {
    ECN::ECValue value;
    DgnDbStatus status = GetPropertyValue(value, propertyName, arrayIdx);
    BeAssert(DgnDbStatus::Success == status);
    UNUSED_VARIABLE(status);
    return value.IsNull() ? false : value.GetBoolean();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
double DgnElement::GetPropertyValueDouble(Utf8CP propertyName, PropertyArrayIndex const& arrayIdx) const
    {
    ECN::ECValue value;
    DgnDbStatus status = GetPropertyValue(value, propertyName, arrayIdx);
    BeAssert(DgnDbStatus::Success == status);
    UNUSED_VARIABLE(status);
    return value.IsNull() ? 0.0 : value.GetDouble();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t DgnElement::GetPropertyValueInt32(Utf8CP propertyName, PropertyArrayIndex const& arrayIdx) const
    {
    ECN::ECValue value;
    DgnDbStatus status = GetPropertyValue(value, propertyName, arrayIdx);
    BeAssert(DgnDbStatus::Success == status);
    UNUSED_VARIABLE(status);
    return value.IsNull() ? 0 : value.GetInteger();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t DgnElement::GetPropertyValueUInt64(Utf8CP propertyName, PropertyArrayIndex const& arrayIdx) const
    {
    ECN::ECValue value;
    DgnDbStatus status = GetPropertyValue(value, propertyName, arrayIdx);
    BeAssert(DgnDbStatus::Success == status);
    UNUSED_VARIABLE(status);
    return value.IsNull() ? 0 : static_cast<uint64_t>(value.GetLong());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnElement::GetPropertyValueString(Utf8CP propertyName, PropertyArrayIndex const& arrayIdx) const
    {
    ECN::ECValue value;
    DgnDbStatus status = GetPropertyValue(value, propertyName, arrayIdx);
    BeAssert(DgnDbStatus::Success == status);
    UNUSED_VARIABLE(status);
    return value.GetUtf8CP();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::SetPropertyValue(Utf8CP propertyName, DPoint3dCR pt, PropertyArrayIndex const& arrayIdx)
    {
    DgnDbStatus status = SetPropertyValue(propertyName, ECValue(pt), arrayIdx);
    BeAssert(DgnDbStatus::Success == status);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::SetPropertyValue(Utf8CP propertyName, DPoint2dCR pt, PropertyArrayIndex const& arrayIdx)
    {
    DgnDbStatus status = SetPropertyValue(propertyName, ECValue(pt), arrayIdx);
    BeAssert(DgnDbStatus::Success == status);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::SetPropertyValue(Utf8CP propertyName, bool value, PropertyArrayIndex const& arrayIdx)
    {
    DgnDbStatus status = SetPropertyValue(propertyName, ECValue(value), arrayIdx);
    BeAssert(DgnDbStatus::Success == status);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::SetPropertyValue(Utf8CP propertyName, double value, PropertyArrayIndex const& arrayIdx)
    {
    DgnDbStatus status = SetPropertyValue(propertyName, ECValue(value), arrayIdx);
    BeAssert(DgnDbStatus::Success == status);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::SetPropertyValue(Utf8CP propertyName, int32_t value, PropertyArrayIndex const& arrayIdx)
    {
    DgnDbStatus status = SetPropertyValue(propertyName, ECValue(value), arrayIdx);
    BeAssert(DgnDbStatus::Success == status);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::SetPropertyValue(Utf8CP propertyName, BeInt64Id id, PropertyArrayIndex const& arrayIdx)
    {
    ECValue value(id.GetValueUnchecked());
    if (!id.IsValid())
        value.SetToNull();

    DgnDbStatus status = SetPropertyValue(propertyName, value, arrayIdx);
    BeAssert(DgnDbStatus::Success == status);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::SetPropertyValue(Utf8CP propertyName, Utf8CP value, PropertyArrayIndex const& arrayIdx)
    {
    DgnDbStatus status = SetPropertyValue(propertyName, ECValue(value, false), arrayIdx);
    BeAssert(DgnDbStatus::Success == status);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
YawPitchRollAngles DgnElement::GetPropertyValueYpr(Utf8CP yawName, Utf8CP pitchName, Utf8CP rollName) const
    {
    YawPitchRollAngles angles;
    angles.SetYaw  (AngleInDegrees::FromDegrees(GetPropertyValueDouble(yawName)));
    angles.SetPitch(AngleInDegrees::FromDegrees(GetPropertyValueDouble(pitchName)));
    angles.SetRoll (AngleInDegrees::FromDegrees(GetPropertyValueDouble(rollName)));
    return angles;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::SetPropertyValueYpr(YawPitchRollAnglesCR angles, Utf8CP yawName, Utf8CP pitchName, Utf8CP rollName)
    {
    DgnDbStatus status;
    if (DgnDbStatus::Success != (status = SetPropertyValue(yawName, angles.GetYaw().Degrees())))
        return status;
    if (DgnDbStatus::Success != (status = SetPropertyValue(pitchName, angles.GetPitch().Degrees())))
        return status;
    if (DgnDbStatus::Success != (status = SetPropertyValue(rollName, angles.GetRoll().Degrees())))
        return status;
    return DgnDbStatus::Success;
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
    CachedECSqlStatementPtr statement = element.GetDgnDb().GetPreparedECSqlStatement("INSERT INTO " BIS_SCHEMA(BIS_CLASS_ElementExternalKey) " ([ElementId],[AuthorityId],[ExternalKey]) VALUES (?,?,?)");
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
    CachedECSqlStatementPtr statement = element.GetDgnDb().GetPreparedECSqlStatement("SELECT [ExternalKey] FROM " BIS_SCHEMA(BIS_CLASS_ElementExternalKey) " WHERE [ElementId]=? AND [AuthorityId]=?");
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
    CachedECSqlStatementPtr statement = element.GetDgnDb().GetPreparedECSqlStatement("DELETE FROM " BIS_SCHEMA(BIS_CLASS_ElementExternalKey) " WHERE [ElementId]=? AND [AuthorityId]=?");
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
    struct Pair { Utf8CP name; uint64_t action; };

    static const Pair s_pairs[] = 
        {
            { "clone",          Clone },
            { "setparent",      SetParent },
            { "insertchild",    InsertChild },
            { "updatechild",    UpdateChild },
            { "deletechild",    DeleteChild },
            { "setcode",        SetCode },
            { "move",           Move },
            { "setcategory",    SetCategory },
            { "setgeometry",    SetGeometry },
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
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_SetCode(DgnCode const& code)
    {
    if (GetElementHandler()._IsRestrictedAction(RestrictedAction::SetCode))
        return DgnDbStatus::MissingHandler;

    m_code = code;
    return DgnDbStatus::Success;
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
* @bsimethod                                    Sam.Wilson                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement2d::GetPlacementProperty(ECN::ECValueR value, Utf8CP name) const
    {
    bool isplcprop;
    if ((isplcprop = (0 == strcmp(name, "Origin"))))
        value.SetPoint2d(GetPlacement().GetOrigin());
    else if ((isplcprop = (0 == strcmp(name, "Rotation"))))
        value.SetDouble(GetPlacement().GetAngle().Degrees());
    else if ((isplcprop = (0 == strcmp(name, "BBoxLow"))))
        value.SetPoint2d(GetPlacement().GetElementBox().low);
    else if ((isplcprop = (0 == strcmp(name, "BBoxHigh"))))
        value.SetPoint2d(GetPlacement().GetElementBox().high);
        
    return isplcprop? DgnDbStatus::Success: DgnDbStatus::NotFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement2d::SetPlacementProperty(Utf8CP name, ECN::ECValueCR value)
    {
    Placement2d plc;
    bool isplcprop;

    if ((isplcprop = (0 == strcmp(name, "Origin"))))
        (plc = GetPlacement()).GetOriginR() = value.GetPoint2d();
    else if ((isplcprop = (0 == strcmp(name, "Rotation"))))
        (plc = GetPlacement()).GetAngleR() = AngleInDegrees::FromRadians(value.GetDouble());
    else if ((isplcprop = (0 == strcmp(name, "BBoxLow"))))
        (plc = GetPlacement()).GetElementBoxR().low = value.GetPoint2d();
    else if ((isplcprop = (0 == strcmp(name, "BBoxHigh"))))
        (plc = GetPlacement()).GetElementBoxR().high = value.GetPoint2d();
        
   if (!isplcprop)
       return DgnDbStatus::NotFound;
   
   SetPlacement(plc);
   return DgnDbStatus::Success;
   }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement2d::_GetPropertyValue(ECN::ECValueR value, Utf8CP name, PropertyArrayIndex const& arrayIdx) const
    {
    if (0 == strcmp(name, "CategoryId"))
        {
        value.SetLong(GetCategoryId().GetValueUnchecked());
        return DgnDbStatus::Success;
        }

    DgnDbStatus status;
    if (DgnDbStatus::NotFound != (status = GetPlacementProperty(value, name)))
        return status;

    if (DgnDbStatus::NotFound != (status = GetGeometricElementPropertyValue(value, name)))
        return status;

    return T_Super::_GetPropertyValue(value, name, arrayIdx);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement2d::_SetPropertyValue(Utf8CP name, ECN::ECValueCR value, PropertyArrayIndex const& arrayIdx)
    {
    if (0 == strcmp(name, GEOM_FacetCount))
        {
        _RecordFacetCount((size_t)value.GetLong());
        return DgnDbStatus::Success;
        }

    if (0 == strcmp(name, "CategoryId"))
        return SetCategoryId(DgnCategoryId((uint64_t)value.GetLong()));

    DgnDbStatus status;
    if (DgnDbStatus::NotFound != (status = SetPlacementProperty(name, value)))
        return status;

    if (DgnDbStatus::NotFound != (status = SetGeometricElementPropertyValue(name, value)))
        return status;

    return T_Super::_SetPropertyValue(name, value, arrayIdx);
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

    if (el.Is3d())
        placementTrans = el.ToGeometrySource3d()->GetPlacement().GetTransform();
    else
        placementTrans = el.ToGeometrySource2d()->GetPlacement().GetTransform();

    DPoint3d    originPt;
    RotMatrix   rMatrix;

    Transform transform;
    transform.InitProduct(transformIn, placementTrans);
    transform.GetTranslation(originPt);
    transform.GetMatrix(rMatrix);
            
    YawPitchRollAngles  angles;

    if (!YawPitchRollAngles::TryFromRotMatrix(angles, rMatrix))
        return DgnDbStatus::BadArg;

    if (el.Is3d())
        {
        Placement3d placement = el.ToGeometrySource3d()->GetPlacement();

        placement.GetOriginR() = originPt;
        placement.GetAnglesR() = angles;

        return el.ToGeometrySource3dP()->SetPlacement(placement);
        }
        
    Placement2d placement = el.ToGeometrySource2d()->GetPlacement();

    placement.GetOriginR() = DPoint2d::From(originPt);
    placement.GetAngleR() = angles.GetYaw();

    return el.ToGeometrySource2dP()->SetPlacement(placement);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnEditElementCollector::DgnEditElementCollector() 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnEditElementCollector::~DgnEditElementCollector() 
    {
    EmptyAll();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnEditElementCollector::DgnEditElementCollector(DgnEditElementCollector const& rhs)
    {
    CopyFrom(rhs);
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
bool DgnEditElementCollector::Equals(DgnEditElementCollector const& other, bset<Utf8String> const& ignore) const
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
        if (!el->Equals(*otherel, ignore))
            return false;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnEditElementCollector::Dump(Utf8StringR str, bset<Utf8String> const& ignore) const 
    {
    str.append("{\n");
    for (auto el : *this)
        {
        str.append(Utf8PrintfString("%llx ", (uint64_t)(intptr_t)el).c_str());
        el->Dump(str, ignore);
        }
    str.append("}\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnEditElementCollector::DumpTwo(Utf8StringR str, DgnEditElementCollector const& other, bset<Utf8String> const& ignore) const
    {
    str.append("{\n");
    for (size_t i=0; i<m_elements.size(); ++i)
        {
        auto el = m_elements.at(i);
        auto otherel = other.m_elements.at(i);
        str.append(Utf8PrintfString("%llx ", (uint64_t)(intptr_t)el).c_str());
        el->Dump(str, ignore);
        str.append(Utf8PrintfString("%llx ", (uint64_t)(intptr_t)otherel).c_str());
        otherel->Dump(str, ignore);
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
* @bsimethod                                                    Sam.Wilson      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_ReadSelectParams(ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    // See ElementInstanceAdapter for where we read auto-handled properties
    return DgnDbStatus::Success;
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
    m_facetCount = stmt.GetValueUInt64(params.GetSelectIndex(GEOM_FacetCount));

    // Read GeomStream
    auto geomIndex = params.GetSelectIndex(GEOM_GeometryStream);
    if (stmt.IsValueNull(geomIndex))
        return DgnDbStatus::Success;    // no geometry...

    int blobSize;
    void const* blob = stmt.GetValueBinary(geomIndex, &blobSize);
    return m_geom.ReadGeometryStream(GetDgnDb().Elements().GetSnappyFrom(), GetDgnDb(), blob, blobSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement::_BindInsertParams(ECSqlStatement& stmt)
    {
    auto stat = T_Super::_BindInsertParams(stmt);
    return DgnDbStatus::Success == stat ? BindParams(stmt) : stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement::_BindUpdateParams(ECSqlStatement& stmt)
    {
    auto stat = T_Super::_BindUpdateParams(stmt);
    return DgnDbStatus::Success == stat ? BindParams(stmt) : stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement::BindParams(ECSqlStatement& stmt)
    {
    stmt.BindId(stmt.GetParameterIndex(GEOM_Category), m_categoryId);
    stmt.BindInt64(stmt.GetParameterIndex(GEOM_FacetCount), static_cast<int64_t>(m_facetCount));
    return m_geom.BindGeometryStream(m_multiChunkGeomStream, GetDgnDb().Elements().GetSnappyTo(), stmt, GEOM_GeometryStream);
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
bool GeometricElement::_EqualProperty(ECN::ECPropertyCR prop, DgnElementCR other, bset<Utf8String> const& ignore, PropertyArrayIndex const& aidx) const
    {
    if (!prop.GetName().Equals(GEOM_GeometryStream))
        {
        return T_Super::_EqualProperty(prop, other, ignore, aidx);
        }

    if (ignore.find(GEOM_GeometryStream) != ignore.end())
        return true;

    // We need custom logic to compare GeometryStreams because the base class implementation of _EqualProperty cannot get this property's value as an ECN::ECValue.
    // We DON'T need custom logic to compare any of the other common GeometricElement properties, such as FacetCount or CategoryId,
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
DgnDbStatus GeometricElement2d::BindParams(ECSqlStatement& stmt)
    {
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

    return DgnDbStatus::Success;
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
* @bsimethod                                                    Paul.Connelly   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement2d::_BindInsertParams(ECSqlStatement& stmt)
    {
    auto stat = T_Super::_BindInsertParams(stmt);
    return DgnDbStatus::Success == stat ? BindParams(stmt) : stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement2d::_BindUpdateParams(ECSqlStatement& stmt)
    {
    auto stat = T_Super::_BindUpdateParams(stmt);
    return DgnDbStatus::Success == stat ? BindParams(stmt) : stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement3d::BindParams(ECSqlStatement& stmt)
    {
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

    return DgnDbStatus::Success;
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
* @bsimethod                                                    Paul.Connelly   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement3d::_BindInsertParams(ECSqlStatement& stmt)
    {
    auto stat = T_Super::_BindInsertParams(stmt);
    return DgnDbStatus::Success == stat ? BindParams(stmt) : stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus GeometricElement3d::_BindUpdateParams(ECSqlStatement& stmt)
    {
    auto stat = T_Super::_BindUpdateParams(stmt);
    return DgnDbStatus::Success == stat ? BindParams(stmt) : stat;
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
        CachedECSqlStatementPtr statement = db.GetPreparedECSqlStatement("DELETE FROM " BIS_SCHEMA(BIS_REL_ElementUsesGeometryParts) " WHERE SourceECInstanceId=? AND TargetECInstanceId=?");
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
* @bsimethod                                                    Sam.Wilson      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr DgnElements::CreateElement(DgnDbStatus* inStat, ECN::IECInstanceCR properties)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(stat, inStat);

    DgnClassId classId(properties.GetClass().GetId().GetValue());
    auto handler = dgn_ElementHandler::Element::FindHandler(GetDgnDb(), classId);
    if (nullptr == handler)
        {
        BeAssert(false);
        stat = DgnDbStatus::MissingHandler;
        return nullptr;
        }

    return handler->_CreateNewElement(inStat, GetDgnDb(), properties);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::CreateParams DgnElement::InitCreateParamsFromECInstance(DgnDbStatus* inStat, DgnDbR db, ECN::IECInstanceCR properties)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(stat, inStat);

    DgnModelId mid;
        {
        ECN::ECValue v;
        if (ECN::ECObjectsStatus::Success != properties.GetValue(v, BIS_ELEMENT_PROP_ModelId) || v.IsNull())
            {
            stat = DgnDbStatus::BadArg;
            return CreateParams(db, DgnModelId(), DgnClassId());
            }
        mid = DgnModelId((uint64_t)v.GetLong());
        if (!mid.IsValid())
            {
            stat = DgnDbStatus::BadArg;
            return CreateParams(db, DgnModelId(), DgnClassId());
            }
        }

    DgnClassId classId(properties.GetClass().GetId().GetValue());

    DgnCode code;
        //! The authority ID must be non-null and identify a valid authority.
        //! The namespace may not be null, but may be a blank string.
        //! The value may be null if and only if the namespace is blank, signifying that the authority
        //! assigns no special meaning to the object's code.
        //! The value may not be an empty string.
        {
        ECN::ECValue v;
        if (ECN::ECObjectsStatus::Success != properties.GetValue(v, BIS_ELEMENT_PROP_CodeAuthorityId) || v.IsNull())
            {
            stat = DgnDbStatus::BadArg;
            return CreateParams(db, DgnModelId(), classId);
            }
        DgnAuthorityId id((uint64_t) v.GetLong());

        if (ECN::ECObjectsStatus::Success != properties.GetValue(v, BIS_ELEMENT_PROP_CodeNamespace) || v.IsNull())
            {
            stat = DgnDbStatus::BadArg;
            return CreateParams(db, DgnModelId(), classId);
            }
        Utf8String codeName(v.GetUtf8CP());

        if (ECN::ECObjectsStatus::Success != properties.GetValue(v, BIS_ELEMENT_PROP_CodeValue) || (v.IsNull() && !Utf8String::IsNullOrEmpty(codeName.c_str())) ||
            (!v.IsNull() && 0 == strlen(v.GetUtf8CP())))
            {
            stat = DgnDbStatus::BadArg;
            return CreateParams(db, DgnModelId(), classId);
            }

        code.From(id, v.GetUtf8CP(), codeName);
        }

    DgnElement::CreateParams params(db, mid, classId, code);

    auto ecinstanceid = properties.GetInstanceId();                 // Note that ECInstanceId is not a normal property and will not be returned by the property collection below
    if (!ecinstanceid.empty())
        {
        uint64_t idvalue;
        if (BSISUCCESS != BeStringUtilities::ParseUInt64(idvalue, ecinstanceid.c_str()))
            {
            stat = DgnDbStatus::BadArg;
            return CreateParams(db, DgnModelId(), DgnClassId());
            }
        params.SetElementId(DgnElementId(idvalue));
        }

    return params;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_SetPropertyValues(ECN::IECInstanceCR properties)
    {
#ifdef WIP_AUTO_HANDLED_PROPERTIES // *** ECValuesCollection does not return all properties!?
    ECValuesCollectionPtr propValues = ECValuesCollection::Create(properties);
    for (ECN::ECPropertyValue const& propValue : *propValues)
        {
        auto propName = propValue.GetValueAccessor().GetPropertyName();

        if (propName.Equals(BIS_ELEMENT_PROPNAME_ModelId))     // already processed ModelId property above
            continue;

        ECN::ECValueCR value = propValue.GetValue();
#else
    for (auto prop : GetElementClass()->GetProperties(true))
        {
        Utf8StringCR propName = prop->GetName();

        // Skip special properties that were passed in CreateParams. Generally, these are set once and then read-only properties.
        if (propName.Equals(BIS_ELEMENT_PROP_ModelId) || propName.Equals("Id") || propName.Equals(BIS_ELEMENT_PROP_ECInstanceId) ||
            propName.Equals(BIS_ELEMENT_PROP_CodeAuthorityId) || propName.Equals(BIS_ELEMENT_PROP_CodeNamespace) || propName.Equals(BIS_ELEMENT_PROP_CodeValue))
            continue;

        // Geometry related properties.  These can't be imported here.
        if (propName.Equals(GEOM3_InSpatialIndex))
            continue;

        ECN::ECValue value;
        if (ECN::ECObjectsStatus::Success != properties.GetValue(value, propName.c_str()))
            continue;
#endif

        if (!value.IsNull())
            {
            DgnDbStatus stat;
            // *** WIP_AUTO_HANDLED_PROPERTIES Structs, Arrays
            if (DgnDbStatus::Success != (stat = _SetPropertyValue(propName.c_str(), value, PropertyArrayIndex())))
                {
                if (DgnDbStatus::ReadOnly == stat) // Not sure what to do when caller wants to 
                    {
                    BeAssert(false && "Attempt to set read-only property value.");
                    }
                else
                    {
                    BeAssert(false && "Failed to set property value. _SetPropertyValue is probably missing a case.");
                    }
                return stat;
                }
            }
        }
    
    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnElement::CreateParams dgn_ElementHandler::Element::_InitCreateParams(DgnDbStatus* inStat, DgnDbR db, ECN::IECInstanceCR properties)
    {
    return DgnElement::InitCreateParamsFromECInstance(inStat, db, properties);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr dgn_ElementHandler::Element::_CreateNewElement(DgnDbStatus* inStat, DgnDbR db, ECN::IECInstanceCR properties)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(stat, inStat);
    auto params = _InitCreateParams(inStat, db, properties);
    if (!params.IsValid())
        return nullptr;
    auto ele = _CreateInstance(params);
    if (nullptr == ele)
        {
        BeAssert(false && "when would a handler fail to construct an element?");
        return nullptr;
        }
    stat = ele->_SetPropertyValues(properties);
    return (DgnDbStatus::Success == stat)? ele: nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ElementInstanceAdapter::ElementInstanceAdapter(DgnElement const& el) : m_element(const_cast<DgnElement&>(el))
    {
    m_eclass = m_element.GetElementClass();

    if (DgnElement::PropState::Unknown == m_element.m_flags.m_propState)
        LoadProperties();

    if (DgnElement::PropState::NotFound == m_element.m_flags.m_propState)
        {
        m_eclass = nullptr; // This object cannot be used to access properties on this element
        BeAssert(!IsValid());
        BeAssert(nullptr == m_element.m_ecPropertyData);
        return; // This element has no auto-handled properties
        }

    BeAssert(IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ElementInstanceAdapter::LoadProperties()
    {
    BeAssert(IsValid());
    BeAssert(DgnElement::PropState::Unknown == m_element.m_flags.m_propState);
    BeAssert(nullptr == m_element.m_ecPropertyData);
    
    // *** WIP_AUTO_HANDLED_PROPERTIES -- We should cache this select argument list, not build it every time.

    Utf8String props;
    Utf8CP comma = "";
    bvector<ECN::ECPropertyCP> autoHandledProperties;
    for (auto prop : AutoHandledPropertiesCollection(*m_eclass, m_element.GetDgnDb(), ECSqlClassParams::StatementType::Select, false))
        {
        Utf8StringCR propName = prop->GetName();
        props.append(comma).append("[").append(propName).append("]");
        comma = ",";
        }

    if (props.empty())
        {
        m_element.m_flags.m_propState = DgnElement::PropState::NotFound;
        return BSIERROR;
        }

    // *********************************************************************************************************
    // From this point on, we must return BSISUCCESS. 
    // The ECClass says it has auto-handled properties. They may be NULL or not yet defined, but they are there.
    // *********************************************************************************************************

    m_element.m_flags.m_propState = DgnElement::PropState::InBuffer;

    auto stmt = m_element.GetDgnDb().GetPreparedECSqlStatement(Utf8PrintfString("SELECT %s FROM %s WHERE ECInstanceId=?", props.c_str(), m_eclass->GetECSqlName().c_str()).c_str());

    auto const& classLayout = _GetClassLayout();
    m_element.m_ecPropertyDataSize = CalculateInitialAllocation(classLayout);
    m_element.m_ecPropertyData = (Byte*)bentleyAllocator_malloc(m_element.m_ecPropertyDataSize);
    InitializeMemory(classLayout, m_element.m_ecPropertyData, m_element.m_ecPropertyDataSize, true);

    stmt->BindId(1, m_element.GetElementId());
    if (BE_SQLITE_ROW != stmt->Step())
        {
        return BSISUCCESS; // element is not persistent => all props are null at this point
        }

    ECInstanceECSqlSelectAdapter adapter(*stmt);
    adapter.SetInstanceData(*this, true);

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementInstanceAdapter::IsValidValue(ECN::ECPropertyCR prop, ECN::ECValueCR value)
    {
    BeAssert(IsValid());
    if (value.IsNull())
        {
        ECN::ECDbPropertyMap propertyMap;
        // *** WIP_AUTO_HANDLED_PROPERTIES -- must somehow cache this kind of metadata
        if (ECN::ECDbMapCustomAttributeHelper::TryGetPropertyMap(propertyMap, prop))
            {
            bool isNullable;
            if (ECN::ECObjectsStatus::Success == propertyMap.TryGetIsNullable(isNullable) && !isNullable)
                return false;
            }
        }

    // *** TBD: do range validation
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementInstanceAdapter::IsValidForStatementType(ECN::ECPropertyCR prop, ECSqlClassParams::StatementType stypeNeeded)
    {
    BeAssert(IsValid());
    // *** WIP_AUTO_HANDLED_PROPERTIES -- must somehow cache this kind of metadata

    auto propertyStatementType = m_element.GetDgnDb().Schemas().GetECClass(BIS_ECSCHEMA_NAME, "AutoHandledProperty");
    auto stypeCA = prop.GetCustomAttribute(*propertyStatementType);
    if (!stypeCA.IsValid())
        return true;

    ECN::ECValue stypeValue;
    stypeCA->GetValue(stypeValue, "StatementTypes");

    return 0 != ((uint32_t)stypeNeeded & stypeValue.GetInteger());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::EC::ECInstanceUpdater* ElementInstanceAdapter::GetUpdater()
    {
    BeAssert(IsValid());

    DgnClassId eclassid(m_eclass->GetId().GetValue());

    auto& updaterCache = m_element.GetDgnDb().Elements().m_updaterCache;
    auto iupdater = updaterCache.find(eclassid);
    if (iupdater != updaterCache.end())
        return iupdater->second;

    bvector<ECN::ECPropertyCP> autoHandledProperties;
    for (auto prop : AutoHandledPropertiesCollection(*m_eclass, m_element.GetDgnDb(), ECSqlClassParams::StatementType::InsertUpdate, false))
        {
        autoHandledProperties.push_back(prop);
        }

    if (autoHandledProperties.empty())
        return updaterCache[eclassid] = nullptr;

    return updaterCache[eclassid] = new EC::ECInstanceUpdater(m_element.GetDgnDb(), *m_eclass, autoHandledProperties);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ElementInstanceAdapter::UpdateProperties()
    {
    BeAssert(IsValid());
    BeAssert(DgnElement::PropState::Dirty == m_element.m_flags.m_propState);
    BeAssert(nullptr != m_element.m_ecPropertyData);

    m_element.m_flags.m_propState = DgnElement::PropState::InBuffer;

    ECInstanceUpdater* updater = GetUpdater();
    if (nullptr == updater)
        {
        BeAssert(false);
        return DgnDbStatus::WrongClass;
        }

    return (BSISUCCESS == updater->Update(*this))? DgnDbStatus::Success: DgnDbStatus::WriteError;
    }

/*---------------------------------------------------------------------------------**//**
* Adapted from MemoryECBaseInstance
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ElementInstanceAdapter::_CopyFromBuffer (ECDBufferCR src)
    {
    //DgnElement const* fromMemoryInstance = dynamic_cast<DgnElement const*> (&src);
    //if (NULL != fromMemoryInstance && GetClassLayout().Equals (fromMemoryInstance->GetClassLayout()))
    //    {
    //    SetUsageBitmask (fromMemoryInstance->GetUsageBitmask());
    //    memcpy (m_perPropertyFlagsHolder.perPropertyFlags, fromMemoryInstance->GetPerPropertyFlagsData(), m_perPropertyFlagsHolder.numPerPropertyFlagsEntries * sizeof(uint32_t));
    //    }

    return CopyPropertiesFromBuffer (src);
    }

/*---------------------------------------------------------------------------------**//**
* Adapted from MemoryECBaseInstance
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ElementInstanceAdapter::GetBytesUsed () const
    {
    if (NULL == m_element.m_ecPropertyData)
        return 0;

    return CalculateBytesUsed ();
    }

/*---------------------------------------------------------------------------------**//**
* Adapted from MemoryECBaseInstance
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/        
void ElementInstanceAdapter::_ClearValues ()
    {
    //if (m_structInstances)
    //    m_structInstances->clear ();

    InitializeMemory (GetClassLayout(), m_element.m_ecPropertyData, m_element.m_ecPropertyDataSize);

    //ClearAllPerPropertyFlags ();
    }
   
/*---------------------------------------------------------------------------------**//**
* Adapted from MemoryECBaseInstance
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ElementInstanceAdapter::_ModifyData (uint32_t offset, void const * newData, uint32_t dataLength)
    {
    PRECONDITION (NULL != m_element.m_ecPropertyData, ECObjectsStatus::PreconditionViolated);
    PRECONDITION (offset + dataLength <= m_element.m_ecPropertyDataSize, ECObjectsStatus::MemoryBoundsOverrun);

    Byte * dest = m_element.m_ecPropertyData + offset;
    memcpy (dest, newData, dataLength);
    
    return ECObjectsStatus::Success;
    }
    
/*---------------------------------------------------------------------------------**//**
* Adapted from MemoryECBaseInstance
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ElementInstanceAdapter::_MoveData (uint32_t toOffset, uint32_t fromOffset, uint32_t dataLength)
    {
    PRECONDITION (NULL != m_element.m_ecPropertyData, ECObjectsStatus::PreconditionViolated);
    PRECONDITION (toOffset + dataLength <= m_element.m_ecPropertyDataSize, ECObjectsStatus::MemoryBoundsOverrun);

    Byte* data = m_element.m_ecPropertyData;
    memmove (data+toOffset, data+fromOffset, dataLength);

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* Adapted from MemoryECBaseInstance
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ElementInstanceAdapter::_ShrinkAllocation ()
    {
    uint32_t newAllocation = GetBytesUsed();
    if (0 == newAllocation)
        _FreeAllocation();
    else if (newAllocation != _GetBytesAllocated())
        {
        Byte* reallocedData = (Byte*)bentleyAllocator_realloc(m_element.m_ecPropertyData, newAllocation);
        if (NULL == reallocedData)
            {
            BeAssert (false);
            return ECObjectsStatus::UnableToAllocateMemory;
            }

        m_element.m_ecPropertyData = reallocedData;
        m_element.m_ecPropertyDataSize = newAllocation;
        }

    return ECObjectsStatus::Success;
    } 

/*---------------------------------------------------------------------------------**//**
* Adapted from MemoryECBaseInstance
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementInstanceAdapter::_FreeAllocation ()
    {
    //if (!m_usingSharedMemory)
    //    {
        if (m_element.m_ecPropertyData)
            bentleyAllocator_free(m_element.m_ecPropertyData);

        //if (m_perPropertyFlagsHolder.perPropertyFlags)
        //    {
        //    free (m_perPropertyFlagsHolder.perPropertyFlags); 
        //    m_perPropertyFlagsHolder.perPropertyFlags = NULL;
        //    }
    //    }

    m_element.m_ecPropertyData = NULL;

    //if (m_structInstances)
    //    {
    //    m_structInstances->clear ();
    //    delete m_structInstances;
    //    m_structInstances = NULL;
    //    }
    }

/*---------------------------------------------------------------------------------**//**
* Adapted from MemoryECBaseInstance
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ElementInstanceAdapter::_GrowAllocation (uint32_t bytesNeeded)
    {
    DEBUG_EXPECT (m_element.m_ecPropertyDataSize > 0);
    DEBUG_EXPECT (NULL != m_element.m_ecPropertyData);
        
    uint32_t newSize = 2 * (m_element.m_ecPropertyDataSize + bytesNeeded); // Assume the growing trend will continue.

    Byte * reallocedData = (Byte*)bentleyAllocator_realloc(m_element.m_ecPropertyData, newSize);
    DEBUG_EXPECT (NULL != reallocedData);
    if (NULL == reallocedData)
        return ECObjectsStatus::UnableToAllocateMemory;
    
    m_element.m_ecPropertyData = reallocedData;
    m_element.m_ecPropertyDataSize = newSize;

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayoutCR ElementInstanceAdapter::_GetClassLayout () const
    {
    return m_eclass->GetDefaultStandaloneEnabler()->GetClassLayout();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ElementECPropertyAccessor::ElementECPropertyAccessor(DgnElement const& el, Utf8CP propName) :
    ElementInstanceAdapter(el)
    {
    if (!ElementInstanceAdapter::IsValid())
        return;

    m_ecprop = m_eclass->GetPropertyP(propName);
    if ((nullptr == m_ecprop) || m_element.IsCustomHandledProperty(*m_ecprop))
        {
        // This is not an auto-handled property
        m_ecprop = nullptr;     // This object cannot be used to access this property
        BeAssert(!IsValid());
        return;
        }

    BeAssert(IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ElementECPropertyAccessor::SetPropertyValue(ECValueCR value, DgnElement::PropertyArrayIndex const& arrayIdx)
    {
    if (!IsValidValue(*m_ecprop, value))
        return DgnDbStatus::BadArg;

    if (!IsValidForStatementType(*m_ecprop, m_element.GetElementId().IsValid() ? 
                                 ECSqlClassParams::StatementType::Update : ECSqlClassParams::StatementType::Insert))
        return DgnDbStatus::ReadOnly;

    auto status = SetValueToMemory(m_ecprop->GetName().c_str(), value, arrayIdx.m_hasIndex, arrayIdx.m_index);
        
    if ((ECObjectsStatus::Success != status) && (ECObjectsStatus::PropertyValueMatchesNoChange != status))
        return DgnDbStatus::BadArg; // probably a type mismatch

    m_element.m_flags.m_propState = DgnElement::PropState::Dirty;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ElementECPropertyAccessor::GetPropertyValue(ECN::ECValueR value, DgnElement::PropertyArrayIndex const& arrayIdx)
    {
    if (ECN::ECObjectsStatus::Success != GetValueFromMemory(value, m_ecprop->GetName().c_str(), arrayIdx.m_hasIndex, arrayIdx.m_index))
        return DgnDbStatus::BadRequest;
    
    return DgnDbStatus::Success;
    }
