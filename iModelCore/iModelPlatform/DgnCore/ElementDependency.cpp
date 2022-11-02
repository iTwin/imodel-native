/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#define EDGE_QUEUE_TABLE_NAME  "TxnEdgeQueue"
#define EDGE_QUEUE TEMP_TABLE(EDGE_QUEUE_TABLE_NAME)
#define NODES_TABLE_NAME "TxnNodes"
#define NODES TEMP_TABLE(NODES_TABLE_NAME)

#define EDGLOGGER NativeLogging::CategoryLogger("ElementDep")
#define EDGLOG(sev,...) { EDGLOGGER.messagev(sev, __VA_ARGS__); }

static int s_debugGraph = 0;
static bool s_debugGraph_showElementIds;

BEGIN_BENTLEY_DGN_NAMESPACE
namespace ElementDependency {

struct EdgeStatusAccessor
{
    union
    {
        struct
        {
            uint32_t m_failed:1;           // NB: bit layout must match EdgeStatus
            uint32_t m_unused1:6;          // NB: bit layout must match EdgeStatus
            uint32_t m_deferred:1;         // NB: bit layout must match EdgeStatus
            uint32_t m_unused2:24;
        };
        uint32_t    m_flags;
    };

    explicit EdgeStatusAccessor(EdgeStatus s) : m_flags((uint32_t)s) {}
    explicit EdgeStatusAccessor(uint32_t s) : m_flags(s) {}
    EdgeStatus ToEdgeStatus() const {return (EdgeStatus)m_flags;}
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct Graph::TableApi
{
    Graph& m_graph;
    TableApi(Graph& g) : m_graph(g) {}
    DgnDbR GetDgnDb() const {return m_graph.GetDgnDb();}
    TxnManager& GetTxnMgr() const {return m_graph.m_txnMgr;}
    virtual DbResult UpdateEdgeStatusInDb(Edge const& edge, EdgeStatus newStatus) {return BE_SQLITE_OK;}
};

//=======================================================================================
//  Wrapper for the ElementDrivesElement ECRelationship
// @bsiclass
//=======================================================================================
struct Graph::ElementDrivesElement : Graph::TableApi
    {
    private:
    CachedStatementPtr m_selectByRootInDirectChanges, m_selectByRelationshipInDirectChanges, m__selectByRoot__, m__updateStatus__;

    DbResult SelectEdge(Edge&, Statement&);

    public:
    ElementDrivesElement(Graph&);

    DbResult DoPrepare();

    DbResult StepSelectWhereRootInDirectChanges(Edge&);
    DbResult StepSelectWhereRelationshipInDirectChanges(Edge&);
    void BindSelectByRoot(DgnElementId);
    DbResult StepSelectByRoot(Edge&);
    // DbResult SelectEdgeByRelId(Edge&, ECInstanceId);
    DbResult UpdateEdgeStatusInDb(Edge const& edge, EdgeStatus newStatus);
    };

//=======================================================================================
//  Traverses designated ElementOwnsChildElements relationships
// @bsiclass
//=======================================================================================
struct Graph::ChildPropagatesChangesToParent : Graph::TableApi
    {
    private:
    CachedStatementPtr m_selectByChildInDirectChanges, m__selectByChild__;
    bool m_none{};

    DbResult SelectEdge(Edge&, Statement&);

    public:
    ChildPropagatesChangesToParent(Graph&);

    DbResult DoPrepare();

    DbResult StepSelectWhereChildInDirectChanges(Edge&);
    void BindSelectByChild(DgnElementId);
    DbResult StepSelectByChild(Edge&);
    Utf8String GetParentRelMatch();
    void PrepareSelectAllStatement(Statement&);
    DbResult StepSelectAllStatement(Edge&, Statement&);
    };

//=======================================================================================
//  The queue of all element dependency graph edges found.
// @bsiclass
//=======================================================================================
struct Graph::EdgeQueue : Graph::TableApi
    {
    private:
    CachedStatementPtr m_insert, m_select, m_selbyp, m_selbyo, m__updateStatus__;
    CachedStatementPtr m_setEdgeColor, m_getEdgeColor;
    Statement m_checkPathStmt;

    DbResult SelectEdge(Edge&, Statement&);

    public:
    EdgeQueue(Graph& g);
    ~EdgeQueue();
    DbResult AddEdge(Edge&);
    DbResult StepSelectAll(Edge& edge) {return SelectEdge(edge, *m_select);}
    void ResetSelectAll() { m_select->Reset(); }

    void ResetSelectAllOrderedByPriority() {m_selbyp->Reset();}
    DbResult StepSelectAllOrderedByPriority(Edge& edge) {return SelectEdge(edge, *m_selbyp);}
    DbResult BindSelectByOutput(DgnElementId);
    DbResult StepSelectByOutput(Edge& edge) {return SelectEdge(edge, *m_selbyo);}
    EdgeColor GetEdgeColor(ECInstanceKeyCR);
    void SetEdgeColor(ECInstanceKeyCR, EdgeColor);
    };

//=======================================================================================
//  The collection of nodes found in element dependency graph.
// @bsiclass
//=======================================================================================
struct Graph::Nodes : Graph::TableApi
    {
    DEFINE_T_SUPER(Graph::TableApi)

    private:
        CachedStatementPtr m_insert, m_allInputsProcessed, m_anyOutputsProcessed, m_selectInDegree;
        CachedStatementPtr m_incrementInDegree, m_incrementInputsProcessed, m_incrementOutputsProcessed;
        CachedStatementPtr m_setDirect, m_selectDirect;

    public:
        Nodes(Graph&);
        ~Nodes();

        DbResult InsertNode(DgnElementId nodeId);
        DbResult IncrementInDegree(DgnElementId nodeId);
        int GetInDegree(DgnElementId);

        //! Increment the processed inputs counter. Input here is an edge ending at the given node.
        //! @param[in] nodeId   Node that the input (edge) points to.
        DbResult IncrementInputsProcessed(DgnElementId nodeId);

        //! Increment the processed outputs counter. Output here is an edge starting at the given node.
        //! @param[in] nodeId   Node that the output (edge) points from (start at).
        DbResult IncrementOutputsProcessed(DgnElementId nodeId);

        //! Check if all inputs to the given node were processed.
        bool AllInputsProcessed(DgnElementId);

        //! Check if any outputs of the given node are processed.
        bool AnyOutputsProcessed(DgnElementId);

        //! A direct change has been propagated to this node from one of its inputs
        DbResult SetInputWasDirectlyChanged(DgnElementId);

        //! Were any direct changes propagated to any input of this node?
        bool WasInputDirectlyChanged(DgnElementId);

        bool WasElementDirectlyChanged(DgnElementId);
    };
} // end ElementDependency namespace

END_BENTLEY_DGN_NAMESPACE

using namespace ElementDependency;

HANDLER_DEFINE_MEMBERS(Handler)

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Handler& getEdeHandler(Graph const& graph, Edge const& edge) {
    BeAssert(edge.m_isEDE);
    auto relClassId = edge.GetKey().GetClassId();
    auto& domains = graph.GetDgnDb().Domains();
    Handler* handler = (Handler*) domains.LookupHandler(relClassId);
    if (nullptr != handler)
        return *handler;

    return (Handler&) *domains.FindHandler(relClassId, domains.GetClassId(Handler::GetHandler()));
}

static DgnElementCPtr getDriver(Graph const& graph, DgnElementId id) {return graph.GetDgnDb().Elements().Get<DgnElement>(id);}

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void callJsDriverFunc(Graph const& graph, DgnElementId id, Utf8CP methodName) {
    auto& jsTxns = graph.m_jsTxns;
    if (jsTxns == nullptr)
        return;
    auto& db = graph.GetDgnDb();
    DgnDb::CallJsFunction(graph.m_jsTxns, methodName, {db.GetJsClassName(id), db.ToJsString(id)});
}

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void callJsEdgeFunc(Graph const& graph, Edge const& edge, Utf8CP methodName) {
    auto& jsTxns = graph.m_jsTxns;
    if (jsTxns == nullptr)
        return;
    auto& db = graph.GetDgnDb();
    Napi::Object props = Napi::Object::New(jsTxns.Env());

    auto relClass = db.Schemas().GetClass(edge.GetKey().GetClassId());
    if (relClass != nullptr)
        props["classFullName"] = db.ToJsString(relClass->GetFullName());
    props["id"] = db.ToJsString(edge.GetKey().GetInstanceId());
    props["sourceId"] = db.ToJsString(edge.m_ein);
    props["targetId"] = db.ToJsString(edge.m_eout);
    DgnDb::CallJsFunction(jsTxns, methodName, {props});
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Graph::Nodes::Nodes(Graph& graph) : T_Super(graph)
    {
    auto& db = GetDgnDb();
    if (!db.TableExists(NODES))
        db.CreateTable(NODES, "ElementId INTEGER PRIMARY KEY, InDegree INTEGER DEFAULT 0, InputsProcessed INTEGER DEFAULT 0, OutputsProcessed INTEGER DEFAULT 0, Direct INTEGER DEFAULT 0");

    db.GetCachedStatement(m_insert, "INSERT INTO " NODES " (ElementId) VALUES(?)");
    db.GetCachedStatement(m_incrementInDegree, "UPDATE " NODES " SET InDegree=InDegree+1 WHERE ElementId=?");
    db.GetCachedStatement(m_selectInDegree, "SELECT InDegree FROM " NODES " WHERE ElementId=?");
    db.GetCachedStatement(m_setDirect, "UPDATE " NODES " SET Direct=1 WHERE ElementId=?");
    db.GetCachedStatement(m_selectDirect, "SELECT Direct FROM " NODES " WHERE ElementId=?");
    db.GetCachedStatement(m_allInputsProcessed, "SELECT EXISTS(SELECT 1 FROM " NODES " WHERE ElementId=? AND InDegree=InputsProcessed LIMIT 1)");
    db.GetCachedStatement(m_anyOutputsProcessed, "SELECT EXISTS(SELECT 1 FROM " NODES " WHERE ElementId=? AND OutputsProcessed!=0 LIMIT 1)");
    db.GetCachedStatement(m_incrementInputsProcessed, "UPDATE " NODES " SET InputsProcessed=InputsProcessed+1 WHERE ElementId=?");
    db.GetCachedStatement(m_incrementOutputsProcessed, "UPDATE " NODES " SET OutputsProcessed=OutputsProcessed+1 WHERE ElementId=?");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Graph::Nodes::~Nodes()
    {
    GetDgnDb().ExecuteSql("DELETE FROM " NODES);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool Graph::Nodes::AllInputsProcessed(DgnElementId nodeId)
    {
    BeAssert(nodeId.IsValid());

    m_allInputsProcessed->Reset();
    m_allInputsProcessed->BindId(1, nodeId);

    auto stat = m_allInputsProcessed->Step();
    return BE_SQLITE_ROW != stat ? false :  m_allInputsProcessed->GetValueBoolean(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool Graph::Nodes::AnyOutputsProcessed(DgnElementId nodeId)
    {
    BeAssert(nodeId.IsValid());

    m_anyOutputsProcessed->Reset();
    m_anyOutputsProcessed->BindId(1, nodeId);

    auto stat = m_anyOutputsProcessed->Step();
    return (BE_SQLITE_ROW != stat) ?  false :  m_anyOutputsProcessed->GetValueBoolean(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Graph::Nodes::InsertNode(DgnElementId nodeId)
    {
    BeAssert(nodeId.IsValid());

    m_insert->Reset();
    m_insert->BindId(1, nodeId);
    return m_insert->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Graph::Nodes::IncrementInDegree(DgnElementId nodeId)
    {
    BeAssert(nodeId.IsValid());

    m_incrementInDegree->Reset();
    m_incrementInDegree->BindId(1, nodeId);
    return m_incrementInDegree->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int Graph::Nodes::GetInDegree(DgnElementId nodeId)
    {
    BeAssert(nodeId.IsValid());

    m_selectInDegree->Reset();
    m_selectInDegree->BindId(1, nodeId);

    auto stat = m_selectInDegree->Step();
    if (BE_SQLITE_ROW != stat)
        return 0;

    int inDegree = m_selectInDegree->GetValueInt(0);
    if (inDegree < 0)
        return 0;

    return inDegree;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Graph::Nodes::SetInputWasDirectlyChanged(DgnElementId nodeId)
    {
    BeAssert(nodeId.IsValid());

    m_setDirect->Reset();
    m_setDirect->BindId(1, nodeId);
    return m_setDirect->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool Graph::Nodes::WasInputDirectlyChanged(DgnElementId nodeId)
    {
    BeAssert(nodeId.IsValid());

    m_selectDirect->Reset();
    m_selectDirect->BindId(1, nodeId);

    auto stat = m_selectDirect->Step();
    if (BE_SQLITE_ROW != stat)
        return 0;

    return m_selectDirect->GetValueBoolean(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Graph::Nodes::IncrementInputsProcessed(DgnElementId nodeId)
    {
    BeAssert(nodeId.IsValid());

    m_incrementInputsProcessed->Reset();
    m_incrementInputsProcessed->BindId(1, nodeId);
    return m_incrementInputsProcessed->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Graph::Nodes::IncrementOutputsProcessed(DgnElementId nodeId)
    {
    BeAssert(nodeId.IsValid());

    m_incrementOutputsProcessed->Reset();
    m_incrementOutputsProcessed->BindId(1, nodeId);
    return m_incrementOutputsProcessed->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Graph::FmtEdge(Edge const& edge)
    {
    Utf8String s;
    s.append(FmtElement(edge.m_ein));
    s.append("-[");
    s.append(FmtRel(edge));
    s.append("]->");
    s.append(FmtElement(edge.m_eout));
    s.append(Utf8PrintfString(" P=%lld", edge.m_priority));
    if (edge.m_status != 0)
        s.append(Utf8PrintfString(" S=%x", edge.m_status));
    if (edge.m_direct)
        s.append(Utf8PrintfString(" *"));
    return s;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Graph::FmtEdges(bvector<Edge> const& edges)
    {
    Utf8String cpath;
    for (auto iedge = edges.rbegin(); iedge != edges.rend(); ++iedge)
        {
        if (!cpath.empty())
            cpath.append(" / ");
        cpath.append(FmtEdge(*iedge));
        }
    return cpath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Graph::FmtCyclePath(Edge const& edge, bvector<Edge> const& pathToEdge)
    {
    Utf8String cpath(FmtEdge(edge));
    for (auto pred = pathToEdge.rbegin(); pred != pathToEdge.rend(); ++pred)
        {
        Edge emittingEdge = *pred;

        cpath.append("\n");
        cpath.append(FmtEdge(emittingEdge));

        if (emittingEdge.m_eout == edge.m_ein)
            break;
        }
    return cpath;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Graph::GetElementCode(DgnElementId eid)
    {
    ECSqlStatement s;
    s.Prepare(GetDgnDb(), "SELECT CodeValue FROM " BIS_SCHEMA(BIS_CLASS_Element) " WHERE ECInstanceId=?");
    s.BindId(1, eid);
    s.Step();
    return s.GetValueText(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Graph::FmtElement(DgnElementId eid)
    {
    auto dc = m_nodes->WasElementDirectlyChanged(eid)? "!": "";
    auto dci = m_nodes->WasInputDirectlyChanged(eid)? "*": "";

    if (s_debugGraph_showElementIds)
        return Utf8PrintfString("%s(%lld)%s%s", GetElementCode(eid).c_str(), eid.GetValue(), dc, dci);

    return GetElementCode(eid).append(dc).append(dci);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Graph::FmtElementPath(Utf8CP epath)
    {
    Utf8String path;

    Utf8CP start = epath;
    Utf8CP dot = epath;
    for (;;)
        {
        if ('.' == *dot || 0 == *dot)
            {
            uint64_t id;
            if (Utf8String::Sscanf_safe(start, "%" SCNu64, &id) == 1)
                {
                if (!path.empty())
                    path.append(".");
                path.append(FmtElement(DgnElementId(id)));
                }

            if (0 == *dot)
                return path;

            start = dot+1;
            }
        ++dot;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Graph::FmtRel(Edge const& edge)
    {
    auto const& key = edge.GetKey();
    return Utf8PrintfString("%c%lld_%lld", edge.m_isEDE? 'D': 'P', key.GetClassId().GetValue(), key.GetInstanceId().GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8Char dot_removeInvalidChars(Utf8Char c)
    {
    if (c==L'\"' || !isprint(c))
        return L' ';
    return c;
    }

static Utf8String GetValidDotName(Utf8StringCR str)
    {
    Utf8String desc(str);
    std::transform(desc.begin(), desc.end(), desc.begin(), dot_removeInvalidChars);
    return desc;
    }

void Graph::WriteDotEdge(FILE* fp, Edge const& edge)
    {
    auto incode  = GetValidDotName(GetElementCode(edge.m_ein));
    auto outcode = GetValidDotName(GetElementCode(edge.m_eout));
    auto reldesc = GetValidDotName(FmtRel(edge));

    fprintf(fp, "%s -> %s [label=%s]", incode.c_str(), outcode.c_str(), reldesc.c_str());
    fprintf(fp, ";\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Graph::WriteDot(BeFileNameCR dotFilename, bvector<bvector<uint64_t>> const& cyclePaths)
    {
    FILE* fp;
    auto err = BeFile::Fopen(&fp, dotFilename.GetNameUtf8().c_str(), "w+");
    if (0 != err)
        return;

    fwprintf(fp, L"digraph G {\n");

    Statement edges;               // 0        1        2  3      4
    edges.Prepare(GetDgnDb(), "SELECT SourceId,TargetId,Id,Status,ECClassId FROM " BIS_TABLE(BIS_REL_ElementDrivesElement));

    while (edges.Step() == BE_SQLITE_ROW)
        {
        Edge edge;
        edge.m_ein   = edges.GetValueId<DgnElementId>(0);
        edge.m_eout  = edges.GetValueId<DgnElementId>(1);
        auto instanceId = edges.GetValueId<ECInstanceId>(2);
        edge.m_status = (uint8_t)edges.GetValueInt(3);
        auto ecclassId = edges.GetValueId<ECClassId>(4);

        edge.m_instanceKey = ECInstanceKey(ecclassId, instanceId);
        edge.m_isEDE = 1;

        if (!edge.m_ein.IsValid() || !edge.GetKey().IsValid())
            continue;

        WriteDotEdge(fp, edge);
        }

    ChildPropagatesChangesToParent childPropagatesChangesToParents(*this);
    Statement selectchildPropagatesChangesToParents;
    childPropagatesChangesToParents.PrepareSelectAllStatement(selectchildPropagatesChangesToParents);
    Edge edge;
    while (BE_SQLITE_ROW == childPropagatesChangesToParents.StepSelectAllStatement(edge, selectchildPropagatesChangesToParents))
        {
        WriteDotEdge(fp, edge);
        }


    fwprintf(fp, L"}\n");

    fclose(fp);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Graph::WriteEdgeQueueToDot(BeFileNameCR dotFilename)
    {
    FILE* fp;
    auto err = BeFile::Fopen(&fp, dotFilename.GetNameUtf8().c_str(), "w+");
    if (0 != err)
        return;

    fwprintf(fp, L"digraph G {\n");

    m_edgeQueue->ResetSelectAllOrderedByPriority();
    Edge edge;
    while (m_edgeQueue->StepSelectAllOrderedByPriority(edge) == BE_SQLITE_ROW)
        {
        WriteDotEdge(fp, edge);
        }

    fwprintf(fp, L"}\n");

    fclose(fp);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
BentleyStatus Graph::CheckDirection(Edge const& edge)
    {
    /* *** WIP_DEPGRAPH_MODEL_TREE  */
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::ElementDep::_PropagateChanges()
    {
    Graph graph(m_txnMgr);
    graph.InvokeAffectedDependencyHandlers();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Graph::LogDependencyFound(Statement& stmt, Edge const& edge)
    {
    if (!EDGLOGGER.isSeverityEnabled(LOG_TRACE))
        return;

    auto ein    = stmt.GetValueId<DgnElementId>(3);
    auto eout   = stmt.GetValueId<DgnElementId>(4);
    auto epath  = stmt.GetValueText(5);
    EDGLOG(LOG_TRACE, "\t%s->%s->%s (%s)",
                        FmtElement(ein).c_str(),
                            FmtRel(edge).c_str(),
                                  FmtElement(eout).c_str(),
                                       FmtElementPath(epath).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Graph::InvokeHandler(Edge const& edge) {
    size_t errorCount = m_txnMgr.NumValidationErrors(); // in-degree == 0 means that this is a root; it has no inputs
    if (!m_nodes->AnyOutputsProcessed(edge.m_ein) && m_nodes->GetInDegree(edge.m_ein) == 0 && m_nodes->WasElementDirectlyChanged(edge.m_ein)) {
        auto driver = getDriver(*this, edge.m_ein);
        if (driver.IsValid()) {
            driver->_OnBeforeOutputsHandled(*this, edge);
            callJsDriverFunc(*this, edge.m_ein, "_onBeforeOutputsHandled");
        }
    }

    if (!edge.IsDeleted()) {
        if (edge.m_isEDE) {
            getEdeHandler(*this, edge)._OnRootChanged(*this, edge);
            callJsEdgeFunc(*this, edge,  "_onRootChanged");
        }
    } else {
        InvokeHandlerForDeletedRelationship(edge.GetKey());
    }

    m_nodes->IncrementOutputsProcessed(edge.m_ein);
    m_nodes->IncrementInputsProcessed(edge.m_eout);

    if (m_nodes->AllInputsProcessed(edge.m_eout)) {
        auto driver = getDriver(*this, edge.m_eout);
        if (driver.IsValid()) {
            if (m_nodes->WasInputDirectlyChanged(edge.m_eout)) {
                driver->_OnAllInputsHandled(*this, edge);
                callJsDriverFunc(*this, edge.m_eout, "_onAllInputsHandled");
            }
        }
    }

    SetFailedEdgeStatusInDb(edge, (m_txnMgr.NumValidationErrors() > errorCount));
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Graph::UpdateEdgeStatusInDb(Edge const& edge, EdgeStatus newStatus)
    {
    if (edge.m_status == newStatus)
        return BE_SQLITE_DONE;

    if (newStatus == EdgeStatus::EDGESTATUS_Failed)
        {
        EDGLOG(LOG_ERROR, "Failed %s\n", FmtEdge(edge).c_str());
        }
    else
        {
        EDGLOG(LOG_INFO, "Satisfied %s\n", FmtEdge(edge).c_str());
        }

    return m_elementDrivesElement->UpdateEdgeStatusInDb(edge, newStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Graph::SetFailedEdgeStatusInDb(Edge const& edge, bool failed)
    {
    if (failed)
        m_txnMgr.ElementDependencies().AddFailedTarget(edge.m_eout);

    EdgeStatusAccessor saccs(edge.m_status);
    saccs.m_failed = failed;        // set or clear the failed bit
    EdgeStatus newStatus = saccs.ToEdgeStatus();

    return UpdateEdgeStatusInDb(edge, newStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Graph::EdgeQueue::~EdgeQueue()
    {
    GetDgnDb().ExecuteSql("DELETE FROM " EDGE_QUEUE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Graph::EdgeQueue::EdgeQueue(Graph& graph) : Graph::TableApi(graph)
    {
    auto& db = GetDgnDb();
    if (!db.TableExists(EDGE_QUEUE))
        {
        db.CreateTable(EDGE_QUEUE,
            "ECClassId INTEGER"
            ", InstanceId INTEGER"
            ", SourceId INTEGER"
            ", TargetId INTEGER"
            ", status INTEGER DEFAULT 0"
            ", Priority INTEGER DEFAULT 0"
            ", color INTEGER DEFAULT 0"
            ", Deleted INTEGER DEFAULT 0 "
            ", Direct INTEGER DEFAULT 0"
            ", IsEDE INTEGER DEFAULT 0"
            ", PRIMARY KEY(ECClassId, InstanceId)");

        // The IsEDE column is 1 for edges created from ElementDrivesElement instances and 0 for all other kinds of edges.

        // NB default color must be 0, i.e., EdgeColor::White

        // The value of column 'Deleted' determines which method to call on the handler:
        //  0 - call _OnRootChanged
        //  1 - call _ProcessDeletedDependency

        // *** WIP_GRAPH_PRIORITY
        //      -- currently, the Priority column is NULL unless the user happens to populate it.
        //          That is a problem, as we want the default priority to be order of creation.
        //      -- must change dgn.0.02.ecschema.xml to put an index on the priority column, so that ORDER BY Priority will work
        }

    db.GetCachedStatement(m_insert, "INSERT INTO " EDGE_QUEUE " (InstanceId,SourceId,TargetId,ECClassId,status,Priority,Deleted,IsEDE) VALUES(?,?,?,?,?,?,?,?)");
    db.GetCachedStatement(m_setEdgeColor, "UPDATE " EDGE_QUEUE " SET color=? WHERE ECClassId=? AND InstanceId=?");
    db.GetCachedStatement(m_getEdgeColor, "SELECT color FROM " EDGE_QUEUE " WHERE ECClassId=? AND InstanceId=?");

    // NB: All select statements must specify the same columns and in the same order, as they are all processed by a single function, "SelectEdge"

                  // 0          1        2        3         4      5        6       7      8
#define Q_SEL_COLS " InstanceId,SourceId,TargetId,ECClassId,status,Priority,Deleted,Direct,IsEDE"
#define Q_TABLES   " " EDGE_QUEUE " "

    db.GetCachedStatement(m_select, "SELECT " Q_SEL_COLS " FROM " Q_TABLES);
    db.GetCachedStatement(m_selbyp, "SELECT " Q_SEL_COLS " FROM " Q_TABLES " ORDER BY Priority DESC");
    db.GetCachedStatement(m_selbyo, "SELECT " Q_SEL_COLS " FROM " Q_TABLES " WHERE (TargetId=?) ORDER BY Priority DESC");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Graph::EdgeQueue::SelectEdge(Edge& edge, Statement& stmt)
    {
    auto stat = stmt.Step();
    if (BE_SQLITE_ROW != stat)
        return stat;

    auto instanceId = stmt.GetValueId<ECInstanceId>(0);
    edge.m_ein = stmt.GetValueId<DgnElementId>(1);
    edge.m_eout = stmt.GetValueId<DgnElementId>(2);
    auto relClassId = stmt.GetValueId<ECClassId>(3);
    edge.m_status = stmt.GetValueInt(4);
    edge.m_priority = stmt.GetValueInt64(5);
    edge.m_deleted = stmt.GetValueBoolean(6);
    edge.m_direct = stmt.GetValueBoolean(7);
    edge.m_isEDE = stmt.GetValueBoolean(8);

    edge.m_instanceKey = ECInstanceKey(relClassId, instanceId);

    BeAssert(edge.IsValid());
    return BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Graph::EdgeQueue::BindSelectByOutput(DgnElementId eout)
    {
    m_selbyo->Reset();
    return m_selbyo->BindId(1, eout);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Graph::EdgeQueue::AddEdge(Edge& edge)
    {
    // Don't add the edge to the graph if it's deferred
    if (edge.IsDeferred())
        return BE_SQLITE_DONE;

    BeAssert(edge.m_ein.IsValid() && edge.m_eout.IsValid() && edge.GetKey().IsValid());

    //  Add edge+path to queue
    m_insert->Reset();
    m_insert->BindId(1, edge.GetKey().GetInstanceId());
    m_insert->BindId(2, edge.m_ein);
    m_insert->BindId(3, edge.m_eout);
    m_insert->BindId(4, edge.GetKey().GetClassId());
    m_insert->BindInt(5, edge.m_status);
    m_insert->BindInt(6, static_cast<int>(edge.m_priority));
    m_insert->BindBoolean(7, edge.m_deleted);
    m_insert->BindBoolean(8, edge.m_isEDE);
    auto stat = m_insert->Step();

    if (BeSQLiteLib::IsConstraintDbResult(stat))
        {
        //  We already discovered this edge. Nothing to do.
        EDGLOG(LOG_TRACE, "(KNOWN %s)", m_graph.FmtEdge(edge).c_str());
        return stat;
        }

    if (m_graph.CheckDirection(edge) != BSISUCCESS)
        return BE_SQLITE_DONE;

    // EDGLOG(LOG_TRACE, "FOUND %s", m_graph.FmtEdge(edge).c_str());

    BeAssert(edge.IsValid());
    return BE_SQLITE_DONE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Graph::EdgeColor Graph::EdgeQueue::GetEdgeColor(ECInstanceKeyCR key)
    {
    m_getEdgeColor->Reset();
    m_getEdgeColor->BindId(1, key.GetClassId());
    m_getEdgeColor->BindId(2, key.GetInstanceId());
    if (m_getEdgeColor->Step() != BE_SQLITE_ROW)
        return EdgeColor::White;
    return (EdgeColor)m_getEdgeColor->GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+------c---------+---------------+------*/
void Graph::EdgeQueue::SetEdgeColor(ECInstanceKeyCR key, EdgeColor color)
    {
    m_setEdgeColor->Reset();
    m_setEdgeColor->BindInt(1,(int)color);
    m_setEdgeColor->BindId(2, key.GetClassId());
    m_setEdgeColor->BindId(3, key.GetInstanceId());
    auto stat = m_setEdgeColor->Step();
    UNUSED_VARIABLE(stat);
    BeAssert(stat == BE_SQLITE_DONE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Graph::InvokeHandlersInTopologicalOrder_OneGraph(Edge const& edge, bvector<Edge> const& pathToSupplier)
    {
    auto& edges = *m_edgeQueue;

    //  Detect if we have already processed this edge ... or are in the middle of processing it.
    auto color = edges.GetEdgeColor(edge.GetKey());
    if (color != EdgeColor::White)
        {
        if (color == EdgeColor::Gray)
            {
            m_txnMgr.ReportError(true, "cycle", FmtCyclePath(edge, pathToSupplier).c_str());
            SetFailedEdgeStatusInDb(edge, true); // mark at least this edge as failed. maybe we should mark the entire cycle??
            }
        return;
        }

    edges.SetEdgeColor(edge.GetKey(), EdgeColor::Gray);

    // Schedule suppliers of edge's input first.
    auto pathToEdge(pathToSupplier);
    pathToEdge.push_back(edge);

    edges.BindSelectByOutput(edge.m_ein);      // I want find edges that OUTPUT my input

    bvector<Edge> suppliers;
        {
        Edge supplier;
        while (edges.StepSelectByOutput(supplier) == BE_SQLITE_ROW)
            suppliers.push_back(supplier);
        }

    for (auto const& supplier : suppliers)
        InvokeHandlersInTopologicalOrder_OneGraph(supplier, pathToEdge);

    //  edge can now be fired.
    edges.SetEdgeColor(edge.GetKey(), EdgeColor::Black);

    InvokeHandler(edge);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Graph::InvokeHandlerForDeletedRelationship(ECInstanceKeyCR key)
    {
    auto const& deletedRels = m_txnMgr.ElementDependencies().m_deletedRels;
    auto deletedRel = std::find_if(deletedRels.begin(), deletedRels.end(), [key] (auto const& depRel) {return depRel.m_relKey == key;});
    if (deletedRels.end() == deletedRel)
        return;

    Edge edge;
    edge.m_ein = deletedRel->m_source;
    edge.m_eout = deletedRel->m_target;
    edge.m_instanceKey = deletedRel->m_relKey;
    edge.m_isEDE = deletedRel->m_isEDE;

    if (edge.m_isEDE)
        {
        getEdeHandler(*this, edge)._OnDeletedDependency(*this, edge);
        callJsEdgeFunc(*this, edge,  "_onDeletedDependency");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Graph::InvokeHandlersInTopologicalOrder()
    {
    // This is a total topological sort of the Edge queue.
    m_edgeQueue->ResetSelectAllOrderedByPriority();
    Edge edge;
    while (m_edgeQueue->StepSelectAllOrderedByPriority(edge) == BE_SQLITE_ROW)
        InvokeHandlersInTopologicalOrder_OneGraph(edge, bvector<Edge>());

    m_txnMgr.ElementDependencies().m_deletedRels.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Graph::ElementDrivesElement::ElementDrivesElement(Graph& graph) : TableApi(graph){}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool Graph::Nodes::WasElementDirectlyChanged(DgnElementId eid)
    {
    auto stmt = GetTxnMgr().GetTxnStatement("SELECT COUNT(*) FROM " TEMP_TABLE(TXN_TABLE_Elements) " WHERE ElementId=?");
    stmt->BindId(1, eid);
    stmt->Step();
    return stmt->GetValueInt(0) != 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Graph::ElementDrivesElement::DoPrepare()
    {
    //  NB All Select statements must specify the same columns in the same order

    m_selectByRootInDirectChanges = GetTxnMgr().GetTxnStatement(
        "SELECT SourceId,TargetId,Id as relid,ECClassId,Status,Priority FROM " BIS_TABLE(BIS_REL_ElementDrivesElement)
        " WHERE (SourceId IN (SELECT ElementId FROM " TEMP_TABLE(TXN_TABLE_Elements) "))");

    m_selectByRelationshipInDirectChanges = GetTxnMgr().GetTxnStatement(
        "SELECT SourceId,TargetId,Id as relid,ECClassId,Status,Priority FROM " BIS_TABLE(BIS_REL_ElementDrivesElement)
        " WHERE (Id IN (SELECT ECInstanceId FROM " TEMP_TABLE(TXN_TABLE_Depend) "))");

    m__selectByRoot__ = GetTxnMgr().GetTxnStatement(
        "SELECT SourceId,TargetId,Id as relid,ECClassId,Status,Priority FROM " BIS_TABLE(BIS_REL_ElementDrivesElement)
        " WHERE SourceId=?");

    m__updateStatus__ = GetTxnMgr().GetTxnStatement("UPDATE " BIS_TABLE(BIS_REL_ElementDrivesElement) " SET Status=? WHERE Id=?");

    return BE_SQLITE_OK;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Graph::ElementDrivesElement::UpdateEdgeStatusInDb(Edge const& edge, EdgeStatus newStatus)
    {
    m__updateStatus__->Reset();
    m__updateStatus__->BindInt(1,(int)newStatus);
    m__updateStatus__->BindId(2, edge.GetKey().GetInstanceId());
    return m__updateStatus__->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Graph::ElementDrivesElement::SelectEdge(Edge& edge, Statement& stmt)
    {
    auto stat = stmt.Step();
    if (BE_SQLITE_ROW != stat)
        return stat;

    edge.m_ein = stmt.GetValueId<DgnElementId>(0);
    edge.m_eout = stmt.GetValueId<DgnElementId>(1);
    auto instanceId = stmt.GetValueId<ECInstanceId>(2);
    auto ecclassId = stmt.GetValueId<ECClassId>(3);
    edge.m_status = (uint8_t)stmt.GetValueInt(4);
    edge.m_priority = stmt.GetValueInt64(5);

    edge.m_instanceKey = ECInstanceKey(ecclassId, instanceId);
    edge.m_direct = 0; // this value is not in the EDE table. Default to 0!
    edge.m_isEDE = 1;

    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Graph::ElementDrivesElement::StepSelectWhereRootInDirectChanges(Edge& edge)
    {
    return SelectEdge(edge, *m_selectByRootInDirectChanges);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Graph::ElementDrivesElement::StepSelectWhereRelationshipInDirectChanges(Edge& edge)
    {
    return SelectEdge(edge, *m_selectByRelationshipInDirectChanges);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Graph::ElementDrivesElement::BindSelectByRoot(DgnElementId rootId)
    {
    m__selectByRoot__->Reset();
    m__selectByRoot__->BindId(1, rootId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Graph::ElementDrivesElement::StepSelectByRoot(Edge& edge)
    {
    return SelectEdge(edge, *m__selectByRoot__);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus TxnManager::AddChildPropagatesChangesToParentRelationship(Utf8StringCR schemaName, Utf8StringCR relClassName)
    {
    auto ecclass = GetDgnDb().Schemas().GetClass(schemaName, relClassName);
    if (nullptr == ecclass)
        {
        EDGLOG(LOG_ERROR, "%s.%s - class not found", schemaName.c_str(), relClassName.c_str());
        return BSIERROR;
        }
    if (!ecclass->IsRelationshipClass())
        {
        EDGLOG(LOG_ERROR, "%s.%s - not a relationship class", schemaName.c_str(), relClassName.c_str());
        return BSIERROR;
        }
    auto parentOwnsChildElements = GetDgnDb().Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements);
    if (nullptr == parentOwnsChildElements)
        {
        BeAssert(false);
        return BSIERROR;
        }
    if (ecclass != parentOwnsChildElements && !ecclass->Is(parentOwnsChildElements))
        {
        EDGLOG(LOG_ERROR, "%s.%s - is not a subclass of %s ", schemaName.c_str(), relClassName.c_str(), BIS_SCHEMA(BIS_REL_ElementOwnsChildElements));
        return BSIERROR;
        }

    auto ecclassid = ecclass->GetId();
    if (std::find(m_childPropagatesChangesToParentRels.begin(), m_childPropagatesChangesToParentRels.end(), ecclassid) == m_childPropagatesChangesToParentRels.end())
        m_childPropagatesChangesToParentRels.push_back(ecclassid);

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Graph::ChildPropagatesChangesToParent::ChildPropagatesChangesToParent(Graph& graph)
    :
    TableApi(graph),
    m_none(graph.m_txnMgr.GetChildPropagatesChangesToParentRelationships().empty())
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String Graph::ChildPropagatesChangesToParent::GetParentRelMatch()
    {
    auto const& relIds = GetTxnMgr().GetChildPropagatesChangesToParentRelationships();
    Utf8String matchParentRels;
    if (relIds.size() == 1)
        matchParentRels.Sprintf(" = %s", relIds.front().ToHexStr().c_str());
    else
        {
        matchParentRels = " IN (";
        Utf8CP sep = "";
        for (auto relId : relIds)
            {
            matchParentRels.append(sep).append(relId.ToHexStr());
            sep = ",";
            }
        matchParentRels.append(")");
        }
    return matchParentRels;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Graph::ChildPropagatesChangesToParent::DoPrepare()
    {
    if (m_none)
        return DbResult::BE_SQLITE_NOTFOUND;

    Utf8String matchParentRels = GetParentRelMatch();

    Utf8String sql;

    //  NB All Select statements must specify the same columns in the same order

    sql.Sprintf(
        "SELECT el.id, el.parentid, el.ParentRelECClassId "
        " FROM " BIS_TABLE(BIS_CLASS_Element) " el,"
                 TEMP_TABLE(TXN_TABLE_Elements) " direct"
        " WHERE el.id = direct.ElementId AND el.parentid != 0x1 AND el.ParentRelECClassId %s", matchParentRels.c_str()
        );

    m_selectByChildInDirectChanges = GetTxnMgr().GetTxnStatement(sql.c_str());

    sql.Sprintf(
        "SELECT id, parentid, ParentRelECClassId"
        " FROM " BIS_TABLE(BIS_CLASS_Element)
        " WHERE id = ? AND parentid != 0x1 AND ParentRelECClassId %s", matchParentRels.c_str()
        );

    m__selectByChild__ = GetTxnMgr().GetTxnStatement(sql.c_str());

    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Graph::ChildPropagatesChangesToParent::PrepareSelectAllStatement(Statement& stmt)
    {
    if (m_none)
        return;

    Utf8String matchParentRels = GetParentRelMatch();

    Utf8PrintfString sql (
        "SELECT id, parentid, ParentRelECClassId"
        " FROM " BIS_TABLE(BIS_CLASS_Element)
        " WHERE parentid != 0x1 AND ParentRelECClassId %s", matchParentRels.c_str()
    );

    stmt.Prepare(GetDgnDb(), sql.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Graph::ChildPropagatesChangesToParent::StepSelectAllStatement(Edge& edge, Statement& stmt)
    {
    if (m_none)
        return BE_SQLITE_DONE;
    return SelectEdge(edge, stmt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Graph::ChildPropagatesChangesToParent::SelectEdge(Edge& edge, Statement& stmt)
    {
    BeAssert(!m_none);

    auto stat = stmt.Step();
    if (BE_SQLITE_ROW != stat)
        return stat;

    edge.m_ein = stmt.GetValueId<DgnElementId>(0);
    edge.m_eout = stmt.GetValueId<DgnElementId>(1);
    auto ecClassId = stmt.GetValueId<ECN::ECClassId>(2);

    edge.m_instanceKey = ECInstanceKey(ecClassId, ECInstanceId(edge.m_ein.GetValue())); // use the child element's ID as the edge's instance ID
    edge.m_status = 0;
    edge.m_priority = 0;
    edge.m_direct = 0;
    edge.m_isEDE = 0;

    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Graph::ChildPropagatesChangesToParent::StepSelectWhereChildInDirectChanges(Edge& edge)
    {
    if (m_none)
        return BE_SQLITE_DONE;
    return SelectEdge(edge, *m_selectByChildInDirectChanges);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Graph::ChildPropagatesChangesToParent::BindSelectByChild(DgnElementId ChildId)
    {
    if (m_none)
        return;
    m__selectByChild__->Reset();
    m__selectByChild__->BindId(1, ChildId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult Graph::ChildPropagatesChangesToParent::StepSelectByChild(Edge& edge)
    {
    if (m_none)
        return BE_SQLITE_DONE;
    return SelectEdge(edge, *m__selectByChild__);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Graph::OnDiscoverNodes(Edge const& edge)
    {
    auto rcin = m_nodes->InsertNode(edge.m_ein);
    auto rcout = m_nodes->InsertNode(edge.m_eout);

    m_nodes->IncrementInDegree(edge.m_eout);

    if (edge.WasInputDirectlyChanged())
        {
        EDGLOG(LOG_TRACE, "PROPAGATE DIRECT %s", FmtEdge(edge).c_str());
        m_nodes->SetInputWasDirectlyChanged(edge.m_eout);
        }

    if (!BeSQLiteLib::IsConstraintDbResult(rcin))
        EDGLOG(LOG_TRACE, "DISCOVERED %s", FmtElement(edge.m_ein).c_str());
    if (!BeSQLiteLib::IsConstraintDbResult(rcout))
        EDGLOG(LOG_TRACE, "DISCOVERED %s", FmtElement(edge.m_eout).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Graph::DiscoverEdges()
    {
    auto& queue = *m_edgeQueue;
    auto& elementDrivesElement = *m_elementDrivesElement;
    auto& childPropagatesChangesToParent = *m_childPropagatesChangesToParent;

    bvector<Edge> fringe;
    bset<ECInstanceKey> edges_seen;

    //  Find edges where source or properties were directly changed
    Edge directlyChanged;
    while (elementDrivesElement.StepSelectWhereRootInDirectChanges(directlyChanged) == BE_SQLITE_ROW)
        {
        directlyChanged.SetInputWasDirectlyChanged();
        fringe.push_back(directlyChanged);
        }

    while (elementDrivesElement.StepSelectWhereRelationshipInDirectChanges(directlyChanged) == BE_SQLITE_ROW)
        fringe.push_back(directlyChanged);

    while (childPropagatesChangesToParent.StepSelectWhereChildInDirectChanges(directlyChanged) == BE_SQLITE_ROW)
        {
        directlyChanged.SetInputWasDirectlyChanged();
        fringe.push_back(directlyChanged);
        }

    // Find and schedule the EDEs that where deleted.
    for (auto deletedRel : m_txnMgr.ElementDependencies().m_deletedRels)
        {
        auto reversedEdge = std::find_if(fringe.begin(), fringe.end(), [deletedRel] (Edge const& edge) { return edge.m_ein == deletedRel.m_target && edge.m_eout == deletedRel.m_source; });
        if (fringe.end() != reversedEdge)
            {
            // Add deleted relationship's downstream dependencies
            elementDrivesElement.BindSelectByRoot(deletedRel.m_target);
            Edge affected;
            while (elementDrivesElement.StepSelectByRoot(affected) == BE_SQLITE_ROW)
                {
                fringe.push_back(affected);
                }
            continue;
            }

        Edge deletedEdge;
        deletedEdge.m_ein = deletedRel.m_source;
        deletedEdge.m_eout = deletedRel.m_target;
        deletedEdge.m_deleted = true;
        deletedEdge.m_instanceKey = deletedRel.m_relKey;
        deletedEdge.m_isEDE = deletedRel.m_isEDE;
        fringe.push_back(deletedEdge);
        }

    //  Find edges reachable from the initial set found above.
    while (!fringe.empty())
        {
        bvector<Edge> working_fringe;
        std::swap(working_fringe, fringe);
        for (auto& currEdge : working_fringe)
            {
            if (edges_seen.find(currEdge.GetKey()) != edges_seen.end())
                continue;
            edges_seen.insert(currEdge.GetKey());

            EDGLOG(LOG_TRACE, "VISIT %s", FmtEdge(currEdge).c_str());
            queue.AddEdge(currEdge);

            OnDiscoverNodes(currEdge);

            // Find all relationships with dependency handlers that take currEdge.m_eout as their input or their output.
            // Add all of those relationships to the queue
            Edge affected;
            affected.m_ein = currEdge.m_eout;

            //  The output of currEdge could be the input of other ElementDrivesElement dependencies. Those downstream dependencies must be fired as a result.
            elementDrivesElement.BindSelectByRoot(currEdge.m_eout);
            while (elementDrivesElement.StepSelectByRoot(affected) == BE_SQLITE_ROW)
                {
                if (currEdge.WasInputDirectlyChanged())
                    affected.SetInputWasDirectlyChanged();
                fringe.push_back(affected);
                }

            //  The output of currEdge could be a child element that drives its parent.
            childPropagatesChangesToParent.BindSelectByChild(currEdge.m_eout);
            while (childPropagatesChangesToParent.StepSelectByChild(affected) == BE_SQLITE_ROW)
                {
                if (currEdge.WasInputDirectlyChanged())
                    affected.SetInputWasDirectlyChanged();
                fringe.push_back(affected);
                EDGLOG(LOG_TRACE, "FOUND PARENT EDGE %s", FmtEdge(affected).c_str());
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Graph::InvokeHandlersInDependencyOrder()
    {
    EDGLOG(LOG_TRACE, "-----------------InvokeHandlersInDependencyOrder---------------------");

    EdgeQueue queue(*this);
    Nodes nodes(*this);
    ElementDrivesElement elementDrivesElement(*this);
    elementDrivesElement.DoPrepare();

    ChildPropagatesChangesToParent childPropagatesChangesToParent(*this);
    childPropagatesChangesToParent.DoPrepare();

    m_edgeQueue = &queue;
    m_nodes = &nodes;
    m_elementDrivesElement = &elementDrivesElement;
    m_childPropagatesChangesToParent = &childPropagatesChangesToParent;

    DiscoverEdges(); // populates m_edgeQueue

    InvokeHandlersInTopologicalOrder(); // searches m_edgeQueue

    // NotifySharedAndDirectChanges();

    m_elementDrivesElement = nullptr;
    m_childPropagatesChangesToParent = nullptr;
    m_edgeQueue = nullptr;
    m_nodes = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Graph::InvokeAffectedDependencyHandlers()
    {
    EDGLOG(LOG_TRACE, "--------------------------------------------------");

    if (s_debugGraph > 1)
        WriteDot(BeFileName(L"D:\\tmp\\ChangePropagationRelationships.dot"), bvector<bvector<uint64_t>>());

    InvokeHandlersInDependencyOrder();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Graph::WriteAffectedGraphToFile(BeFileNameCR dotFilename, bvector<DgnElementId> const& directlyChangedElements, bvector<ECInstanceId> const& directlyChangedDepRels)
    {
    m_txnMgr.OnBeginValidate();

    auto& txnElements = m_txnMgr.Elements();
    auto& elements = GetDgnDb().Elements();
    for (DgnElementId elementId : directlyChangedElements)
        {
        auto el = elements.GetElement(elementId);
        if (el.IsValid())
            txnElements.AddElement(elementId, el->GetModelId(), TxnTable::ChangeType::Update, el->GetElementClassId(), false);
        }

    auto& dependencies = m_txnMgr.ElementDependencies();
    for (ECInstanceId deprel : directlyChangedDepRels)
        dependencies.AddDependency(deprel, TxnTable::ChangeType::Update);

    EdgeQueue queue(*this);
    Nodes nodes(*this);
    ElementDrivesElement elementDrivesElement(*this);
    elementDrivesElement.DoPrepare();

    ChildPropagatesChangesToParent childPropagatesChangesToParent(*this);
    childPropagatesChangesToParent.DoPrepare();

    m_edgeQueue = &queue;
    m_nodes = &nodes;
    m_elementDrivesElement = &elementDrivesElement;
    m_childPropagatesChangesToParent = &childPropagatesChangesToParent;

    DiscoverEdges(); // populates m_edgeQueue

    WriteEdgeQueueToDot(dotFilename); // processes m_edgeQueue

    m_elementDrivesElement = nullptr;
    m_childPropagatesChangesToParent = nullptr;
    m_edgeQueue = nullptr;
    m_nodes = nullptr;

    m_txnMgr.OnEndValidate();
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Graph::Init()
    {
    m_elementDrivesElement = nullptr;
    m_jsTxns = GetDgnDb().GetJsTxns();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Graph::Graph(TxnManager& mgr) : m_txnMgr(mgr)
    {
    Init();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Graph::SetElementDrivesElementPriority(ECInstanceId relid, int64_t newPriority)
    {
    CachedStatementPtr updatePriority = m_txnMgr.GetTxnStatement("UPDATE " BIS_TABLE(BIS_REL_ElementDrivesElement) " SET Priority=? WHERE Id=?");
    updatePriority->BindInt64(1, newPriority);
    updatePriority->BindId(2, relid);
    return updatePriority->Step() == BE_SQLITE_DONE ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Graph::SetEdgeDeferred(Edge& edge, bool isDeferred)
    {
    EdgeStatusAccessor accs(edge.m_status);
    accs.m_deferred = isDeferred;
    edge.m_status = (uint8_t)accs.ToEdgeStatus();
    return UpdateEdgeStatusInDb(edge, accs.ToEdgeStatus()) == BE_SQLITE_DONE ? BSISUCCESS : BSIERROR;
    }

// ----------------------------------------------------------------------------------------------
// Future: EDE.onValidateOutput
//          Element.onDirectlyChanged
// ----------------------------------------------------------------------------------------------

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
// Edge Graph::QueryEdgeByInstanceKey(ECInstanceKeyCR key)
//     {
//     Edge edge;
//     auto ecclass = GetDgnDb().Schemas().GetClass(key.GetClassId());
//     if (ecclass->Is(GetDgnDb().Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_REL_ElementDrivesElement)))
//         {
//         ElementDrivesElement deprel(*this);
//         deprel.SelectEdgeByRelId(edge, key.GetInstanceId());
//         }
//     else
//         {
//         ChildPropagatesChangesToParent childPropagatesChangesToParent(*this);
//         childPropagatesChangesToParent.BindSelectByChild(DgnElementId(key.GetInstanceId().GetValue()));
//         childPropagatesChangesToParent.StepSelectByChild(edge);
//         }
//     return edge;
//     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+------c---------+---------------+------*/
// bool Graph::EdgeQueue::HasEdge(ECInstanceId edgeId)
//     {
//     m_getEdgeColor->Reset();
//     m_getEdgeColor->BindId(1, edgeId);
//     return BE_SQLITE_ROW == m_getEdgeColor->Step();
//     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
// bset<ECInstanceKeyCR> Graph::GetEdgesThatOutputElement(DgnElementId eid, BeSQLite::Statement& findEdeByTarget)
//     {
//     bset<ECInstanceKeyCR> rels;
//     findEdeByTarget.Reset();
//     findEdeByTarget.BindId(1, eid);
//     while (BE_SQLITE_ROW == findEdeByTarget.Step()) // Find all EDEs (in or out of the graph) that output the same element
//         {
//         auto relId = findEdeByTarget.GetValueId<ECInstanceKeyCR>(0);
//         // EDGLOG(LOG_TRACE, "Edge %s", FmtEdge(QueryEdgeByInstanceKey(relId)).c_str());
//         rels.insert(relId);
//         }
//     return rels;
//     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
// DbResult Graph::ElementDrivesElement::SelectEdgeByRelId(Edge& edge, ECInstanceId eid)
//     {
//     CachedStatementPtr selectByRelId = GetTxnMgr().GetTxnStatement(
//         "SELECT SourceId,TargetId,Id as relid,ECClassId,Status,Priority FROM " BIS_TABLE(BIS_REL_ElementDrivesElement) " WHERE Id=?");

//     selectByRelId->BindId(1, eid);
//     return SelectEdge(edge, *selectByRelId);
//     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
// void Graph::InvokeHandlerForValidation(Edge const& edge)
//     {
//     if (!edge.m_isEDE)
//          return;
//     int errorCount = m_txnMgr.NumValidationErrors();
//     getEdeHandler(*this, edge)._OnValidateOutput(*this, edge);
//     callJsEdgeFunc(*this, edge,  "_onValidateOutput");
//     SetFailedEdgeStatusInDb(edge, (m_txnMgr.NumValidationErrors() > errorCount));
//     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
// void Graph::ValidateRels(bset<ECInstanceKeyCR> const& relIds)
//     {
//     for (auto relId : relIds)
//         {
//         if (m_edgeQueue->HasEdge(relId)) // If the edge is in the graph, then it got an OnRootsChanged callback.
//             continue;                    // Don't give it an additional onValidate callback.
//         auto rel = QueryEdgeByInstanceKey(relId);
//         InvokeHandlerForValidation(rel);
//         }
//     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
// void Graph::NotifySharedAndDirectChanges()
//     {
//     Statement findEdeByTarget;
//     findEdeByTarget.Prepare(GetDgnDb(), "SELECT Id FROM " BIS_TABLE(BIS_REL_ElementDrivesElement) " WHERE TargetId=?");

//     DgnElementIdSet outputsSeen;

//     Statement allDirect;
//     allDirect.Prepare(GetDgnDb(), "SELECT ElementId FROM " TEMP_TABLE(TXN_TABLE_Elements));
//     while (BE_SQLITE_ROW == allDirect.Step())
//         {
//         auto eid = allDirect.GetValueId<DgnElementId>(0);
//         outputsSeen.insert(eid);

//         // See if the element is involved in the graph.
//         auto rels = GetEdgesThatOutputElement(eid, findEdeByTarget);
//         if (rels.empty())
//             continue;

//         // If the output of an EDE is directly changed, the EDE must be notified to validate the element's state.
//         // EDGLOG(LOG_TRACE, "Directly changed Node %s", FmtElement(eid).c_str());
//         ValidateRels(rels);

//         if (!m_nodes->WasInputDirectlyChanged(eid)) // If an element was directly changed and had no changed inputs, the element itself should be notified
//             {
//             callJsDriverFunc(*this, eid, "_onDirectChange");
//             for (auto relId: rels)
//                  {
//                  if (m_edgeQueue->HasEdge(relId)) // If the edge is in the graph, then its input got a normal callback.
//                      continue;
//                  auto rel = QueryEdgeByInstanceKey(relId);
//                  callJsDriverFunc(*this, rel.m_ein, "_onOutputDirectlyChanged");
//                  }
//             }
//         }

//     Statement allTargetsInGraph;
//     allTargetsInGraph.Prepare(GetDgnDb(), "SELECT TargetId [Id] FROM " EDGE_QUEUE);
//     while (BE_SQLITE_ROW == allTargetsInGraph.Step())
//         {
//         auto eid = allTargetsInGraph.GetValueId<DgnElementId>(0);
//         if (!outputsSeen.insert(eid).second)
//             continue;

//         // EDGLOG(LOG_TRACE, "Target of edge %s", FmtElement(eid).c_str());

//         auto rels = GetEdgesThatOutputElement(eid, findEdeByTarget);

//         // If more two or more EDEs output the same element, any that are not in the graph must get a validate callback
//         if (rels.size() <= 1)
//             continue;

//         ValidateRels(rels);
//         }
//     }
