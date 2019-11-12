/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//__PUBLISH_SECTION_START__
#pragma once
#include <DgnPlatform/RepositoryManager.h>
#include <DgnPlatform/DgnChangeSummary.h>
#include <BeSQLite/BeSQLite.h>
#include <ECDb/ChangeSummary.h>
#include <ECObjects/ECObjects.h>
#include <ECObjects/ECSchema.h>
#include <ECPresentation/IECPresentationManager.h>
#include <Geom/GeomApi.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

USING_NAMESPACE_BENTLEY_ECPRESENTATION

struct VersionCompareChangeSummary;
typedef RefCountedPtr<struct VersionCompareChangeSummary> VersionCompareChangeSummaryPtr;

typedef bset<BentleyApi::BeSQLite::EC::ECInstanceKey> ECInstanceKeySet;

//=======================================================================================
// Contains helper methods to query for changed elements
// This change summary class also handles schema changes and it works by creating
// multiple change summaries as needed by the given changesets
//
// @bsistruct                                                   Diego.Pinate    08/17
//=======================================================================================
struct VersionCompareChangeSummary : RefCountedBase
    {
    //=======================================================================================
    // @bsistruct                                                   Diego.Pinate    09/17
    //=======================================================================================
    struct SummaryElementInfo
        {
        BentleyApi::BeSQLite::DbOpcode          m_opcode;
        BentleyApi::ECN::ECClassId              m_ecclassId;
        BentleyApi::Dgn::DgnModelId             m_modelId;
        BentleyApi::AxisAlignedBox3d            m_bbox;

        DGNPLATFORM_EXPORT void AccumulateChange(SummaryElementInfo info, bool backwards);
        DGNPLATFORM_EXPORT bool IsValid();
        DGNPLATFORM_EXPORT void Invalidate();

        SummaryElementInfo(BentleyApi::BeSQLite::DbOpcode opcode, BentleyApi::ECN::ECClassId classId, BentleyApi::Dgn::DgnModelId modelId, BentleyApi::AxisAlignedBox3d bbox) :
            m_opcode(opcode), m_ecclassId(classId), m_modelId(modelId), m_bbox(bbox) { }

        SummaryElementInfo() { }
        }; // SummaryElementInfo

    //=======================================================================================
    // @bsistruct                                                   Diego.Pinate    09/17
    //=======================================================================================
    struct RelatedPathCache
        {
        struct PathNames
            {
            Utf8String m_sourceClassName;
            Utf8String m_relationshipClassName;
            Utf8String m_targetClassName;
            PathNames() { }
            PathNames(Utf8String src, Utf8String rel, Utf8String trg) : m_sourceClassName(src), m_relationshipClassName(rel), m_targetClassName(trg) { }
            bool operator<(const PathNames& rhs) const
                {
                return Utf8String(m_sourceClassName + m_relationshipClassName + m_targetClassName) <
                       Utf8String(rhs.m_sourceClassName + rhs.m_relationshipClassName + rhs.m_targetClassName);
                }
            };
    private:
        bset<PathNames>  m_data;
    public:
        int     size() { return m_data.size(); }
        bool    IsValid() { return !m_data.empty(); }
        void    Add(Utf8String src, Utf8String rel, Utf8String trg) { m_data.insert(PathNames(src, rel, trg)); }

        bset<PathNames> const&     Paths() const { return m_data; }
        }; // RelatedPathCache

private:
    IECPresentationManagerR m_presentationManager;
    BentleyApi::Dgn::DgnDbPtr                               m_targetDb;
    BentleyApi::Dgn::DgnDbR                                 m_db;
    bvector<BentleyApi::Dgn::DgnRevisionPtr>                m_changesets;
    bmap<BentleyApi::BeSQLite::EC::ECInstanceKey, SummaryElementInfo> m_changedElements;
    Utf8String  m_rulesetId;

    bool    m_filterSpatial;
    bool    m_filterLastMod;

    bmap<BentleyApi::BeSQLite::EC::ECInstanceKey, BentleyApi::BeSQLite::DbOpcode>    m_changedInstances;

    BentleyApi::BeSQLite::EC::ECSqlStatement        m_isElementStatement;
    BentleyApi::BeSQLite::EC::ECSqlStatement        m_isOfStatement;
    BentleyApi::BeSQLite::EC::ECSqlStatementCache*  m_statementCache;

    bmap<Utf8String, bvector<BentleyApi::ECPresentation::SelectClassInfo>>  m_classPathCache;
    bmap<Utf8String, RelatedPathCache>  m_relatedClassCache;

    bool    m_backwardsComparison;

    //! Gets a cached sql statement
    BentleyApi::BeSQLite::EC::CachedECSqlStatementPtr GetCachedStatement(BentleyApi::Dgn::DgnDbR db, Utf8String sql);
    //! Gets the changesets separated by the ones that contain schema changes and such
    StatusInt   GetAppliableChangesets(bvector<bvector<BentleyApi::Dgn::DgnRevisionPtr>>& appliableChangesets);
    //! Processes the given changesets to generate change summaries
    StatusInt   ProcessChangesets();
    //! De-allocates memory for change summary pointers
    void        CleanUp();
    //! Rolls a db, closes it and re-opens it to avoid conflicts with schema changes
    StatusInt   RollTargetDb(bvector<BentleyApi::Dgn::DgnRevisionPtr> const& changesets, bool backwardsComparison);
    //! Cache all related class paths of a class
    void        CacheRelatedPaths(Utf8String schemaName, Utf8String className);
    //! Process the contents of a change summary to obtain the changed elements
    void        ProcessChangeSummary(BentleyApi::Dgn::DgnChangeSummary* changeSummary, ECInstanceKeySet& inserted, ECInstanceKeySet& updated, ECInstanceKeySet& deleted);
    //! Process a set of changed element IDs and their opcode. Accumulates the changes across change summaries
    void        ProcessChangedElements(ECInstanceKeySet const& changedIntancesKeys, BentleyApi::BeSQLite::DbOpcode opcode);
    //! Clones the current db to obtain a target one
    static BentleyApi::Dgn::DgnDbPtr    CloneDb(BentleyApi::Dgn::DgnDbR db);
    //! Gets changed instances of a class
    void        GetChangedInstances(ECInstanceKeySet& instanceIds, BentleyApi::Dgn::DgnChangeSummary* changeSummary, BentleyApi::ECN::ECClassId ecclassId, BentleyApi::BeSQLite::EC::ChangeSummary::QueryDbOpcode opcode);
//! Obtain the changes of instances of a particular class based on presentation rules related paths
    void        AddInstancesWithPresentationRulesUpdates(ECInstanceKeySet& instanceIds, Utf8String schemaName, Utf8String className, BentleyApi::Dgn::DgnChangeSummary* changeSummary, ECInstanceKeySet const& insertedInstances, ECInstanceKeySet const& deletedInstances);

    //! Functions adapted from Raman's DgnChangeSummary class to obtain changed elements by looking at their aspects
    void            FindChangedRelationshipEndIds (BentleyApi::Dgn::DgnChangeSummary* changeSummary, BentleyApi::BeSQLite::EC::ECInstanceIdSet& endInstanceIds, Utf8CP relationshipSchemaName, Utf8CP relationshipClassName, BentleyApi::ECN::ECRelationshipEnd relationshipEnd, BentleyApi::BeSQLite::EC::ChangeSummary::QueryDbOpcode opcode);
    void            FindUpdatedInstanceIds(BentleyApi::Dgn::DgnChangeSummary* changeSummary, BentleyApi::BeSQLite::EC::ECInstanceIdSet& updatedInstanceIds, Utf8CP schemaName, Utf8CP className, BentleyApi::BeSQLite::EC::ChangeSummary::QueryDbOpcode opcode);
    void            FindRelatedInstanceIds(ECInstanceKeySet& relatedInstances, Utf8CP ecsql, BentleyApi::BeSQLite::EC::ECInstanceIdSet const& inInstances, BentleyApi::BeSQLite::EC::ChangeSummary::QueryDbOpcode opcode);
    StatusInt       ParseClassFullName(Utf8StringR schemaName, Utf8StringR className, Utf8CP classFullName);
    StatusInt       GetInstancesWithAspectUpdates(BentleyApi::Dgn::DgnChangeSummary* changeSummary, ECInstanceKeySet& instances, Utf8CP elementClassFullName, Utf8CP aspectRelationshipClassFullName, Utf8CP aspectClassFullName);

    //! Get an element pointer by searching the correct Db based on the opcode
    DgnElementCPtr  GetElement(BentleyApi::Dgn::DgnElementId elementId, BentleyApi::BeSQLite::DbOpcode opcode);

    //! Gets the content classes from Presentation Rules to know the paths relevant for change inspection
    bvector<BentleyApi::ECPresentation::SelectClassInfo> GetContentClasses(Utf8String schemaName, Utf8String className);

    //! Gets the content of an element's properties using presentation rules
    StatusInt   GetElementContent(BentleyApi::Dgn::DgnDbPtr db, BentleyApi::Dgn::DgnElementId elementId, BentleyApi::ECN::ECClassId ecclassId, JsonValueR content);

    //! Constructor
    VersionCompareChangeSummary(BentleyApi::Dgn::DgnDbR db, IECPresentationManagerR presentationManager, bool backwards) : m_db(db), m_targetDb(nullptr), m_presentationManager(presentationManager), m_statementCache(nullptr), m_backwardsComparison(backwards), m_filterSpatial(false), m_filterLastMod(false) { }

    //! This method will process the changesets and obtain all the changed instances
    //! @param[in] changesets Vector of DgnRevisionPtr containing the changesets to compile together
    StatusInt   SetChangesets(bvector<BentleyApi::Dgn::DgnRevisionPtr>& changesets);

    //! Gets the top assembly based on an instance ID. Used to correctly obtain top-level node for property comparison
    //! @param[out] topInstanceId ECInstanceId of the top assembly to be used in GetElementContent calls
    //! @param[out] classId ECClassId of the top assembly
    //! @param[in] db to be used to obtain the top assembly
    //! @param[in] instanceId of the element that is being inspected
    StatusInt           GetTopAssembly(BentleyApi::BeSQLite::EC::ECInstanceId& topInstanceId, BentleyApi::ECN::ECClassId& classId, BentleyApi::Dgn::DgnDbPtr db, BentleyApi::Dgn::DgnElementId instanceId);

public:
    DGNPLATFORM_EXPORT ~VersionCompareChangeSummary();

    //! Get the target DgnDb that is computed throughout generation of the summary
    //! @return target db, may be invalid if no comparison has been processed
    DGNPLATFORM_EXPORT BentleyApi::Dgn::DgnDbPtr GetTargetDb();

    //! Gets the comparison of properties for an element in JSON format, flattens out nested content for easier inspection
    //! Formatting is like so:
    //! { "UniqueAspect -> PropertyA": {
    //! path: "UniqueAspect ->",
    //! propertyName: "PropertyA",
    //! displayLabel: "Property A",
    //! currentValue: ...,
    //! currentDisplayValue: ...,
    //! targetValue: ...,
    //! targetDisplayValue: ...,
    //! },
    //! "PropertyB": { ... } }}
    //! @param[in] elementId Element to inspect
    //! @param[in] ecclassId ECClass of element
    //! @param[in] changedOnly only return properties with changes
    //! @param[out] content JSON object containing data
    DGNPLATFORM_EXPORT StatusInt     GetPropertyComparison(BentleyApi::Dgn::DgnElementId elementId, BentleyApi::ECN::ECClassId ecclassId, bool changedOnly, JsonValueR content);

    //! Obtain both the current and target state's properties for an element that has changed into a JSON object
    //! @param[in] elementId ID of an element that changed
    //! @param[in] ecclassId ECClassId of the element
    //! @param[in] changedOnly determines if this call should only return the properties that changed on the element
    //! @param[out] content The JSON object containing all the property information merged from presentation rules
    DGNPLATFORM_EXPORT StatusInt     GetPropertyContentComparison(BentleyApi::Dgn::DgnElementId elementId, BentleyApi::ECN::ECClassId ecclassId, bool changedOnly, JsonValueR content, bool nestedContent = false);

    //! Obtain both the current and target state's properties for an element that has changed into a JSON object
    //! @param[in] elementId ID of an element that changed
    //! @param[in] ecclassId ECClassId of the element
    //! @param[in] changedOnly determines if this call should only return the properties that changed on the element
    //! @param[out] content The JSON object containing all the property information merged from presentation rules
    DGNPLATFORM_EXPORT StatusInt     GetPropertyContentComparisonNested(BentleyApi::Dgn::DgnElementId elementId, BentleyApi::ECN::ECClassId ecclassId, bool changedOnly, JsonValueR content);

    //! Returns vectors for element ids, ecclass ids and opcodes of the elements that have changed
    //! @param[out] elementIds Element IDs of elements affected by changesets
    //! @param[out] ecclassIds ECClass IDs for all elementIds
    //! @param[out] opcodes Types of changes of each of the elements
    DGNPLATFORM_EXPORT StatusInt   GetChangedElements(bvector<BentleyApi::Dgn::DgnElementId>& elementIds, bvector<BentleyApi::ECN::ECClassId>& ecclassIds, bvector<BentleyApi::BeSQLite::DbOpcode>& opcodes, bvector<BentleyApi::Dgn::DgnModelId>& modelIds, bvector<BentleyApi::AxisAlignedBox3d>& bboxes);

    //! Returns changed elements that are of a particular class
    //! @param[out] elementIds Elements that changed
    //! @param[out] opcodes types of changes
    //! @param[in] classp ECClassCP being looked for
    DGNPLATFORM_EXPORT StatusInt     GetChangedElementsOfClass(bvector<BentleyApi::Dgn::DgnElementId>& elementIds, bvector<BentleyApi::BeSQLite::DbOpcode>& opcodes, bvector<BentleyApi::Dgn::DgnModelId>& modelIds, bvector<BentleyApi::AxisAlignedBox3d>& bboxes, BentleyApi::ECN::ECClassCP classp);

    //! Get the models that changed
    //! @param[out] modelIds all DgnModelId that changed
    //! @param[out] opcodes types of changes
    DGNPLATFORM_EXPORT StatusInt     GetChangedModels(bset<BentleyApi::Dgn::DgnModelId>& modelIds, bvector<BentleyApi::BeSQLite::DbOpcode>& opcodes);


    //! More performant version of GetElement call by providing the class Id no need to search for the instance key
    //! @param[out] element The element given the ElementID and ECClassId
    //! @param[in] elementId ID of element
    //! @param[in] classId ID of the ecclasss of the element
    //! @param[in] targetState whether it should be obtained from the temporary Db or not
    //! @return SUCCESS if element ID found
    DGNPLATFORM_EXPORT StatusInt     GetElement(BentleyApi::Dgn::DgnElementCPtr& element, BentleyApi::Dgn::DgnElementId elementId, BentleyApi::ECN::ECClassId classId, bool targetState);

    //! Gets the element in the current or target db
    //! Returned element pointer may be invalid if the element is not present in the wanted db (targetState defines which db is used)
    //! The method will return ERROR only if the element ID is not found in the changed elements or if comparison hasn't started
    //! @param[out] element the element given an ID, it may be an invalid pointer if the element is not present in such state
    //! @param[in] elementId the ID of the element
    //! @param[in] targetState this should be TRUE for obtaining elements in the target DB
    //! @return SUCCESS if the element ID is found in the change summary, ERROR if not
    DGNPLATFORM_EXPORT StatusInt     GetElement(BentleyApi::Dgn::DgnElementCPtr& element, BentleyApi::Dgn::DgnElementId elementId, bool targetState);

    //! Creates a change summary that compares the db after applying the given changesets
    //! This will generate a temporary Db that is rolled to the target state to be able to obtain elements from it
    //! This db will be destroyed once the version compare change summary is destroyed
    //! Note: The Generate function above should be preferred over this function, but if needed, use VersionSelector::GetChangeSetsToApply to
    //! obtain the changesets and the roll direction
    //! @param[in] db current DgnDb
    //! @param[in] changesets Vector of DgnRevisionPtr containing all changesets to be applied
    //! @param[in] rulesetId Name of the presentation rules to use
    //! @param[in] backwardsRoll whether this changesets are applied to go backwards or forwards in the db's history
    //! @return VersionCompareChangeSummaryPtr with the results
    DGNPLATFORM_EXPORT static VersionCompareChangeSummaryPtr Generate(BentleyApi::Dgn::DgnDbR db, bvector<BentleyApi::Dgn::DgnRevisionPtr> &changesets, IECPresentationManagerR presentationManager, Utf8String rulesetId, bool backwardsRoll);

    //! Creates a change summary that compares the db after applying the given changesets
    //! This will generate a temporary Db that is rolled to the target state to be able to obtain elements from it
    //! This db will be destroyed once the version compare change summary is destroyed
    //! Note: The Generate function above should be preferred over this function, but if needed, use VersionSelector::GetChangeSetsToApply to
    //! obtain the changesets and the roll direction
    //! @param[in] db current DgnDb
    //! @param[in] changesets Vector of DgnRevisionPtr containing all changesets to be applied
    //! @param[in] rulesetId Name of the Presentation Rulesets to use
    //! @param[in] backwardsRoll whether this changesets are applied to go backwards or forwards in the db's history
    //! @param[in] filterSpatial whether to only show SpatialElements in the results
    //! @param[in] filterLastMod whether to filter out updates that only have last modified date changes
    //! @return VersionCompareChangeSummaryPtr with the results
    DGNPLATFORM_EXPORT static VersionCompareChangeSummaryPtr     Generate(BentleyApi::Dgn::DgnDbR db, bvector<BentleyApi::Dgn::DgnRevisionPtr>& changesets, IECPresentationManagerR presentationManager, Utf8String rulesetId, bool backwardsRoll, bool filterSpatial, bool filterLastMod);
    }; // VersionCompareChangeSummary

typedef bmap<BentleyApi::BeSQLite::EC::ECInstanceKey, VersionCompareChangeSummary::SummaryElementInfo> ChangedElementsMap;

END_BENTLEY_DGNPLATFORM_NAMESPACE
//__PUBLISH_SECTION_END__
