/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ElementDependencyGraph.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#define ECSQL_SOURCEECINSTANCEID    "SourceECInstanceId"
#define ECSQL_ECINSTANCEID          "ECInstanceId"

#define EDGE_QUEUE_TABLE_NAME  "TxnEdgeQueue"
#define EDGE_QUEUE TEMP_TABLE(EDGE_QUEUE_TABLE_NAME)

#define NODES_TABLE_NAME "TxnNodes"
#define NODES TEMP_TABLE(NODES_TABLE_NAME)

DPILOG_DEFINE(ElementDependencyGraph)

#define EDGLOG(sev,...) {if (ElementDependencyGraph_getLogger().isSeverityEnabled(sev)) { ElementDependencyGraph_getLogger().messagev(sev, __VA_ARGS__); }}

static int s_debugGraph = 0;
static bool s_debugGraph_showElementIds;


BEGIN_BENTLEY_DGN_NAMESPACE

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

    explicit EdgeStatusAccessor(DgnElementDependencyGraph::EdgeStatus s) : m_flags((uint32_t)s) {}
    explicit EdgeStatusAccessor(uint32_t s) : m_flags(s) {}
    DgnElementDependencyGraph::EdgeStatus ToEdgeStatus() const {return (DgnElementDependencyGraph::EdgeStatus)m_flags;}
};

//=======================================================================================
// @bsiclass                                                    Sam.Wilson  01/15
//=======================================================================================
struct DgnElementDependencyGraph::TableApi
{
    DgnElementDependencyGraph& m_graph;
    TableApi(DgnElementDependencyGraph& g) : m_graph(g) {}
    DgnDbR GetDgnDb() const {return m_graph.GetDgnDb();}
    TxnManager& GetTxnMgr() const {return m_graph.m_txnMgr;}
    virtual DbResult UpdateEdgeStatusInDb(Edge const& edge, EdgeStatus newStatus) {return BE_SQLITE_OK;}
};

//=======================================================================================
//  Wrapper for the ElementDrivesElement ECRelationship
// @bsiclass                                                    Sam.Wilson  01/15
//=======================================================================================
struct DgnElementDependencyGraph::ElementDrivesElement : DgnElementDependencyGraph::TableApi
    {
    private:
    CachedStatementPtr m_selectByRootInDirectChanges, m_selectByDependentInDirectChanges, m_selectByRelationshipInDirectChanges, m__selectByRoot__, m__selectByDependent__, m__updateStatus__;

    DbResult SelectEdge(DgnElementDependencyGraph::Edge&, Statement&);

    public:
    ElementDrivesElement(DgnElementDependencyGraph&);
        
    DbResult DoPrepare();

    DbResult StepSelectWhereRootInDirectChanges(Edge&);
    DbResult StepSelectWhereDependentInDirectChanges(Edge&);
    DbResult StepSelectWhereRelationshipInDirectChanges(Edge&);
    void BindSelectByRoot(DgnElementId);
    DbResult StepSelectByRoot(Edge&);
    void BindSelectByDependent(DgnElementId);
    DbResult StepSelectByDependent(Edge&);
    DbResult SelectEdgeByRelId(Edge&, ECInstanceId);
    DbResult UpdateEdgeStatusInDb(Edge const& edge, EdgeStatus newStatus);
    };

//=======================================================================================
//  The queue of all element dependency graph edges found.
// @bsiclass                                                    Sam.Wilson  01/15
//=======================================================================================
struct DgnElementDependencyGraph::EdgeQueue : DgnElementDependencyGraph::TableApi
    {
    private:
    CachedStatementPtr m_insert, m_select, m_selbyp, m_selbyo, m_selbyso, m__updateStatus__;
    CachedStatementPtr m_selectNodeIds, m_countInputs;
    CachedStatementPtr m_setEdgeColor, m_getEdgeColor, m_setHaveSharedOutput;
    Statement m_checkPathStmt;

    DbResult SelectEdge(DgnElementDependencyGraph::Edge&, Statement&);

    public:
    EdgeQueue(DgnElementDependencyGraph& g);
    ~EdgeQueue();
    DbResult AddEdge(DgnElementDependencyGraph::Edge&);
    DbResult StepSelectAll(DgnElementDependencyGraph::Edge& edge) {return SelectEdge(edge, *m_select);}
    void ResetSelectAll() { m_select->Reset(); }

    void ResetSelectAllOrderedByPriority() {m_selbyp->Reset();}
    DbResult StepSelectAllOrderedByPriority(DgnElementDependencyGraph::Edge& edge) {return SelectEdge(edge, *m_selbyp);}
    DbResult BindSelectByOutput(DgnElementId);
    DbResult StepSelectByOutput(Edge& edge) {return SelectEdge(edge, *m_selbyo);}
    EdgeColor GetEdgeColor(ECInstanceId);
    void SetEdgeColor(ECInstanceId, EdgeColor);
    void SetHaveSharedOutput(ECInstanceId);
    void ResetSelectHaveSharedOutput() {m_selbyso->Reset();}
    DbResult StepSelectHaveSharedOutput(DgnElementDependencyGraph::Edge& edge) {return SelectEdge(edge, *m_selbyso);}

    int GetNodeInputCount(DgnElementId);
    bset<DgnElementId> GetAllNodeIds();
    };

//=======================================================================================
//  The collection of nodes found in element dependency graph.
// @bsiclass                                              Mindaugas.Butkus  04/18
//=======================================================================================
struct DgnElementDependencyGraph::Nodes : DgnElementDependencyGraph::TableApi
    {
    DEFINE_T_SUPER(DgnElementDependencyGraph::TableApi)

    private:
        CachedStatementPtr m_insert, m_allInputsProcessed, m_anyOutputsProcessed, m_selectInDegree;
        CachedStatementPtr m_setInDegree, m_incrementInputsProcessed, m_incrementOutputsProcessed;

    public:
        Nodes(DgnElementDependencyGraph& g);
        ~Nodes();

        DbResult InsertNode(DgnElementId nodeId);
        DbResult SetInDegree(DgnElementId nodeId, size_t newInDegree);
        int GetInDegree(DgnElementId);

        //! Increment the processed inputs counter. Input here is an edge ending at the given node.
        //! @param[in] nodeId   Node that the input (edge) points to.
        DbResult IncrementInputsProcessed(DgnElementId nodeId);

        //! Increment the processed outputs counter. Output here is an edge starting at the given node.
        //! @param[in] nodeId   Node that the output (edge) points from (start at).
        DbResult IncrementOutputsProcessed(DgnElementId nodeId);

        //! Check if all inputs to the given node are processed.
        bool AllInputsProcessed(DgnElementId);

        //! Check if any outputs of the given node are processed.
        bool AnyOutputsProcessed(DgnElementId);
    };

END_BENTLEY_DGN_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mindaugas.Butkus                04/18
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementDependencyGraph::Nodes::Nodes(DgnElementDependencyGraph& g)
    : T_Super(g)
    {
    if (!GetDgnDb().TableExists(NODES))
        {
        GetDgnDb().CreateTable(NODES, "ElementId INTEGER PRIMARY KEY, InDegree INTEGER DEFAULT 0, InputsProcessed INTEGER DEFAULT 0, OutputsProcessed INTEGER DEFAULT 0");
        }

    GetDgnDb().GetCachedStatement(m_insert, "INSERT INTO " NODES " (ElementId) VALUES(?)");
    GetDgnDb().GetCachedStatement(m_setInDegree, "UPDATE " NODES " SET InDegree=? WHERE ElementId=?");
    GetDgnDb().GetCachedStatement(m_selectInDegree, "SELECT InDegree FROM " NODES " WHERE ElementId=?");
    GetDgnDb().GetCachedStatement(m_allInputsProcessed, "SELECT EXISTS(SELECT 1 FROM " NODES " WHERE ElementId=? AND InDegree=InputsProcessed LIMIT 1)");
    GetDgnDb().GetCachedStatement(m_anyOutputsProcessed, "SELECT EXISTS(SELECT 1 FROM " NODES " WHERE ElementId=? AND OutputsProcessed!=0 LIMIT 1)");
    GetDgnDb().GetCachedStatement(m_incrementInputsProcessed, "UPDATE " NODES " SET InputsProcessed=InputsProcessed+1 WHERE ElementId=?");
    GetDgnDb().GetCachedStatement(m_incrementOutputsProcessed, "UPDATE " NODES " SET OutputsProcessed=OutputsProcessed+1 WHERE ElementId=?");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mindaugas.Butkus                04/18
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementDependencyGraph::Nodes::~Nodes()
    {
    GetDgnDb().ExecuteSql("DELETE FROM " NODES);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mindaugas.Butkus                04/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnElementDependencyGraph::Nodes::AllInputsProcessed(DgnElementId nodeId)
    {
    BeAssert(nodeId.IsValid());

    m_allInputsProcessed->Reset();
    m_allInputsProcessed->BindId(1, nodeId);

    auto stat = m_allInputsProcessed->Step();
    if (BE_SQLITE_ROW != stat)
        return false;

    return m_allInputsProcessed->GetValueBoolean(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mindaugas.Butkus                04/18
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnElementDependencyGraph::Nodes::AnyOutputsProcessed(DgnElementId nodeId)
    {
    BeAssert(nodeId.IsValid());

    m_anyOutputsProcessed->Reset();
    m_anyOutputsProcessed->BindId(1, nodeId);

    auto stat = m_anyOutputsProcessed->Step();
    return (BE_SQLITE_ROW != stat) ?  false :  m_anyOutputsProcessed->GetValueBoolean(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mindaugas.Butkus                04/18
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnElementDependencyGraph::Nodes::InsertNode(DgnElementId nodeId)
    {
    BeAssert(nodeId.IsValid());

    m_insert->Reset();
    m_insert->BindId(1, nodeId);
    return m_insert->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mindaugas.Butkus                04/18
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnElementDependencyGraph::Nodes::SetInDegree(DgnElementId nodeId, size_t inDegree)
    {
    BeAssert(nodeId.IsValid());

    m_setInDegree->Reset();
    m_setInDegree->BindInt(1, static_cast<int>(inDegree));
    m_setInDegree->BindId(2, nodeId);
    return m_setInDegree->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mindaugas.Butkus                04/18
+---------------+---------------+---------------+---------------+---------------+------*/
int DgnElementDependencyGraph::Nodes::GetInDegree(DgnElementId nodeId)
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
* @bsimethod                                    Mindaugas.Butkus                04/18
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnElementDependencyGraph::Nodes::IncrementInputsProcessed(DgnElementId nodeId)
    {
    BeAssert(nodeId.IsValid());

    m_incrementInputsProcessed->Reset();
    m_incrementInputsProcessed->BindId(1, nodeId);
    return m_incrementInputsProcessed->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mindaugas.Butkus                04/18
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnElementDependencyGraph::Nodes::IncrementOutputsProcessed(DgnElementId nodeId)
    {
    BeAssert(nodeId.IsValid());

    m_incrementOutputsProcessed->Reset();
    m_incrementOutputsProcessed->BindId(1, nodeId);
    return m_incrementOutputsProcessed->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnElementDependencyGraph::FmtEdge(Edge const& edge)
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
    return s;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnElementDependencyGraph::FmtEdges(bvector<Edge> const& edges)
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
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnElementDependencyGraph::FmtCyclePath(Edge const& edge, bvector<Edge> const& pathToEdge)
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
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnElementDependencyGraph::GetElementCode(DgnElementId eid)
    {
    ECSqlStatement s;
    s.Prepare(GetDgnDb(), "SELECT CodeValue FROM " BIS_SCHEMA(BIS_CLASS_Element) " WHERE ECInstanceId=?");
    s.BindId(1, eid);
    s.Step();
    return s.GetValueText(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnElementDependencyGraph::FmtElement(DgnElementId eid)
    {
    if (s_debugGraph_showElementIds)
        return Utf8PrintfString("%s(%lld)", GetElementCode(eid).c_str(), eid.GetValue());

    return GetElementCode(eid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnElementDependencyGraph::FmtElementPath(Utf8CP epath)
    {
    Utf8String path;

    Utf8CP start = epath;
    Utf8CP dot = epath;
    for (;;)
        {
        if ('.' == *dot || 0 == *dot)
            {
            uint64_t id;
            if (sscanf(start, "%" SCNu64, &id) == 1)
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
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnElementDependencyGraph::FmtRel(Edge const& edge)
    {
    return Utf8PrintfString("D%lld", edge.m_relId.GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2005
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElementDependencyGraph::WriteDot(BeFileNameCR dotFilename, bvector<bvector<uint64_t>> const& cyclePaths)
    {
    auto fp = fopen(Utf8String(dotFilename).c_str(), "w+");
    if (NULL == fp)
        return;

    fwprintf(fp, L"digraph G {\n");

    Statement edges;
    edges.Prepare(GetDgnDb(), "SELECT SourceId,TargetId,Id,Status FROM " BIS_TABLE(BIS_REL_ElementDrivesElement));

    while (edges.Step() == BE_SQLITE_ROW)
        {
        Edge edge;
        edge.m_ein   = edges.GetValueId<DgnElementId>(0);
        edge.m_eout  = edges.GetValueId<DgnElementId>(1);
        edge.m_relId = edges.GetValueId<ECInstanceId>(2);
        edge.m_status = (uint8_t)edges.GetValueInt(3);

        if (!edge.m_ein.IsValid() || !edge.m_relId.IsValid())
            continue;

        auto incode  = GetValidDotName(GetElementCode(edge.m_ein));
        auto outcode = GetValidDotName(GetElementCode(edge.m_eout));
        auto reldesc = GetValidDotName(FmtRel(edge));

        fprintf(fp, "%s -> %s [label=%s]", incode.c_str(), outcode.c_str(), reldesc.c_str());

        /*
        uint32_t cycleId;
        if (isInCyclePath (cycleId, inid, outid, cyclePaths))
            fprintf (fp, "[label=%d color=red fontcolor=red]", cycleId);
            */

        fprintf(fp, ";\n");
        }

    fwprintf(fp, L"}\n");

    fclose(fp);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnElementDependencyGraph::CheckDirection(Edge const& edge)
    {
    /* *** WIP_DEPGRAPH_MODEL_TREE  */
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::ElementDep::_PropagateChanges()
    {
    DgnElementDependencyGraph graph(m_txnMgr);
    graph.InvokeAffectedDependencyHandlers();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElementDependencyGraph::LogDependencyFound(Statement& stmt, Edge const& edge)
    {
    if (!ElementDependencyGraph_getLogger().isSeverityEnabled(LOG_TRACE)) 
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
 @bsimethod                                    Keith.Bentley                    11/18
+---------------+---------------+---------------+---------------+---------------+------*/
Napi::Object DgnElementDependencyGraph::EdgeToRelProps(Edge const& edge) {
    Napi::Object props = Napi::Object::New(m_jsTxns.Env());

    DgnDbR db = GetDgnDb();
    auto relClass = db.Schemas().GetClass(edge.GetRelClassId());
    if (relClass != nullptr) 
        props["classFullName"] = db.ToJsString(relClass->GetFullName());
    props["id"] = db.ToJsString(edge.m_relId);
    props["sourceId"] = db.ToJsString(edge.m_ein);
    props["targetId"] = db.ToJsString(edge.m_eout);
    return props;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElementDependencyGraph::InvokeHandler(Edge const& edge, size_t indentLevel)
    {
    if (m_jsTxns == nullptr)  // no dependency propagation
        return;

    DgnDbR db = GetDgnDb();
    size_t errorCount = m_txnMgr.NumValidationErrors();
    if (!m_nodes->AnyOutputsProcessed(edge.m_ein) && m_nodes->GetInDegree(edge.m_ein) == 0)
        DgnDb::CallJsFunction(m_jsTxns, "_onBeforeOutputsHandled", {db.GetJsClassName(edge.m_ein), db.ToJsString(edge.m_ein)});

    if (!edge.IsDeleted())
        DgnDb::CallJsFunction(m_jsTxns, "_onRootChanged", {EdgeToRelProps(edge)});
    else
        InvokeHandlerForDeletedRelationship(edge.m_relId);
    
    m_nodes->IncrementOutputsProcessed(edge.m_ein);
    m_nodes->IncrementInputsProcessed(edge.m_eout);

    if (m_nodes->AllInputsProcessed(edge.m_eout))
        DgnDb::CallJsFunction(m_jsTxns, "_onAllInputsHandled", {db.GetJsClassName(edge.m_eout), db.ToJsString(edge.m_eout)});

    SetFailedEdgeStatusInDb(edge, (m_txnMgr.NumValidationErrors() > errorCount));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElementDependencyGraph::InvokeHandlerForValidation(Edge const& edge)
    {
    if (m_jsTxns == nullptr)  // no dependency propagation
        return;

    int errorCount = m_txnMgr.NumValidationErrors();
    DgnDb::CallJsFunction(m_jsTxns, "_onValidateOutput", { EdgeToRelProps(edge) });
    SetFailedEdgeStatusInDb(edge, (m_txnMgr.NumValidationErrors() > errorCount));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnElementDependencyGraph::UpdateEdgeStatusInDb(Edge const& edge, EdgeStatus newStatus) 
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
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnElementDependencyGraph::SetFailedEdgeStatusInDb(Edge const& edge, bool failed) 
    {
    if (failed)
        m_txnMgr.ElementDependencies().AddFailedTarget(edge.m_eout);

    EdgeStatusAccessor saccs(edge.m_status);
    saccs.m_failed = failed;        // set or clear the failed bit
    EdgeStatus newStatus = saccs.ToEdgeStatus();

    return UpdateEdgeStatusInDb(edge, newStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementDependencyGraph::EdgeQueue::~EdgeQueue() 
    {
    GetDgnDb().ExecuteSql("DELETE FROM " EDGE_QUEUE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementDependencyGraph::EdgeQueue::EdgeQueue(DgnElementDependencyGraph& graph) 
    : 
    DgnElementDependencyGraph::TableApi(graph)
    {
    if (!GetDgnDb().TableExists(EDGE_QUEUE))
        {                                                                                                   // NB default color must be 0, i.e., EdgeColor::White
        GetDgnDb().CreateTable(EDGE_QUEUE, "ECRelationshipId INTEGER PRIMARY KEY, SourceId INTEGER, TargetId INTEGER, ECClassId INTEGER, status INTEGER DEFAULT 0, Priority INTEGER DEFAULT 0, shared_output INTEGER DEFAULT 0, color INTEGER DEFAULT 0, Deleted INTEGER DEFAULT 0");
        // The value of column 'Deleted' determines which method to call on the handler: 
        //  0 - call _OnRootChanged
        //  1 - call _ProcessDeletedDependency

        // *** WIP_GRAPH_PRIORITY
        //      -- currently, the Priority column is NULL unless the user happens to populate it. 
        //          That is a problem, as we want the default priority to be order of creation.
        //      -- must change dgn.0.02.ecschema.xml to put an index on the priority column, so that ORDER BY Priority will work
        }

    GetDgnDb().GetCachedStatement(m_insert, "INSERT INTO " EDGE_QUEUE " (ECRelationshipId,SourceId,TargetId,ECClassId,status,Priority,Deleted) VALUES(?,?,?,?,?,?,?)");
    GetDgnDb().GetCachedStatement(m_setHaveSharedOutput, "UPDATE " EDGE_QUEUE " SET shared_output=1 WHERE ECRelationshipId=?");
    GetDgnDb().GetCachedStatement(m_setEdgeColor, "UPDATE " EDGE_QUEUE " SET color=? WHERE ECRelationshipId=?");
    GetDgnDb().GetCachedStatement(m_getEdgeColor, "SELECT color FROM " EDGE_QUEUE " WHERE ECRelationshipId=?");

    // NB: All select statements must specify the same columns and in the same order, as they are all processed by a single function, "SelectEdge"

                  // 0                1        2        3         4      5        6
#define Q_SEL_COLS " ECRelationshipId,SourceId,TargetId,ECClassId,status,Priority,Deleted"
#define Q_TABLES   " " EDGE_QUEUE " "

    GetDgnDb().GetCachedStatement(m_select, "SELECT " Q_SEL_COLS " FROM " Q_TABLES);
    GetDgnDb().GetCachedStatement(m_selbyp, "SELECT " Q_SEL_COLS " FROM " Q_TABLES " ORDER BY Priority DESC");
    GetDgnDb().GetCachedStatement(m_selbyo, "SELECT " Q_SEL_COLS " FROM " Q_TABLES " WHERE (TargetId=?) ORDER BY Priority DESC");
    GetDgnDb().GetCachedStatement(m_selbyso,"SELECT " Q_SEL_COLS " FROM " Q_TABLES " WHERE (shared_output > 1)");
    GetDgnDb().GetCachedStatement(m_selectNodeIds, "SELECT NodeId FROM (SELECT SourceId AS NodeId FROM " EDGE_QUEUE " UNION SELECT TargetId AS NodeId FROM " EDGE_QUEUE ")");
    GetDgnDb().GetCachedStatement(m_countInputs, "SELECT COUNT(TargetId) FROM " EDGE_QUEUE " WHERE TargetId=?");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mindaugas.Butkus                04/18
+---------------+---------------+---------------+---------------+---------------+------*/
bset<DgnElementId> DgnElementDependencyGraph::EdgeQueue::GetAllNodeIds()
    {
    bset<DgnElementId> nodeIds;
    m_selectNodeIds->Reset();

    while (BE_SQLITE_ROW == m_selectNodeIds->Step())
        nodeIds.insert(m_selectNodeIds->GetValueId<DgnElementId>(0));

    return nodeIds;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mindaugas.Butkus                04/18
+---------------+---------------+---------------+---------------+---------------+------*/
int DgnElementDependencyGraph::EdgeQueue::GetNodeInputCount(DgnElementId nodeId)
    {
    m_countInputs->Reset();
    m_countInputs->BindId(1, nodeId);
    auto stat = m_countInputs->Step();
    if (BE_SQLITE_ROW != stat)
        return 0;

    int count = m_countInputs->GetValueInt(0);
    return (count < 0) ?  0 :  count;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnElementDependencyGraph::EdgeQueue::SelectEdge(DgnElementDependencyGraph::Edge& edge, Statement& stmt)
    {
    auto stat = stmt.Step();
    if (BE_SQLITE_ROW != stat)
        return stat;

    edge.m_relId = stmt.GetValueId<ECInstanceId>(0);
    edge.m_ein = stmt.GetValueId<DgnElementId>(1);
    edge.m_eout = stmt.GetValueId<DgnElementId>(2);
    edge.m_relClassId = stmt.GetValueId<ECClassId>(3);
    edge.m_status = stmt.GetValueInt(4);
    edge.m_priority = stmt.GetValueInt64(5);
    edge.m_deleted = stmt.GetValueBoolean(6);

    BeAssert(edge.IsValid());
    return BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnElementDependencyGraph::EdgeQueue::BindSelectByOutput(DgnElementId eout)
    {
    m_selbyo->Reset();
    return m_selbyo->BindId(1, eout);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnElementDependencyGraph::EdgeQueue::AddEdge(DgnElementDependencyGraph::Edge& edge)
    {
    // Don't add the edge to the graph if it's deferred
    if (edge.IsDeferred())
        return BE_SQLITE_DONE;

    BeAssert(edge.m_ein.IsValid() && edge.m_eout.IsValid() && edge.m_relId.IsValid());

    //  Add edge+path to queue
    m_insert->Reset();
    m_insert->BindId(1, edge.m_relId);
    m_insert->BindId(2, edge.m_ein);
    m_insert->BindId(3, edge.m_eout);
    m_insert->BindId(4, edge.m_relClassId);
    m_insert->BindInt(5, edge.m_status);
    m_insert->BindInt(6, static_cast<int>(edge.m_priority));
    m_insert->BindBoolean(7, edge.m_deleted);
    auto stat = m_insert->Step();

    if (BeSQLiteLib::IsConstraintDbResult(stat))
        {
        //  We already discovered this edge. Nothing to do.
        EDGLOG(LOG_TRACE, "(KNOWN %s)", m_graph.FmtEdge(edge).c_str());
        return stat;
        }

    if (m_graph.CheckDirection(edge) != BSISUCCESS)
        return BE_SQLITE_DONE;

    EDGLOG(LOG_TRACE, "FOUND %s", m_graph.FmtEdge(edge).c_str());

    BeAssert(edge.IsValid());
    return BE_SQLITE_DONE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementDependencyGraph::EdgeColor DgnElementDependencyGraph::EdgeQueue::GetEdgeColor(ECInstanceId edgeId)
    {
    m_getEdgeColor->Reset();
    m_getEdgeColor->BindId(1, edgeId);
    if (m_getEdgeColor->Step() != BE_SQLITE_ROW)
        return EdgeColor::White;
    return (EdgeColor)m_getEdgeColor->GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+------c---------+---------------+------*/
void DgnElementDependencyGraph::EdgeQueue::SetHaveSharedOutput(ECInstanceId edgeId)
    {
    m_setHaveSharedOutput->Reset();
    m_setHaveSharedOutput->BindId(1, edgeId);
    auto stat = m_setEdgeColor->Step();
    BeAssert(stat == BE_SQLITE_DONE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+------c---------+---------------+------*/
void DgnElementDependencyGraph::EdgeQueue::SetEdgeColor(ECInstanceId edgeId, EdgeColor color)
    {
    m_setEdgeColor->Reset();
    m_setEdgeColor->BindInt(1,(int)color);
    m_setEdgeColor->BindId(2, edgeId);
    auto stat = m_setEdgeColor->Step();
    BeAssert(stat == BE_SQLITE_DONE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElementDependencyGraph::InvokeHandlersInTopologicalOrder_OneGraph(Edge const& edge, bvector<Edge> const& pathToSupplier)
    {
    auto& edges = *m_edgeQueue;

    //  Detect if we have already processed this edge ... or are in the middle of processing it.
    auto color = edges.GetEdgeColor(edge.GetECRelationshipId());
    if (color != EdgeColor::White)
        {
        if (color == EdgeColor::Gray)
            {
            m_txnMgr.ReportError(true, "cycle", FmtCyclePath(edge, pathToSupplier).c_str());
            SetFailedEdgeStatusInDb(edge, true); // mark at least this edge as failed. maybe we should mark the entire cycle??
            }
        return;
        }

    edges.SetEdgeColor(edge.GetECRelationshipId(), EdgeColor::Gray);

    // Schedule suppliers of edge's input first.
    auto pathToEdge(pathToSupplier);
    pathToEdge.push_back(edge);

    edges.BindSelectByOutput(edge.m_ein);      // I want find edges that OUTPUT my input

    bvector<Edge> suppliers;
    Edge supplier;
    while (edges.StepSelectByOutput(supplier) == BE_SQLITE_ROW)
        suppliers.push_back(supplier);

    for (auto const& supplier : suppliers)
        InvokeHandlersInTopologicalOrder_OneGraph(supplier, pathToEdge);

    //  edge can now be fired.
    edges.SetEdgeColor(edge.GetECRelationshipId(), EdgeColor::Black);

    InvokeHandler(edge, pathToSupplier.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElementDependencyGraph::InvokeHandlerForDeletedRelationship(ECInstanceId relId)
    {
    auto const& deletedRels = m_txnMgr.ElementDependencies().m_deletedRels;
    auto deletedRel = std::find_if(deletedRels.begin(), deletedRels.end(), [relId] (auto const& depRel) {return depRel.m_relKey.GetInstanceId() == relId;});
    if (deletedRels.end() == deletedRel)
        return;

    Edge edge;
    edge.m_ein = deletedRel->m_source;
    edge.m_eout = deletedRel->m_target;
    edge.m_relClassId = deletedRel->m_relKey.GetClassId();
    edge.m_relId = deletedRel->m_relKey.GetInstanceId();
    DgnDb::CallJsFunction(m_jsTxns, "_onDeletedDependency", {EdgeToRelProps(edge)});
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElementDependencyGraph::InvokeHandlersInTopologicalOrder() 
    {
    // This is a total topological sort of the Edge queue.
    m_edgeQueue->ResetSelectAllOrderedByPriority();
    Edge edge;
    while (m_edgeQueue->StepSelectAllOrderedByPriority(edge) == BE_SQLITE_ROW)
        {
        InvokeHandlersInTopologicalOrder_OneGraph(edge, bvector<Edge>());
        }

    m_txnMgr.ElementDependencies().m_deletedRels.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementDependencyGraph::ElementDrivesElement::ElementDrivesElement(DgnElementDependencyGraph& graph) : TableApi(graph){}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnElementDependencyGraph::ElementDrivesElement::DoPrepare()
    {
    //  NB All Select statements must specify the same columns in the same order

    m_selectByRootInDirectChanges = GetTxnMgr().GetTxnStatement(
        "SELECT SourceId,TargetId,Id as relid,ECClassId,Status,Priority FROM " BIS_TABLE(BIS_REL_ElementDrivesElement)
        " WHERE (SourceId IN (SELECT ElementId FROM " TEMP_TABLE(TXN_TABLE_Elements) "))");

    m_selectByDependentInDirectChanges = GetTxnMgr().GetTxnStatement(
        "SELECT SourceId,TargetId,Id as relid,ECClassId,Status,Priority FROM " BIS_TABLE(BIS_REL_ElementDrivesElement)
        " WHERE (TargetId IN (SELECT ElementId FROM " TEMP_TABLE(TXN_TABLE_Elements) "))");

    m_selectByRelationshipInDirectChanges = GetTxnMgr().GetTxnStatement(
        "SELECT SourceId,TargetId,Id as relid,ECClassId,Status,Priority FROM " BIS_TABLE(BIS_REL_ElementDrivesElement)
        " WHERE (Id IN (SELECT ECInstanceId FROM " TEMP_TABLE(TXN_TABLE_Depend) "))");

    m__selectByRoot__ = GetTxnMgr().GetTxnStatement(
        "SELECT SourceId,TargetId,Id as relid,ECClassId,Status,Priority FROM " BIS_TABLE(BIS_REL_ElementDrivesElement)
        " WHERE SourceId=?");

    m__selectByDependent__ = GetTxnMgr().GetTxnStatement(
        "SELECT SourceId,TargetId,Id as relid,ECClassId,Status,Priority FROM " BIS_TABLE(BIS_REL_ElementDrivesElement)
        " WHERE TargetId=?");

    m__updateStatus__ = GetTxnMgr().GetTxnStatement("UPDATE " BIS_TABLE(BIS_REL_ElementDrivesElement) " SET Status=? WHERE Id=?");
    
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnElementDependencyGraph::ElementDrivesElement::SelectEdgeByRelId(Edge& edge, ECInstanceId eid)
    {
    CachedStatementPtr selectByRelId = GetTxnMgr().GetTxnStatement(
        "SELECT SourceId,TargetId,Id as relid,ECClassId,Status,Priority FROM " BIS_TABLE(BIS_REL_ElementDrivesElement) " WHERE Id=?");

    selectByRelId->BindId(1, eid);
    return SelectEdge(edge, *selectByRelId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnElementDependencyGraph::ElementDrivesElement::UpdateEdgeStatusInDb(Edge const& edge, EdgeStatus newStatus) 
    {
    m__updateStatus__->Reset();
    m__updateStatus__->BindInt(1,(int)newStatus);
    m__updateStatus__->BindId(2, edge.m_relId);
    return m__updateStatus__->Step();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnElementDependencyGraph::ElementDrivesElement::SelectEdge(Edge& edge, Statement& stmt)
    {
    auto stat = stmt.Step();
    if (BE_SQLITE_ROW != stat)
        return stat;
        
    edge.m_ein       = stmt.GetValueId<DgnElementId>(0);
    edge.m_eout      = stmt.GetValueId<DgnElementId>(1);
    edge.m_relId     = stmt.GetValueId<ECInstanceId>(2);
    edge.m_relClassId = stmt.GetValueId<ECClassId>(3);
    edge.m_status    = (uint8_t)stmt.GetValueInt(4);
    edge.m_priority  = stmt.GetValueInt64(5);

    return stat;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnElementDependencyGraph::ElementDrivesElement::StepSelectWhereRootInDirectChanges(Edge& edge)
    {
    return SelectEdge(edge, *m_selectByRootInDirectChanges);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnElementDependencyGraph::ElementDrivesElement::StepSelectWhereDependentInDirectChanges(Edge& edge)
    {
    return SelectEdge(edge, *m_selectByDependentInDirectChanges);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnElementDependencyGraph::ElementDrivesElement::StepSelectWhereRelationshipInDirectChanges(Edge& edge)
    {
    return SelectEdge(edge, *m_selectByRelationshipInDirectChanges);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElementDependencyGraph::ElementDrivesElement::BindSelectByRoot(DgnElementId rootId)
    {
    m__selectByRoot__->Reset();
    m__selectByRoot__->BindId(1, rootId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnElementDependencyGraph::ElementDrivesElement::StepSelectByRoot(Edge& edge)
    {
    return SelectEdge(edge, *m__selectByRoot__);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElementDependencyGraph::ElementDrivesElement::BindSelectByDependent(DgnElementId depId)
    {
    m__selectByDependent__->Reset();
    m__selectByDependent__->BindId(1, depId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnElementDependencyGraph::ElementDrivesElement::StepSelectByDependent(Edge& edge)
    {
    return SelectEdge(edge, *m__selectByDependent__);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElementDependencyGraph::DiscoverEdges()
    {
    auto& queue = *m_edgeQueue;
    auto& elementDrivesElement = *m_elementDrivesElement;

    bvector<Edge> fringe;
    bset<ECInstanceId> edges_seen;

    //  Find edges affected by the Elements that were directly changed
    Edge directlyChanged;
    while (elementDrivesElement.StepSelectWhereRootInDirectChanges(directlyChanged) == BE_SQLITE_ROW)
        fringe.push_back(directlyChanged);

    while (elementDrivesElement.StepSelectWhereDependentInDirectChanges(directlyChanged) == BE_SQLITE_ROW)
        fringe.push_back(directlyChanged);

    while (elementDrivesElement.StepSelectWhereRelationshipInDirectChanges(directlyChanged) == BE_SQLITE_ROW)
        fringe.push_back(directlyChanged);

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
        deletedEdge.m_relId = deletedRel.m_relKey.GetInstanceId();
        deletedEdge.m_relClassId = deletedRel.m_relKey.GetClassId();
        fringe.push_back(deletedEdge);
        }

    //  Find edges reachable from the initial set found above.
    while (!fringe.empty())
        {
        bvector<Edge> working_fringe;
        std::swap(working_fringe, fringe);
        for (auto& currEdge : working_fringe)
            {
            if (edges_seen.find(currEdge.m_relId) != edges_seen.end())
                continue;
            edges_seen.insert(currEdge.m_relId);

            EDGLOG(LOG_TRACE, "VISIT %s", FmtEdge(currEdge).c_str());
            queue.AddEdge(currEdge);

            // Find all relationships with dependency handlers that take currEdge.m_eout as their input or their output.
            // Add all of those relationships to the queue
            Edge affected;
            affected.m_ein = currEdge.m_eout;

            //  The output of currEdge could be the input of other ElementDrivesElement dependencies. Those downstream dependencies must be fired as a result.
            elementDrivesElement.BindSelectByRoot(currEdge.m_eout);
            while (elementDrivesElement.StepSelectByRoot(affected) == BE_SQLITE_ROW)
                {
                fringe.push_back(affected);
                }
            }
        }

    // Fill the NODES table
    bset<DgnElementId> nodeIds = queue.GetAllNodeIds();
    for (DgnElementId const& id : nodeIds)
        {
        m_nodes->InsertNode(id);

        int inputCount = queue.GetNodeInputCount(id);
        m_nodes->SetInDegree(id, inputCount);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElementDependencyGraph::VerifyOverlappingDependencies()
    {
    int count = 0;

    m_edgeQueue->ResetSelectHaveSharedOutput();
    Edge outputsElement;
    while (m_edgeQueue->StepSelectHaveSharedOutput(outputsElement) == BE_SQLITE_ROW)
        {
        if (++count == 1)
            EDGLOG(LOG_TRACE, ".............VerifyOverlappingDependencies................");
        
        InvokeHandlerForValidation(outputsElement);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElementDependencyGraph::InvokeHandlersInDependencyOrder()
    {
    EDGLOG(LOG_TRACE, "-----------------InvokeHandlersInDependencyOrder---------------------");

    EdgeQueue queue(*this);
    Nodes nodes(*this);
    ElementDrivesElement ElementDrivesElement(*this);
    ElementDrivesElement.DoPrepare();

    m_edgeQueue = &queue;
    m_nodes = &nodes;
    m_elementDrivesElement = &ElementDrivesElement;

    DiscoverEdges(); // populates m_edgeQueue

    InvokeHandlersInTopologicalOrder(); // searches m_edgeQueue

    VerifyOverlappingDependencies();

    m_elementDrivesElement = nullptr;
    m_edgeQueue = nullptr;
    m_nodes = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElementDependencyGraph::InvokeAffectedDependencyHandlers()
    {
    EDGLOG(LOG_TRACE, "--------------------------------------------------");
    
    if (s_debugGraph > 1)
        WriteDot(BeFileName(L"D:\\tmp\\ChangePropagationRelationships.dot"), bvector<bvector<uint64_t>>());

    InvokeHandlersInDependencyOrder();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnElementDependencyGraph::WhatIfChanged(bvector<DgnElementId> const& directlyChangedElements, bvector<ECInstanceId> const& directlyChangedDepRels)
    {
    m_txnMgr.OnBeginValidate();

    auto& txnElements = m_txnMgr.Elements();
    auto& elements = GetDgnDb().Elements();
    for (DgnElementId elementId : directlyChangedElements)
        {
        auto el = elements.GetElement(elementId);
        if (el.IsValid())
            txnElements.AddElement(elementId, el->GetModelId(), TxnTable::ChangeType::Update, el->GetElementClassId());
        }

    auto& dependencies = m_txnMgr.ElementDependencies();
    for (ECInstanceId deprel : directlyChangedDepRels)
        dependencies.AddDependency(deprel, TxnTable::ChangeType::Update);

    InvokeAffectedDependencyHandlers();

    m_txnMgr.OnEndValidate();
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElementDependencyGraph::Init()
    {
    m_elementDrivesElement = nullptr;
    m_jsTxns = GetDgnDb().GetJsTxns();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementDependencyGraph::DgnElementDependencyGraph(TxnManager& mgr) : m_txnMgr(mgr)
    {
    Init();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnElementDependencyGraph::SetElementDrivesElementPriority(ECInstanceId relid, int64_t newPriority)
    {
    CachedStatementPtr updatePriority = m_txnMgr.GetTxnStatement("UPDATE " BIS_TABLE(BIS_REL_ElementDrivesElement) " SET Priority=? WHERE Id=?");
    updatePriority->BindInt64(1, newPriority);
    updatePriority->BindId(2, relid);
    return updatePriority->Step() == BE_SQLITE_DONE ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnElementDependencyGraph::SetEdgeDeferred(Edge& edge, bool isDeferred)
    {
    EdgeStatusAccessor accs(edge.m_status);
    accs.m_deferred = isDeferred;
    edge.m_status = (uint8_t)accs.ToEdgeStatus();
    return UpdateEdgeStatusInDb(edge, accs.ToEdgeStatus()) == BE_SQLITE_DONE ? BSISUCCESS : BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementDependencyGraph::Edge DgnElementDependencyGraph::QueryEdgeByRelationshipId(ECInstanceId relid)
    {
    Edge edge;
    ElementDrivesElement deprel(*this);
    deprel.SelectEdgeByRelId(edge, relid);
    return edge;
    }
