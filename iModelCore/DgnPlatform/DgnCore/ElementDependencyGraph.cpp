/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ElementDependencyGraph.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#define ECSQL_SOURCEECINSTANCEID    "SourceECInstanceId"
#define ECSQL_ECINSTANCEID          "ECInstanceId"

#define EDGE_QUEUE_TABLE_NAME  "TxnEdgeQueue"
#define EDGE_QUEUE TEMP_TABLE(EDGE_QUEUE_TABLE_NAME)

DPILOG_DEFINE(ElementDependencyGraph)

#define EDGLOG(sev,...) {if (ElementDependencyGraph_getLogger().isSeverityEnabled(sev)) { ElementDependencyGraph_getLogger().messagev(sev, __VA_ARGS__); }}

static int s_debugGraph = 0;
static bool s_debugGraph_showElementIds;

HANDLER_DEFINE_MEMBERS(DgnElementDependencyHandler)

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

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
        
    DbResult DoPrepare(DgnModelId);

    DbResult StepSelectWhereRootInDirectChanges(Edge&);
    DbResult StepSelectWhereDependentInDirectChanges(Edge&);
    DbResult StepSelectWhereRelationshipInDirectChanges(Edge&);
    void BindSelectByRoot(DgnElementId);
    DbResult StepSelectByRoot(Edge&);
    void BindSelectByDependent(DgnElementId);
    DbResult StepSelectByDependent(Edge&);
    DbResult SelectEdgeByRelId(Edge&, EC::ECInstanceId);

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
    CachedStatementPtr m_setEdgeColor, m_getEdgeColor, m_setHaveSharedOutput;
    Statement          m_checkPathStmt;

    DbResult SelectEdge(DgnElementDependencyGraph::Edge&, Statement&);

    public:
    EdgeQueue(DgnElementDependencyGraph& g);
    ~EdgeQueue();
    DbResult AddEdge(DgnElementDependencyGraph::Edge&);
    DbResult StepSelectAll(DgnElementDependencyGraph::Edge& edge) {return SelectEdge(edge, *m_select);}
    void              ResetSelectAllOrderedByPriority() {m_selbyp->Reset();}
    DbResult StepSelectAllOrderedByPriority(DgnElementDependencyGraph::Edge& edge) {return SelectEdge(edge, *m_selbyp);}
    DbResult BindSelectByOutput(DgnElementId);
    DbResult StepSelectByOutput(Edge& edge) {return SelectEdge(edge, *m_selbyo);}
    EdgeColor GetEdgeColor(EC::ECInstanceId);
    void SetEdgeColor(EC::ECInstanceId, EdgeColor);
    void SetHaveSharedOutput(EC::ECInstanceId);
    void              ResetSelectHaveSharedOutput() {m_selbyso->Reset();}
    DbResult StepSelectHaveSharedOutput(DgnElementDependencyGraph::Edge& edge) {return SelectEdge(edge, *m_selbyso);}

    };

END_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElementDependencyHandler::_OnRootChanged(DgnDbR db, EC::ECInstanceId, DgnElementId, DgnElementId)
    {
    db.Txns().ReportError(*new DgnElementDependencyGraph::MissingHandlerError(""));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnElementDependencyGraph::FmtHandler(DgnClassId hid)
    {
    DgnDomain::Handler* handler = DgnElementDependencyHandler::GetHandler().FindHandler(GetDgnDb(), hid);
    return handler ? handler->GetClassName() : "??UnknownDependencyHandler??";
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
    s.append(": ");
    s.append(FmtHandler(edge.GetHandlerId()));
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
    ECSqlSelectBuilder b;
    b.Select("Code").From(*GetDgnDb().Schemas().GetECClass(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_Element), true).Where(ECSQL_ECINSTANCEID "=?");
    ECSqlStatement s;
    s.Prepare(GetDgnDb(), b.ToString().c_str());
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
            int64_t id;
            if (sscanf(start, "%" SCNd64, &id) == 1)
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
    edges.Prepare(GetDgnDb(), "SELECT RootElementId, DependentElementId, ECInstanceId, Status FROM " DGN_TABLE(DGN_RELNAME_ElementDrivesElement));

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElementDependencyGraph::UpdateModelDependencyIndex()
    {
    // Do a total topological sort on the Model table, where the roots are the models
    // that are not the target/dependent of any ModelDrivesModel.
    // Update the dgn.Model.DependencyIndex column with the results.
    // Note that this assigns a new DependencyIndex value to every row in the Model table.

// NB: The SELECT stmt in a CTE must specify columns in the *same order* as propagate's arguments. This is required even if SELECT specifies aliases.
    CachedStatementPtr stmt;
    auto sqlitestat = GetDgnDb().GetCachedStatement(stmt, 
        "WITH RECURSIVE"
         " propagate(input_model,output_model,mpath,plevel,iscycle) AS ("
          " SELECT 0,MODEL.Id,('.'||MODEL.Id),0,0"
           " FROM " DGN_TABLE(DGN_CLASSNAME_Model) " MODEL"
           " WHERE ( MODEL.Id NOT IN ( SELECT DependentModelId FROM " DGN_TABLE(DGN_RELNAME_ModelDrivesModel) " ))"
          " UNION ALL"
           " SELECT DEPREL.RootModelId, DEPREL.DependentModelId, (propagate.mpath||'.'||DEPREL.DependentModelId), (propagate.plevel + 1), (instr(propagate.mpath,DEPREL.DependentModelId) == 1)"
            " FROM " DGN_TABLE(DGN_RELNAME_ModelDrivesModel) " DEPREL, propagate"
            " WHERE ((propagate.iscycle = 0) AND (DEPREL.RootModelId = propagate.output_model) AND (propagate.plevel < 100000))"
           ")"
        " SELECT propagate.plevel, propagate.input_model, propagate.output_model, propagate.mpath, propagate.iscycle FROM propagate");
//                  0                1                      2                       3                4
    if (sqlitestat != BE_SQLITE_OK)
        {
        EDGLOG(LOG_ERROR, "error creating computeModelDependencyIndex CTE -> %x", sqlitestat);
        BeAssert(false);
        return;
        }

    CachedStatementPtr updateIdx;
    sqlitestat = GetDgnDb().GetCachedStatement(updateIdx, "UPDATE " DGN_TABLE(DGN_CLASSNAME_Model) " SET DependencyIndex=? WHERE Id=?");

    bset<DgnModelId> modelsSeen;

    int idx = 0;
    while (stmt->Step() == BE_SQLITE_ROW)
        {
        auto output_model = stmt->GetValueId<DgnModelId>(2);
        auto iscycle    = stmt->GetValueInt(4);

        if (iscycle)
            {
            auto mpath = stmt->GetValueText(3);
            ReportValidationError(*new CyclesDetectedError(FmtElementPath(mpath).c_str()), nullptr);
            return;
            }

        //printf ("%d %lld -> %lld mpath:%s cycle? %d\n", level, input_model.IsValid()? input_model.GetValue(): 0LL, output_model.GetValue(), mpath, iscycle);
        updateIdx->Reset();
        updateIdx->ClearBindings();
        updateIdx->BindInt(1, idx);
        updateIdx->BindId(2, output_model);
        sqlitestat = updateIdx->Step();
        BeAssert( BE_SQLITE_DONE == sqlitestat );
        ++idx;

        // in order to count how many models we see, we must use a set.
        //  that is because the recursive query will visit the same model multiple
        //  times, once for each path to it.
        modelsSeen.insert(output_model);
        }

    //  Check for cycles with no roots leading into them.
    CachedStatementPtr modelsCount;
    sqlitestat = GetDgnDb().GetCachedStatement(modelsCount, "SELECT COUNT(*) FROM " DGN_TABLE(DGN_CLASSNAME_Model));
    modelsCount->Step();
    auto count = modelsCount->GetValueInt64(0);
    if (modelsSeen.size() != count)
        {
        ReportValidationError(*new CyclesDetectedError(Utf8PrintfString("%d models involved",(count-idx))), nullptr);
        return;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                  03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnElementDependencyGraph::CheckDirection(Edge const& edge)
    {
    auto& models = GetDgnDb().Models();
    uint64_t sidx, tidx;
    DgnModelType stype, ttype;
    models.QueryModelDependencyIndexAndType(sidx, stype, GetDgnDb().Elements().QueryModelId(edge.m_ein));
    models.QueryModelDependencyIndexAndType(tidx, ttype, GetDgnDb().Elements().QueryModelId(edge.m_eout));
    if (sidx > tidx || stype > ttype)
        ReportValidationError(*new DirectionValidationError(FmtEdge(edge).c_str()), &edge);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_TxnTable::ModelDep::_PropagateChanges()
    {
    if (!HasChanges())
        return;

    DgnElementDependencyGraph graph(m_txnMgr);
    graph.UpdateModelDependencyIndex();
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
    EDGLOG(LOG_TRACE, "\t%s->%s/%s->%s (%s)", 
                        FmtElement(ein).c_str(), 
                            FmtRel(edge).c_str(), FmtHandler(edge.GetHandlerId()).c_str(), 
                                  FmtElement(eout).c_str(), 
                                       FmtElementPath(epath).c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElementDependencyGraph::ReportValidationError(TxnManager::ValidationError& error, Edge const* edge)
    {
    if (m_processor != nullptr)
        {
        EDGLOG(LOG_TRACE, "PROCESS VALIDATION ERROR %s", error.GetDescription());
        m_processor->_OnValidationError(error, edge);
        }
    else
        m_txnMgr.ReportError(error);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8String fmtIndent(size_t indentLevel)
    {
    Utf8String tabs;
    tabs.assign(indentLevel, ' '); 
    return tabs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElementDependencyGraph::InvokeHandler(Edge const& edge, size_t indentLevel)
    {
    auto handler = DgnElementDependencyHandler::GetHandler().FindHandler(GetDgnDb(), edge.GetHandlerId());
    if (nullptr == handler)
        {
        BeAssert(false);
        return;
        }

    if (m_processor != nullptr)
        {
        EDGLOG(LOG_TRACE, "%sPROCESS %s(%llx)", fmtIndent(indentLevel).c_str(), FmtEdge(edge).c_str(),(intptr_t)handler);

        m_processor->_ProcessEdge(edge, handler);
        return;
        }

    size_t errorCount = m_txnMgr.GetErrors().size();

    if (handler != NULL)
        {
        EDGLOG(LOG_TRACE, "%sCALL %s(%llx)", fmtIndent(indentLevel).c_str(), FmtEdge(edge).c_str(),(intptr_t)&handler);
        handler->_OnRootChanged(GetDgnDb(), edge.m_relId, edge.m_ein, edge.m_eout);
        }
    else
        {
        EDGLOG(LOG_ERROR, "Missing handler for %s", FmtEdge(edge).c_str());
        m_txnMgr.ReportError(*new MissingHandlerError(FmtEdge(edge).c_str()));
        BeAssert(false);
        }

    SetFailedEdgeStatusInDb(edge,(m_txnMgr.GetErrors().size() > errorCount));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElementDependencyGraph::InvokeHandlerForValidation(Edge const& edge)
    {
    auto handler = DgnElementDependencyHandler::GetHandler().FindHandler(GetDgnDb(), edge.GetHandlerId());
    if (nullptr == handler)
        {
        BeAssert(false);
        return;
        }

    if (m_processor != nullptr)
        {
        EDGLOG(LOG_TRACE, "PROCESS FOR VALIDATION %s(%llx)", FmtEdge(edge).c_str(),(intptr_t)handler);

        m_processor->_ProcessEdgeForValidation(edge, handler);
        return;
        }

    size_t errorCount = m_txnMgr.GetErrors().size();

    if (handler != NULL)
        {
        EDGLOG(LOG_TRACE, "VALCALL %s(%llx)", FmtEdge(edge).c_str(),(intptr_t)&handler);
        handler->_ValidateOutput(GetDgnDb(), edge.m_relId, edge.m_ein, edge.m_eout);
        }

    SetFailedEdgeStatusInDb(edge,(m_txnMgr.GetErrors().size() > errorCount));
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
DgnElementDependencyGraph::EdgeQueue::EdgeQueue(DgnElementDependencyGraph& g) 
    : 
    DgnElementDependencyGraph::TableApi(g)
    {
    if (!GetDgnDb().TableExists(EDGE_QUEUE))
        {                                                                                                   // NB default color must be 0, i.e., EdgeColor::White
        GetDgnDb().CreateTable(EDGE_QUEUE, "ECRelationshipId INTEGER PRIMARY KEY, shared_output INTEGER DEFAULT 0, color INTEGER DEFAULT 0");

        // *** WIP_GRAPH_PRIORITY
        //      -- currently, the Priority column is NULL unless the user happens to populate it. 
        //          That is a problem, as we want the default priority to be order of creation.
        //      -- must change dgn.0.02.ecschema.xml to put an index on the priority column, so that ORDER BY Priority will work
        }

    GetDgnDb().GetCachedStatement(m_insert, "INSERT INTO " EDGE_QUEUE " (ECRelationshipId) VALUES(?)");
    GetDgnDb().GetCachedStatement(m_setHaveSharedOutput, "UPDATE " EDGE_QUEUE " SET shared_output=1 WHERE ECRelationshipId=?");
    GetDgnDb().GetCachedStatement(m_setEdgeColor, "UPDATE " EDGE_QUEUE " SET color=? WHERE ECRelationshipId=?");
    GetDgnDb().GetCachedStatement(m_getEdgeColor, "SELECT color FROM " EDGE_QUEUE " WHERE ECRelationshipId=?");

    // NB: All select statements must specify the same columns and in the same order, as they are all processed by a single function, "SelectEdge"

                  // 0                  1               2                    3           4        5      
#define Q_SEL_COLS " Q.ECRelationshipId,E.RootElementId,E.DependentElementId,E.ECClassId,E.status,E.Priority"
#define Q_TABLES   " " EDGE_QUEUE " AS Q JOIN " DGN_TABLE(DGN_RELNAME_ElementDrivesElement) " AS E ON (Q.ECRelationshipId = E.ECInstanceId)"

    GetDgnDb().GetCachedStatement(m_select, "SELECT " Q_SEL_COLS " FROM " Q_TABLES);
    GetDgnDb().GetCachedStatement(m_selbyp, "SELECT " Q_SEL_COLS " FROM " Q_TABLES " ORDER BY E.Priority DESC");
    GetDgnDb().GetCachedStatement(m_selbyo, "SELECT " Q_SEL_COLS " FROM " Q_TABLES " WHERE (E.DependentElementId=?) ORDER BY E.Priority DESC");
    GetDgnDb().GetCachedStatement(m_selbyso,"SELECT " Q_SEL_COLS " FROM " Q_TABLES " WHERE (Q.shared_output > 1)");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnElementDependencyGraph::EdgeQueue::SelectEdge(DgnElementDependencyGraph::Edge& edge, Statement& stmt)
    {
    auto stat = stmt.Step();
    if (BE_SQLITE_ROW != stat)
        return stat;

    edge.m_relId   = stmt.GetValueId<EC::ECInstanceId>(0);
    edge.m_ein      = stmt.GetValueId<DgnElementId>(1);
    edge.m_eout     = stmt.GetValueId<DgnElementId>(2);
    edge.m_relClassId = stmt.GetValueInt64(3);
    edge.m_status   = stmt.GetValueInt(4);
    edge.m_priority = stmt.GetValueInt64(5);

    BeAssert(edge.IsValid());
    return BE_SQLITE_ROW;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnElementDependencyGraph::EdgeQueue::BindSelectByOutput(DgnElementId eout)
    {
    m_selbyo->Reset();
    m_selbyo->ClearBindings();
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

    BeAssert(edge.m_ein.IsValid() && edge.m_eout.IsValid() && edge.GetHandlerId().IsValid() && edge.m_relId.IsValid());

    //  Add edge+path to queue
    m_insert->Reset();
    m_insert->ClearBindings();
    m_insert->BindId(1, edge.m_relId);
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
DgnElementDependencyGraph::EdgeColor DgnElementDependencyGraph::EdgeQueue::GetEdgeColor(EC::ECInstanceId edgeId)
    {
    m_getEdgeColor->Reset();
    m_getEdgeColor->ClearBindings();
    m_getEdgeColor->BindId(1, edgeId);
    if (m_getEdgeColor->Step() != BE_SQLITE_ROW)
        return EdgeColor::White;
    return (EdgeColor)m_getEdgeColor->GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+------c---------+---------------+------*/
void DgnElementDependencyGraph::EdgeQueue::SetHaveSharedOutput(EC::ECInstanceId edgeId)
    {
    m_setHaveSharedOutput->Reset();
    m_setHaveSharedOutput->ClearBindings();
    m_setHaveSharedOutput->BindId(1, edgeId);
    auto stat = m_setEdgeColor->Step();
    BeAssert(stat == BE_SQLITE_DONE);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+------c---------+---------------+------*/
void DgnElementDependencyGraph::EdgeQueue::SetEdgeColor(EC::ECInstanceId edgeId, EdgeColor color)
    {
    m_setEdgeColor->Reset();
    m_setEdgeColor->ClearBindings();
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
            m_txnMgr.ReportError(*new CyclesDetectedError(FmtCyclePath(edge, pathToSupplier).c_str()));
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
void DgnElementDependencyGraph::InvokeHandlersInTopologicalOrder() 
    {
    // This is a total topological sort of the Edge queue.
    m_edgeQueue->ResetSelectAllOrderedByPriority();
    Edge edge;
    while (m_edgeQueue->StepSelectAllOrderedByPriority(edge) == BE_SQLITE_ROW)
        {
        InvokeHandlersInTopologicalOrder_OneGraph(edge, bvector<Edge>());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementDependencyGraph::ElementDrivesElement::ElementDrivesElement(DgnElementDependencyGraph& g) : TableApi(g)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnElementDependencyGraph::ElementDrivesElement::DoPrepare(DgnModelId mid)
    {
    //  NB All Select statements must specify the same columns in the same order

    m_selectByRootInDirectChanges = GetTxnMgr().GetTxnStatement(
        "SELECT RootElementId,DependentElementId,ECInstanceId as relid,ECClassId,Status,Priority FROM " DGN_TABLE(DGN_RELNAME_ElementDrivesElement)
        " WHERE (RootElementId IN (SELECT ElementId FROM " TEMP_TABLE(TXN_TABLE_Elements) " WHERE ModelId=?))");

    m_selectByRootInDirectChanges->BindId(1, mid);

    m_selectByDependentInDirectChanges = GetTxnMgr().GetTxnStatement(
        "SELECT RootElementId,DependentElementId,ECInstanceId as relid,ECClassId,Status,Priority FROM " DGN_TABLE(DGN_RELNAME_ElementDrivesElement)
        " WHERE (DependentElementId IN (SELECT ElementId FROM " TEMP_TABLE(TXN_TABLE_Elements) " WHERE ModelId=?))");

    m_selectByDependentInDirectChanges->BindId(1, mid);

    m_selectByRelationshipInDirectChanges = GetTxnMgr().GetTxnStatement(
        "SELECT RootElementId,DependentElementId,ECInstanceId as relid,ECClassId,Status,Priority FROM " DGN_TABLE(DGN_RELNAME_ElementDrivesElement)
        " WHERE (ECInstanceId IN (SELECT ECInstanceId FROM " TEMP_TABLE(TXN_TABLE_Depend) " WHERE ModelId=?))");

    m_selectByRelationshipInDirectChanges->BindId(1, mid);

    m__selectByRoot__ = GetTxnMgr().GetTxnStatement(
        "SELECT RootElementId,DependentElementId,ECInstanceId as relid,ECClassId,Status,Priority FROM " DGN_TABLE(DGN_RELNAME_ElementDrivesElement)
        " WHERE RootElementId=?");

    m__selectByDependent__ = GetTxnMgr().GetTxnStatement(
        "SELECT RootElementId,DependentElementId,ECInstanceId as relid,ECClassId,Status,Priority FROM " DGN_TABLE(DGN_RELNAME_ElementDrivesElement)
        " WHERE DependentElementId=?");

    m__updateStatus__ = GetTxnMgr().GetTxnStatement("UPDATE " DGN_TABLE(DGN_RELNAME_ElementDrivesElement) " SET Status=? WHERE ECInstanceId=?");
    
    return BE_SQLITE_OK;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnElementDependencyGraph::ElementDrivesElement::SelectEdgeByRelId(Edge& edge, EC::ECInstanceId eid)
    {
    CachedStatementPtr selectByRelId = GetTxnMgr().GetTxnStatement(
        "SELECT RootElementId,DependentElementId,ECInstanceId as relid,ECClassId,Status,Priority FROM " DGN_TABLE(DGN_RELNAME_ElementDrivesElement) " WHERE ECInstanceId=?");

    selectByRelId->BindId(1, eid);
    return SelectEdge(edge, *selectByRelId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DbResult DgnElementDependencyGraph::ElementDrivesElement::UpdateEdgeStatusInDb(Edge const& edge, EdgeStatus newStatus) 
    {
    m__updateStatus__->Reset();
    m__updateStatus__->ClearBindings();
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
    edge.m_relClassId = stmt.GetValueInt64(3);
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
    m__selectByRoot__->ClearBindings();
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
    m__selectByDependent__->ClearBindings();
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
void DgnElementDependencyGraph::DiscoverEdges(DgnModelId mid)
    {
    auto& queue = *m_edgeQueue;
    auto& elementDrivesElement = *m_elementDrivesElement;

    //  Find edges affected by the Elements that were directly changed
    Edge directlyChanged;
    while (elementDrivesElement.StepSelectWhereRootInDirectChanges(directlyChanged) == BE_SQLITE_ROW)
        queue.AddEdge(directlyChanged);

    while (elementDrivesElement.StepSelectWhereDependentInDirectChanges(directlyChanged) == BE_SQLITE_ROW)
        queue.AddEdge(directlyChanged);

    while (elementDrivesElement.StepSelectWhereRelationshipInDirectChanges(directlyChanged) == BE_SQLITE_ROW)
        queue.AddEdge(directlyChanged);

    //  Find edges reachable from the initial set found above.
    Edge currEdge;
    while (queue.StepSelectAll(currEdge) == BE_SQLITE_ROW)
        {
        //  Return currEdge as result
        EDGLOG(LOG_TRACE, "VISIT %s", FmtEdge(currEdge).c_str());

        // Find all relationships with dependency handlers that take currEdge.m_eout as their input or their output.
        // Add all of those relationships to the queue
        Edge affected;
        affected.m_ein = currEdge.m_eout;

        //  The output of currEdge could be the input of other ElementDrivesElement dependencies. Those downstream dependencies must be fired as a result.
        elementDrivesElement.BindSelectByRoot(currEdge.m_eout);
        while (elementDrivesElement.StepSelectByRoot(affected) == BE_SQLITE_ROW)
            queue.AddEdge(affected);

        //  The output of currEdge could be the *output* of other ElementDrivesElement dependencies. Those "collaborating" dependencies must also be fired.
        bset<EC::ECInstanceId> suppliers;
        elementDrivesElement.BindSelectByDependent(currEdge.m_eout);
        while (elementDrivesElement.StepSelectByDependent(affected) == BE_SQLITE_ROW)
            {
            queue.AddEdge(affected);
            suppliers.insert(affected.GetECRelationshipId());
            }
        if (suppliers.size() > 1)                                                                   
            {
            for (auto supplier : suppliers)
                queue.SetHaveSharedOutput(supplier);
            }
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
void DgnElementDependencyGraph::InvokeHandlersInDependencyOrder(DgnModelId mid)
    {
    EDGLOG(LOG_TRACE, "-----------------InvokeHandlersInDependencyOrder---------------------");

    EdgeQueue queue(*this);
    ElementDrivesElement ElementDrivesElement(*this);
    ElementDrivesElement.DoPrepare(mid);

    m_edgeQueue = &queue;
    m_elementDrivesElement = &ElementDrivesElement;

    DiscoverEdges(mid); // populates m_edgeQueue

    InvokeHandlersInTopologicalOrder(); // searches m_edgeQueue

    VerifyOverlappingDependencies();

    m_elementDrivesElement = nullptr;
    m_edgeQueue = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElementDependencyGraph::InvokeAffectedDependencyHandlers()
    {
    EDGLOG(LOG_TRACE, "--------------------------------------------------");
    
    if (s_debugGraph > 1)
        WriteDot(BeFileName(L"D:\\tmp\\ChangePropagationRelationships.dot"), bvector<bvector<uint64_t>>());

    // Step through the affected models in dependency order
    CachedStatementPtr modelsInOrder=m_txnMgr.GetTxnStatement("SELECT Id From " DGN_TABLE(DGN_CLASSNAME_Model) " WHERE "
        "Id IN (SELECT ModelId FROM " TEMP_TABLE(TXN_TABLE_Elements) ") OR "
        "Id IN (SELECT ModelId FROM " TEMP_TABLE(TXN_TABLE_Depend) ") OR " 
        "Id IN (SELECT ModelId FROM " TEMP_TABLE(TXN_TABLE_Models) ") "
        " ORDER BY Type,DependencyIndex");

    while (modelsInOrder->Step() == BE_SQLITE_ROW)
        {
        auto mid = modelsInOrder->GetValueId<DgnModelId>(0);

        EDGLOG(LOG_TRACE, "Model %lld", mid.GetValue());

        InvokeHandlersInDependencyOrder(mid);

        if (m_txnMgr.HasFatalErrors())
            break;

        DgnModelPtr model = GetDgnDb().Models().GetModel(mid);
        if (model.IsValid())
            {
            model->_OnValidate();

            if (m_txnMgr.HasFatalErrors())
                break;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnElementDependencyGraph::WhatIfChanged(IEdgeProcessor& proc, bvector<DgnElementId> const& directlyChangedElements, bvector<EC::ECInstanceId> const& directlyChangedDepRels)
    {
    m_txnMgr.OnBeginValidate();

    auto& txnElements = m_txnMgr.Elements();
    auto& elements = GetDgnDb().Elements();
    for (DgnElementId elementId : directlyChangedElements)
        {
        auto el = elements.GetElement(elementId);
        if (el.IsValid())
            txnElements.AddElement(elementId, el->GetModelId(), TxnTable::ChangeType::Update);
        }

    auto& dependencies = m_txnMgr.ElementDependencies();
    for (EC::ECInstanceId deprel : directlyChangedDepRels)
        dependencies.AddDependency(deprel, TxnTable::ChangeType::Update);

    m_processor = &proc;
    InvokeAffectedDependencyHandlers();
    m_processor = nullptr;

    m_txnMgr.OnEndValidate();
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElementDependencyGraph::Init()
    {
    m_elementDrivesElement = nullptr;
    m_processor = nullptr;
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
BentleyStatus DgnElementDependencyGraph::SetElementDrivesElementPriority(EC::ECInstanceId relid, int64_t newPriority)
    {
    CachedStatementPtr updatePriority = m_txnMgr.GetTxnStatement("UPDATE " DGN_TABLE(DGN_RELNAME_ElementDrivesElement) " SET Priority=? WHERE ECInstanceId=?");
    updatePriority->BindInt64(1, newPriority);
    updatePriority->BindId(2, relid);
    return (updatePriority->Step() == BE_SQLITE_DONE)? BSISUCCESS: BSIERROR;
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
DgnElementDependencyGraph::Edge DgnElementDependencyGraph::QueryEdgeByRelationshipId(EC::ECInstanceId relid)
    {
    Edge edge;
    ElementDrivesElement deprel(*this);
    deprel.SelectEdgeByRelId(edge, relid);
    return edge;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementDependencyGraph::~DgnElementDependencyGraph() 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      01/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementDependencyHandler* DgnElementDependencyHandler::FindHandler(DgnDbR db, DgnClassId handlerId) 
    {
    // quick check for a handler already known
    DgnDomain::Handler* handler = db.Domains().LookupHandler(handlerId);
    if (nullptr != handler)
        return dynamic_cast<DgnElementDependencyHandler*>(handler);

    // not there, check via base classes
    handler = db.Domains().FindHandler(handlerId, db.Domains().GetClassId(GetHandler()));
    return handler ? dynamic_cast<DgnElementDependencyHandler*>(handler) : nullptr;
    }
