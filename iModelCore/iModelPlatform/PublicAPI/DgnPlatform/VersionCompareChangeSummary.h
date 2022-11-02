/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <DgnPlatform/DgnChangeSummary.h>
#include <BeSQLite/BeSQLite.h>
#include <ECDb/ChangeSummary.h>
#include <ECObjects/ECObjects.h>
#include <ECObjects/ECSchema.h>
#include <ECPresentation/ECPresentationManager.h>
#include <Geom/GeomApi.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

USING_NAMESPACE_BENTLEY_ECPRESENTATION

#define STATEMENT_CACHE_SIZE 50

struct VersionCompareChangeSummary;
typedef RefCountedPtr<struct VersionCompareChangeSummary> VersionCompareChangeSummaryPtr;

typedef bmap<BentleyApi::ECN::ECClassId, bset<Utf8String>> HiddenPropertyMap;

#define VC_DEFAULT_RELATIONSHIP_CACHE_SIZE 200000

//=======================================================================================
// Used to maintain hidden property cache between runs of processing to ensure
// the agent caches properties from each run without maintaining it in typescript
// @bsistruct
//=======================================================================================
struct HiddenPropertyCache
    {
    // Map of hidden properties based on iModel Id
    HiddenPropertyMap m_cache;
    // SQL statement for querying properties
    BentleyApi::BeSQLite::Statement m_hiddenPropertiesStmt;

    void Clear();
    void CleanUpStatement();
    bool Has(BentleyApi::ECN::ECClassId classId);
    bool IsHiddenProperty(DgnDbR db, BentleyApi::ECN::ECClassId classId, Utf8CP property);
    void QueryHiddenProperties(DgnDbR db, BentleyApi::ECN::ECClassId classId);
    }; // HiddenPropertyCache

//=======================================================================================
// Options to generate version compare change summary
// @bsistruct
//=======================================================================================
struct SummaryOptions
    {
    bool filterSpatial;
    bool filterLastMod;
    bool wantTargetState;
    bool wantParents;
    bool wantPropertyChecksums;
    bool wantBriefcaseRoll;
    bool wantRelationshipCaching;
    bool wantChunkTraversal;
    int relationshipCacheSize;

    ECPresentationManager* presentationManager;

    BeFileName tempLocation;
    Utf8String rulesetId;

    SummaryOptions()
        {
        filterSpatial = false;
        filterLastMod = true;
        wantTargetState = false;
        wantChunkTraversal = false;
        wantParents = false;
        wantPropertyChecksums = true;
        wantBriefcaseRoll = false;
        presentationManager = nullptr;
        wantRelationshipCaching = true;
        // Default to 1 million max cache entries
        relationshipCacheSize = VC_DEFAULT_RELATIONSHIP_CACHE_SIZE;
        }
    }; // SummaryOptions

//=======================================================================================
// Maintain the checksum of the old and new values for a property
// Used to determine whether a property was flipped back and forth to the same value
// @bsistruct
//=======================================================================================
struct PropertyChecksum
    {
    uint32_t m_oldValue;
    uint32_t m_newValue;

    PropertyChecksum(): m_oldValue(0), m_newValue(0) { }
    PropertyChecksum(uint32_t oldValue, uint32_t newValue): m_oldValue(oldValue), m_newValue(newValue) { }
    };

//=======================================================================================
// Used to maintain type of change for instances during processing
// @bsistruct
//=======================================================================================
struct ElementChangesType
    {
    // Type of change flags
    int m_flags;
    // Set of changed properties
    bset<Utf8String> m_properties;
    // Map of property name -> property checksums for the change
    bmap<Utf8String, PropertyChecksum> m_propertyChecksums;

    enum Type
        {
        Mask_Property   = 0b1,
        Mask_Geometry   = 0b10,
        Mask_Placement  = 0b100,
        Mask_Indirect   = 0b1000,
        Mask_Hidden     = 0b10000,
        Mask_Parent     = 0b100000,
        };

    ElementChangesType() { m_flags = 0; }
    ElementChangesType(int flags)
        {
        m_flags = flags;
        }
    ElementChangesType(int flags, bvector<Utf8String> const& props)
        {
        for (Utf8String const& prop : props)
            AddProperty(prop);

        m_flags = flags;
        }
    ElementChangesType(int flags, bvector<Utf8String> const& props, bvector<uint32_t> const& oldChecksums, bvector<uint32_t> const& newChecksums)
        {
        BeAssert(props.size() == oldChecksums.size());
        BeAssert(props.size() == newChecksums.size());
        for (size_t i = 0; i < props.size(); ++i)
            AddProperty(props[i], oldChecksums[i], newChecksums[i]);

        m_flags = flags;
        }
    ElementChangesType(ElementChangesType const& other)
        {
        for (Utf8String const& prop : other.m_properties)
            {
            auto pair = other.m_propertyChecksums.find(prop);
            if (pair != other.m_propertyChecksums.end())
                AddProperty(prop, pair->second.m_oldValue, pair->second.m_newValue);
            else
                AddProperty(prop);
            }


        m_flags = other.m_flags;
        }

    void ClearProperties()
        {
        m_properties.clear();
        }

    void AddProperty(Utf8String const& in)
        {
        m_properties.insert(in);
        m_propertyChecksums[in] = PropertyChecksum(0, 0);
        }

    void AddProperty(Utf8String const& in, uint32_t oldValueChecksum, uint32_t newValueChecksum)
        {
        m_properties.insert(in);
        m_propertyChecksums[in] = PropertyChecksum(oldValueChecksum, newValueChecksum);
        }

    void MergeProperty(Utf8String const& prop, PropertyChecksum checksum)
        {
        if (m_properties.find(prop) == m_properties.end())
            {
            m_properties.insert(prop);
            m_propertyChecksums[prop] = checksum;
            }
        else
            {
            // Update the value checksum based on compare direction
            m_propertyChecksums[prop].m_newValue = checksum.m_newValue;
            }
        }

    void Merge(ElementChangesType const& other)
        {
        for (Utf8String const& prop : other.m_properties)
            {
            auto pair = other.m_propertyChecksums.find(prop);
            if (pair != other.m_propertyChecksums.end())
                {
                auto const& checksums = pair->second;
                MergeProperty(prop, checksums);
                }
            }

        m_flags |= other.m_flags;
        }

    void MergeProperties(ElementChangesType const& other)
        {
        m_flags |= ElementChangesType::Type::Mask_Property;

        for (Utf8String const& prop : other.m_properties)
            {
            auto pair = other.m_propertyChecksums.find(prop);
            if (pair != other.m_propertyChecksums.end())
                {
                auto const& checksums = pair->second;
                MergeProperty(prop, checksums);
                }
            }
        }

    bool HasChange(Type mask)
        {
        return (m_flags & mask) == mask;
        }

    void AddType(Type mask)
        {
        m_flags |= mask;
        }

    bool Equals(ElementChangesType const& other)
        {
        return m_flags == other.m_flags;
        }
    };

typedef bset<BentleyApi::BeSQLite::EC::ECInstanceKey> ECInstanceKeySet;
typedef bmap<BentleyApi::BeSQLite::EC::ECInstanceKey, ElementChangesType> ChangesTypeMap;


//=======================================================================================
// Simple struct for maintaining information about a changed element
// @bsistruct
//=======================================================================================
struct ChangedElement
    {
    BentleyApi::Dgn::DgnElementId           m_elementId;
    BentleyApi::ECN::ECClassId              m_classId;
    BentleyApi::Dgn::DgnModelId             m_modelId;
    BentleyApi::AxisAlignedBox3d            m_bbox;
    BentleyApi::BeSQLite::DbOpcode          m_opcode;
    BentleyApi::BeSQLite::EC::ECInstanceKey m_parentKey;
    ElementChangesType                      m_changes;

    ChangedElement(
        BentleyApi::Dgn::DgnElementId   elementId,
        BentleyApi::ECN::ECClassId      classId,
        BentleyApi::Dgn::DgnModelId     modelId,
        BentleyApi::AxisAlignedBox3d    bbox,
        BentleyApi::BeSQLite::DbOpcode  opcode,
        ElementChangesType              changes,
        BentleyApi::BeSQLite::EC::ECInstanceKey parentKey
        ): m_elementId(elementId), m_classId(classId), m_modelId(modelId), m_bbox(bbox), m_opcode(opcode), m_changes(changes), m_parentKey(parentKey) { }
    }; // ChangedElement

//=======================================================================================
// Struct to maintain a related property path
// @bsistruct
//=======================================================================================
struct PathNames
    {
    BentleyApi::ECN::ECClassId m_relationshipClassId;
    Utf8String m_sourceClassName;
    Utf8String m_relationshipClassName;
    Utf8String m_targetClassName;
    bool m_forward;

    PathNames() { }
    PathNames(Utf8StringCR src, Utf8StringCR rel, Utf8StringCR trg, BentleyApi::ECN::ECClassId const& relClassId, bool forward)
        : m_sourceClassName(src), m_relationshipClassName(rel), m_relationshipClassId(relClassId), m_targetClassName(trg), m_forward(forward) { }

    Utf8String MakeKey() const
        {
        return Utf8String(m_sourceClassName + m_relationshipClassName + m_targetClassName + (m_forward ? "F" : "B"));
        }

    bool operator<(const PathNames& rhs) const
        {
        return MakeKey() < rhs.MakeKey();
        }
    };

//=======================================================================================
// Related Property Path Cache
// @bsistruct
//=======================================================================================
struct RelatedPropertyPathCache : RefCountedBase
    {
    struct RelatedPaths
        {
    private:
        //! Property path
        bvector<PathNames>  m_data;
        //! Target class Id
        BentleyApi::ECN::ECClassId m_targetClassId;
        //! Set of class Ids of elements that give properties to the base element class
        bset<BentleyApi::ECN::ECClassId> m_targetClasses;
    public:
        size_t  size() { return m_data.size(); }
        bool    IsValid() { return !m_data.empty(); }
        void    Add(Utf8StringCR src, Utf8StringCR rel, Utf8StringCR trg, BentleyApi::ECN::ECClassId const& relClassId, bool forward)
            {
            m_data.push_back(PathNames(src, rel, trg, relClassId, forward));
            }

        void    SetTargetClass(BentleyApi::Dgn::DgnDbR db, BentleyApi::ECN::ECClassCP targetClass);

        //! Check if the given class is a property provider class for bis.element of this related property path
        //! @param [in] db Db to check class id against target class id
        //! @param [in] classId ECClassId to check
        //! @return true if class results in properties being shown on an element for this given property path
        bool    IsPropertyProvider(BentleyApi::ECN::ECClassId const& classId) const;

        BentleyApi::ECN::ECClassId GetTargetClassId() { return m_targetClassId; }
        bset<BentleyApi::ECN::ECClassId> const& GetTargetClassIds() { return m_targetClasses; }

        //! Make identifier key for the related path
        Utf8String MakeKey();

        //! Paths of the related property path
        //! @return vector of paths for the related property
        bvector<PathNames> const&     Paths() const { return m_data; }
        }; // RelatedPaths

private:
    bvector<RelatedPaths> m_cache;

    Utf8String m_rulesetId;
    ECPresentationManagerR m_presentationManager;

    BentleyApi::BeSQLite::EC::ECSqlStatementCache m_statementCache;

    //! Gets the content classes from Presentation Rules to know the paths relevant for change inspection
    bvector<BentleyApi::ECPresentation::SelectClassInfo> GetContentClasses(BentleyApi::Dgn::DgnDbR db, Utf8StringCR schemaName, Utf8StringCR className);

    BentleyApi::BeSQLite::EC::CachedECSqlStatementPtr GetCachedStatement(BentleyApi::Dgn::DgnDbR db, Utf8StringCR sql);

public:
    //! Constructor
    //! @param[in] presentationManager Pointer to the EC Presentation manager
    //! @param[in] rulesetId Ruleset to use for finding property paths
    RelatedPropertyPathCache(ECPresentationManagerR presentationManager, Utf8StringCR rulesetId)
        : m_presentationManager(presentationManager), m_rulesetId(rulesetId), m_statementCache(STATEMENT_CACHE_SIZE) { }

    //! Finds and caches the related property paths for the given class
    //! @param[in] Db to use for finding the related property paths
    //! @param[in] schemaName name of the schema
    //! @param[in] className name of the class
    void CachePaths(BentleyApi::Dgn::DgnDbR db, Utf8StringCR schemaName, Utf8StringCR className);

    //! Get the related property paths for a given class name
    //! @return Related paths for the given class names
    bvector<RelatedPaths> const& Get() const;
    }; // RelatedPropertyPathCache

//=======================================================================================
// Cache entry for a relationship that got deleted
// Contains map from sources to targets and vice versa for faster lookups
// than iterating over temp tables on the Db each time we want to traverse property paths
// @bsistruct
//=======================================================================================
struct RelationshipCacheEntry
    {
    bmap<BentleyApi::BeSQLite::EC::ECInstanceKey, bset<BentleyApi::BeSQLite::EC::ECInstanceKey>> m_targetToSourcesMap;
    bmap<BentleyApi::BeSQLite::EC::ECInstanceKey, bset<BentleyApi::BeSQLite::EC::ECInstanceKey>> m_sourceToTargetsMap;
    }; // RelationshipCacheEntry


//=======================================================================================
// Options for caching relationships for faster performance
// @bsistruct
//=======================================================================================
struct RelationshipCachingOptions
    {
    bool m_wantRelationshipCaching;
    int m_cacheSize;

    RelationshipCachingOptions() : m_wantRelationshipCaching(true), m_cacheSize(VC_DEFAULT_RELATIONSHIP_CACHE_SIZE) { }
    RelationshipCachingOptions(bool wantCaching, int cacheSize = VC_DEFAULT_RELATIONSHIP_CACHE_SIZE)
        : m_wantRelationshipCaching(wantCaching), m_cacheSize(cacheSize) { }
    }; // RelationshipCachingOptions

//=======================================================================================
// Query helper for chunk traversal of property paths
// @bsistruct
//=======================================================================================
struct ChunkRelationshipQueryHelper
    {
private:
    BentleyApi::BeSQLite::EC::ECSqlStatementCache m_statementCache;
    BentleyApi::BeSQLite::Statement m_deletedRelationshipStmt;

    //! Get cached ECSql statement
    BentleyApi::BeSQLite::EC::CachedECSqlStatementPtr GetCachedStatement(BentleyApi::Dgn::DgnDbR db, Utf8StringCR sql);

public:
    //! Get relationships that exist for the given path in the db
    RelationshipCacheEntry GetCurrentRelatedInstances(BentleyApi::Dgn::DgnDbR db, PathNames const& path, bset<ECInstanceKey> const& instanceKeys);
    //! Get deleted relationship changes from summary or cached if already queried for
    RelationshipCacheEntry GetDeletedRelationshipChanges(BentleyApi::Dgn::DgnDbR db, BentleyApi::Dgn::DgnChangeSummary& changeSummary, BentleyApi::ECN::ECClassId const& relationshipClassId, bset<ECInstanceKey> const& instanceKeys);

    ChunkRelationshipQueryHelper(BentleyApi::Dgn::DgnDbR db, BentleyApi::Dgn::DgnChangeSummary& changeSummary);
    ~ChunkRelationshipQueryHelper();
    }; // RelationshipQueryHelper

//=======================================================================================
// Allows querying for existing and deleted relationships
// Caches results while being cogniscient of memory usage
// @bsistruct
//=======================================================================================
struct RelationshipQueryHelper
    {
private:
    RelationshipCachingOptions m_options;
    //! Cache for deleted relationships' affected instances
    bmap<ECClassId, RelationshipCacheEntry> m_deletedRelationshipInstancesCache;
    //! Cache for current relationship entries existing in the db
    bmap<ECClassId, RelationshipCacheEntry> m_currentRelationshipInstancesCache;
    //! Set of class Ids that we can support caching since they won't result in huge cache sizes and memory usage
    //! This is data dependent
    bset<BentleyApi::ECN::ECClassId> m_cacheableClassIds;

    BentleyApi::BeSQLite::EC::ECSqlStatementCache  m_statementCache;
    BentleyApi::BeSQLite::Statement m_deletedRelationshipStmt;

    //! Gets the current related instances of the given instance
    void GetCurrentRelatedInstances(bset<BentleyApi::BeSQLite::EC::ECInstanceKey>& outInstances, BentleyApi::Dgn::DgnDbR db, PathNames const& path, ECInstanceKey const& instanceKey);
    //! Get how many relationships of the given path are in the Db
    int GetCountOfCurrentRelationships(BentleyApi::Dgn::DgnDbR db, Utf8String const& relationshipClassName);
    //! Get deleted relationship changes from summary or cached if already queried for
    RelationshipCacheEntry& GetDeletedRelationshipChanges(BentleyApi::Dgn::DgnDbR db, BentleyApi::Dgn::DgnChangeSummary& changeSummary, BentleyApi::ECN::ECClassId const& relationshipClassId);
    //! Get current relationship entries that exist in the db
    RelationshipCacheEntry& GetCurrentRelationships(BentleyApi::Dgn::DgnDbR db, PathNames const& path);
    //! Get cached ECSql statement
    BentleyApi::BeSQLite::EC::CachedECSqlStatementPtr GetCachedStatement(BentleyApi::Dgn::DgnDbR db, Utf8StringCR sql);

public:
    //! Find the source instance nodes in the related property path
    void FindSourcesOfPathInDb(bset<BentleyApi::BeSQLite::EC::ECInstanceKey>& sources, BentleyApi::Dgn::DgnDbR db, PathNames const& path, BentleyApi::BeSQLite::EC::ECInstanceKey const& target);
    //! Find the source instance nodes in the related property path via the change summary's deleted instances
    void FindSourcesOfPathInSummary(bset<BentleyApi::BeSQLite::EC::ECInstanceKey>& sources, BentleyApi::Dgn::DgnDbR db, BentleyApi::Dgn::DgnChangeSummary& changeSummary, PathNames const& path, BentleyApi::BeSQLite::EC::ECInstanceKey const& target);
    //! Find which classes can be cached based on the amount of existing relationships
    void FindCacheableClasses(BentleyApi::Dgn::DgnDbR db, BentleyApi::Dgn::DgnChangeSummary& changeSummary, bmap<Utf8String, RelatedPropertyPathCache::RelatedPaths> const& m_relatedPropertyPaths);
    //! Set options for caching
    void SetCachingOptions(RelationshipCachingOptions const& options) { m_options = options; }

    RelationshipQueryHelper(BentleyApi::Dgn::DgnDbR db, BentleyApi::Dgn::DgnChangeSummary& changeSummary);
    ~RelationshipQueryHelper();
    }; // RelationshipQueryHelper

//=======================================================================================
// Related Property Path Explorer that traverses the paths to find related instances
// @bsistruct
//=======================================================================================
struct RelatedPropertyPathExplorer
    {
private:
    //! Helper for obtaining relationships and related instances
    RelationshipQueryHelper m_helper;
    //! Helper for obtaining relationships and related instances via chunk traversal of property paths
    ChunkRelationshipQueryHelper m_chunkHelper;

    //! Traverse a path in the Db
    BentleyApi::BeSQLite::EC::ECInstanceKey TraversePathInDb(BentleyApi::Dgn::DgnDbR db, PathNames const &path, BentleyApi::BeSQLite::EC::ECInstanceId const &instanceId);
public:
    //! Traverse path to find related affected instance
    void Traverse(bset<BentleyApi::BeSQLite::EC::ECInstanceKey>& output, BentleyApi::Dgn::DgnDbR db, BentleyApi::Dgn::DgnChangeSummary& changeSummary, bvector<PathNames> const &paths, BentleyApi::BeSQLite::EC::ECInstanceKey const &instance);

    //! Traverse path in chunks for a bunch of instance ids with the same class id
    void TraverseInChunks(bmap<ECInstanceKey,ECInstanceKeySet>& output, BentleyApi::Dgn::DgnDbR db, BentleyApi::Dgn::DgnChangeSummary& changeSummary, bvector<PathNames> const& paths, bset<ECInstanceKey> const& instanceKeys);

    //! Initialize
    void FindCacheableClasses(BentleyApi::Dgn::DgnDbR db, BentleyApi::Dgn::DgnChangeSummary& changeSummary, bmap<Utf8String, RelatedPropertyPathCache::RelatedPaths> const& m_relatedPropertyPaths);

    //! Set caching options
    void SetCachingOptions(RelationshipCachingOptions const& options)
        {
        m_helper.SetCachingOptions(options);
        }

    RelatedPropertyPathExplorer(BentleyApi::Dgn::DgnDbR db, BentleyApi::Dgn::DgnChangeSummary& changeSummary);
    }; // RelatedPropertyPathExplorer

//=======================================================================================
// Record for a changed element containing changes type, opcode, and more
// @bsistruct
//=======================================================================================
struct ChangedElementRecord
    {
    BentleyApi::BeSQLite::DbOpcode          m_opcode;
    BentleyApi::ECN::ECClassId              m_ecclassId;
    BentleyApi::Dgn::DgnModelId             m_modelId;
    BentleyApi::AxisAlignedBox3d            m_bbox;
    BentleyApi::BeSQLite::EC::ECInstanceKey m_parentKey;
    ElementChangesType                      m_changes;

    DGNPLATFORM_EXPORT void AccumulateChange(ChangedElementRecord info);
    DGNPLATFORM_EXPORT bool IsValid();
    DGNPLATFORM_EXPORT void Invalidate();

    ChangedElementRecord(BentleyApi::BeSQLite::DbOpcode opcode, BentleyApi::ECN::ECClassId classId, BentleyApi::Dgn::DgnModelId modelId, BentleyApi::AxisAlignedBox3d bbox) :
        m_opcode(opcode), m_ecclassId(classId), m_modelId(modelId), m_bbox(bbox) { }

    ChangedElementRecord(BentleyApi::BeSQLite::DbOpcode opcode, BentleyApi::ECN::ECClassId classId, BentleyApi::Dgn::DgnModelId modelId, BentleyApi::AxisAlignedBox3d bbox, int changes) :
        m_opcode(opcode), m_ecclassId(classId), m_modelId(modelId), m_bbox(bbox), m_changes(changes) { }

    ChangedElementRecord(BentleyApi::BeSQLite::DbOpcode opcode, BentleyApi::ECN::ECClassId classId, BentleyApi::Dgn::DgnModelId modelId, BentleyApi::AxisAlignedBox3d bbox, int changes, bvector<Utf8String> const& props) :
        m_opcode(opcode), m_ecclassId(classId), m_modelId(modelId), m_bbox(bbox), m_changes(changes, props) { }

    ChangedElementRecord(BentleyApi::BeSQLite::DbOpcode opcode, BentleyApi::ECN::ECClassId classId, BentleyApi::Dgn::DgnModelId modelId, BentleyApi::AxisAlignedBox3d bbox, int changes, bvector<Utf8String> const& props, BentleyApi::BeSQLite::EC::ECInstanceKey const& parentKey) :
        m_opcode(opcode), m_ecclassId(classId), m_modelId(modelId), m_bbox(bbox), m_changes(changes, props), m_parentKey(parentKey) { }

    ChangedElementRecord(BentleyApi::BeSQLite::DbOpcode opcode, BentleyApi::ECN::ECClassId classId, BentleyApi::Dgn::DgnModelId modelId, BentleyApi::AxisAlignedBox3d bbox, int changes, bvector<Utf8String> const& props, bvector<uint32_t> const& oldChecksums, bvector<uint32_t> const& newChecksums, BentleyApi::BeSQLite::EC::ECInstanceKey const& parentKey) :
        m_opcode(opcode), m_ecclassId(classId), m_modelId(modelId), m_bbox(bbox), m_changes(changes, props, oldChecksums, newChecksums), m_parentKey(parentKey) { }

    ChangedElementRecord() { }
    }; // ChangedElementRecord

//=======================================================================================
// ECInstanceKey of changed element and its record
// @bsistruct
//=======================================================================================
struct ChangedElementInfo
    {
    BentleyApi::BeSQLite::EC::ECInstanceKey m_instanceKey;
    ChangedElementRecord m_record;

    ChangedElementInfo(BentleyApi::BeSQLite::EC::ECInstanceKey const& key, ChangedElementRecord const& record): m_instanceKey(key), m_record(record) { }
    }; // ChangedElementInfo

//=======================================================================================
// Finds parents and top parents of an element by searching in the Db or change summary
// @bsistruct
//=======================================================================================
struct ParentFinder
    {
private:
    //! Map for child to parent caching of changed ElementOwnsChildElements relationships
    bmap<BentleyApi::BeSQLite::EC::ECInstanceKey, BentleyApi::BeSQLite::EC::ECInstanceKey> m_deletedChildToParentMap;
    bmap<BentleyApi::BeSQLite::EC::ECInstanceKey, BentleyApi::BeSQLite::EC::ECInstanceKey> m_currentChildToParentMap;

    void CacheDeletedChildToParentMap(BentleyApi::Dgn::DgnDbR db, BentleyApi::Dgn::DgnChangeSummary& changeSummary);
    void CacheCurrentChildToParentMap(BentleyApi::Dgn::DgnDbR db);

    BentleyApi::BeSQLite::EC::ECInstanceKey FindDeletedParentKey(BentleyApi::BeSQLite::EC::ECInstanceKey const& elementKey);
    BentleyApi::BeSQLite::EC::ECInstanceKey FindCurrentParentKey(BentleyApi::BeSQLite::EC::ECInstanceKey const& elementKey);

public:
    BentleyApi::BeSQLite::EC::ECInstanceKey FindParentKey(BentleyApi::Dgn::DgnDbR db, BentleyApi::Dgn::DgnChangeSummary& changeSummary, BentleyApi::BeSQLite::EC::ECInstanceKey const& key);
    BentleyApi::BeSQLite::EC::ECInstanceKey FindTopParentKey(BentleyApi::Dgn::DgnDbR db, BentleyApi::Dgn::DgnChangeSummary& changeSummary, BentleyApi::BeSQLite::EC::ECInstanceKey const& key);

    void CacheParents(BentleyApi::Dgn::DgnDbR db, BentleyApi::Dgn::DgnChangeSummary& changeSummary);
    }; // ParentFinder

//=======================================================================================
// Finds related instances based on a change summary and related property paths
// @bsistruct
//=======================================================================================
struct RelatedInstanceFinder
    {
private:
    BentleyApi::Dgn::DgnChangeSummary& m_changeSummary;
    BentleyApi::Dgn::DgnDbR     m_db;
    bmap<Utf8String, RelatedPropertyPathCache::RelatedPaths> m_relatedPropertyPaths;
    RelatedPropertyPathExplorer m_relatedPropertyExplorer;

public:
    //! Constructor
    //! @param[in] db in after state for comparison
    //! @param[in] changeSummary to query for changes
    //! @param[in] presentationManager
    //! @param[in] rulesetId for finding related instances
    RelatedInstanceFinder(BentleyApi::Dgn::DgnDbR db, BentleyApi::Dgn::DgnChangeSummary& changeSummary, ECPresentationManagerR presentationManager, Utf8StringCR rulesetId, RelationshipCachingOptions const& options);

    //! Copy constructor
    RelatedInstanceFinder(RelatedInstanceFinder& finder): m_changeSummary(finder.m_changeSummary), m_db(finder.m_db), m_relatedPropertyExplorer(finder.m_db, finder.m_changeSummary)
        {
        m_relatedPropertyPaths = finder.m_relatedPropertyPaths;
        }

    //! Add related property paths from the passed cache to be used when finding related instances
    //! @param[in] cache of related property paths
    void AddRelatedPropertyPaths(RelatedPropertyPathCache const& cache);

    //! Gets all related instance keys of an instance, taking into account that those instances may have other related property paths
    //! @param [out] output Set to add all related instances to
    //! @param [in] instance Instance key of element to get related instances for
    void GetAllRelatedInstances(bset<BentleyApi::BeSQLite::EC::ECInstanceKey>& output, BentleyApi::BeSQLite::EC::ECInstanceKey const& instance);

    //! Gets all related instances for a set of instances with the given class id
    //! @param [out] output map from target (leaf node with changes) to changed element
    void GetAllRelatedInstancesForClassIds(bmap<BentleyApi::BeSQLite::EC::ECInstanceKey, ECInstanceKeySet>& output, bmap<ECClassId,bset<ECInstanceKey>>& classToInstanceKeys);

    //! Gets the number of property paths that are cached
    size_t GetNumberOfPropertyPaths() { return m_relatedPropertyPaths.size(); }

    //! Set caching options
    void SetCachingOptions(RelationshipCachingOptions options)
        {
        m_relatedPropertyExplorer.SetCachingOptions(options);
        }
    }; // RelatedInstanceFinder

//=======================================================================================
// Finds changed elements from a DgnChangeSummary
// @bsistruct
//=======================================================================================
struct ChangedElementFinder
    {
private:
    bmap<BentleyApi::BeSQLite::EC::ECInstanceKey, ChangedElementRecord> m_changedInstances;
    bmap<BentleyApi::BeSQLite::EC::ECInstanceKey, ChangedElementRecord> m_relatedInstanceChanges;
    HiddenPropertyCache m_hiddenPropertyCache;
    ParentFinder m_parentFinder;
    ECPresentationManagerR m_presentationManager;
    Utf8String m_rulesetId;
    Utf8String m_elementClassFullName;
    bool m_wantParentKeys;
    bool m_wantChecksums;
    bool m_wantRelationshipCaching;
    bool m_wantChunkTraversal;
    int m_relationshipCacheSize;

    //! Query all model Ids of related instances that are not found in the change summary
    void QueryRelatedInstanceModelIds(BentleyApi::Dgn::DgnDbR db);
    //! Insert the changed instance as a ChangedElementRecord in the elements map
    void ProcessInstance(BentleyApi::Dgn::DgnDbR db, DgnChangeSummary& changeSummary, ChangeSummary::Instance const& instance, RelatedInstanceFinder& finder);
    void FindRelatedInstances(RelatedInstanceFinder& finder, BentleyApi::Dgn::DgnDbR db, DgnChangeSummary& changeSummary);
    void GetChangesType(ElementChangesType& changes, DgnModelId& instanceModelId, BentleyApi::Dgn::DgnDbR db, BentleyM0200::BeSQLite::EC::ChangeSummary::Instance const& instance);
    void ProcessRelatedInstances(BentleyApi::Dgn::DgnDbR db, BentleyApi::Dgn::DgnChangeSummary& changeSummary, ECInstanceKey const& instance, ChangedElementRecord const& record, RelatedInstanceFinder& finder);
    void FindElementClassIds(bset<BentleyApi::ECN::ECClassId>& classIds, BentleyApi::Dgn::DgnDbR db);

    RelatedInstanceFinder CreateRelatedInstanceFinder(BentleyApi::Dgn::DgnDbR db, DgnChangeSummary& changeSummary, RelatedPropertyPathCache& beforeStateCache);

public:
    //! Constructor
    ChangedElementFinder(ECPresentationManagerR presentationManager, Utf8StringCR rulesetId, Utf8StringCR elementClassFullName, bool wantParents = true, bool wantChecksums = true, bool wantRelationshipCaching = true, int relationshipCacheSize = VC_DEFAULT_RELATIONSHIP_CACHE_SIZE, bool wantChunkTraversal = false)
        : m_presentationManager(presentationManager), m_rulesetId(rulesetId), m_elementClassFullName(elementClassFullName), m_wantParentKeys(wantParents), m_wantChecksums(wantChecksums), m_wantRelationshipCaching(wantRelationshipCaching), m_relationshipCacheSize(relationshipCacheSize), m_wantChunkTraversal(wantChunkTraversal) { }

    //! Destructor
    ~ChangedElementFinder()
        {
        m_hiddenPropertyCache.CleanUpStatement();
        m_hiddenPropertyCache.Clear();
        }

    //! Goes through the change summary instances and finds all elements and related elements via related property paths
    //! @param[out] elements Output vector for elements
    //! @param[in] db DgnDb pointer to use
    //! @param[in] changeSummary DgnChangeSummary to use to find all changed instances
    //! @param[in] beforeStateCache cache of related property paths in the older Db state
    StatusInt GetChangedElementsFromSummary(bvector<ChangedElementInfo>& elements, BentleyApi::Dgn::DgnDbR db, DgnChangeSummary& changeSummary, RelatedPropertyPathCache& beforeStateCache);
    }; // ChangedElementFinder

//=======================================================================================
// Contains helper methods to query for changed elements
// This change summary class also handles schema changes and it works by creating
// multiple change summaries as needed by the given changesets
//
// @bsistruct
//=======================================================================================
struct VersionCompareChangeSummary : RefCountedBase
    {
private:

    ECPresentationManagerR m_presentationManager;
    BentleyApi::Dgn::DgnDbPtr                               m_targetDb;
    bvector<BentleyApi::Dgn::DgnRevisionPtr>                m_changesets;
    bmap<BentleyApi::BeSQLite::EC::ECInstanceKey, ChangedElementRecord> m_changedElements;
    Utf8String  m_rulesetId;
    BeFileName  m_tempLocation;
    BeFileName  m_dbFilename;

    bool    m_filterSpatial;
    bool    m_filterLastMod;
    bool    m_wantTargetState;
    bool    m_wantParentKeys;
    bool    m_wantPropertyChecksums;
    bool    m_wantBriefcaseRoll;
    bool    m_wantRelationshipCaching;
    bool    m_wantChunkTraversal;
    int     m_relationshipCacheSize;

    bmap<BentleyApi::BeSQLite::EC::ECInstanceId, BentleyApi::BeSQLite::EC::ChangeSummary::Instance> m_elementCache;

    //! Whether we should create a clone of the db and maintain the target state of comparison for querying properties
    //! Is more efficient to not clone the db or roll the briefcase in cases that we only want to process a single changeset
    //! This is a performance improvement for the agent case in which processing is done by single changesets
    bool WantTargetState() { return m_wantTargetState || (m_changesets.size() != 1); }
    //! Tells the change summary to avoid creating a cloned version of the database if possible
    //! Will only work when rolling a single changeset
    void SetWantTargetState(bool wantTargetState) { m_wantTargetState = wantTargetState; }

    //! Gets the changesets separated by the ones that contain schema changes and such
    StatusInt   GetAppliableChangesets(bvector<bvector<BentleyApi::Dgn::DgnRevisionPtr>>& appliableChangesets);
    //! Processes the given changesets to generate change summaries
    StatusInt   ProcessChangesets();
    //! Rolls a db, closes it and re-opens it to avoid conflicts with schema changes
    StatusInt   RollTargetDb(bvector<BentleyApi::Dgn::DgnRevisionPtr> const& changesets);
    //! Clones the current db to obtain a target one
    static BentleyApi::Dgn::DgnDbPtr    CloneDb(BeFileNameCR dbFilename, BeFileNameCR location);

    //! Get an element pointer by searching the correct Db based on the opcode
    DgnElementCPtr  GetElement(BentleyApi::Dgn::DgnElementId elementId, BentleyApi::BeSQLite::DbOpcode opcode);

    //! Constructor
    VersionCompareChangeSummary(BeFileName dbFilename, ECPresentationManagerR presentationManager) : m_dbFilename(dbFilename), m_targetDb(nullptr), m_presentationManager(presentationManager), m_filterSpatial(false), m_filterLastMod(false), m_wantTargetState(false) { }

    //! This method will process the changesets and obtain all the changed instances
    //! @param[in] changesets Vector of DgnRevisionPtr containing the changesets to compile together
    StatusInt   SetChangesets(bvector<BentleyApi::Dgn::DgnRevisionPtr>& changesets);

public:
    DGNPLATFORM_EXPORT ~VersionCompareChangeSummary();

    //! Get the target DgnDb that is computed throughout generation of the summary
    //! @return target db, may be invalid if no comparison has been processed
    DGNPLATFORM_EXPORT BentleyApi::Dgn::DgnDbPtr GetTargetDb();

    //! Returns vectors for element ids, ecclass ids and opcodes of the elements that have changed
    //! @param[out] elements Changed Elements struct containing element id, class id, opcode, model id and type of change
    DGNPLATFORM_EXPORT StatusInt   GetChangedElements(bvector<ChangedElement>& elements);

    //! Returns changed elements that are of a particular class
    //! @param[out] elements Changed Elements struct containing element id, class id, opcode, model id and type of change
    //! @param[in] classp ECClassCP being looked for
    DGNPLATFORM_EXPORT StatusInt     GetChangedElementsOfClass(bvector<ChangedElement>& elements, BentleyApi::ECN::ECClassCP classp);

    //! Get the models that changed
    //! @param[out] modelIds all DgnModelId that changed
    //! @param[out] opcodes types of changes
    DGNPLATFORM_EXPORT StatusInt     GetChangedModels(bset<BentleyApi::Dgn::DgnModelId>& modelIds, bvector<BentleyApi::BeSQLite::DbOpcode>& opcodes);


    //! Creates a change summary that compares the db after applying the given changesets
    //! This will generate a temporary Db that is rolled to the target state to be able to obtain elements from it
    //! This db will be destroyed once the version compare change summary is destroyed
    //! Note: The Generate function above should be preferred over this function, but if needed, use VersionSelector::GetChangeSetsToApply to
    //! obtain the changesets and the roll direction
    //! @param[in] dbFilename Filename of Db to process
    //! @param[in] changesets Vector of DgnRevisionPtr containing all changesets to be applied
    //! @param[in] rulesetId Name of the presentation rules to use
    //! @return VersionCompareChangeSummaryPtr with the results
    DGNPLATFORM_EXPORT static VersionCompareChangeSummaryPtr Generate(BeFileName dbFilename, bvector<BentleyApi::Dgn::DgnRevisionPtr> &changesets, ECPresentationManagerR presentationManager, Utf8StringCR rulesetId);

    //! Creates a change summary that compares the db after applying the given changesets
    //! This will generate a temporary Db that is rolled to the target state to be able to obtain elements from it
    //! This db will be destroyed once the version compare change summary is destroyed
    //! Note: The Generate function above should be preferred over this function, but if needed, use VersionSelector::GetChangeSetsToApply to
    //! obtain the changesets and the roll direction
    //! @param[in] dbFilename Filename of Db to process
    //! @param[in] changesets Vector of DgnRevisionPtr containing all changesets to be applied
    //! @param[in] options SummaryOptions class with settings for summary generation
    //! @return VersionCompareChangeSummaryPtr with the results
    DGNPLATFORM_EXPORT static VersionCompareChangeSummaryPtr     Generate(BeFileName dbFilename, bvector<BentleyApi::Dgn::DgnRevisionPtr>& changesets, SummaryOptions const& options);
    }; // VersionCompareChangeSummary

typedef bmap<BentleyApi::BeSQLite::EC::ECInstanceKey, ChangedElementRecord> ChangedElementsMap;

END_BENTLEY_DGNPLATFORM_NAMESPACE
