/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <ECDb/ECDb.h>
#include <Bentley/BeEvent.h>
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! Schema change event type
//! @ingroup ECDbGroup
//+===============+===============+===============+===============+===============+======
enum class SchemaChangeType {
    SchemaImport,
    SchemaChangesetApply
};


using SchemaChangeEvent = BeEvent<ECDbCR,SchemaChangeType> ;

//=======================================================================================
//! Options for how to refer to an ECSchema when looking it up using the SchemaManager
//! @ingroup ECDbGroup
// @bsiclass
//+===============+===============+===============+===============+===============+======
enum class SchemaLookupMode
    {
    ByName, //!< Schema is referred to by its name
    ByAlias, //!< Schema is referred to by its alias
    AutoDetect //!< Detect automatically whether schema is referred to by name or alias
    };

//=======================================================================================
//! Class map info returned for a given class.
//! @ingroup ECDbGroup
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct ClassMapStrategy {
    enum class MapStrategy {
        NotMapped = 0,
        OwnTable = 1,
        TablePerHierarchy = 2,
        ExistingTable = 3,
        ForeignKeyRelationshipInTargetTable = 10,
        ForeignKeyRelationshipInSourceTable = 11
    };

private:
    MapStrategy m_strategy;
    ECN::ECClassCP m_class;

public:
    ClassMapStrategy(MapStrategy strategy, ECN::ECClassCR ecClass): m_strategy(strategy),m_class(&ecClass){}
    ClassMapStrategy(): m_strategy(MapStrategy::NotMapped),m_class(nullptr){}
    bool IsLinkTableRelationship() const { BeAssert(!IsEmpty()); return IsRelationshipClass() && (IsTablePerHierarchy() || IsOwnTable()); }
    bool IsTablePerHierarchy() const { BeAssert(!IsEmpty()); return m_strategy == MapStrategy::TablePerHierarchy; }
    bool IsOwnTable() const { BeAssert(!IsEmpty()); return m_strategy == MapStrategy::OwnTable; }
    bool IsExistingTable() const { BeAssert(!IsEmpty()); return m_strategy == MapStrategy::ExistingTable; }
    bool IsForeignKeyRelationship() const { BeAssert(!IsEmpty()); return m_strategy == MapStrategy::ForeignKeyRelationshipInSourceTable || m_strategy == MapStrategy::ForeignKeyRelationshipInTargetTable; }
    bool IsRelationshipClass() const { BeAssert(!IsEmpty()); return m_class->IsRelationshipClass(); }
    bool IsEntityClass() const { BeAssert(!IsEmpty()); return m_class->IsEntityClass(); }
    bool IsNotMapped() const { BeAssert(!IsEmpty()); return m_strategy == MapStrategy::NotMapped; }
    bool IsEmpty() const { return m_class == nullptr; }
    ECN::ECClassCP GetClass() const { BeAssert(!IsEmpty()); return m_class; }
    MapStrategy GetStrategy() const { BeAssert(!IsEmpty()); return m_strategy; }
};

//=======================================================================================
//! Instance finder used by drop schema
//! @ingroup ECDbGroup
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct InstanceFinder {

    struct ForignKeyRelation final {
        private:
            ECInstanceKey m_thisEnd;
            ECInstanceKey m_otherEnd;
        public:
            ForignKeyRelation(ECInstanceKey&& thisEnd, ECInstanceKey&& otherEnd) :m_thisEnd(thisEnd), m_otherEnd(otherEnd) {}
            ECInstanceKey const& GetThisEndKey() const { return m_thisEnd; }
            ECInstanceKey const& GetOtherEndKey() const { return m_otherEnd; }
    };
    struct LinkTableRelation final {
        private:
            ECInstanceKey m_relationKey;
            ECInstanceKey m_sourceKey;
            ECInstanceKey m_targetKey;
        public:
            LinkTableRelation(ECInstanceKey&& relationKey, ECInstanceKey&& sourceKey, ECInstanceKey&& targetKey)
                :m_relationKey(relationKey), m_sourceKey(sourceKey), m_targetKey(targetKey) {}
            ECInstanceKey const& GetRelationKey() const { return m_relationKey; }
            ECInstanceKey const& GetSourceKey() const { return m_sourceKey; }
            ECInstanceKey const& GetTargetKey() const { return m_targetKey; }
    };
//=======================================================================================
//! Search resutl return by drop schema
//! @ingroup ECDbGroup
// @bsiclass
//+===============+===============+===============+===============+===============+======
    struct SearchResults {
        struct JsonFormatOptions {
            private:
                ECDbCR m_ecdb;
                bool m_useClassNamesForInstanceKey;
                bool m_useClassNamesForBaseClassNames;
            public:
                JsonFormatOptions(ECDbCR ecdb): m_ecdb(ecdb),m_useClassNamesForInstanceKey(false),m_useClassNamesForBaseClassNames(false){}
                ECDbCR GetECDb() const { return m_ecdb;}
                bool GetUseClassNameForInstanceKey() const { return m_useClassNamesForInstanceKey;}
                void SetUseClassNameForInstanceKey(bool v) { m_useClassNamesForInstanceKey = v;}
                bool GetUseClassNameForBaseClass() const { return m_useClassNamesForBaseClassNames;}
                void SetUseClassNameForBaseClass(bool v) { m_useClassNamesForBaseClassNames = v;}
        };
        private:
            std::map<ECN::ECClassId, std::vector<ECInstanceKey>> m_entityKeyMap;
            std::map<ECN::ECClassId, std::vector<LinkTableRelation>> m_linktableRelMap;
            std::map<ECN::ECClassId, std::vector<ForignKeyRelation>> m_foreignKeyMap;
            ECDB_EXPORT static void ToJson(BeJsValue v, ECInstanceKey const& key, JsonFormatOptions const* options);
            ECDB_EXPORT static void ToJson(BeJsValue v, ForignKeyRelation const& key, JsonFormatOptions const* options);
            ECDB_EXPORT static void ToJson(BeJsValue v, LinkTableRelation const& key, JsonFormatOptions const* options);
        public:
            SearchResults() {}
            SearchResults(
                std::map<ECN::ECClassId, std::vector<ECInstanceKey>>&& entities,
                std::map<ECN::ECClassId, std::vector<LinkTableRelation>>&& linkTableRels,
                std::map<ECN::ECClassId, std::vector<ForignKeyRelation>>&& fkRels
                ) : m_entityKeyMap(entities), m_linktableRelMap(linkTableRels), m_foreignKeyMap(fkRels){}
            SearchResults(SearchResults&& rhs)
                :m_linktableRelMap(std::move(rhs.m_linktableRelMap)),
                    m_entityKeyMap(std::move(rhs.m_entityKeyMap)),
                    m_foreignKeyMap(std::move(rhs.m_foreignKeyMap)) {}
            SearchResults& operator =(SearchResults&& rhs) {
                if (this != &rhs) {
                    m_entityKeyMap = std::move(rhs.m_entityKeyMap);
                    m_linktableRelMap = std::move(rhs.m_linktableRelMap);
                    m_foreignKeyMap = std::move(rhs.m_foreignKeyMap);
                }
                return *this;
            }
            auto& GetEntityKeyMap() const { return m_entityKeyMap; }
            auto& GetLinkTableRelMap() const { return m_linktableRelMap; }
            auto& GetForeignKeyRelMap() const { return m_foreignKeyMap; }
            bool IsEmpty() const { return m_entityKeyMap.empty() && m_linktableRelMap.empty() && m_foreignKeyMap.empty(); }
            ECDB_EXPORT void ToJson(BeJsValue& v, JsonFormatOptions const* options = nullptr) const;
            ECDB_EXPORT void Debug(ECDbCR ecdb);
    };
private:
    ECDB_EXPORT static std::vector<ECN::ECClassCP> GetRootEntityAndRelationshipClasses(ECDbCR ecdb);
    ECDB_EXPORT static std::vector<ECN::NavigationECPropertyCP> GetNavigationProps(ECDbCR ecdb);
public:
    ECDB_EXPORT static SearchResults FindInstances(ECDbCR ecdb, ECN::ECSchemaId schemaId, bool polymorphic);
    ECDB_EXPORT static SearchResults FindInstances(ECDbCR ecdb, ECN::ECClassId classId, bool polymorphic);
    ECDB_EXPORT static SearchResults FindInstances(ECDbCR ecdb, BeIdSet&& classIds);
};
//=======================================================================================
//! Drop schema result
//! @ingroup ECDbGroup
// @bsiclass
//+===============+===============+===============+===============+===============+======
struct DropSchemaResult {
    enum Status {
        Success = SUCCESS,
        Error = ERROR,
        ErrorSchemaNotFound,
        ErrorDeletedSchemaIsReferencedByAnotherSchema,
        ErrorDeleteSchemaHasClassesWithInstances,
        ErrorDbIsReadonly,
        ErrorDbHasLocalChanges,
        ErrorSchemaLockFailed,
        ErrorCouldNotAcquireLocksOrCodes
    };
    private:
        InstanceFinder::SearchResults m_searchResults;
        bvector<Utf8String> m_referencedBy;
        Status m_status;
    public:
        DropSchemaResult(Status status):m_status(status){}
        DropSchemaResult(Status status, InstanceFinder::SearchResults && results):m_status(status), m_searchResults(std::move(results)){}
        DropSchemaResult(Status status, bvector<Utf8String> && referencedBy):m_status(status),m_referencedBy(std::move(referencedBy)){}
        DropSchemaResult(DropSchemaResult && rhs): 
            m_searchResults(std::move(rhs.m_searchResults)), 
            m_referencedBy(std::move(rhs.m_referencedBy)),
            m_status(rhs.m_status){}
        DropSchemaResult& operator =(DropSchemaResult&& rhs) {
            if (this != &rhs) {
                m_searchResults = std::move(rhs.m_searchResults);
                m_referencedBy = std::move(rhs.m_referencedBy);
                m_status = std::move(rhs.m_status);
            }
            return *this;
        }            
        bvector<Utf8String> const& GetReferencedBySchemas() const {return m_referencedBy;}
        InstanceFinder::SearchResults const& GetInstances() const {return m_searchResults;}
        Status GetStatus() const { return m_status;}
        bool IsSuccess() const {return m_status == Status::Success; }
        bool IsError() const {return m_status != Status::Success; }
        bool HasInstances() const {return m_status == Status::ErrorDeleteSchemaHasClassesWithInstances; }
        bool HasDependendSchemas() const {return m_status == Status::ErrorDeletedSchemaIsReferencedByAnotherSchema; }
        ECDB_EXPORT Utf8CP GetStatusAsString() const;
};
//=======================================================================================
//! The SchemaManager manages @ref ECN::ECSchema "ECSchemas" in the @ref ECDbFile "ECDb file". 
//! Clients can import @ref ECN::ECSchema "ECSchemas" into or retrieve @ref ECN::ECSchema "ECSchemas" or 
//! individual @ref ECN::ECClass "ECClasses" from the %ECDb file using the %SchemaManager.
//!
//! ###Incremental Loading of ECClasses
//! SchemaManager supports incremental loading of ECClasses. So unlike with ECObjects calling
//! SchemaManager::GetClass doesn't load the rest of the ECN::ECSchema. 
//! When incrementally loading ECClasses you shouldn't use ECN::ECSchema::GetClass because 
//! the ECN::ECSchema might not be fully populated yet. Always use the respective SchemaManager
//! methods instead.
//!
//! ####Details
//! 
//! (which you don't need to be aware of if you consistently use the SchemaManager API to get ECClasses)
//!
//! * SchemaManager::GetSchema with <c>ensureAllClassesLoaded=false</c> 
//!     * Returns the ECSchema with only those ECClasses that have been loaded previously already. If
//!       no ECClasses have been loaded previously, the returned ECSchema is empty.
//! * SchemaManager::GetClass
//!     * loads the specified ECClass
//!     * loads all base classes of the specified ECClass
//!     * loads all struct classes used by the specified ECClass
//!     * loads all CustomAttribute classes used by the specified ECClass
//!     * does @b not load derived classes of the specified ECClass
//! * SchemaManager::GetClass for a relationship class
//!     * same as for regular classes
//!     * loads the ECClasses specified in the constraints of the relationship class
//!       (but @b not their derived ECClasses)
//!        
//! ### %SchemaManager is an %IECSchemaLocater
//! SchemaManager also implements ECN::IECSchemaLocater so it can be used to locate ECSchemas
//! already stored in the %ECDb file when reading an ECSchema from disk, for example:
//! 
//!     ECN::ECSchemaReadContextPtr ecSchemaContext = ECN::ECSchemaReadContext::CreateContext();
//!     ecSchemaContext->AddSchemaLocater(ecdb.GetSchemaLocater());
//!     ECN::SchemaKey schemaKey("foo", 1, 0);
//!     ECN::ECSchemaPtr fooSchema = ECN::ECSchema::LocateSchema(schemaKey, *ecSchemaContext);
//!
//! @see @ref ECDbOverview, @ref ECDbTransactions, @ref ECDbCodeSamples
//! @ingroup ECDbGroup
// @bsimethod
//+===============+===============+===============+===============+===============+======
struct SchemaManager final : ECN::IECSchemaLocater, ECN::IECClassLocater
    {
    public:

        //! Schema import options. Not needed by regular callers. They are specific to certain
        //! exceptional workflows and therefore only used by them.
        enum class SchemaImportOptions
            {
            None                                        = 0,        //! binary 0000
            DoNotFailSchemaValidationForLegacyIssues    = 1 << 0,   //! binary 0001. Not needed by regular caller
            DisallowMajorSchemaUpgrade                  = 1 << 1,   //! binary 0010. If specified, schema upgrades where the major version has changed, are not supported.
            DoNotFailForDeletionsOrModifications        = 1 << 2    //! binary 0100. This is for the case of domain schemas that differ between files even though the schema name and versions are unchanged.  In such a case, we only want to merge in acceptable changes, not delete anything
            };

#if !defined (DOCUMENTATION_GENERATOR)
        struct Dispatcher;
#endif
    private:
        Dispatcher* m_dispatcher = nullptr;

        //not copyable
        SchemaManager(SchemaManager const&) = delete;
        SchemaManager& operator=(SchemaManager const&) = delete;


        //! Implementation of IECSchemaLocater
        ECN::ECSchemaPtr _LocateSchema(ECN::SchemaKeyR, ECN::SchemaMatchType, ECN::ECSchemaReadContextR) override;

        //! Implementation of IECClassLocater
        //! @note You can either pass a schema name or a schema alias to the method.
        ECN::ECClassCP _LocateClass(Utf8CP schemaNameOrAlias, Utf8CP className) override { return GetClass(Utf8String(schemaNameOrAlias), Utf8String(className), SchemaLookupMode::AutoDetect); }
        ECN::ECClassCP _LocateClass(ECN::ECClassId const &classId) override { return GetClass(classId); }
        //! @note You can either pass a schema name or a schema alias to the method.
        ECN::ECClassId _LocateClassId(Utf8CP schemaNameOrAlias, Utf8CP className) override { return GetClassId(Utf8String(schemaNameOrAlias), Utf8String(className), SchemaLookupMode::AutoDetect); }
        void ClearIdsForSchemaNotOwnedByThisECDb(bvector<ECN::ECSchemaCP>& schemas);
    public:
#if !defined (DOCUMENTATION_GENERATOR)
        SchemaManager(ECDb const&, BeMutex&);
        ~SchemaManager();
#endif
        //! Drop a leaf schema from ecdb as long as it has no instances
        //! @param[in] name  name of schema to be dropped.
        //! @param [in] token Token required to perform ECSchema imports if the
        //! the ECDb file was set-up with the option "ECSchema import token validation".
        //! If the option is set, the schema import will fail without a valid token.
        //! If the option is not set, nullptr can be passed for @p token.
        //! See documentation of the respective ECDb subclass to find out whether the option is enabled or not.
        //! @return BentleyStatus::SUCCESS or BentleyStatus::ERROR (error details are being logged)
        //! @see @ref ECDbECSchemaImportAndUpgrade
        ECDB_EXPORT DropSchemaResult DropSchema(Utf8StringCR name, SchemaImportToken const* token = nullptr, bool logIssue = true) const;

        //! Imports the list of @ref ECN::ECSchema "ECSchemas" (which must include all its references)
        //! into the @ref ECDbFile "ECDb file".
        //! ECSchemas that already exist in the file are updated (see @ref ECDbECSchemaUpgradeSupportedFeatures).
        //! @note After importing the schemas, any pointers to the existing schemas should be discarded and
        //! they should be obtained as needed through the SchemaManager API.
        //! @remarks ECDb always persists ECSchemas in their invariant culture. That means localization ECSchemas are ignored
        //! during the import.
        //! @param[in] schemas  List of ECSchemas to import, including all referenced ECSchemas.
        //!                     If the referenced ECSchemas are known to have already been imported, they are not required, but it does no harm to include them again
        //!                     (the method detects that they are already imported, and simply skips them)
        //!                     All schemas should have been deserialized from a single ECN::ECSchemaReadContext. 
        //!                     If any duplicates are found in @p schemas an error will returned.
        //! @param [in] token Token required to perform ECSchema imports if the
        //! the ECDb file was set-up with the option "ECSchema import token validation".
        //! If the option is set, the schema import will fail without a valid token.
        //! If the option is not set, nullptr can be passed for @p token.
        //! See documentation of the respective ECDb subclass to find out whether the option is enabled or not.
        //! @return BentleyStatus::SUCCESS or BentleyStatus::ERROR (error details are being logged)
        //! @see @ref ECDbECSchemaImportAndUpgrade
        BentleyStatus ImportSchemas(bvector<ECN::ECSchemaCP> const& schemas, SchemaImportToken const* token = nullptr) const { return ImportSchemas(schemas, SchemaImportOptions::None, token); }

        //! Imports the list of @ref ECN::ECSchema "ECSchemas" (which must include all its references)
        //! into the @ref ECDbFile "ECDb file".
        //! ECSchemas that already exist in the file are updated (see @ref ECDbECSchemaUpgradeSupportedFeatures).
        //! @note After importing the schemas, any pointers to the existing schemas should be discarded and
        //! they should be obtained as needed through the SchemaManager API.
        //! @remarks ECDb always persists ECSchemas in their invariant culture. That means localization ECSchemas are ignored
        //! during the import.
        //! @param[in] schemas  List of ECSchemas to import, including all referenced ECSchemas.
        //!                     If the referenced ECSchemas are known to have already been imported, they are not required, but it does no harm to include them again
        //!                     (the method detects that they are already imported, and simply skips them)
        //!                     All schemas should have been deserialized from a single ECN::ECSchemaReadContext. 
        //!                     If any duplicates are found in @p schemas an error will returned.
        //! @param [in] options Schema import options
        //! @param [in] token Token required to perform ECSchema imports if the
        //! the ECDb file was set-up with the option "ECSchema import token validation".
        //! If the option is set, the schema import will fail without a valid token.
        //! If the option is not set, nullptr can be passed for @p token.
        //! See documentation of the respective ECDb subclass to find out whether the option is enabled or not.
        //! @return BentleyStatus::SUCCESS or BentleyStatus::ERROR (error details are being logged)
        //! @see @ref ECDbECSchemaImportAndUpgrade
        ECDB_EXPORT BentleyStatus ImportSchemas(bvector<ECN::ECSchemaCP> const& schemas, SchemaImportOptions options, SchemaImportToken const* token = nullptr) const;

        //! Gets all @ref ECN::ECSchema "ECSchemas" stored in the @ref ECDbFile "ECDb file"
        //! @remarks If called with @p loadSchemaEntities = true this can be a costly call as all schemas and their content would be loaded into memory.
        //! Consider retrieving single classes instead.
        //! @param[in] loadSchemaEntities true, if all ECClasses, ECEnumerations, KindOfQuantities in the ECSchema should be pro-actively loaded into memory. false,
        //!                                   if they are loaded on-demand.
        //! @param[in] tableSpace Table space containing the schemas - in case other ECDb files are attached to this. Passing nullptr means to search all table spaces (starting with the primary one, aka main).
        //! @return Vector of all ECSchemas stored in the file
        ECDB_EXPORT bvector<ECN::ECSchemaCP> GetSchemas(bool loadSchemaEntities = true, Utf8CP tableSpace = nullptr) const;

        //! Checks whether the ECDb file contains the ECSchema with the specified name or not.
        //! @param[in] schemaNameOrAlias Name (not full name) or alias of the schema
        //! @param[in] mode indicates whether @p schemaNameOrAlias is a schema name or a schema alias
        //! @param[in] tableSpace Table space containing the schema - in case other ECDb files are attached to this. Passing nullptr means to search all table spaces (starting with the primary one, aka main).
        //! @return true if the ECDb file contains the ECSchema. false otherwise.
        ECDB_EXPORT bool ContainsSchema(Utf8StringCR schemaNameOrAlias, SchemaLookupMode mode = SchemaLookupMode::ByName, Utf8CP tableSpace = nullptr) const;

        //! Get an ECSchema by name
        //! @param[in] schemaNameOrAlias Name (not full name) or alias of the schema
        //! @param[in] loadSchemaEntities true, if all ECClasses, ECEnumerations, KindOfQuantities in the ECSchema should be pro-actively loaded into memory. false,
        //!                                   if they are loaded on-demand.
        //! @param[in] mode indicates whether @p schemaNameOrAlias is a schema name or a schema alias
        //! @param[in] tableSpace Table space containing the schema - in case other ECDb files are attached to this. Passing nullptr means to search all table spaces (starting with the primary one, aka main).
        //! @return The retrieved ECSchema or nullptr if not found
        ECDB_EXPORT ECN::ECSchemaCP GetSchema(Utf8StringCR schemaNameOrAlias, bool loadSchemaEntities = true, SchemaLookupMode mode = SchemaLookupMode::ByName, Utf8CP tableSpace = nullptr) const;

        //! Gets the ECClass for the specified name.
        //! @param[in] schemaNameOrAlias Name (not full name) or alias of the schema containing the class (@see @p mode)
        //! @param[in] className Name of the class to be retrieved
        //! @param[in] mode indicates whether @p schemaNameOrAlias is a schema name or a schema alias
        //! @param[in] tableSpace Table space containing the class - in case other ECDb files are attached to this. Passing nullptr means to search all table spaces (starting with the primary one, aka main).
        //! @return The retrieved ECClass or nullptr if not found
        ECDB_EXPORT ECN::ECClassCP GetClass(Utf8StringCR schemaNameOrAlias, Utf8StringCR className, SchemaLookupMode mode = SchemaLookupMode::ByName, Utf8CP tableSpace = nullptr) const;
        
        //! Gets the ECClass for the specified name.
        //! @param[in] qualifiedClassName Name of the class in qualified form which is schema-name-or-alias[.|:]class-name
        //! @param[in] tableSpace Table space containing the class - in case other ECDb files are attached to this. Passing nullptr means to search all table spaces (starting with the primary one, aka main).
        //! @return The retrieved ECClass or nullptr if not found
        ECDB_EXPORT ECN::ECClassCP FindClass(Utf8StringCR qualifiedClassName, Utf8CP tableSpace = nullptr) const;

        //! Gets the ECClass map info in addition to class
        //! @param[in] schemaNameOrAlias Name (not full name) or alias of the schema containing the class (@see @p mode)
        //! @param[in] className Name of the class to be retrieved
        //! @param[in] mode indicates whether @p schemaNameOrAlias is a schema name or a schema alias
        //! @param[in] tableSpace Table space containing the class - in case other ECDb files are attached to this. Passing nullptr means to search all table spaces (starting with the primary one, aka main).
        //! @return map info including ecclass. IsEmpty() should be called to see if result is valid.
        ECDB_EXPORT ClassMapStrategy GetClassMapStrategy(Utf8StringCR schemaNameOrAlias, Utf8StringCR className, SchemaLookupMode mode = SchemaLookupMode::ByName, Utf8CP tableSpace = nullptr) const;


        //! Gets the ECClass for the specified ECClassId.
        //! @param[in] classId Id of the ECClass to retrieve
        //! @param[in] tableSpace Table space containing the class - in case other ECDb files are attached to this. Passing nullptr means to search all table spaces (starting with the primary one, aka main).
        //! @return The retrieved ECClass or nullptr if not found
        ECDB_EXPORT ECN::ECClassCP GetClass(ECN::ECClassId classId, Utf8CP tableSpace = nullptr) const;

        //! Gets the ECClassId for the ECClass with the specified name.
        //! @param[in] schemaNameOrAlias Name (not full name) or alias of the schema containing the class (@see @p mode)
        //! @param[in] className Name of the class to be retrieved
        //! @param[in] mode indicates whether @p schemaNameOrAlias is a schema name or a schema alias
        //! @param[in] tableSpace Table space containing the class - in case other ECDb files are attached to this. Passing nullptr means to search all table spaces (starting with the primary one, aka main).
        //! @return ECClassId of the requested ECClass. If the ECClass does not exist in the %ECDb file, an invalid class id is returned
        ECDB_EXPORT ECN::ECClassId GetClassId(Utf8StringCR schemaNameOrAlias, Utf8StringCR className, SchemaLookupMode mode = SchemaLookupMode::ByName, Utf8CP tableSpace = nullptr) const;

        //! Gets the derived classes of @p baseClass. The derived classes are loaded, if they are not yet.
        //! Callers should use this method in favor of ECN::ECClass::GetDerivedClasses to ensure
        //! that derived classes are actually loaded from the ECDb file.
        //! This method allows to just load the inheritance hierarchy of a given ECClass without having
        //! to load entire ECSchemas.
        //! @note This does not recurse into derived classes of derived classes. It just returns the first level
        //! of inheriting ECClasses.
        //! @param[in] baseClass ECClass to return derived classes for.
        //! @param[in] tableSpace Table space containing the class hierarchy - in case other ECDb files are attached to this. Passing nullptr means to search all table spaces (starting with the primary one, aka main).
        //! @return Derived classes list
        //! @see ECN::ECClass::GetDerivedClasses
        ECDB_EXPORT ECN::ECDerivedClassesList const& GetDerivedClasses(ECN::ECClassCR baseClass, Utf8CP tableSpace = nullptr) const;

        //! Gets the ECEnumeration for the specified name.
        //! @param[in] schemaNameOrAlias Name (not full name) or alias of the schema containing the enumeration (@see @p mode)
        //! @param[in] enumName Name of the ECEnumeration to be retrieved
        //! @param[in] mode indicates whether @p schemaNameOrAlias is a schema name or a schema alias
        //! @param[in] tableSpace Table space containing the enumeration - in case other ECDb files are attached to this. Passing nullptr means to search all table spaces (starting with the primary one, aka main).
        //! @return The retrieved ECEnumeration or nullptr if not found
        ECDB_EXPORT ECN::ECEnumerationCP GetEnumeration(Utf8StringCR schemaNameOrAlias, Utf8StringCR enumName, SchemaLookupMode mode = SchemaLookupMode::ByName, Utf8CP tableSpace = nullptr) const;

        //! Gets the KindOfQuantity for the specified name.
        //! @param[in] schemaNameOrAlias Name (not full name) or alias of the schema containing the KindOfQuantity (@see @p mode)
        //! @param[in] koqName Name of the KindOfQuantity to be retrieved
        //! @param[in] mode indicates whether @p schemaNameOrAlias is a schema name or a schema alias
        //! @param[in] tableSpace Table space containing the KindOfQuantity - in case other ECDb files are attached to this. Passing nullptr means to search all table spaces (starting with the primary one, aka main).
        //! @return The retrieved KindOfQuantity or nullptr if not found
        ECDB_EXPORT ECN::KindOfQuantityCP GetKindOfQuantity(Utf8StringCR schemaNameOrAlias, Utf8StringCR koqName, SchemaLookupMode mode = SchemaLookupMode::ByName, Utf8CP tableSpace = nullptr) const;

        //! Gets the ECUnit for the specified name.
        //! @param[in] schemaNameOrAlias Name (not full name) or alias of the schema containing the ECUnit (@see @p mode)
        //! @param[in] unitName Name of the ECUnit to be retrieved
        //! @param[in] mode indicates whether @p schemaNameOrAlias is a schema name or a schema alias
        //! @param[in] tableSpace Table space containing the ECUnit - in case other ECDb files are attached to this. Passing nullptr means to search all table spaces (starting with the primary one, aka main).
        //! @return The retrieved ECUnit or nullptr if not found
        ECDB_EXPORT ECN::ECUnitCP GetUnit(Utf8StringCR schemaNameOrAlias, Utf8StringCR unitName, SchemaLookupMode mode = SchemaLookupMode::ByName, Utf8CP tableSpace = nullptr) const;

        //! Gets the UnitSystem for the specified name.
        //! @param[in] schemaNameOrAlias Name (not full name) or alias of the schema containing the UnitSystem (@see @p mode)
        //! @param[in] unitSystemName Name of the UnitSystem to be retrieved
        //! @param[in] mode indicates whether @p schemaNameOrAlias is a schema name or a schema alias
        //! @param[in] tableSpace Table space containing the UnitSystem - in case other ECDb files are attached to this. Passing nullptr means to search all table spaces (starting with the primary one, aka main).
        //! @return The retrieved UnitSystem or nullptr if not found
        ECDB_EXPORT ECN::UnitSystemCP GetUnitSystem(Utf8StringCR schemaNameOrAlias, Utf8StringCR unitSystemName, SchemaLookupMode mode = SchemaLookupMode::ByName, Utf8CP tableSpace = nullptr) const;

        //! Gets the Phenomenon for the specified name.
        //! @param[in] schemaNameOrAlias Name (not full name) or alias of the schema containing the Phenomenon (@see @p mode)
        //! @param[in] phenName Name of the Phenomenon to be retrieved
        //! @param[in] mode indicates whether @p schemaNameOrAlias is a schema name or a schema alias
        //! @param[in] tableSpace Table space containing the Phenomenon - in case other ECDb files are attached to this. Passing nullptr means to search all table spaces (starting with the primary one, aka main).
        //! @return The retrieved Phenomenon or nullptr if not found
        ECDB_EXPORT ECN::PhenomenonCP GetPhenomenon(Utf8StringCR schemaNameOrAlias, Utf8StringCR phenName, SchemaLookupMode mode = SchemaLookupMode::ByName, Utf8CP tableSpace = nullptr) const;

        //! Gets the ECFormat for the specified name.
        //! @param[in] schemaNameOrAlias Name (not full name) or alias of the schema containing the ECFormat (@see @p mode)
        //! @param[in] formatName Name of the ECFormat to be retrieved
        //! @param[in] mode indicates whether @p schemaNameOrAlias is a schema name or a schema alias
        //! @param[in] tableSpace Table space containing the ECFormat - in case other ECDb files are attached to this. Passing nullptr means to search all table spaces (starting with the primary one, aka main).
        //! @return The retrieved ECFormat or nullptr if not found
        ECDB_EXPORT ECN::ECFormatCP GetFormat(Utf8StringCR schemaNameOrAlias, Utf8StringCR formatName, SchemaLookupMode mode = SchemaLookupMode::ByName, Utf8CP tableSpace = nullptr) const;

        //! Gets the PropertyCategory for the specified name.
        //! @param[in] schemaNameOrAlias Name (not full name) or alias of the schema containing the category (@see @p mode)
        //! @param[in] propertyCategoryName Name of the PropertyCategory to be retrieved
        //! @param[in] mode indicates whether @p schemaNameOrAlias is a schema name or a schema alias
        //! @param[in] tableSpace Table space containing the PropertyCategory - in case other ECDb files are attached to this. Passing nullptr means to search all table spaces (starting with the primary one, aka main).
        //! @return The retrieved PropertyCategory or nullptr if not found
        ECDB_EXPORT ECN::PropertyCategoryCP GetPropertyCategory(Utf8StringCR schemaNameOrAlias, Utf8StringCR propertyCategoryName, SchemaLookupMode mode = SchemaLookupMode::ByName, Utf8CP tableSpace = nullptr) const;

        //! Creates or updates views in the ECDb file to visualize the EC content as ECClasses and ECProperties rather than tables and columns.
        //! This can help debugging the EC data, especially when ECClasses and ECProperties share tables and columns or are spread across multiple tables.
        //! @note The views are strictly intended for developers for debugging purpose only. They should not be used in application code. 
        //! No code should depend on these views.
        //! @return SUCCESS or ERROR
        ECDB_EXPORT BentleyStatus CreateClassViewsInDb() const;

        //! Check if the schema is currently owned by current connection.
        //! @return true if own by current ecdb connection.
        ECDB_EXPORT bool OwnsSchema(ECN::ECSchemaCR schema) const;

        //! Creates or updates views in the ECDb file to visualize the EC content as ECClasses and ECProperties rather than tables and columns.
        //! This can help debugging the EC data, especially when ECClasses and ECProperties share tables and columns or are spread across multiple tables.
        //! @note The views are strictly intended for developers for debugging purpose only. They should not be used in application code. 
        //! No code should depend on these views.
        //! @param[in] ecclassids List of ECClassIds of those ECClasses for which to create ECClass views.
        //! @note ECClass views can only be created for ECEntityClasses and ECRelationshipClasses and only if they are mapped
        //! to the database. Otherwise the method will return an error.
        //! @return SUCCESS or ERROR
        ECDB_EXPORT BentleyStatus CreateClassViewsInDb(bvector<ECN::ECClassId> const& ecclassids) const;

        //! Called before any schema changes are applied
        ECDB_EXPORT SchemaChangeEvent& OnBeforeSchemaChanges() const;

        //! Called after any schema changes are applied or if apply process failed
        ECDB_EXPORT SchemaChangeEvent& OnAfterSchemaChanges() const;
#if !defined (DOCUMENTATION_GENERATOR)
        //! Truncates and repopulates ECDb's cache tables.
        //! @remarks ECDb maintains a few cache tables that cache meta data for performance reasons.
        //! @note In regular workflows (e.g. when calling SchemaManager::ImportECSchemas)
        //! <b>this method does not have to be called</b>. ECDb maintains the cache tables autonomously.
        //! @return SUCCESS or ERROR
        BentleyStatus RepopulateCacheTables() const;
        
        //! Automatically upgrade any existing ECInstance if required after ECSchema import.
        //! @note In regular workflows (e.g. when calling SchemaManager::ImportECSchemas)
        //! <b>this method does not have to be called</b>.
        //! @return SUCCESS or ERROR
        BentleyStatus UpgradeECInstances() const;

        void ClearCache() const;
        ECN::ECDerivedClassesList const* GetDerivedClassesInternal(ECN::ECClassCR baseClass, Utf8CP tableSpace = nullptr) const;
        Dispatcher const& GetDispatcher() const;
        struct MainSchemaManager const& Main() const;
#endif

    public:
        //=======================================================================================
        //! An IUnitsContext over all schemas in the schema manager
        // @bsiclass
        //+===============+===============+===============+===============+===============+======
        struct SchemaManagerUnitsContext final : public Units::IUnitsContext
            {
            SchemaManager const& m_ref;
            SchemaManagerUnitsContext(SchemaManager const& ref): m_ref(ref) {}

        protected:
            //! IUnitsContext methods
            ECDB_EXPORT ECN::ECUnitP _LookupUnitP(Utf8CP name, bool useFullName) const override;
            ECDB_EXPORT ECN::PhenomenonP _LookupPhenomenonP(Utf8CP name, bool useFullName) const override;
            ECDB_EXPORT ECN::UnitSystemP _LookupUnitSystemP(Utf8CP name, bool useFullName) const override;
            ECDB_EXPORT void _AllPhenomena(bvector<Units::PhenomenonCP>& allPhenomena) const override;
            ECDB_EXPORT void _AllUnits(bvector<Units::UnitCP>& allUnits) const override;
            ECDB_EXPORT void _AllSystems(bvector<Units::UnitSystemCP>& allUnitSystems) const override;
            };

        SchemaManagerUnitsContext GetUnitsContext() const { return SchemaManagerUnitsContext(*this); }
    };

typedef SchemaManager const& SchemaManagerCR;

ENUM_IS_FLAGS(SchemaManager::SchemaImportOptions);

END_BENTLEY_SQLITE_EC_NAMESPACE
