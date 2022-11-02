/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
// Most Recently Used cache of DgnElements for a DgnDb. Holds the N most recently used
// elements for a DgnDb. As a newer element is added, the least recently used one is dropped
// if the cache holds more than the maximum size.
// @bsiclass
//=======================================================================================
struct ElementMRU
{
    struct ElemEntry : NonCopyableClass
    {
        uint64_t m_id;
        DgnElementCPtr m_el;
        ElemEntry(uint64_t id, DgnElementCPtr el) : m_id(id), m_el(el) {}
    };

    typedef std::list<ElemEntry> EntryList;

    EntryList m_list;
    std::unordered_map<uint64_t, EntryList::iterator> m_map;
    uint32_t m_maxSize;

    explicit ElementMRU(uint32_t size = 2000) : m_maxSize(size) {}
    void SetMaxSize(uint32_t newSize) { m_maxSize = newSize; Purge(); }
    void Clear() {m_map.clear(); m_list.clear();}
    void MoveToFront(EntryList::iterator it) {m_list.splice(m_list.begin(), m_list, it);}

    // add an element to the front of the MRU cache.
    void AddElement(DgnElementCR el)
        {
        if (0 == m_maxSize)
            return;

        uint64_t id = el.GetElementId().GetValue();
        auto iter = m_map.find(id);
        if (iter != m_map.end())
            {
            iter->second->m_el = &el;
            MoveToFront(iter->second);
            return;
            }

        m_list.emplace_front(id, &el);
        m_map[id] = m_list.begin();
        Purge();
        }

    // look for the element in the MRU cache. If found, move it to most recent
    DgnElementCP FindElement(DgnElementId eid)
        {
        auto id = eid.GetValue();
        auto iter = m_map.find(id);
        if (iter == m_map.end())
            return nullptr;

        MoveToFront(iter->second);
        return iter->second->m_el.get();
        }

    // drop an element from MRU cache.
    bool DropElement(DgnElementId eid)
        {
        auto id = eid.GetValue();
        auto iter = m_map.find(id);
        if (iter == m_map.end())
            return false;

        m_list.erase(iter->second);
        m_map.erase(iter);
        return true;
        }

    // purge MRU cache until it is less than the maximum size.
    void Purge()
        {
        while (m_map.size() > m_maxSize)
            {
            m_map.erase(m_list.back().m_id);
            m_list.pop_back();
            }
        }
};

END_BENTLEY_DGN_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElements::~DgnElements()
    {
    Destroy();
    BeAssert(0 == m_extant);       // make sure nobody has any DgnElements around from this DgnDb
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElements::Destroy()
    {
    ClearECCaches();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElements::SetCacheSize(uint32_t newSize)
    {
    BeMutexHolder _v_v(m_mutex);
    m_mruCache->SetMaxSize(newSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElements::ClearCache()
    {
    BeMutexHolder _v_v(m_mutex);
    m_mruCache->Clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElements::AddToPool(DgnElementCR element) const
    {
    BeMutexHolder _v_v(m_mutex);
    m_mruCache->AddElement(element);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElements::DropFromPool(DgnElementCR element) const
    {
    BeMutexHolder _v_v(m_mutex);
    m_mruCache->DropElement(element.GetElementId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCP DgnElements::FindLoadedElement(DgnElementId id) const
    {
    BeMutexHolder _v_v(m_mutex);
    return m_mruCache->FindElement(id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CachedStatementPtr DgnElements::GetStatement(Utf8CP sql) const
    {
    CachedStatementPtr stmt;
    m_stmts.GetPreparedStatement(stmt, *m_dgndb.GetDbFile(), sql);
    return stmt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::DgnElement(CreateParams const& params) :  m_elementId(params.m_id),
    m_dgndb(params.m_dgndb), m_modelId(params.m_modelId), m_classId(params.m_classId),
    m_federationGuid(params.m_federationGuid), m_code(params.m_code), m_parent(params.m_parentId, params.m_parentId.IsValid() ? params.m_parentRelClassId : DgnClassId()),
    m_userLabel(params.m_userLabel), m_ecPropertyData(nullptr), m_ecPropertyDataSize(0), m_structInstances(nullptr), m_napiObj(nullptr)
    {
#if !defined (NDEBUG)
    auto& elements = GetDgnDb().Elements();
    BeMutexHolder lock(elements.GetMutex());
    ++elements.m_extant;  // only for detecting leaks
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::~DgnElement()
    {
    BeAssert(m_napiObj == nullptr); // should only be set during insert/update
    ClearAllAppData();

    UnloadAutoHandledProperties();

#if !defined (NDEBUG)
    auto& elements = GetDgnDb().Elements();
    BeMutexHolder lock(elements.GetMutex());
    --elements.m_extant;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElements::DgnElements(DgnDbR dgndb) : DgnDbTable(dgndb), m_stmts(20), m_snappyFrom(m_snappyFromBuffer, _countof(m_snappyFromBuffer))
    {
    m_mruCache.reset(new ElementMRU());
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Element::_OnApply() {
    if (!m_txnMgr.IsInAbandon())
        _OnValidate();
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Element::_OnApplied() {
    if (!m_txnMgr.IsInAbandon())
        _OnValidated();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Element::_OnAppliedAdd(BeSQLite::Changes::Change const& change) {
    if (!m_txnMgr.IsInAbandon())
        AddChange(change, ChangeType::Insert, false);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Element::_OnAppliedDelete(BeSQLite::Changes::Change const& change) {
    if (!m_txnMgr.IsInAbandon())
        AddChange(change, ChangeType::Delete, false);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Element::_OnAppliedUpdate(BeSQLite::Changes::Change const& change) {
    if (!m_txnMgr.IsInAbandon())
        AddChange(change, ChangeType::Update, false);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnElements::LoadElement(DgnElement::CreateParams const& params, Utf8CP jsonProps, bool makePersistent) const {
    ElementHandlerP elHandler = dgn_ElementHandler::Element::FindHandler(m_dgndb, params.m_classId);
    if (nullptr == elHandler) {
        BeAssert(false);
        return nullptr;
    }

    DgnElementPtr el = elHandler->Create(params);
    if (!el.IsValid()) {
        BeAssert(false);
        return nullptr;
    }

    if (jsonProps)
        el->m_jsonProps.Parse(jsonProps);

    if (DgnDbStatus::Success != el->_LoadFromDb())
        return nullptr;

    el->_OnLoadedJsonProperties();

    if (makePersistent) {
        el->GetModel()->_OnLoadedElement(*el);
        AddToPool(*el);
    }

    return el;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnElements::LoadElement(DgnElementId elementId,  bool makePersistent) const {
    enum Column : int {ClassId=0,ModelId=1,CodeSpec=2,CodeScope=3,CodeValue=4,UserLabel=5,ParentId=6,ParentRelClassId=7,FederationGuid=8,JsonProps=9};
    CachedStatementPtr stmt = GetStatement("SELECT ECClassId,ModelId,CodeSpecId,CodeScopeId,CodeValue,UserLabel,ParentId,ParentRelECClassId,FederationGuid,JsonProperties FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE Id=?");
    stmt->BindId(1, elementId);

    DbResult result = stmt->Step();
    if (BE_SQLITE_ROW != result)
        return nullptr;

    DgnCode code(stmt->GetValueId<CodeSpecId>(Column::CodeSpec), stmt->GetValueId<DgnElementId>(Column::CodeScope), stmt->GetValueText(Column::CodeValue));

    DgnElement::CreateParams createParams(m_dgndb, stmt->GetValueId<DgnModelId>(Column::ModelId),
                    stmt->GetValueId<DgnClassId>(Column::ClassId),
                    code,
                    stmt->GetValueText(Column::UserLabel),
                    stmt->GetValueId<DgnElementId>(Column::ParentId),
                    stmt->GetValueId<DgnClassId>(Column::ParentRelClassId),
                    stmt->GetValueGuid(Column::FederationGuid));

    createParams.SetElementId(elementId);
    createParams.SetIsLoadingElement(true);

     try {
        return LoadElement(createParams, stmt->GetValueText(Column::JsonProps), makePersistent);
     } catch (Napi::Error const& jsError) {
        throw jsError; // allow Javascript to handle exception
     } catch (std::exception const& e) {
        BeAssert(false && "exception in loadElement");
        LOG.errorv("exception in loadElement: %s", e.what());
        return nullptr;
     } catch (...) {
        BeAssert(false && "exception in loadElement");
        LOG.error("exception in loadElement");
        return nullptr;
     }
 }

 /*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
 DgnElementCPtr DgnElements::GetElement(DgnElementId elementId) const {
     if (!elementId.IsValid())
         return nullptr;

     // since we can load elements on more than one thread, we need to check that the element doesn't already exist
     // *with the lock held* before we load it. This avoids a race condition where an element is loaded on more than one thread.
     BeMutexHolder _v(m_mutex);
     DgnElementCP element = FindLoadedElement(elementId);
     return (nullptr != element) ? element : LoadElement(elementId, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElements::LoadGeometryStream(GeometryStreamR geom, void const* blob, int blobSize)
    {
    BeMutexHolder _v(m_mutex);
    return geom.ReadGeometryStream(GetSnappyFrom(), GetDgnDb(), blob, blobSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnElements::QueryElementByFederationGuid(BeGuidCR federationGuid) const
    {
    if (!federationGuid.IsValid())
        return nullptr;

    CachedStatementPtr statement = GetStatement("SELECT Id FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE FederationGuid=?");
    statement->BindGuid(1, federationGuid);
    return (BE_SQLITE_ROW != statement->Step()) ? nullptr : GetElement(statement->GetValueId<DgnElementId>(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ElementIterator DgnElements::MakeIterator(Utf8CP className, Utf8CP whereClause, Utf8CP orderByClause, PolymorphicQuery polymorphic) const
    {
    Utf8String sql("SELECT ECInstanceId,ECClassId,FederationGuid,CodeSpec.Id,CodeScope.Id,CodeValue,Model.Id,Parent.Id,UserLabel,LastMod FROM ");
    if (PolymorphicQuery::No == polymorphic)
        sql.append("ONLY ");

    sql.append(className);

    if (whereClause)
        {
        sql.append(" ");
        sql.append(whereClause);
        }

    if (orderByClause)
        {
        sql.append(" ");
        sql.append(orderByClause);
        }

    ElementIterator iterator;
    iterator.Prepare(m_dgndb, sql.c_str(), 0 /* Index of ECInstanceId */);
    return iterator;
    }

DgnElementId ElementIteratorEntry::GetElementId() const {return m_statement->GetValueId<DgnElementId>(0);}
DgnClassId ElementIteratorEntry::GetClassId() const {return m_statement->GetValueId<DgnClassId>(1);}
BeGuid ElementIteratorEntry::GetFederationGuid() const {return m_statement->GetValueGuid(2);}
DgnCode ElementIteratorEntry::GetCode() const {return DgnCode(m_statement->GetValueId<CodeSpecId>(3), m_statement->GetValueId<DgnElementId>(4), m_statement->GetValueText(5));}
Utf8CP ElementIteratorEntry::GetCodeValue() const {return m_statement->GetValueText(5);}
DgnModelId ElementIteratorEntry::GetModelId() const {return m_statement->GetValueId<DgnModelId>(6);}
DgnElementId ElementIteratorEntry::GetParentId() const {return m_statement->GetValueId<DgnElementId>(7);}
Utf8CP ElementIteratorEntry::GetUserLabel() const {return m_statement->GetValueText(8);}
DateTime ElementIteratorEntry::GetLastModifyTime() const {return m_statement->GetValueDateTime(9);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ElementAspectIterator DgnElement::MakeAspectIterator() const
    {
    ElementAspectIterator iterator;
    iterator.Prepare(GetDgnDb(),
        "SELECT ECInstanceId,ECClassId,Element.Id FROM " BIS_SCHEMA(BIS_CLASS_ElementUniqueAspect) " WHERE Element.Id=:ElementIdParam1 UNION "
        "SELECT ECInstanceId,ECClassId,Element.Id FROM " BIS_SCHEMA(BIS_CLASS_ElementMultiAspect)  " WHERE Element.Id=:ElementIdParam2",
        0 /* Index of ECInstanceId */);

    ECSqlStatement* statement = iterator.GetStatement();
    if (statement)
        {
        statement->BindId(statement->GetParameterIndex("ElementIdParam1"), GetElementId());
        statement->BindId(statement->GetParameterIndex("ElementIdParam2"), GetElementId());
        }

    return iterator;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ElementAspectIterator DgnElements::MakeAspectIterator(Utf8CP className, Utf8CP whereClause, Utf8CP orderByClause) const
    {
    Utf8String sql("SELECT ECInstanceId,ECClassId,Element.Id FROM ");
    sql.append(className);

    if (whereClause)
        {
        sql.append(" ");
        sql.append(whereClause);
        }

    if (orderByClause)
        {
        sql.append(" ");
        sql.append(orderByClause);
        }

    ElementAspectIterator iterator;
    iterator.Prepare(m_dgndb, sql.c_str(), 0 /* Index of ECInstanceId */);
    return iterator;
    }

ECInstanceId ElementAspectIteratorEntry::GetECInstanceId() const {return m_statement->GetValueId<ECInstanceId>(0);}
DgnClassId ElementAspectIteratorEntry::GetClassId() const {return m_statement->GetValueId<DgnClassId>(1);}
DgnElementId ElementAspectIteratorEntry::GetElementId() const {return m_statement->GetValueId<DgnElementId>(2);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnElements::PerformInsert(DgnElementR element, DgnDbStatus& stat)
    {
    if (element.m_flags.m_preassignedId)  {
        // the Id was supplied by the caller. Make sure we increase max value if the value we're inserting is higher than current.
         m_dgndb.GetElementIdSequence().CheckMaxValue(element.m_elementId.GetValueUnchecked());
    } else {
        if (BE_SQLITE_OK != m_dgndb.GetElementIdSequence().GetNextValue(element.m_elementId))
            return nullptr;
    }

    if (DgnDbStatus::Success != (stat = element._OnInsert()))
        return nullptr;

    // ask parent whether its ok to add this child.
    DgnElementCPtr parent = GetElement(element.m_parent.m_id);
    if (parent.IsValid() && DgnDbStatus::Success != (stat=parent->_OnChildInsert(element)))
        return nullptr;

    ECClassCP elementClass = element.GetElementClass();
    if (nullptr == elementClass)
        {
        BeAssert(false);
        stat = DgnDbStatus::BadElement;
        return nullptr;
        }

    if (DgnDbStatus::Success != (stat = element._InsertInDb()))
        return nullptr;

    DgnElement::CopyFromOptions opts;
    opts.copyEcPropertyData = false;
    DgnElementPtr newElement = element.CopyForEditInternal(opts);
    AddToPool(*newElement);

    newElement->_OnInserted(&element);

    if (parent.IsValid())
        parent->_OnChildInserted(*newElement);

    return newElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCPtr DgnElements::InsertElement(DgnElementR element, DgnDbStatus* outStat)
    {
    DgnDb::VerifyClientThread();

    DgnDbStatus ALLOW_NULL_OUTPUT(stat,outStat);

    // don't allow elements that already have an id unless the "preassignedId" flag is set
    if (element.m_elementId.IsValid() && !element.m_flags.m_preassignedId)
        {
        stat = DgnDbStatus::WrongElement; // this element must already be persistent
        return nullptr;
        }

    if (&element.GetDgnDb() != &m_dgndb)
        {
        stat = DgnDbStatus::WrongDgnDb; // attempting to add an element from a different DgnDb
        return nullptr;
        }

    if (!element.GetModel().IsValid())
        {
        stat = DgnDbStatus::BadModel; // they gave us an element with an invalid ModelId
        return nullptr;
        }

    DgnElementCPtr newEl = PerformInsert(element, stat);
    if (!newEl.IsValid())
        element.m_elementId = DgnElementId(); // Insert failed, make sure to invalidate the DgnElementId so they don't accidentally use it

    element.m_flags.m_preassignedId = 0; // ensure flag is set to default value
    return newEl;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::CopyIdentityFrom(DgnElementId elementId, BeGuidCR federationGuid)
    {
    m_flags.m_preassignedId = true;
    m_elementId = elementId;
    m_federationGuid = federationGuid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElements::UpdateElement(DgnElementR replacement)
    {
    DgnDb::VerifyClientThread();

    // Get a pointer to the original element. Note: this means that the pre-changed element will be in the
    // MRU cache for the duration of this method. Until we're done all other threads will see the unchanged element.
    DgnElementCPtr orig = GetElement(replacement.GetElementId());
    if (!orig.IsValid())
        return  DgnDbStatus::InvalidId;

    DgnElementCR element = *orig;
    if (&element.GetDgnDb() != &replacement.GetDgnDb())
        return DgnDbStatus::WrongDgnDb;

    auto stat = replacement._OnUpdate(element);
    if (DgnDbStatus::Success != stat)
        return stat; // something rejected proposed change

    bool parentChanged = false;
    DgnElementId prevParent = element.m_parent.m_id;
    if (prevParent != replacement.m_parent.m_id) // did parent change?
        {
        parentChanged = true;
        // ask original parent if it is okay to drop the child
        DgnElementCPtr originalParent = GetElement(element.m_parent.m_id);
        if (originalParent.IsValid() && DgnDbStatus::Success != (stat = originalParent->_OnChildDrop(element)))
            return stat;

        // ask new parent if it is okay to add the child
        DgnElementCPtr replacementParent = GetElement(replacement.m_parent.m_id);
        if (replacementParent.IsValid() && DgnDbStatus::Success != (stat = replacementParent->_OnChildAdd(replacement)))
            return stat;
        }
    else
        {
        // ask parent whether it is ok to update its child.
        DgnElementCPtr parent = GetElement(element.m_parent.m_id);
        if (parent.IsValid() && DgnDbStatus::Success != (stat = parent->_OnChildUpdate(element, replacement)))
            return stat;
        }

    stat = replacement._UpdateInDb();   // perform the actual update in the database
    if (DgnDbStatus::Success != stat)
        return stat;

    replacement._OnUpdated(element);

    if (parentChanged) // did parent change?
        {
        // notify original parent that child has been dropped
        DgnElementCPtr originalParent = GetElement(prevParent);
        if (originalParent.IsValid())
            originalParent->_OnChildDropped(element);

        // notify new parent that child has been added
        DgnElementCPtr replacementParent = GetElement(replacement.m_parent.m_id);
        if (replacementParent.IsValid())
            replacementParent->_OnChildAdded(replacement);
        }
    else
        {
        // notify parent that its child has been updated
        DgnElementCPtr parent = GetElement(replacement.m_parent.m_id);
        if (parent.IsValid())
            parent->_OnChildUpdated(element);
        }

    // now drop the old element from MRU cache. The next request will load the updated element.
    DropFromPool(element);

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElements::PerformDelete(DgnElementCR element)
    {
    // delete children, if any.
    DgnElementIdSet children = element.QueryChildren();
    for (auto childId : children)
        {
        auto child = GetElement(childId);
        if (!child.IsValid())
            continue;

        auto stat = PerformDelete(*child);
        if (DgnDbStatus::Success != stat)
            return stat;
        }

    auto stat = element._DeleteInDb();
    if (DgnDbStatus::Success != stat)
        return stat;

    element._OnDeleted();
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElements::Delete(DgnElementCR elementIn)
    {
    DgnDb::VerifyClientThread();

    if (&elementIn.GetDgnDb() != &m_dgndb)
        return DgnDbStatus::WrongDgnDb;

    DgnElementCPtr el = &elementIn;
    if (!el.IsValid())
        return DgnDbStatus::BadElement;

    DgnElementCR element = *el;

    DgnDbStatus stat = element._OnDelete();
    if (DgnDbStatus::Success != stat)
        return stat;

    // ask parent whether its ok to delete his child.
    auto parent = GetElement(element.m_parent.m_id);
    if (parent.IsValid() && DgnDbStatus::Success != (stat=parent->_OnChildDelete(element)))
        return stat;

    stat = PerformDelete(element);
    if (DgnDbStatus::Success != stat)
        return stat;

    if (parent.IsValid())
        parent->_OnChildDeleted(element);

    return DgnDbStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnModelId DgnElements::QueryModelId(DgnElementId elementId) const
    {
    CachedStatementPtr stmt=GetStatement("SELECT ModelId FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE Id=?");
    stmt->BindId(1, elementId);
    return (BE_SQLITE_ROW != stmt->Step()) ? DgnModelId() : stmt->GetValueId<DgnModelId>(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DgnElementId DgnElements::QueryElementIdByCode(DgnCodeCR code) const
    {
    if (!code.IsValid() || code.IsEmpty())
        return DgnElementId(); // An invalid code won't be found; an empty code won't be unique. So don't bother.

    return QueryElementIdByCode(code.GetCodeSpecId(), code.GetScopeElementId(GetDgnDb()), code.GetValueUtf8());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId DgnElements::QueryElementIdByCode(Utf8CP codeSpec, DgnElementId scopeElementId, Utf8StringCR value) const
    {
    return QueryElementIdByCode(GetDgnDb().CodeSpecs().QueryCodeSpecId(codeSpec), scopeElementId, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId DgnElements::QueryElementIdByCode(CodeSpecId codeSpec, DgnElementId scopeElementId, Utf8StringCR value) const
    {
    CachedStatementPtr statement=GetStatement("SELECT Id FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE CodeSpecId=? AND CodeScopeId=? AND CodeValue=? LIMIT 1");
    statement->BindId(1, codeSpec);
    statement->BindId(2, scopeElementId);
    statement->BindText(3, value, Statement::MakeCopy::No);
    return (BE_SQLITE_ROW != statement->Step()) ? DgnElementId() : statement->GetValueId<DgnElementId>(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
struct GenericClassParamsProvider : IECSqlClassParamsProvider
    {
    DgnClassId m_classId;
    DgnElements const& m_elements;
    GenericClassParamsProvider(DgnClassId classId, DgnElements const& e) : m_classId(classId), m_elements(e) {}
    void _GetClassParams(ECSqlClassParamsR ecSqlParams) override;
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void GenericClassParamsProvider::_GetClassParams(ECSqlClassParamsR ecSqlParams)
    {
    // *** WIP_AUTO_HANDLED_PROPERTIES: "ECInstanceId" is handled specially. It's in the table but not in the properties collection
    ecSqlParams.Add("ECInstanceId", ECSqlClassParams::StatementType::Insert);

    auto ecclass = m_elements.GetDgnDb().Schemas().GetClass(ECN::ECClassId(m_classId.GetValue()));
    if (ecclass == nullptr) {
        LOG.errorv("GenericClassParamsProvider::_GetClassParams(): Unable to find class with ID=0x%" PRIx64, m_classId.GetValue());
        return;
    }
    AutoHandledPropertiesCollection props(*ecclass, m_elements.GetDgnDb(), ECSqlClassParams::StatementType::All, true);
    for (auto i = props.begin(); i != props.end(); ++i)
        {
        ecSqlParams.Add((*i)->GetName(), i.GetStatementType());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSqlClassParams const& DgnElements::GetECSqlClassParams(DgnClassId classId) const
    {
    BeMutexHolder _v(m_mutex);

    ECSqlClassParams& params = m_classParams[classId];
    if (!params.IsInitialized())
        {
        GenericClassParamsProvider provider(classId, *this);
        params.Initialize(provider);
        }
    return params;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR DgnElements::GetSelectEcPropsECSql(ECSqlClassInfo& classInfo, ECN::ECClassCR ecclass) const
    {
    BeMutexHolder _v(m_mutex);  // guard lazy initialization of classInfo.m_selectEcProps

    if (!classInfo.m_selectEcProps.empty())
        return classInfo.m_selectEcProps;

    Utf8String props;
    Utf8CP comma = "";
    bvector<ECN::ECPropertyCP> autoHandledProperties;
    for (auto prop : AutoHandledPropertiesCollection(ecclass, GetDgnDb(), ECSqlClassParams::StatementType::Select, false))
        {
        Utf8StringCR propName = prop->GetName();
        props.append(comma).append("[").append(propName).append("]");
        comma = ",";
        }

    if (props.empty())
        return classInfo.m_selectEcProps = "";

    classInfo.m_selectEcProps = Utf8PrintfString("SELECT %s FROM %s WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter",
                                                            props.c_str(), ecclass.GetECSqlName().c_str());
    return classInfo.m_selectEcProps;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR DgnElements::GetAutoHandledPropertiesSelectECSql(ECN::ECClassCR ecclass) const
    {
    ECSqlClassInfo& classInfo = FindClassInfo(DgnClassId(ecclass.GetId().GetValue())); // Note: This "Find" method will create a ClassInfo if necessary
    return GetSelectEcPropsECSql(classInfo, ecclass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSqlClassInfo& DgnElements::FindClassInfo(DgnClassId classId) const
    {
    BeMutexHolder _v(m_mutex);
    auto found = m_classInfos.find(classId);
    if (m_classInfos.end() != found)
        return found->second;

    ECSqlClassInfo& classInfo = m_classInfos[classId];
    ECSqlClassParams const& params = GetECSqlClassParams(classId);

    bool populated = params.BuildClassInfo(classInfo, GetDgnDb(), classId);
    BeAssert(populated);
    UNUSED_VARIABLE(populated);

    return classInfo;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
JsonECSqlSelectAdapter const& DgnElements::GetJsonSelectAdapter(BeSQLite::EC::ECSqlStatement const& stmt) const
    {
    BeMutexHolder _v(m_mutex);

    auto it = m_jsonSelectAdapterCache.find(stmt.GetHashCode());
    if (it != m_jsonSelectAdapterCache.end())
        return *it->second;

    // As the adapter is cached, we can avoid that the member names are copied into the generated JSON. Instead
    // the generated JSON objects reference the member names held by the adapter.
    // Note: Must make sure the JSON objects don't live longer than the adapter.
    JsonECSqlSelectAdapter::FormatOptions jsonFormatOps(JsonECSqlSelectAdapter::MemberNameCasing::LowerFirstChar, ECJsonInt64Format::AsHexadecimalString);
    jsonFormatOps.SetRowFormat(JsonECSqlSelectAdapter::RowFormat::IModelJs);
    std::unique_ptr<JsonECSqlSelectAdapter> adapter = std::make_unique<JsonECSqlSelectAdapter>(stmt, jsonFormatOps,
                                                                   false); // don't make copy of member names for every JSON as the adapter is cached.
    JsonECSqlSelectAdapter* adapterP = adapter.get();
    m_jsonSelectAdapterCache[stmt.GetHashCode()] = std::move(adapter);
    return *adapterP;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSqlClassInfo& DgnElements::FindClassInfo(DgnElementCR el) const
    {
    return FindClassInfo(el.GetElementClassId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElements::ElementSelectStatement DgnElements::GetPreparedSelectStatement(DgnElementR el) const
    {
    auto stmt = FindClassInfo(el).GetSelectStmt(GetDgnDb(), ECInstanceId(el.GetElementId().GetValue()));
    return ElementSelectStatement(stmt.get(), GetECSqlClassParams(el.GetElementClassId()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CachedECSqlStatementPtr DgnElements::GetPreparedInsertStatement(DgnElementR el) const
    {
    // Not bothering to cache per class...use our general-purpose ECSql statement cache
    return FindClassInfo(el).GetInsertStmt(GetDgnDb());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CachedECSqlStatementPtr DgnElements::GetPreparedUpdateStatement(DgnElementR el) const
    {
    // Not bothering to cache per class...use our general-purpose ECSql statement cache
    return FindClassInfo(el).GetUpdateStmt(GetDgnDb(), ECInstanceId(el.GetElementId().GetValue()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIdSet DgnElements::FindGeometryPartReferences(BeSQLite::IdSet<DgnGeometryPartId> const& partIdsToFind, bool is2d) const
    {
    DgnElementIdSet refs;
    if (partIdsToFind.empty())
        return refs;

    Utf8String sql("SELECT ElementId FROM ");
    auto tableName = is2d ? BIS_TABLE(BIS_CLASS_GeometricElement2d) : BIS_TABLE(BIS_CLASS_GeometricElement3d);
    sql.append(tableName);
    sql.append(" WHERE GeometryStream IS NOT NULL");

    BeSQLite::IdSet<DgnGeometryPartId> foundPartIds;
    auto stmt = GetStatement(sql.c_str());
    while (BE_SQLITE_ROW == stmt->Step())
        {
        auto elemId = stmt->GetValueId<DgnElementId>(0);
        auto elem = GetElement(elemId);
        auto src = elem.IsValid() ? elem->ToGeometrySource() : nullptr;
        if (nullptr == src)
            continue;

        auto const& stream = src->GetGeometryStream();
        uint8_t const* data = stream.GetData();
        size_t size = stream.GetSize();
        foundPartIds.clear();
        GeometryStreamIO::Collection geom(data, size);
        geom.GetGeometryPartIds(foundPartIds, GetDgnDb());
        for (auto foundPartId : foundPartIds)
            {
            if (partIdsToFind.end() != partIdsToFind.find(foundPartId))
                {
                refs.insert(elemId);
                break;
                }
            }
        }

    return refs;
    }

