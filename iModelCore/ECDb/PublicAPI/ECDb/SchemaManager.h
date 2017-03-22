/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/SchemaManager.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <ECDb/ECDb.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#if !defined (DOCUMENTATION_GENERATOR)
struct DbMap;
struct SchemaReader;
struct SchemaImportContext;
#endif

//=======================================================================================
//! Options for how to specify the ECSchema when calling SchemaManager::GetClass
//! @ingroup ECDbGroup
// @bsiclass                                                Muhammad.Zaighum      10/2014
//+===============+===============+===============+===============+===============+======
enum class ResolveSchema
    {
    BySchemaName, //!< ECClass is qualified by schema name
    BySchemaAlias, //!< ECClass is qualified by schema alias
    AutoDetect//!< Detect automatically whether ECClass is qualified by schema name or alias
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
// @bsimethod                                                  Affan.Khan        05/2012
//+===============+===============+===============+===============+===============+======
struct SchemaManager final : ECN::IECSchemaLocater, ECN::IECClassLocater, NonCopyableClass
    {
    private:
        ECDb const& m_ecdb;
        SchemaReader* m_schemaReader;
        DbMap* m_dbMap;
        BeMutex& m_mutex;

        BentleyStatus DoImportSchemas(SchemaImportContext&, bvector<ECN::ECSchemaCP> const& schemas, SchemaImportToken const*) const;
        BentleyStatus PersistSchemas(SchemaImportContext&, bvector<ECN::ECSchemaCP> const&) const;

        ECN::ECSchemaCP GetSchema(ECN::ECSchemaId, bool loadSchemaEntities) const;

        //! Implementation of IECSchemaLocater
        ECN::ECSchemaPtr _LocateSchema(ECN::SchemaKeyR, ECN::SchemaMatchType, ECN::ECSchemaReadContextR) override;

        //! Implementation of IECClassLocater
        ECN::ECClassCP _LocateClass(Utf8CP schemaName, Utf8CP className) override;

    public:
#if !defined (DOCUMENTATION_GENERATOR)
        SchemaManager(ECDb const&, BeMutex&);
        ~SchemaManager();
#endif

        //! Imports the list of @ref ECN::ECSchema "ECSchemas" (which must include all its references)
        //! into the @ref ECDbFile "ECDb file".
        //! ECSchemas that already exist in the file are updated (see @ref ECDbECSchemaUpdateSupportedFeatures).
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
        //! @see @ref ECDbECSchemaImportAndUpdate
        ECDB_EXPORT BentleyStatus ImportSchemas(bvector<ECN::ECSchemaCP> const& schemas, SchemaImportToken const* token = nullptr) const;

#if !defined (DOCUMENTATION_GENERATOR)
        //only for legacy support which cannot yet follow the strict bim rules
        ECDB_EXPORT BentleyStatus ImportSchemas(bvector<ECN::ECSchemaCP> const& schemas, bool doNotFailSchemaValidationForLegacyIssues, SchemaImportToken const* token = nullptr) const;
#endif

        //! Checks whether the ECDb file contains the ECSchema with the specified name or not.
        //! @param[in] schemaName Name of the ECSchema to check for
        //! @return true if the ECDb file contains the ECSchema. false otherwise.
        ECDB_EXPORT bool ContainsSchema(Utf8CP schemaName) const;

        //! Get an ECSchema by name
        //! @param[in] schemaName Name (not full name) of the ECSchema to retrieve
        //! @param[in] loadSchemaEntities true, if all ECClasses, ECEnumerations, KindOfQuantities in the ECSchema should be pro-actively loaded into memory. false,
        //!                                   if they are loaded on-demand.
        //! @return The retrieved ECSchema or nullptr if not found
        ECDB_EXPORT ECN::ECSchemaCP GetSchema(Utf8CP schemaName, bool loadSchemaEntities = true) const;

        //! Gets all @ref ECN::ECSchema "ECSchemas" stored in the @ref ECDbFile "ECDb file"
        //! @param[in] loadSchemaEntities true, if all ECClasses, ECEnumerations, KindOfQuantities in the ECSchema should be pro-actively loaded into memory. false,
        //!                                   if they are loaded on-demand.
        //! @return Vector of all ECSchemas stored in the file
        ECDB_EXPORT bvector<ECN::ECSchemaCP> GetSchemas(bool loadSchemaEntities = true) const;

        //! Gets the ECClass for the specified name.
        //! @param[in] schemaNameOrAlias Name (not full name) or alias of the schema containing the class (@see @p resolveSchema)
        //! @param[in] className Name of the class to be retrieved
        //! @param[in] resolveSchema indicates whether @p schemaNameOrAlias is a schema name or a schema alias
        //! @return The retrieved ECClass or nullptr if not found
        ECDB_EXPORT ECN::ECClassCP GetClass(Utf8StringCR schemaNameOrAlias, Utf8StringCR className, ResolveSchema resolveSchema = ResolveSchema::BySchemaName) const;

        //! Gets the ECClassId for the ECClass with the specified name.
        //! @param[in] schemaNameOrAlias Name (not full name) or alias of the schema containing the class (@see @p resolveSchema)
        //! @param[in] className Name of the class to be retrieved
        //! @param[in] resolveSchema indicates whether @p schemaNameOrAlias is a schema name or a schema alias
        //! @return ECClassId of the requested ECClass. If the ECClass does not exist in the %ECDb file, an invalid class id is returned
        ECDB_EXPORT ECN::ECClassId GetClassId(Utf8StringCR schemaNameOrAlias, Utf8StringCR className, ResolveSchema resolveSchema = ResolveSchema::BySchemaName) const;

        //! Gets the ECClass for the specified ECClassId.
        //! @param[in] ecClassId Id of the ECClass to retrieve
        ECDB_EXPORT ECN::ECClassCP GetClass(ECN::ECClassId ecClassId) const;

        //! Gets the derived classes of @p baseECClass. The derived classes are loaded, if they are not yet.
        //! Callers should use this method in favor of ECN::ECClass::GetDerivedClasses to ensure
        //! that derived classes are actually loaded from the ECDb file.
        //! This method allows to just load the inheritance hierarchy of a given ECClass without having
        //! to load entire ECSchemas.
        //! @note This does not recurse into derived classes of derived classes. It just returns the first level
        //! of inheriting ECClasses.
        //! @param[in] baseECClass ECClass to return derived classes for.
        //! @return Derived classes list
        //! @see ECN::ECClass::GetDerivedClasses
        ECDB_EXPORT ECN::ECDerivedClassesList const& GetDerivedClasses(ECN::ECClassCR baseECClass) const;

        //! Gets the ECEnumeration for the specified name.
        //! @param[in] schemaName Name (not full name) of the schema containing the ECEnumeration
        //! @param[in] enumName Name of the ECEnumeration to be retrieved
        //! @return The retrieved ECEnumeration or nullptr if not found
        ECDB_EXPORT ECN::ECEnumerationCP GetEnumeration(Utf8CP schemaName, Utf8CP enumName) const;

        //! Gets the KindOfQuantity for the specified name.
        //! @param[in] schemaName Name (not full name) of the schema containing the KindOfQuantity
        //! @param[in] koqName Name of the KindOfQuantity to be retrieved
        //! @return The retrieved KindOfQuantity or nullptr if not found
        ECDB_EXPORT ECN::KindOfQuantityCP GetKindOfQuantity(Utf8CP schemaName, Utf8CP koqName) const;

        //! Creates or updates views in the ECDb file to visualize the EC content as ECClasses and ECProperties rather than tables and columns.
        //! This can help debugging the EC data, especially when ECClasses and ECProperties share tables and columns or are spread across multiple tables.
        //! @note The views are strictly intended for developers for debugging purpose only. They should not be used in application code. 
        //! No code should depend on these views.
        //! @return SUCCESS or ERROR
        ECDB_EXPORT BentleyStatus CreateClassViewsInDb() const;

        //! Creates or updates views in the ECDb file to visualize the EC content as ECClasses and ECProperties rather than tables and columns.
        //! This can help debugging the EC data, especially when ECClasses and ECProperties share tables and columns or are spread across multiple tables.
        //! @note The views are strictly intended for developers for debugging purpose only. They should not be used in application code. 
        //! No code should depend on these views.
        //! @param[in] ecclassids List of ECClassIds of those ECClasses for which to create ECClass views.
        //! @note ECClass views can only be created for ECEntityClasses and ECRelationshipClasses and only if they are mapped
        //! to the database. Otherwise the method will return an error.
        //! @return SUCCESS or ERROR
        ECDB_EXPORT BentleyStatus CreateClassViewsInDb(bvector<ECN::ECClassId> const& ecclassids) const;

#if !defined (DOCUMENTATION_GENERATOR)
        //! Only use this until we solved handling legacy v8 class inheritance issues when BISifying v8 ECSchemas.
        //! Gets the SQLite SELECT SQL statement to detect those issues. The SELECT clause of the SQL has these columns:
        //! ECSchema Name, ECSchema Alias, ECClass Name, Table Name, Issue Type (INT), Issue Type Description, Issue (JSON)
        //! @return SQLite Validate SELECT SQL
        ECDB_EXPORT static Utf8CP GetValidateDbMappingSql();

        void ClearCache() const;
        SchemaReader const& GetReader() const;
        DbMap const& GetDbMap() const;
        ECDb const& GetECDb() const;
#endif
    };

typedef SchemaManager const& SchemaManagerCR;


END_BENTLEY_SQLITE_EC_NAMESPACE
