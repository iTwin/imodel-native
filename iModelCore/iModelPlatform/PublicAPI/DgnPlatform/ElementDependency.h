/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/TxnManager.h>

BEGIN_BENTLEY_DGN_NAMESPACE

namespace ElementDependency {

//=======================================================================================
//! Base class for dependency handlers for the dgn.ElementDrivesElement ECRelationship class. 
//! Subclasses should derive from this class, register it using DgnDomain::RegisterHandler, and override the virtual methods to do something useful
//! @see ElementDependency::Graph.
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Handler : DgnDomain::Handler {
    DOMAINHANDLER_DECLARE_MEMBERS(BIS_REL_ElementDrivesElement, Handler, DgnDomain::Handler, DGNPLATFORM_EXPORT)

    //! Called by DgnElementDependencyGraph after the ElementDrivesElement ECRelationship itself is created and then whenever the
    //! source or target DgnElement is changed directly or by an upstream dependency.
    //! @note This callback is \em not invoked when the source or target element is deleted. See _OnDeletedDependency
    //! @param[in] graph The ElementDependency::Graph being evaluated.
    //! @param[in] edge The relationship's data
    virtual void _OnRootChanged(Graph const& graph, Edge const& ege) {}

    //! Called by DgnElementDependencyGraph after the specified dependency is deleted.
    //! @note A dependency relationship is automatically deleted when its source or target element is deleted.
    //! @param[in] graph The ElementDependency::Graph being evaluated.
    //! @param[in] edge The relationship's data (as of the time it was deleted)
    virtual void _OnDeletedDependency(Graph const& graph, Edge const& edge) {}

    //! Called by ElementDependency::Graph after all _OnRootChanged have been invoked, in the case where the output of this dependency is also the output of other dependencies.
    //! The dependency handler should check that its requirements is still enforced. 
    //! Call Txns::LogError to report a validation error.
    //! @note The dependency handler should *not* modify the target element.
    //! @param[in] graph The ElementDependency::Graph being evaluated.
    //! @param[in] edge The relationship's data
    virtual void _OnValidateOutput(Graph const& graph, Edge const& edge) {}
};

//! Indicates if changes have been propagated through an ECRelationship successfully or not.
//! These are bits. The Deferred status and the Satisfied/Deferred status may be changed independently of each other.
enum EdgeStatus 
    {
    EDGESTATUS_Satisfied=0,     //! The requirements of this dependency relationship are satisfied.
    EDGESTATUS_Failed=1,        //! The requirements of this dependency relationship were violated.
    EDGESTATUS_Deferred=0x80    //! Changes are not being propagated through this dependency relationship.
    };

//=======================================================================================
//  An Edge in the graph is an ECRelationship that has a dependency handler.
//  It has an input element and an output element.
// @bsiclass
//=======================================================================================
struct Edge
    {
    friend struct Graph;
    friend struct EdgeQueue;
    friend struct TableApi;
    friend struct ElementDrivesElement;

    DgnElementId   m_ein;
    DgnElementId   m_eout;
    BeSQLite::EC::ECInstanceKey m_instanceKey;
    int64_t       m_priority;
    bool m_deleted; // if true - represents a deleted relationship
    union {
        struct {
            uint32_t      m_status:8;           // NB: size must match EdgeStatus
            uint32_t      m_direct:1;
            uint32_t      m_isEDE:1;
            uint32_t      m_unused2:22;
            };
        uint32_t    m_flags;
        };

    Edge() : m_priority(0), m_flags(0), m_deleted(false) {}

    //! Get the element that is the "input" to this dependency.
    DgnElementId GetRootElementId() const {return m_ein;}
    //! Get the element that is the "output" from this dependency.
    DgnElementId GetDependentElementId() const {return m_eout;}
    //! Get the classid and instanceid of the ECRelationship instance that is represented by this edge. May be a linktable relationship or a navigation property
    BeSQLite::EC::ECInstanceKeyCR GetKey() const {return m_instanceKey;}
    //! Get the priority of this dependency, relative to other dependencies in the same ECRelationshipClass
    //! @see Graph::SetElementDrivesElementPriority
    int64_t GetPriority() const {return m_priority;}
    //! Test if this edge is valid
    bool IsValid() const {return m_instanceKey.IsValid();}
    //! Test if this edge is marked as deferred
    bool IsDeferred() const {return (m_status & EDGESTATUS_Deferred) != 0;}
    //! Test if this edge is marked as having failed
    bool IsFailed() const {return (m_status & EDGESTATUS_Failed) != 0;}
    //! Test if this edge represents a deleted relationship
    bool IsDeleted() const { return m_deleted; }
    //! A direct change has been propagated to the input of this edge.
    void SetInputWasDirectlyChanged() {m_direct=1;}
    //! Were any direct changes propagated to the input of this edge?
    bool WasInputDirectlyChanged() const {return m_direct;}
    };

//=======================================================================================
//! Element dependency graph calls handers in the correct order when a transaction is "validated".
//! Called by Txns::CheckTxnBoundary.
//! 
//! <h3>Only ElementDrivesElement ECRelationships can have Dependency Handlers</h3>
//! Only an ECRelationship derived from dgn.ElementDrivesElement can have a dependency handler.
//! A domain can associate its own dependency handler with a kind of ElementDrivesElement relationship by 
//! 1) defining a subclass of dgn.ElementDrivesElement in its ECSchema, 
//! 2) defining a JavaScript hander that is associated with that ECSchema subclass, and 
//! 3) registering the JavaScript Handler.
//!
//! <h3>Dependency Graph</h3>
//! When you create an ElementDrivesElement ECRelationship, you create a dependency: the target DgnElement depends on the source DgnElement.
//! The registered dependency handler is notified after the content of the source DgnElement is changed. 
//!
//! When you create multiple ECRelationships with dependency handlers, you create a network of dependencies. The target of one may be the source of another. 
//! A change in the content of any given DgnElement can therefore trigger a chain of dependency handler notifications.
//! Such dependencies can define a directed acyclic graph (DAG). Cycles are not allowed.
//!
//! <h3>Dependency Handler Invocation Order</h3>
//! The order in which dependency handlers are invoked is defined as follows:
//!
//! <h4>1. Explicit Dependencies</h4>
//! Handlers are invoked in dependency order. The source of an ECRelationship can be thought of as the dependency's "input". The
//! target of the relationship plays the role of its "output". A handler is not invoked until its input has been produced. 
//!
//! <h4>2. Priority Order</h4>
//! If two ECRelationships are not connected by a chain of inputs and outputs (that is, they are in separate graphs), then 
//! the handlers are invoked in decreasing order of priority, as specified by the relationship's Priority property. The value
//! of the Priority property defaults to the order in which the ElementDrivesElement ECRelationships were created. 
//!
//! <h3>Errors</h3>
//! Circular dependencies are not permitted. If a cycle is detected, that is treated as a fatal error.
//! A missing dependency handler is treated as a warning.
//! A dependency handler's _OnRootChanged method may call TxnSummary::ReportError to reject an invalid change. It can classify the error as fatal or just a warning.
//! 
//! If it detects any fatal transaction validation error, Txns will roll back the transaction as a whole.
//! 
//! See Txns::IValidationError, Graph::CyclesDetectedError, Graph::MissingHandlerError
//=======================================================================================
struct Graph
{
    struct EdgeQueue;
    struct Nodes;
    struct TableApi;
    struct ElementDrivesElement;
    struct ChildPropagatesChangesToParent;
    friend struct Edge;
    friend struct EdgeQueue;

private:
    enum class EdgeColor {White, Gray, Black};  // NB: White must be the default (0) value

    TxnManager& m_txnMgr;
    ElementDrivesElement* m_elementDrivesElement;
    ChildPropagatesChangesToParent* m_childPropagatesChangesToParent;
    EdgeQueue* m_edgeQueue;
    Nodes* m_nodes;

    void Init();

    // Debugging
    Utf8String GetElementCode(DgnElementId eid);
    Utf8String FmtCyclePath(Edge const& edge, bvector<Edge> const& pathToEdge);
    Utf8String FmtEdges(bvector<Edge> const&);
    Utf8String FmtEdge(Edge const&);
    Utf8String FmtElement(DgnElementId eid);
    Utf8String FmtElementPath(Utf8CP epath);
    Utf8String FmtRel(Edge const&);
    void LogDependencyFound(BeSQLite::Statement&, Edge const&);

    // working with handlers
    void InvokeHandler(Edge const& rh);
    void InvokeHandlerForValidation(Edge const& rh);
    void DiscoverEdges();
    void InvokeHandlersInTopologicalOrder();
    void InvokeHandlersInTopologicalOrder_OneGraph(Edge const&, bvector<Edge> const& pathToSupplier);
    void InvokeHandlersInDependencyOrder();
    void InvokeHandlerForDeletedRelationship(BeSQLite::EC::ECInstanceKeyCR);
    BeSQLite::DbResult SetFailedEdgeStatusInDb(Edge const&, bool failed);
    BeSQLite::DbResult UpdateEdgeStatusInDb(Edge const&, EdgeStatus);
    BentleyStatus CheckDirection(Edge const&);

    // void NotifySharedAndDirectChanges();
    // bset<BeSQLite::EC::ECInstanceKey> GetEdgesThatOutputElement(DgnElementId eid, BeSQLite::Statement& findEdeByTarget);
    // void ValidateRels(bset<BeSQLite::EC::ECInstanceKey> const& relIds);
    // void CallOnDirectlyChanged();

    void OnDiscoverNodes(Edge const&);

    void WriteDotEdge(FILE* fp, Edge const& edge);
    void WriteEdgeQueueToDot(BeFileNameCR dotFilename);

public:
    Napi::Object m_jsTxns;

    DGNPLATFORM_EXPORT Graph(TxnManager&);
    ~Graph() {}

    DgnDbR GetDgnDb() const {return m_txnMgr.GetDgnDb();}

    //! Txns calls this when closing a txn
    void InvokeAffectedDependencyHandlers();

    //! Get the "Edge" that represents the specified ECRelationship instance
    //! @param[in] key    Identifies the ECRelationship instance (linktable or parent property)
    DGNPLATFORM_EXPORT Edge QueryEdgeByInstanceKey(BeSQLite::EC::ECInstanceKeyCR key);

    //! Set the priority of an ElementDrivesElement ECRelationship instance
    //! @param[in] relid    Identifies the ElementDrivesElement ECRelationship instance
    //! @param[in] newPriority The new priority value
    //! @see Edge::GetPriority
    DGNPLATFORM_EXPORT BentleyStatus SetElementDrivesElementPriority(BeSQLite::EC::ECInstanceId relid, int64_t newPriority);

    //! Set the deferred status of an ElementDrivesElement or an ElementDrivesElements ECRelationship instance
    //! @param[in] edge         Identifies the ECRelationship instance
    //! @param[in] isDeferred   set the dependency to deferred?
    //! @see Edge::IsDeferred
    DGNPLATFORM_EXPORT BentleyStatus SetEdgeDeferred(Edge& edge, bool isDeferred);

    //! Get a list of the dependencies, in order, that would be evaluated if the specified element and/or ElementDrivesElement relationship were directly changed
    DGNPLATFORM_EXPORT BentleyStatus WriteAffectedGraphToFile(BeFileNameCR dotFilename, bvector<DgnElementId> const& directlyChangedEntities, bvector<BeSQLite::EC::ECInstanceId> const& directlyChangedDepRels);

    DGNPLATFORM_EXPORT void WriteDot(BeFileNameCR dotFilename, bvector<bvector<uint64_t>> const& cyclePaths);
    };

} // end namespace ElementDependency

END_BENTLEY_DGN_NAMESPACE
