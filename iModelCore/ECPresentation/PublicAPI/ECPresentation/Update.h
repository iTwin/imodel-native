/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/NavNode.h>

struct ReportTask;

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

struct HierarchyChangeRecordDiffHandler;

//=======================================================================================
// @bsiclass
//=======================================================================================
struct NodeChanges
{
private:
    NavNodeCPtr m_previousNode;
    NavNodeCPtr m_updatedNode;
    mutable Nullable<uint32_t> m_numChangedFields;

public:
    NodeChanges(): m_previousNode(nullptr), m_updatedNode(nullptr), m_numChangedFields(nullptr) {}
    NodeChanges(NavNodeCR previousNode, NavNodeCR updatedNode): m_previousNode(&previousNode), m_updatedNode(&updatedNode) {}

    NavNodeCPtr GetPreviousNode() const {return m_previousNode;}
    NavNodeCPtr GetUpdatedNode() const {return m_updatedNode;}

    ECPRESENTATION_EXPORT uint32_t GetNumChangedFields() const;

    ECPRESENTATION_EXPORT void FindChanges(HierarchyChangeRecordDiffHandler&) const;
};

//=======================================================================================
// @bsiclass
//=======================================================================================
struct HierarchyChangeRecordDiffHandler
{
protected:
    virtual void _HandleKey(NavNodeKeyCR newValue) = 0;
    virtual void _HandleHasChildren(bool newValue) = 0;
    virtual void _HandleIsChecked(bool newValue) = 0;
    virtual void _HandleIsCheckboxVisible(bool newValue) = 0;
    virtual void _HandleIsCheckboxEnabled(bool newValue) = 0;
    virtual void _HandleShouldAutoExpand(bool newValue) = 0;
    virtual void _HandleDescription(Utf8StringCR newValue) = 0;
    virtual void _HandleImageId(Utf8StringCR newValue) = 0;
    virtual void _HandleForeColor(Utf8StringCR newValue) = 0;
    virtual void _HandleBackColor(Utf8StringCR newValue) = 0;
    virtual void _HandleFontStyle(Utf8StringCR newValue) = 0;
    virtual void _HandleType(Utf8StringCR newValue) = 0;
    virtual void _HandleLabelDefinition(LabelDefinitionCR newValue) = 0;

public:
    virtual ~HierarchyChangeRecordDiffHandler() = default;

    void HandleKey(NavNodeKeyCR newValue) {_HandleKey(newValue);}
    void HandleHasChildren(bool newValue) {_HandleHasChildren(newValue);}
    void HandleIsChecked(bool newValue) {_HandleIsChecked(newValue);}
    void HandleIsCheckboxVisible(bool newValue) {_HandleIsCheckboxVisible(newValue);}
    void HandleIsCheckboxEnabled(bool newValue) {_HandleIsCheckboxEnabled(newValue);}
    void HandleShouldAutoExpand(bool newValue) {_HandleShouldAutoExpand(newValue);}
    void HandleDescription(Utf8StringCR newValue) {_HandleDescription(newValue);}
    void HandleImageId(Utf8StringCR newValue) {_HandleImageId(newValue);}
    void HandleForeColor(Utf8StringCR newValue) {_HandleForeColor(newValue);}
    void HandleBackColor(Utf8StringCR newValue) {_HandleBackColor(newValue);}
    void HandleFontStyle(Utf8StringCR newValue) {_HandleFontStyle(newValue);}
    void HandleType(Utf8StringCR newValue) {_HandleType(newValue);}
    void HandleLabelDefinition(LabelDefinitionCR newValue) {_HandleLabelDefinition(newValue);}
};

//=======================================================================================
//! ECInstance change type.
//! @ingroup GROUP_Presentation
// @bsiclass
//=======================================================================================
enum class ChangeType
    {
    Insert, //!< An ECInstance was inserted.
    Update, //!< An ECInstance was updated.
    Delete, //!< An ECInstance was deleted.
    };

//=======================================================================================
//! A class that represents multiple changes of a single node.
//! @ingroup GROUP_Presentation
// @bsiclass
//=======================================================================================
struct HierarchyChangeRecord
{
private:
    Utf8String m_dbFileName;
    Utf8String m_rulesetId;
    ChangeType m_changeType;
    NavNodeCPtr m_node;
    NavNodeCPtr m_parentNode;
    size_t m_position;
    NodeChanges m_nodeChanges;

public:
    //! A constructor for insert and delete cases.
    //! @param[in] node The inserted node.
    //! @param[in] position The insert position.
    HierarchyChangeRecord(ChangeType changeType, Utf8String rulesetId, Utf8String ecdbFileName, NavNodeCR node, NavNodeCPtr parentNode, size_t position)
        : m_changeType(changeType), m_rulesetId(std::move(rulesetId)), m_dbFileName(std::move(ecdbFileName)),
        m_node(&node), m_parentNode(parentNode), m_position(position)
        {}

    //! A constructor for the update case.
    //! @param[in] node The updated node.
    //! @param[in] changes The list of changes.
    HierarchyChangeRecord(Utf8String rulesetId, Utf8String ecdbFileName, NodeChanges nodeChanges)
        : m_changeType(ChangeType::Update), m_rulesetId(std::move(rulesetId)), m_dbFileName(std::move(ecdbFileName)),
        m_node(nodeChanges.GetUpdatedNode()), m_nodeChanges(nodeChanges)
        {}

    //! Get ID of the ruleset that was used to produce the node
    Utf8StringCR GetRulesetId() const {return m_rulesetId;}

    //! Get file name of the ECDb that was used to produce the node
    Utf8StringCR GetECDbFileName() const {return m_dbFileName;}

    //! Get the change type.
    ChangeType GetChangeType() const {return m_changeType;}

    //! Get the node.
    NavNodeCPtr GetNode() const {return m_node;}

    //! Get the changes that were applied to the node.
    //! @note Only valid for the update case.
    NodeChanges const& GetNodeChanges() const {return m_nodeChanges;}

    //! Get node's parent.
    //! @note only valid for the insert case.
    NavNodeCPtr GetParentNode() const {return m_parentNode;}

    //! Get node's index.
    //! @note Only valid for the insert case.
    size_t GetPosition() const {return m_position;}

    //! Set node's index.
    //! @note Only valid for the insert case.
    void SetPosition(size_t value) {m_position = value;}

    //! Get this record as JSON.
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* = nullptr) const;
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::Document::AllocatorType* = nullptr) const;
};

//=======================================================================================
//! A class that represents updated part of the hierarchy
//! @ingroup GROUP_Presentation
// @bsiclass
//=======================================================================================
struct HierarchyUpdateRecord
{
    //=======================================================================================
    //! A class that represents expanded node in updated part of the hierarchy.
    // @bsiclass
    //=======================================================================================
    struct ExpandedNode
    {
    private:
        NavNodeCPtr m_node;
        size_t m_position;

    public:
        ExpandedNode(NavNodeCR node, size_t position)
            : m_node(&node), m_position(position)
            {}
        NavNodeCPtr GetNode() const {return m_node;}
        size_t GetPosition() const {return m_position;}
        //! Get this node as JSON.
        ECPRESENTATION_EXPORT rapidjson::Document AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* = nullptr) const;
        ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::Document::AllocatorType* = nullptr) const;
    };

private:
    Utf8String m_dbFileName;
    Utf8String m_rulesetId;
    NavNodeCPtr m_parentNode;
    size_t m_nodesCount;
    bvector<ExpandedNode> m_expandedNodes;

public:
    HierarchyUpdateRecord(Utf8String rulesetId, Utf8String ecdbFileName, NavNodeCP parentNode, size_t nodesCount, bvector<ExpandedNode> expandedNodes = bvector<ExpandedNode>())
        : m_rulesetId(std::move(rulesetId)), m_dbFileName(std::move(ecdbFileName)), m_parentNode(parentNode), m_nodesCount(nodesCount), m_expandedNodes(std::move(expandedNodes))
        {}

    //! Get ID of the ruleset that was used to produce the hierarchy
    Utf8StringCR GetRulesetId() const { return m_rulesetId; }

    //! Get file name of the ECDb that was used to produce the hierarchy
    Utf8StringCR GetECDbFileName() const { return m_dbFileName; }

    //! Get parent node of updated hierarchy part
    NavNodeCPtr GetParentNode() const {return m_parentNode;}

    //! Get nodes count in hierarchy level after update
    size_t GetNodesCount() const {return m_nodesCount;}

    //! Get expanded nodes in hierarchy level
    bvector<ExpandedNode> const& GetExpandedNodes() const {return m_expandedNodes;}

    //! Get this record as JSON.
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(ECPresentationSerializerContextR ctx, rapidjson::Document::AllocatorType* = nullptr) const;
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::Document::AllocatorType* = nullptr) const;
};

//=======================================================================================
//! A class that represents a full update request.
//! @ingroup GROUP_Presentation
// @bsiclass
//=======================================================================================
struct FullUpdateRecord
{
    //===================================================================================
    //! Possible full update targets.
    // @bsiclass
    //===================================================================================
    enum class UpdateTarget
        {
        Hierarchy = 1 << 0,              //!< Update all hierarchy controls.
        Content   = 1 << 1,              //!< Update all content controls.
        Both      = Hierarchy | Content, //!< Update all controls including hierarchy and content ones.
        };

private:
    Utf8String m_dbFileName;
    Utf8String m_rulesetId;
    UpdateTarget m_target;

public:
    //! Constructor.
    //! @param[in] rulesetId The ID of the ruleset whose driven controls should be fully updated.
    //! @param[in] ecdbFileName The file name of the ECDb which was used by controls that should be fully updated.
    //! @param[in] target Specifies what should be updated.
    FullUpdateRecord(Utf8String rulesetId, Utf8String ecdbFileName, UpdateTarget target)
        : m_rulesetId(rulesetId), m_dbFileName(ecdbFileName), m_target(target) {}

    //! Get the ruleset ID.
    Utf8StringCR GetRulesetId() const {return m_rulesetId;}

    //! Get the ECDb file name.
    Utf8StringCR GetECDbFileName() const {return m_dbFileName;}

    //! Get the update target.
    UpdateTarget GetUpdateTarget() const {return m_target;}

    //! Set the update target.
    void SetUpdateTarget(UpdateTarget target) {m_target = target;}
};

//=======================================================================================
//! An interface for a class that takes and handles a number of update records.
//! @note Handler methods may be called from multiple different threads.
//! @ingroup GROUP_Presentation
// @bsiclass
//=======================================================================================
struct IUpdateRecordsHandler
    {
    protected:
        //! Called before update to clear caches.
        virtual void _Start() = 0;

        //! Called when hierarchy level is updated.
        //! @param[in] record   The update record that contains information about the change.
        virtual void _Accept(HierarchyUpdateRecord const& record) = 0;

        //! Called when a full update should be performed.
        //! @param[in] record   The update record that contains information about the update.
        virtual void _Accept(FullUpdateRecord const& record) = 0;

        //! Called when the update is finished.
        virtual void _Finish() = 0;

    public:
        virtual ~IUpdateRecordsHandler() {}

        //! Starts a new report.
        void Start() { _Start(); }

        //! Accepts an @ref HierarchyUpdateRecord.
        void Accept(HierarchyUpdateRecord const& record) { _Accept(record); }

        //! Accepts a @ref FullUpdateRecord.
        void Accept(FullUpdateRecord const& record) { _Accept(record); }

        //! Finishes the report.
        void Finish() { _Finish(); }
    };

//=======================================================================================
//! An interface for a class that takes and handles a number of hierarchy change records.
//! @note Handler methods may be called from multiple different threads.
//! @ingroup GROUP_Presentation
// @bsiclass
//=======================================================================================
struct IHierarchyChangeRecordsHandler
{
protected:
    //! Called before update to clear caches.
    virtual void _Start() = 0;

    //! Called when a node is inserted / deleted / updated.
    //! @param[in] record   The update record that contains information about the change.
    virtual void _Accept(HierarchyChangeRecord const& record) = 0;

    //! Called when the update is finished.
    virtual void _Finish() = 0;

public:
    virtual ~IHierarchyChangeRecordsHandler() {}

    //! Starts a new report.
    void Start() {_Start();}

    //! Accepts an @ref HierarchyChangeRecord.
    void Accept(HierarchyChangeRecord const& record) {_Accept(record);}

    //! Finishes the report.
    void Finish() {_Finish();}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
