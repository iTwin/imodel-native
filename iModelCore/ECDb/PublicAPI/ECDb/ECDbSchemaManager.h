/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/ECDbSchemaManager.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <ECDb/ECDbTypes.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#if !defined (DOCUMENTATION_GENERATOR)
struct ECDbMap;
struct ECDbSchemaReader;
struct ECDbSchemaWriter;
struct SchemaImportContext;
#endif

//=======================================================================================
//! The ECDbSchemaManager manages @ref ECN::ECSchema "ECSchemas" in the @ref ECDbFile "ECDb file". 
//! Clients can import @ref ECN::ECSchema "ECSchemas" into or retrieve @ref ECN::ECSchema "ECSchemas" or 
//! individual @ref ECN::ECClass "ECClasses" from the %ECDb file using the %ECDbSchemaManager.
//!
//! ###Incremental Loading of ECClasses
//! ECDbSchemaManager supports incremental loading of ECClasses. So unlike with ECObjects calling
//! ECDbSchemaManager::GetECClass doesn't load the rest of the ECN::ECSchema. 
//! When incrementally loading ECClasses you shouldn't use ECN::ECSchema::GetECClass because 
//! the ECN::ECSchema might not be fully populated yet. Always use the respective ECDbSchemaManager
//! methods instead.
//!
//! ####Details
//! 
//! (which you don't need to be aware of if you consistently use the ECDbSchemaManager API to get ECClasses)
//!
//! * ECDbSchemaManager::GetECSchema with <c>ensureAllClassesLoaded=false</c> 
//!     * Returns the ECSchema with only those ECClasses that have been loaded previously already. If
//!       no ECClasses have been loaded previously, the returned ECSchema is empty.
//! * ECDbSchemaManager::GetECClass
//!     * loads the specified ECClass
//!     * loads all base classes of the specified ECClass
//!     * loads all struct classes used by the specified ECClass
//!     * loads all CustomAttribute classes used by the specified ECClass
//!     * does @b not load derived classes of the specified ECClass
//! * ECDbSchemaManager::GetECClass for a relationship class
//!     * same as for regular classes
//!     * loads the ECClasses specified in the constraints of the relationship class
//!       (but @b not their derived ECClasses)
//!        
//! ### %ECDbSchemaManager is an %IECSchemaLocater
//! ECDbSchemaManager also implements ECN::IECSchemaLocater so it can be used to locate ECSchemas
//! already stored in the %ECDb file when reading an ECSchema from disk, for example:
//! 
//!     ECN::ECSchemaReadContextPtr ecSchemaContext = ECN::ECSchemaReadContext::CreateContext();
//!     ecSchemaContext->AddSchemaLocater(ecdb.GetSchemaLocater());
//!     ECN::SchemaKey schemaKey("foo", 1, 0);
//!     ECN::ECSchemaPtr fooSchema = ECN::ECSchema::LocateSchema(schemaKey, *ecSchemaContext);
//!
//! @see @ref ECDbOverview, @ref ECDbTransactions, @ref ECDbCodeSamples
//! @ingroup ECDbGroup
// @bsimethod                                                  Affan.Khan        05/2012
//+===============+===============+===============+===============+===============+======
struct ECDbSchemaManager : ECN::IECSchemaLocater, ECN::IECClassLocater, NonCopyableClass
    {
    private:
        ECDb const& m_ecdb;
        ECDbMap& m_map;
        ECDbSchemaReader* m_schemaReader;
        mutable BeMutex m_criticalSection;

        BentleyStatus BatchImportECSchemas(SchemaImportContext&, ECN::ECSchemaCacheR) const;

        ECN::ECSchemaCP GetECSchema(ECN::ECSchemaId, bool ensureAllClassesLoaded) const;
        //! Ensure that all direct subclasses of @p ecClass are loaded. Subclasses of its subclasses are not loaded
        //! @param[in] ecClass ECClass whose direct subclasses should be loaded
        //! @return ::SUCCESS or ::ERROR
        BentleyStatus EnsureDerivedClassesExist(ECN::ECClassCR) const;
        //! Implementation of IECSchemaLocater
        virtual ECN::ECSchemaPtr _LocateSchema(ECN::SchemaKeyR, ECN::SchemaMatchType, ECN::ECSchemaReadContextR) override;

        //! Implementation of IECClassLocater
        virtual ECN::ECClassCP _LocateClass(Utf8CP schemaName, Utf8CP className) override;

    public:
#if !defined (DOCUMENTATION_GENERATOR)
        ECDbSchemaManager(ECDb const&, ECDbMap&);
        ~ECDbSchemaManager();
#endif

        //! Imports all @ref ECN::ECSchema "ECSchemas" contained by the @p schemaCache and all
        //! their referenced @ref ECN::ECSchema "ECSchemas" into the @ref ECDbFile "ECDb file".
        //! ECSchemas that already exist in the file are updated (see @ref ECDbECSchemaUpdateSupportedFeatures).
        //! @note After importing the schemas, any pointers to the existing schemas should be discarded and
        //! they should be obtained as needed through the ECDbECSchemaManager API.
        //!
        //! @param[in] schemaCache Typically obtained from ECSchemaReadContext.GetCache() that contains the imported ECSchema and all of its referenced ECSchemas.
        //!                     If the referenced ECSchemas are known to have already been imported, they are not required, but it does no harm to include them again
        //!                     (the method detects that they are already imported, and simply skips them)
        //!                     All schemas should be read from a single ECN::ECSchemaReadContext. 
        //!                     If any duplicates are found in @p schemaCache an error will returned.
        //! @return BentleyStatus::SUCCESS or BentleyStatus::ERROR (error details are being logged)
        //! @see @ref ECDbECSchemaImportAndUpdate
        ECDB_EXPORT BentleyStatus ImportECSchemas(ECN::ECSchemaCacheR schemaCache) const;

        //! Checks whether the ECDb file contains the ECSchema with the specified name or not.
        //! @param[in] schemaName Name of the ECSchema to check for
        //! @return true if the ECDb file contains the ECSchema. false otherwise.
        ECDB_EXPORT bool ContainsECSchema(Utf8CP schemaName) const;

        //! Get an ECSchema by name
        //! @param[in] schemaName Name (not full name) of the ECSchema to retrieve
        //! @param[in] ensureAllClassesLoaded true, if all classes in the ECSchema should be proactively loaded into memory. false,
        //!                                   if they are loaded on-demand.
        //! @return The retrieved ECSchema or nullptr if not found
        ECDB_EXPORT ECN::ECSchemaCP GetECSchema(Utf8CP schemaName, bool ensureAllClassesLoaded = true) const;

        //! Gets all @ref ECN::ECSchema "ECSchemas" stored in the @ref ECDbFile "ECDb file"
        //! @param[out] schemas The retrieved list of ECSchemas
        //! @param[in] ensureAllClassesLoaded true, if all classes in the ECSchema should be proactively loaded into memory. false,
        //!                                   if they are loaded on-demand.
        //! @return BentleyStatus::SUCCESS or BentleyStatus::ERROR
        ECDB_EXPORT BentleyStatus GetECSchemas(bvector<ECN::ECSchemaCP>& schemas, bool ensureAllClassesLoaded = true) const;

        //! Gets the ECClass for the specified name.
        //! @param[in] schemaNameOrPrefix Name (not full name) or namespace prefix of the schema containing the class (@see @p resolveSchema)
        //! @param[in] className Name of the class to be retrieved
        //! @param[in] resolveSchema indicates whether @p schemaNameOrPrefix is a schema name or a schema prefix
        //! @return The retrieved ECClass or nullptr if not found
        ECDB_EXPORT ECN::ECClassCP GetECClass(Utf8CP schemaNameOrPrefix, Utf8CP className, ResolveSchema resolveSchema = ResolveSchema::BySchemaName) const;

        //! Gets the ECClass for the specified ECClassId.
        //! @param[in] ecClassId Id of the ECClass to retrieve
        //! @return The retrieved ECClass or nullptr if not found
        ECDB_EXPORT ECN::ECClassCP GetECClass(ECN::ECClassId ecClassId) const;

        //! Gets the ECClassId for the ECClass with the specified name.
        //! @param[out] id ECClassId of the requested ECClass.
        //! @param[in] schemaNameOrPrefix Name (not full name) or namespace prefix of the schema containing the class (@see @p resolveSchema)
        //! @param[in] className Name of the class to be retrieved
        //! @param[in] resolveSchema indicates whether @p schemaNameOrPrefix is a schema name or a schema prefix
        //! @return true, if the %ECDb file has an ECClass with the given name, false, if the ECClass does not exist in the %ECDb file
        ECDB_EXPORT bool TryGetECClassId(ECN::ECClassId& id, Utf8CP schemaNameOrPrefix, Utf8CP className, ResolveSchema resolveSchema = ResolveSchema::BySchemaName) const;

        //! Gets the ECClassId for the ECClass with the specified name.
        //! @param[in] schemaNameOrPrefix Name (not full name) or namespace prefix of the schema containing the class (@see @p resolveSchema)
        //! @param[in] className Name of the class to be retrieved
        //! @param[in] resolveSchema indicates whether @p schemaNameOrPrefix is a schema name or a schema prefix
        //! @return ECClassId of the requested ECClass. If the ECClass does not exist in the %ECDb file, an invalid class id is returned
        ECN::ECClassId GetECClassId(Utf8CP schemaNameOrPrefix, Utf8CP className, ResolveSchema resolveSchema = ResolveSchema::BySchemaName) const { ECN::ECClassId id; TryGetECClassId(id, schemaNameOrPrefix, className, resolveSchema); return id; }

        //! Gets the derived classes of @p baseECClass. The derived classes are loaded, if they are not yet.
        //! Callers should use this method in favor of ECN::ECClass::GetDerivedECClasses to ensure
        //! that derived classes are actually loaded from the ECDb file.
        //! This method allows to just load the inheritance hierarchy of a given ECClass without having
        //! to load entire ECSchemas.
        //! @note This does not recurse into derived classes of derived classes. It just returns the first level
        //! of inheriting ECClasses.
        //! @param[in] baseECClass ECClass to return derived classes for.
        //! @return Derived classes list
        //! @see ECN::ECClass::GetDerivedECClasses
        ECDB_EXPORT ECN::ECDerivedClassesList const& GetDerivedECClasses(ECN::ECClassCR baseECClass) const;

        //! Gets the ECEnumeration for the specified name.
        //! @param[in] schemaName Name (not full name) of the schema containing the ECEnumeration
        //! @param[in] enumName Name of the ECEnumeration to be retrieved
        //! @return The retrieved ECEnumeration or nullptr if not found
        ECDB_EXPORT ECN::ECEnumerationCP GetECEnumeration(Utf8CP schemaName, Utf8CP enumName) const;

        //! Creates or updates views in the ECDb file to visualize the EC content as ECClasses and ECProperties rather than tables and columns.
        //! This can help debugging the EC data, especially when ECClasses and ECProperties share tables and columns or are spread across multiple tables.
        //! @note The views are strictly intended for developers for debugging purpose only. They should not be used in application code. 
        //! No code should depend on these views.
        //! @return SUCCESS or ERROR
        ECDB_EXPORT BentleyStatus CreateECClassViewsInDb() const;

#if !defined (DOCUMENTATION_GENERATOR)    
        //! For cases where we are working with an ECClass in a referenced ECSchema that is a duplicate of one already persisted
        //! and therefore doesn't have the persistent ECClassId set. Generally, we would prefer that the primary ECSchema had
        //! been deserialized using the persisted copies of the referenced ECSchema, but we cannot ensure that is always the case
        static ECN::ECClassId GetClassIdForECClassFromDuplicateECSchema(ECDbCR, ECN::ECClassCR);

        //! For cases where we are working with an ECProperty in a referenced ECSchema that is a duplicate of one already persisted
        //! and therefore doesn't have the persistent ECPropertyId set. Generally, we would prefer that the primary ECSchema had
        //! been deserialized using the persisted copies of the referenced ECSchema, but we cannot ensure that is always the case
        static ECN::ECPropertyId GetPropertyIdForECPropertyFromDuplicateECSchema(ECDbCR, ECN::ECPropertyCR);

        //! For cases where we are working with an ECSchema in a referenced ECSchema that is a duplicate of one already persisted
        //! and therefore doesn't have the persistent ECSchemaId set. Generally, we would prefer that the primary ECSchema had
        //! been deserialized using the persisted copies of the referenced ECSchema, but we cannot ensure that is always the case
        static ECN::ECSchemaId GetSchemaIdForECSchemaFromDuplicateECSchema(ECDbCR, ECN::ECSchemaCR);

        void ClearCache() const;
        ECDbCR GetECDb() const;
#endif
    };

typedef ECDbSchemaManager const& ECDbSchemaManagerCR;


#if !defined (DOCUMENTATION_GENERATOR)
//=======================================================================================
//! Helper that provide map information
//=======================================================================================
struct ECDbMapDebugInfo
    {
private:
    ECDbMapDebugInfo();
    ~ECDbMapDebugInfo();

public:
    ECDB_EXPORT static BentleyStatus GetMapInfoForSchema(Utf8StringR info, ECDbCR, Utf8CP ecSchemaName, bool skipUnmappedClasses);
    ECDB_EXPORT static BentleyStatus GetMapInfoForClass(Utf8StringR info, ECDbCR, Utf8CP ecClassName);
    ECDB_EXPORT static BentleyStatus GetMapInfoForAllClasses (Utf8StringR info, ECDbCR, bool skipUnmappedClasses);
    };

#endif
END_BENTLEY_SQLITE_EC_NAMESPACE
