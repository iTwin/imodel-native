/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/Update.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once 
//__PUBLISH_SECTION_START__

#include <ECPresentation/ECPresentation.h>
#include <ECPresentation/SelectionManager.h>
#include <ECPresentation/NavNode.h>

//__PUBLISH_SECTION_END__
struct ReportTask;
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

struct UpdateHandler;

//=======================================================================================
//! A class that represents a single property change.
//! @ingroup GROUP_Presentation
// @bsiclass                                    Grigas.Petraitis                02/2016
//=======================================================================================
struct JsonChange
{
private:
    Utf8String m_name;
    rapidjson::Value m_oldValue;
    rapidjson::Value m_newValue;
    rapidjson::MemoryPoolAllocator<> m_allocator;

public:
    //! Constructor.
    //! @param[in] name     The name of the property that changed.
    //! @param[in] oldValue Value before the change.
    //! @param[in] newValue Value after the change.
    JsonChange(Utf8CP name, RapidJsonValueCR oldValue, RapidJsonValueCR newValue)
         : m_name(name), m_allocator(128)
        {
        m_oldValue = rapidjson::Value(oldValue, m_allocator);
        m_newValue = rapidjson::Value(newValue, m_allocator);
        }

    //! Copy constructor.
    JsonChange(const JsonChange& obj)
        {
        m_name = obj.m_name;
        m_oldValue.CopyFrom(obj.m_oldValue, m_allocator);
        m_newValue.CopyFrom(obj.m_newValue, m_allocator);
        }
    
    //! Is this change equal to the supplied one.
    JsonChange& operator=(JsonChange const& other)
        {
        m_name = other.m_name;
        m_oldValue.CopyFrom(other.m_oldValue, m_allocator);
        m_newValue.CopyFrom(other.m_newValue, m_allocator);
        return *this;
        }

    //! Get the name of the property that changed.
    Utf8CP GetName() const {return m_name.c_str();}

    //! Get value before the change.
    RapidJsonValueCR GetOldValue() const {return m_oldValue;}

    //! Get value after the change.
    RapidJsonValueCR GetNewValue() const {return m_newValue;}
};

//=======================================================================================
//! ECInstance change type.
//! @ingroup GROUP_Presentation
// @bsiclass                                        Grigas.Petraitis            01/2016
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
// @bsiclass                                    Grigas.Petraitis                02/2016
//=======================================================================================
struct UpdateRecord
{
private:
    ChangeType m_changeType;
    NavNodePtr m_node;
    bvector<JsonChange> m_changes;
    size_t m_position;

public:
    //! A constructor for the insert case.
    //! @param[in] node The inserted node.
    //! @param[in] position The insert position.
    UpdateRecord(NavNodeR node, size_t position) : m_changeType(ChangeType::Insert), m_node(&node), m_position(position) {}

    //! A constructor for the delete case.
    //! @param[in] node The deleted node.
    UpdateRecord(NavNodeR node) : m_changeType(ChangeType::Delete), m_node(&node) {}

    //! A constructor for the update case.
    //! @param[in] node The updated node.
    //! @param[in] changes The list of changes.
    UpdateRecord(NavNodeR node, bvector<JsonChange>&& changes)
        : m_changeType(ChangeType::Update), m_node(&node), m_changes(std::move(changes))
        {}

    //! Copy constructor.
    UpdateRecord(UpdateRecord const& other)
        : m_changeType(other.m_changeType), m_node(other.m_node), m_changes(other.m_changes), m_position(other.m_position)
        {}

    //! Move constructor.
    UpdateRecord(UpdateRecord&& other)
        : m_changeType(other.m_changeType), m_node(std::move(other.m_node)), m_changes(std::move(other.m_changes)), m_position(other.m_position)
        {}

    //! Assignment operator
    UpdateRecord& operator=(UpdateRecord const& other)
        {
        m_changeType = other.m_changeType;
        m_node = other.m_node;
        m_changes = other.m_changes;
        m_position = other.m_position;
        return *this;
        }

    //! Get the change type.
    ChangeType GetChangeType() const {return m_changeType;}
    
    //! Get the node.
    NavNodePtr GetNode() const {return m_node;}

    //! Get the changes that were applied to the node.
    //! @note Only valid for the update case.
    bvector<JsonChange> const& GetChanges() const {return m_changes;}

    //! Get node's index.
    //! @note Only valid for the insert case.
    size_t GetPosition() const {return m_position;}

    //! Set node's index.
    //! @note Only valid for the insert case.
    void SetPosition(size_t value) {m_position = value;}

    //! Get this record as JSON.
    ECPRESENTATION_EXPORT rapidjson::Document AsJson(rapidjson::Document::AllocatorType* = nullptr) const;
};

//=======================================================================================
//! A class that represents a full update request.
//! @ingroup GROUP_Presentation
// @bsiclass                                    Grigas.Petraitis                04/2016
//=======================================================================================
struct FullUpdateRecord
{
    //===================================================================================
    //! Possible full update targets.
    // @bsiclass                                Grigas.Petraitis                04/2016
    //===================================================================================
    enum class UpdateTarget
        {
        Hierarchy = 1 << 0,              //!< Update all hierarchy controls.
        Content   = 1 << 1,              //!< Update all content controls.
        Both      = Hierarchy | Content, //!< Update all controls including hierarchy and content ones.
        };
private:
    Utf8String m_rulesetId;
    UpdateTarget m_target;
public:
    //! Constructor.
    //! @param[in] rulesetId The ID of the ruleset whose driven controls should be fully updated.
    //! @param[in] target Specifies what should be updated.
    FullUpdateRecord(Utf8String rulesetId, UpdateTarget target) : m_rulesetId(rulesetId), m_target(target) {}

    //! Copy constructor.
    FullUpdateRecord(FullUpdateRecord const& other) : m_rulesetId(other.m_rulesetId), m_target(other.m_target) {}

    //! Move constructor.
    FullUpdateRecord(FullUpdateRecord&& other) : m_rulesetId(std::move(other.m_rulesetId)), m_target(other.m_target) {}

    //! Copy assignment operator.
    FullUpdateRecord& operator=(FullUpdateRecord const& other) {m_rulesetId = other.m_rulesetId; m_target = other.m_target; return *this;}

    //! Get the ruleset ID.
    Utf8StringCR GetRulesetId() const {return m_rulesetId;}

    //! Get the update target.
    UpdateTarget GetUpdateTarget() const {return m_target;}

    //! Set the update target.
    void SetUpdateTarget(UpdateTarget target) {m_target = target;}
};

//=======================================================================================
//! An interface for a class that takes and handles a number of update records.
//! @note Handler methods may be called from multiple different threads.
//! @ingroup GROUP_Presentation
// @bsiclass                                    Grigas.Petraitis                02/2016
//=======================================================================================
struct IUpdateRecordsHandler : RefCountedBase
{
protected:
    //! Called before update to clear caches.
    virtual void _Start() = 0;

    //! Called when a node is inserted / deleted / updated.
    //! @param[in] record   The update record that contains information about the change.
    virtual void _Accept(UpdateRecord const& record) = 0;
    
    //! Called when a full update should be performed.
    //! @param[in] record   The update record that contains information about the update.
    virtual void _Accept(FullUpdateRecord const& record) = 0;

    //! Called when the update is finished.
    virtual void _Finish() = 0;

public:
    //! Starts a new report.
    void Start() {_Start();}

    //! Accepts an @ref UpdateRecord.
    void Accept(UpdateRecord const& record) {_Accept(record);}

    //! Accepts a @ref FullUpdateRecord.
    void Accept(FullUpdateRecord const& record) {_Accept(record);}

    //! Finishes the report.
    void Finish() {_Finish();}
};

//=======================================================================================
//! Takes care of updating selection when hierarchies change (e.g. removes deleted nodes
//! from selection set).
//! @ingroup GROUP_RulesDrivenPresentation
// @bsiclass                                    Grigas.Petraitis                04/2018
//=======================================================================================
struct SelectionUpdateRecordsHandler : IUpdateRecordsHandler
{
private:
    IConnectionCache const& m_connections;
    ISelectionManager& m_selectionManager;
    bmap<IConnectionCP, NavNodeKeyList> m_toRemove;
    bset<IConnectionCP> m_toRefresh;
private:
    SelectionUpdateRecordsHandler(IConnectionCache const& connections, ISelectionManager& selectionManager)
        : m_connections(connections), m_selectionManager(selectionManager)
        {}
protected:
    ECPRESENTATION_EXPORT void _Start() override;
    ECPRESENTATION_EXPORT void _Accept(UpdateRecord const& record) override;
    ECPRESENTATION_EXPORT void _Accept(FullUpdateRecord const& record) override;
    ECPRESENTATION_EXPORT void _Finish() override;
public:
    static RefCountedPtr<SelectionUpdateRecordsHandler> Create(IConnectionCache const& connections, ISelectionManager& selectionManager)
        {
        return new SelectionUpdateRecordsHandler(connections, selectionManager);
        }
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
