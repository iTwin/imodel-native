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
void DgnElements::DropFromPool(DgnElementIdSet ids) const
    {
    BeMutexHolder _v_v(m_mutex);
    for (const auto& id : ids)
        m_mruCache->DropElement(id);
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
        AddChange(change, ChangeType::Insert);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Element::_OnAppliedDelete(BeSQLite::Changes::Change const& change) {
    if (!m_txnMgr.IsInAbandon())
        AddChange(change, ChangeType::Delete);
}

/*---------------------------------------------------------------------------------**/ /**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::Element::_OnAppliedUpdate(BeSQLite::Changes::Change const& change) {
    if (!m_txnMgr.IsInAbandon())
        AddChange(change, ChangeType::Update);
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

    DgnCode code = DgnCode::CreateWithDbContext(m_dgndb, stmt->GetValueId<CodeSpecId>(Column::CodeSpec), stmt->GetValueId<DgnElementId>(Column::CodeScope), stmt->GetValueText(Column::CodeValue));

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

/** Return true if an element with the supplied Id exists */
bool DgnElements::ElementExists(DgnElementId id) {
    // the root subject element always exists. Don't bother to check.
    if (id == GetRootSubjectId())
        return true;

    CachedStatementPtr statement = GetStatement("SELECT 1 FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE Id=?");
    statement->BindId(1, id);
    return BE_SQLITE_ROW == statement->Step();
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
DgnCode ElementIteratorEntry::GetCode() const {
    // dynamic_cast should be cheap on most compilers when casting to the correct type, which
    // we should be here
    const auto* dgndb = dynamic_cast<const DgnDb*>(m_statement->GetECDb());
    BeAssert(dgndb != nullptr);
    if (dgndb == nullptr) [[unlikely]]
        return DgnCode();
    return DgnCode::CreateWithDbContext(*dgndb, m_statement->GetValueId<CodeSpecId>(3), m_statement->GetValueId<DgnElementId>(4), m_statement->GetValueText(5));
}
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
bool BulkElementDeletion::CreateTempTables() const
    {
    // Create a temp table to hold the entries for the elements to be deleted
    auto stat = m_dgndb.CreateTableIfNotExists(TEMP_TABLE(TEMP_ELEMENT_DELETION), 
    R"sql(
        ElementId       INTEGER PRIMARY KEY,
        LogicalParentId INTEGER,
        IsSubModelRoot  INTEGER NOT NULL DEFAULT 0,
        Depth           INTEGER NOT NULL DEFAULT 0,
        IsViolator      INTEGER NOT NULL DEFAULT 0
    )sql");
    if (stat != BE_SQLITE_OK)
        {
        LOG.errorv("Error creating temp table %s: %s", TEMP_TABLE(TEMP_ELEMENT_DELETION), BeSQLiteLib::GetLogError(stat).c_str());
        return false;
        }

    if (BE_SQLITE_OK != m_dgndb.TryExecuteSql("CREATE INDEX IF NOT EXISTS idx_etd_logicalParent ON " TEMP_ELEMENT_DELETION " (LogicalParentId)"))
        return false;

    // Partial index to accelerate IsViolator=1 scans (PruneViolators, deleteAllViolators).
    if (BE_SQLITE_OK != m_dgndb.TryExecuteSql("CREATE INDEX IF NOT EXISTS idx_etd_violator ON " TEMP_ELEMENT_DELETION " (IsViolator) WHERE IsViolator = 1"))
        return false;

    // Partial index to accelerate IsSubModelRoot=1 scans (DeleteLinkTableRelationships, ExecuteDeletion).
    if (BE_SQLITE_OK != m_dgndb.TryExecuteSql("CREATE INDEX IF NOT EXISTS idx_etd_submodelroot ON " TEMP_ELEMENT_DELETION " (IsSubModelRoot) WHERE IsSubModelRoot = 1"))
        return false;

    // Clear out table to avoid stale entries
    if (BE_SQLITE_OK != m_dgndb.TryExecuteSql("DELETE FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION)))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool BulkElementDeletion::ExpandElementIdList() const
    {
    constexpr auto expandSql = 
        "WITH RECURSIVE fullSet(id, logicalParentId, isSubModelRoot, depth) AS ("
            "SELECT e.Id, "
                "e.ParentId, "
                "CASE WHEN m.Id IS NOT NULL THEN 1 ELSE 0 END, "
                "0 "
            "FROM bis_Element e "
            "LEFT JOIN bis_Model m ON m.ModeledElementId = e.Id "
            "WHERE InVirtualSet(?, e.Id) "
            "UNION ALL "
            "SELECT e.Id,"
                "e.ParentId, "
                "CASE WHEN m.Id IS NOT NULL THEN 1 ELSE 0 END, "
                "f.depth + 1 "
            "FROM bis_Element e "
            "INNER JOIN fullSet f ON e.ParentId = f.id "
            "LEFT JOIN bis_Model m ON m.ModeledElementId = e.Id "
            "UNION ALL "
            "SELECT e.Id,"
                "sm.ModeledElementId, "
                "CASE WHEN m.Id IS NOT NULL THEN 1 ELSE 0 END, "
                "f.depth + 1 "
            "FROM bis_Element e "
            "INNER JOIN bis_Model sm ON sm.Id = e.ModelId "
            "INNER JOIN fullSet f ON sm.ModeledElementId = f.id "
            "LEFT JOIN bis_Model m ON m.ModeledElementId = e.Id "
        ") "
        "INSERT OR REPLACE INTO " TEMP_TABLE(TEMP_ELEMENT_DELETION) " (ElementId, LogicalParentId, IsSubModelRoot, Depth) "
        "SELECT id, logicalParentId, isSubModelRoot, depth "
        "FROM fullSet";

    const auto expandStmt = m_dgndb.GetCachedStatement(expandSql);
    if (!expandStmt.IsValid())
        {
        LOG.error("BulkElementDeletion: Failed to add dependent elements");
        return false;
        }
    expandStmt->BindVirtualSet(1, m_originalElementIds);
    if (const auto stat = expandStmt->Step(); stat != BE_SQLITE_DONE)
        {
        LOG.errorv("BulkElementDeletion: Failed to add dependent elements: %s", BeSQLiteLib::GetLogError(stat).c_str());
        return false;
        }

    m_dgndb.TryExecuteSql("ANALYZE " TEMP_TABLE(TEMP_ELEMENT_DELETION));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool BulkElementDeletion::PruneViolators()
    {
    // This is purely for error reporting.
    // While this is a hinderance for the performance, it is important to import the caller exactly which elements failed deletion.
    constexpr auto collectViolatorsSql =
        "SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " WHERE IsViolator = 1";

    auto pruneStmt = m_dgndb.GetCachedStatement(collectViolatorsSql);
    if (!pruneStmt.IsValid())
        {
        LOG.error("BulkElementDeletion: Failed to find constraint violators");
        return false;
        }

    while (BE_SQLITE_ROW == pruneStmt->Step())
        {
        auto id = pruneStmt->GetValueId<DgnElementId>(0);
        if (id.IsValid() && m_originalElementIds.Contains(id))
            m_failedToDelete.insert(id);
        }

    constexpr auto deleteAllViolators = "DELETE FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " WHERE IsViolator = 1";
    if (const auto stat = m_dgndb.TryExecuteSql(deleteAllViolators); stat != BE_SQLITE_OK)
        {
        LOG.errorv("BulkElementDeletion: Failed to delete all constraint violators: %s", BeSQLiteLib::GetLogError(stat).c_str());
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool BulkElementDeletion::FindAndPruneConstraintViolators()
    {
    // Optimization guard: Don't run the expensive search queries if no violators exist
    constexpr auto guardSql = 
        "SELECT 1 FROM bis_Element e "
        "WHERE e.CodeScopeId IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) ") "
            "AND e.Id NOT IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) ") "
        "UNION ALL "
        "SELECT 1 FROM bis_GeometricElement3d g "
        "WHERE g.CategoryId IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) ") "
            "AND g.ElementId NOT IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) ") "
        "UNION ALL "
        "SELECT 1 FROM bis_GeometricElement2d g "
        "WHERE g.CategoryId IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) ") "
            "AND g.ElementId NOT IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) ") "
        "LIMIT 1";
    auto guardStmt = m_dgndb.GetCachedStatement(guardSql);
    if (guardStmt.IsNull())
        {
        LOG.error("BulkElementDeletion: Constraint violator guard check failed");
        return false;
        }

    if (BE_SQLITE_ROW != guardStmt->Step())
        return true;

    constexpr auto findViolators =
        "WITH "
        // Get the direct constraint violators from the delete set
        "directViolators AS ("
            "SELECT DISTINCT t.ElementId "
            "FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " t "
            "WHERE EXISTS ("
                "SELECT 1 FROM bis_Element e "
                "LEFT JOIN " TEMP_TABLE(TEMP_ELEMENT_DELETION) " td ON td.ElementId = e.Id "
                "WHERE e.CodeScopeId = t.ElementId AND td.ElementId IS NULL"
            ") "
            "OR EXISTS ("
                "SELECT 1 FROM bis_GeometricElement3d g3 "
                "LEFT JOIN " TEMP_TABLE(TEMP_ELEMENT_DELETION) " td ON td.ElementId = g3.ElementId "
                "WHERE g3.CategoryId = t.ElementId AND td.ElementId IS NULL"
            ") "
            "OR EXISTS ("
                "SELECT 1 FROM bis_GeometricElement2d g2 "
                "LEFT JOIN " TEMP_TABLE(TEMP_ELEMENT_DELETION) " td ON td.ElementId = g2.ElementId "
                "WHERE g2.CategoryId = t.ElementId AND td.ElementId IS NULL"
            ")"
        "), "
        // Walk UP from each direct violator to find its highest ancestor in the delete set
        "ancestors(id, logicalParentId) AS ("
            "SELECT t.ElementId, t.LogicalParentId "
            "FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " t "
            "WHERE t.ElementId IN (SELECT ElementId FROM directViolators) "
            "UNION ALL "
            "SELECT t.ElementId, t.LogicalParentId "
            "FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " t "
            "INNER JOIN ancestors a ON t.ElementId = a.logicalParentId"
        "), "
        "subtreeRoots AS ("
            "SELECT DISTINCT a.id FROM ancestors a "
            "LEFT JOIN " TEMP_TABLE(TEMP_ELEMENT_DELETION) " p ON p.ElementId = a.logicalParentId "
            "WHERE a.logicalParentId IS NULL OR p.ElementId IS NULL"
        "), "
        // Walk DOWN from each root to collect all descendants
        "subtree(id) AS ("
            "SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " "
            "WHERE ElementId IN (SELECT id FROM subtreeRoots) "
            "UNION ALL "
            "SELECT t.ElementId "
            "FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " t "
            "INNER JOIN subtree s ON t.LogicalParentId = s.id"
        ") "
        "UPDATE " TEMP_TABLE(TEMP_ELEMENT_DELETION) " SET IsViolator = 1 WHERE ElementId IN (SELECT id FROM subtree)";

    if (const auto stat = m_dgndb.TryExecuteSql(findViolators); stat != BE_SQLITE_OK)
        {
        LOG.errorv("BulkElementDeletion: Failed to find constraint violators: %s", BeSQLiteLib::GetLogError(stat).c_str());
        return false;
        }

    return PruneViolators();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool BulkElementDeletion::FindAndPruneInUseDefinitionElements()
    {
    // Collect only the DefinitionElement rows from the temp table.
    // Non-definition elements in the set don't need usage checking.
    constexpr auto defIdsSql = 
        "SELECT t.ElementId "
        "FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " t "
        "INNER JOIN bis_DefinitionElement d ON d.ElementId = t.ElementId";

    auto defStmt = m_dgndb.GetCachedStatement(defIdsSql);
    if (!defStmt.IsValid())
        {
        LOG.error("BulkElementDeletion: Failed to query for definition elements");
        return false;
        }

    DgnElementIdSet definitionIds;
    while (BE_SQLITE_ROW == defStmt->Step())
        {
        if (auto id = defStmt->GetValueId<DgnElementId>(0); id.IsValid())
            definitionIds.insert(id);
        }

    if (definitionIds.empty())
        return true;

    m_definitionElementsExist = true;

    // Exclude intra-set usages from the usage check — elements referencing
    // each other inside the delete set are fine since all will be deleted.
    auto usageInfo = DefinitionElementUsageInfo::Create(m_dgndb, definitionIds, std::make_shared<BeSQLite::IdSet<BeInt64Id>>(BeIdSet(definitionIds.GetBeIdSet())));
    if (!usageInfo.IsValid())
        return true; // no usage info means nothing blocks deletion

    DgnElementIdSet inUse;
    for (const auto& id : definitionIds)
        {
        if (usageInfo->GetUsedIds().Contains(id))
            inUse.insert(id);
        }

    if (inUse.empty())
        return true;

    constexpr auto findDependents = 
        "WITH RECURSIVE ancestry(id, logicalParentId) AS ("
            // Get all in-use elements in the delete list
            "SELECT ElementId, LogicalParentId "
            "FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " "
            "WHERE InVirtualSet(?, ElementId) "

            "UNION ALL "

            // Get all descendants of the current element
            "SELECT t.ElementId, t.LogicalParentId "
            "FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " t "
            "INNER JOIN ancestry a ON t.ElementId = a.logicalParentId"
        ") "
        "Update " TEMP_TABLE(TEMP_ELEMENT_DELETION) " SET IsViolator = 1 WHERE ElementId IN (SELECT id FROM ancestry)";

    const auto findDependentsStmt = m_dgndb.GetCachedStatement(findDependents);
    if (!findDependentsStmt.IsValid())
        {
        LOG.error("BulkElementDeletion: Failed to query for dependent elements");
        return false;
        }
    findDependentsStmt->BindVirtualSet(1, inUse);
    if (const auto stat = findDependentsStmt->Step(); stat != BE_SQLITE_OK)
        {
        LOG.errorv("BulkElementDeletion: Failed to prune in-use definition elements: %s", BeSQLiteLib::GetLogError(stat).c_str());
        return false;
        }

    return PruneViolators();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool BulkElementDeletion::FindAndNullTypeDefinitionReferences() const
    {
    if (!m_definitionElementsExist)
        return true;

    // Optimization guard against unnecessary scans
    auto guard = m_dgndb.GetCachedStatement(
        "SELECT 1 "
        "FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " t "
        "WHERE EXISTS ("
            "SELECT 1 FROM bis_GeometricElement3d g "
            "LEFT JOIN " TEMP_TABLE(TEMP_ELEMENT_DELETION) " td ON td.ElementId = g.ElementId "
            "WHERE g.TypeDefinitionId = t.ElementId AND td.ElementId IS NULL"
        ") "
        "OR EXISTS ("
            "SELECT 1 FROM bis_GeometricElement2d g "
            "LEFT JOIN " TEMP_TABLE(TEMP_ELEMENT_DELETION) " td ON td.ElementId = g.ElementId "
            "WHERE g.TypeDefinitionId = t.ElementId AND td.ElementId IS NULL"
        ") "
        "LIMIT 1");

    if (!guard.IsValid())
        {
        LOG.error("BulkElementDeletion: Failed to query for TypeDefinition references");
        return false;
        }

    if (BE_SQLITE_ROW != guard->Step())
        return true;

    // NULL out TypeDefinitionId on all surviving GeometricElements that point to an element we are about to delete.
    constexpr auto null3dSql =
        "UPDATE bis_GeometricElement3d SET TypeDefinitionId = NULL "
        "WHERE TypeDefinitionId IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) ") "
          "AND EXISTS ("
              "SELECT 1 FROM bis_GeometricElement3d g2 "
              "LEFT JOIN " TEMP_TABLE(TEMP_ELEMENT_DELETION) " td ON td.ElementId = g2.ElementId "
              "WHERE g2.ElementId = bis_GeometricElement3d.ElementId AND td.ElementId IS NULL"
          ")";

    if (const auto stat = m_dgndb.TryExecuteSql(null3dSql); stat != BE_SQLITE_OK)
        {
        LOG.errorv("BulkElementDeletion: TypeDefinitionId NULL (3d) failed: %s", BeSQLiteLib::GetLogError(stat).c_str());
        return false;
        }

    constexpr auto null2dSql =
        "UPDATE bis_GeometricElement2d SET TypeDefinitionId = NULL "
        "WHERE TypeDefinitionId IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) ") "
          "AND EXISTS ("
              "SELECT 1 FROM bis_GeometricElement2d g2 "
              "LEFT JOIN " TEMP_TABLE(TEMP_ELEMENT_DELETION) " td ON td.ElementId = g2.ElementId "
              "WHERE g2.ElementId = bis_GeometricElement2d.ElementId AND td.ElementId IS NULL"
          ")";

    if (const auto stat = m_dgndb.TryExecuteSql(null2dSql); stat != BE_SQLITE_OK)
        {
        LOG.errorv("BulkElementDeletion: TypeDefinitionId NULL (2d) failed: %s", BeSQLiteLib::GetLogError(stat).c_str());
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool BulkElementDeletion::FireAllCallbacks()
    {
    constexpr auto getAllElements =
        "SELECT t.ElementId, t.IsSubModelRoot, t.Depth, "
            "CASE WHEN InVirtualSet(?, t.ElementId) THEN 1 ELSE 0 END AS IsOriginal "
        "FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " t "
        "ORDER BY t.Depth DESC";

    const auto stmt = m_dgndb.GetCachedStatement(getAllElements);
    if (!stmt.IsValid())
        {
        LOG.error("BulkElementDeletion: Failed to fire element callbacks");
        return false;
        }
    stmt->BindVirtualSet(1, m_originalElementIds);

    while (BE_SQLITE_ROW == stmt->Step())
        {
        const auto elementId = stmt->GetValueId<DgnElementId>(0);
        if (!elementId.IsValid())
            return false;

        const auto element = m_dgndb.Elements().GetElement(elementId);
        if (!element.IsValid())
            return false;

        if (const auto isSubModelRoot = stmt->GetValueInt(1); isSubModelRoot != 0)
            {
            if (auto subModel = element->GetSubModel(); subModel.IsValid())
                {
                element->_OnSubModelDelete(*subModel);
                subModel->_OnDeleteNotify();
                subModel->_OnDeleted();
                element->_OnSubModelDeleted(*subModel);
                }
            m_subModelRootExists = true;
            }

        if (stmt->GetValueInt(3) != 0)
            {
            element->_OnDelete();

            if (const auto parentId = element->m_parent.m_id; parentId.IsValid())
                {
                if (const auto parentElement = m_dgndb.Elements().GetElement(parentId); parentElement.IsValid())
                    {
                    parentElement->_OnChildDelete(*element);
                    parentElement->_OnChildDeleted(*element);
                    }
                }
            }

        element->_OnDeleted();
        }
    return true;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool BulkElementDeletion::ExecuteDeletion()
    {
    // Prep the db and handlers for a bulk delete
    if (m_definitionElementsExist)
        m_dgndb.BeginPurgeOperation();
    m_dgndb.Elements().SetBulkOperation(true);
    m_dgndb.TryExecuteSql("PRAGMA defer_foreign_keys = true");

    auto reset = [&]() {
        m_dgndb.Elements().SetBulkOperation(false);
        if (m_definitionElementsExist)
            m_dgndb.EndPurgeOperation();
        m_dgndb.TryExecuteSql("PRAGMA defer_foreign_keys = false");
    };

    const auto stat = m_dgndb.TryExecuteSql("DELETE FROM " BIS_TABLE(BIS_CLASS_Element) " WHERE Id IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) ")");
    if (stat != BE_SQLITE_OK)
        {
        LOG.errorv("BulkElementDeletion: Element deletion failed: %s", BeSQLiteLib::GetLogError(stat).c_str());
        reset();
        return false;
        }

    if (m_subModelRootExists)
        {
        const auto modelStat = m_dgndb.TryExecuteSql("DELETE FROM " BIS_TABLE(BIS_CLASS_Model) " WHERE Id IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " WHERE IsSubModelRoot = 1)");
        if (modelStat != BE_SQLITE_OK)
            {
            LOG.errorv("BulkElementDeletion: Sub-model root deletion failed: %s", BeSQLiteLib::GetLogError(modelStat).c_str());
            reset();
            return false;
            }
        }

    reset();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool BulkElementDeletion::DeleteLinkTableRelationships()
    {
    for (const auto& tableName : { BIS_TABLE(BIS_REL_ElementRefersToElements), BIS_TABLE(BIS_REL_ElementDrivesElement) })
        {
        auto stat = m_dgndb.TryExecuteSql(Utf8PrintfString("DELETE FROM %s WHERE SourceId IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) ")", tableName).c_str());
        if (stat != BE_SQLITE_OK)
            {
            LOG.errorv("BulkElementDeletion: Link table SourceId deletion failed: %s", BeSQLiteLib::GetLogError(stat).c_str());
            return false;
            }

        stat = m_dgndb.TryExecuteSql(Utf8PrintfString("DELETE FROM %s WHERE TargetId IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) ")", tableName).c_str());
        if (stat != BE_SQLITE_OK)
            {
            LOG.errorv("BulkElementDeletion: Link table TargetId deletion failed: %s", BeSQLiteLib::GetLogError(stat).c_str());
            return false;
            }
        }

    if (m_subModelRootExists)
        {
        const auto stat = m_dgndb.TryExecuteSql("DELETE FROM " BIS_TABLE(BIS_REL_ModelSelectorRefersToModels) " WHERE TargetId IN (SELECT ElementId FROM " TEMP_TABLE(TEMP_ELEMENT_DELETION) " WHERE IsSubModelRoot = 1)");
        if (stat != BE_SQLITE_OK)
            {
            LOG.errorv("BulkElementDeletion: Sub-model root deletion failed: %s", BeSQLiteLib::GetLogError(stat).c_str());
            return false;
            }
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIdSet BulkElementDeletion::Execute()
    {
    DgnDb::VerifyClientThread();

    if (!CreateTempTables())
        return m_originalElementIds;

    if (!ExpandElementIdList())
        return m_originalElementIds;

    if (!m_skipFkValidation && !FindAndPruneConstraintViolators())
        return m_originalElementIds;

    // If definition elements exist in the delete set, check usage and null out any externally references type definitions
    if (!FindAndPruneInUseDefinitionElements())
        return m_originalElementIds;

    if (!m_skipFkValidation && !FindAndNullTypeDefinitionReferences())
        return m_originalElementIds;

    // Fire the pre and post delete callbacks at once
    if (!FireAllCallbacks())
        return m_originalElementIds;

    // Bulk delete the elements
    if (!ExecuteDeletion() && m_failedToDelete.empty())
        return m_originalElementIds;

    // Clean up link-table relationships
    if (!DeleteLinkTableRelationships())
        return m_originalElementIds;

    return m_failedToDelete;
    }

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIdSet DgnElements::DeleteElements(const DgnElementIdSet& elementIds, const bool skipFkValidation)
    {
    DgnDb::VerifyClientThread();
    if (elementIds.empty())
        return {};

    return BulkElementDeletion(m_dgndb, elementIds, skipFkValidation).Execute();
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

