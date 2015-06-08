/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnEntity.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnCore/TxnManager.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct DgnElementDependencyGraph;

//=======================================================================================
//! Base class for dependency handlers for the dgn.ElementDrivesElement ECRelationship class. See DgnElementDependencyGraph.
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnElementDrivesElementDependencyHandler : DgnDomain::Handler
{
    DOMAINHANDLER_DECLARE_MEMBERS(DGN_RELNAME_ElementDrivesElement, DgnElementDrivesElementDependencyHandler, DgnDomain::Handler, DGNPLATFORM_EXPORT)

protected:
    friend struct DgnElementDependencyGraph;

    //! Called by DgnElementDependencyGraph after the source DgnElement's content has changed. 
    //! This base class implementation of _OnRootChanged calls Txns::ReportValidationError to indicate a missing handler.
    //! @param[in] db                   The DgnDb in which the handler and ECRelationship reside. 
    //! @param[in] relationshipId       The ECRelationship instance ID
    //! @param[in] source               The ECRelationship's Source DgnElement
    //! @param[in] target               The ECRelationship's Target DgnElement
    //! @param[in] summary              Summary of all DgnElements that were *directly* changed
    //! Call Txns::ReportValidationError to reject an invalid change. The reported error can be classified as fatal or just a warning.
    DGNPLATFORM_EXPORT virtual void _OnRootChanged(DgnDbR db, BeSQLite::EC::ECInstanceId relationshipId, DgnElementId source, DgnElementId target, TxnSummaryR summary);

    //! Called by DgnElementDependencyGraph after all _OnRootChanged have been invoked, in the case where the output of this dependency is also the output of other dependencies.
    //! The dependency handler should check that its requirements is still enforced. 
    //! Call Txns::ReportValidationError to report a validation error. The ElementDependencyGraph::DependencyValidationError error should normally be used. The reported error can be classified as fatal or just a warning.
    //! @note The dependency handler should *not* modify the target element.
    //! @param[in] db                   The DgnDb in which the handler and ECRelationship reside. 
    //! @param[in] relationshipId       The ECRelationship instance ID
    //! @param[in] source               The ECRelationship's Source DgnElement
    //! @param[in] target               The ECRelationship's Target DgnElement
    //! @param[in] summary              Summary of all DgnElements that were *directly* changed, plus all dependencies that failed.
    virtual void _ValidateOutput(DgnDbR db, BeSQLite::EC::ECInstanceId relationshipId, DgnElementId source, DgnElementId target, TxnSummaryR summary) {}

public:
    //! Looks up a registered DgnElementDrivesElementDependencyHandler by its ECRelationship class ID.
    //! @param[in] db               The DgnDb in which the handler and ECRelationship reside. 
    //! @param[out] classId         The ID of the particular ElementDrivesElement ECRelationshipClass
    //! @return The dependency handler that is registered for this ID or NULL if none is registered.
    DGNPLATFORM_EXPORT static DgnElementDrivesElementDependencyHandler* FindHandler(DgnDbR db, DgnClassId classId);
};

//=======================================================================================
//! Element dependency graph calls DgnElementDrivesElementDependencyHandler in the correct order when a transaction is "validated".
//! Called by Txns::CheckTxnBoundary.
//! 
//! <h3>Only ElementDrivesElement ECRelationships can have Dependency Handlers</h3>
//! Only an ECRelationship derived from dgn.ElementDrivesElement can have a dependency handler.
//! A domain can associate its own dependency handler with a kind of ElementDrivesElement relationship by 
//! 1) defining a subclass of dgn.ElementDrivesElement in its ECSchema, 
//! 2) defining a C++ subclass of DgnElementDrivesElementDependencyHandler that is associated with that ECSchema subclass, and 
//! 3) registering the C++ Handler subclass.
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
//! See Txns::IValidationError, DgnElementDependencyGraph::CyclesDetectedError, DgnElementDependencyGraph::MissingHandlerError
//=======================================================================================
struct DgnElementDependencyGraph
{
    struct EdgeQueue;
    struct TableApi;
    struct ElementDrivesElement;

    //! Cycles were detected in the ElementDependencyGraph.
    //! This means that the constraints imposed on Elements by the current set of ECRelationships and their dependency handlers cannot be satisfied. Some
    //! ECRelationships must be removed. 
    //! This error is always fatal. The application should cancel the current transaction.
    struct CyclesDetectedError : TxnSummary::ValidationError
        {
        CyclesDetectedError(Utf8CP path) : TxnSummary::ValidationError(Severity::Fatal, path) {}
        };

    //! An Element dependency was triggered, but the handler for it was not registered.
    //! It is not possible to know if this is a fatal error or not, since the purpose of the handler is not known.
    //! This error should be reported to the user for follow up.
    struct MissingHandlerError : TxnSummary::ValidationError
        {
        MissingHandlerError(Utf8CP handlerId) : TxnSummary::ValidationError(Severity::Warning, handlerId) {}
        };
    
    //! The requirements of an Element dependency were violated by the actions taken by another dependency handler.
    //! It is not possible to know if this is a fatal error or not, since the purpose of the handler is not known.
    //! This error should be reported to the user for follow up.
    struct DependencyValidationError : TxnSummary::ValidationError
        {
        DependencyValidationError(Utf8CP details) : TxnSummary::ValidationError(Severity::Warning, details) {}
        };
    
    //! A dependency attempted to propagate changes from a dependent model to a root model. 
    //! This is illegal, and this dependency must be removed.
    //! This error is always fatal. The application should cancel the current transaction.
    struct DirectionValidationError : TxnSummary::ValidationError
        {
        DirectionValidationError(Utf8CP details) : TxnSummary::ValidationError(Severity::Fatal, details) {}
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
    //  It has an input element and an output element. The dependency handler
    //  callback is the action taken when the edge is traversed.
    // @bsiclass                                                    Sam.Wilson  01/15
    //=======================================================================================
    struct Edge
        {
        friend struct DgnElementDependencyGraph;
        friend struct EdgeQueue;
        friend struct TableApi;
        friend struct ElementDrivesElement;

      private:
        DgnElementId   m_ein;
        DgnElementId   m_eout;
        BeSQLite::EC::ECInstanceId m_relId;
        ECN::ECClassId m_relClassId;
        int64_t       m_priority;
        union {
            struct {
                uint32_t      m_status:8;           // NB: size must match EdgeStatus
                uint32_t      m_unused2:24;
                };
            uint32_t    m_flags;
            };

        Edge() : m_priority(0), m_flags(0) {;}

      public:
        //! Get the element that is the "input" to this dependency.
        DgnElementId GetRootElementId() const {return m_ein;}
        //! Get the element that is the "output" from this dependency.
        DgnElementId GetDependentElementId() const {return m_eout;}
        //! Get the ID of the ECRelationship instance that is represented by this edge
        BeSQLite::EC::ECInstanceId GetECRelationshipId() const {return m_relId;}
        //! Get the ID of the ECRelationshipClass for this edge
        ECN::ECClassId GetECRelationshipClassId() const {return m_relClassId;}
        //! Get the ID of the DependencyHandler for this edge
        DgnClassId GetHandlerId() const {return DgnClassId(m_relClassId);}
        //! Get the priority of this dependency, relative to other dependencies in the same ECRelationshipClass
        //! @see DgnElementDependencyGraph::SetElementDrivesElementPriority
        int64_t GetPriority() const {return m_priority;}
        //! Test if this edge is valid
        bool IsValid() const {return m_relId.IsValid();}
        //! Test if this edge is marked as deferred
        bool IsDeferred() const {return (m_status & EDGESTATUS_Deferred) != 0;}
        //! Test if this edge is marked as having failed
        bool IsFailed() const {return (m_status & EDGESTATUS_Failed) != 0;}
        };

    friend struct Edge;
    friend struct EdgeQueue;

    //! Called to process edges in the graph
    struct IEdgeProcessor
        {
        //! Invoked on an edge in the graph
        virtual void _ProcessEdge(Edge const& edge, DgnElementDrivesElementDependencyHandler* handler) = 0;

        //! Invoked on an edge in the graph for a validation callback
        virtual void _ProcessEdgeForValidation(Edge const& edge, DgnElementDrivesElementDependencyHandler* handler) = 0;

        //! Invoked when a validation error such as a cycle is detected.
        virtual void _OnValidationError(TxnSummary::ValidationError const& error, Edge const* edge) = 0;
        };

private:
    enum class EdgeColor {White, Gray, Black};  // NB: White must be the default (0) value

    DgnDbR                         m_db;
    TxnSummaryR                    m_summary;
    ElementDrivesElement*          m_elementDrivesElement;
    EdgeQueue*                     m_edgeQueue;
    IEdgeProcessor*                m_processor;

    void Init();

    // Debugging
    void WriteDot(BeFileNameCR dotFilename, bvector<bvector<uint64_t>> const& cyclePaths);
    Utf8String GetElementCode(DgnElementId eid);
    Utf8String FmtCyclePath(Edge const& edge, bvector<Edge> const& pathToEdge);
    Utf8String FmtEdges(bvector<Edge> const&);
    Utf8String FmtEdge(Edge const&);
    Utf8String FmtElement(DgnElementId eid);
    Utf8String FmtElementPath(Utf8CP epath);
    Utf8String FmtRel(Edge const&);
    Utf8String FmtHandler(DgnClassId);
    void LogDependencyFound(BeSQLite::Statement&, Edge const&);

    // working with handlers
    void InvokeHandler(Edge const& rh, size_t indentLevel);
    void InvokeHandlerForValidation(Edge const& rh);
    void ReportValidationError (TxnSummary::ValidationError&, Edge const*);

    void DiscoverEdges(DgnModelId mid);

    void VerifyOverlappingDependencies();
    void InvokeHandlersInTopologicalOrder();
    void InvokeHandlersInTopologicalOrder_OneGraph(Edge const&, bvector<Edge> const& pathToSupplier);

    void InvokeHandlersInDependencyOrder(DgnModelId);

    BeSQLite::DbResult SetFailedEdgeStatusInDb(Edge const&, bool failed);
    BeSQLite::DbResult UpdateEdgeStatusInDb(Edge const&, EdgeStatus);

    BentleyStatus CheckDirection(Edge const&);

public:
    DGNPLATFORM_EXPORT DgnElementDependencyGraph(DgnDbR, TxnSummaryR);
    DGNPLATFORM_EXPORT ~DgnElementDependencyGraph();

    DgnDbR GetDgnDb() const {return m_db;}

//__PUBLISH_SECTION_END__
    //! Txns calls this when closing a txn
    void InvokeAffectedDependencyHandlers(TxnSummaryCR summary);

    //! Txns calls this when validating a txn
    void UpdateModelDependencyIndex();
//__PUBLISH_SECTION_START__

    //! Get the "Edge" that represents the specified ECRelationship instance
    //! @param[in] relid    Identifies the ECRelationship instance
    DGNPLATFORM_EXPORT Edge QueryEdgeByRelationshipId(BeSQLite::EC::ECInstanceId relid);

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
    DGNPLATFORM_EXPORT BentleyStatus WhatIfChanged(IEdgeProcessor& processor, bvector<DgnElementId> const& directlyChangedEntities, bvector<BeSQLite::EC::ECInstanceId> const& directlyChangedDepRels);
    };

END_BENTLEY_DGNPLATFORM_NAMESPACE
