/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/ECDbSchemaManager.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <ECDb/ECDbTypes.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#if !defined (DOCUMENTATION_GENERATOR)

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECSchemaKey
    {
private:
    ECN::ECSchemaId m_ecSchemaId;
    Utf8String m_name;
    Utf8String m_displayLabel;
    uint32_t m_versionMajor;
    uint32_t m_versionMinor;
public:
    ECSchemaKey (ECN::ECSchemaId ecSchemaId, Utf8CP name, uint32_t versionMajor, uint32_t versionMinor, Utf8CP displayLabel)
        : m_ecSchemaId (ecSchemaId), m_name (name), m_versionMajor (versionMajor), m_versionMinor (versionMinor), m_displayLabel (displayLabel) {}

    ECN::ECSchemaId GetECSchemaId () const {BeAssert (m_ecSchemaId != 0ULL); return m_ecSchemaId;}
    uint32_t GetVersionMajor () const {return m_versionMajor;}
    uint32_t GetVersionMinor () const {return m_versionMinor; }
    Utf8CP GetName () const {return m_name.c_str ();}
    Utf8CP GetDisplayLabel () const {return m_displayLabel.empty () ? GetName () : m_displayLabel.c_str ();}
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECClassKey
    {
private:
    ECN::ECClassId m_ecClassId;
    Utf8String m_name;
    Utf8String m_displayLabel;
public:
    ECClassKey (ECN::ECClassId ecClassId, Utf8CP name, Utf8CP displayLabel = nullptr) : m_ecClassId (ecClassId), m_name (name), m_displayLabel (displayLabel) {}
    ECN::ECClassId GetECClassId () const {return m_ecClassId;}
    Utf8CP GetName () const {return m_name.c_str ();}
    Utf8CP GetDisplayLabel () const {return m_displayLabel.empty () ? GetName () : m_displayLabel.c_str ();}
    };

typedef bvector<ECSchemaKey> ECSchemaKeys;
typedef bvector<ECClassKey> ECClassKeys;

struct ECDbMap;
struct ECDbSchemaReader;
struct ECDbSchemaWriter;
struct SchemaImportContext;
#endif

typedef bvector<ECN::ECSchemaCP> ECSchemaList;

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
//! ### %ECDbSchemaManager is a Schema Locater
//! ECDbSchemaManager also implements ECN::IECSchemaLocater so it can be used to locate ECSchemas
//! already stored in the %ECDb file when reading an ECSchema from disk, for example:
//! 
//!     ECN::ECSchemaReadContextPtr ecSchemaContext = ECN::ECSchemaReadContext::CreateContext ();
//!     ecSchemaContext->AddSchemaLocater(ecdb.GetSchemaLocater());
//!     ECN::SchemaKey schemaKey ("foo", 1, 0);
//!     ECN::ECSchemaPtr fooSchema = ECN::ECSchema::LocateSchema (schemaKey, *ecSchemaContext);
//!     schemaManager.ImportECSchemas(ecSchemaContext->GetCache());
//!
//! @see @ref ECDbOverview, @ref ECDbTransactions, @ref ECDbCodeSamples
//! @ingroup ECDbGroup
// @bsimethod                                                  Affan.Khan        05/2012
//+===============+===============+===============+===============+===============+======
struct ECDbSchemaManager : ECN::IECSchemaLocater, ECN::IECClassLocater, NonCopyableClass
    {
public:
     //=======================================================================================
    //! Options for importing an ECSchema into an ECDb file.
    //! @see ECDbSchemaManager::ImportECSchemas
    // @bsiclass                                                Krischan.Eberle      04/2014
    //+===============+===============+===============+===============+===============+======
    struct ImportOptions : NonCopyableClass
        {
    private:
        bool m_doSupplementation;
        bool m_updateExistingSchemas;
        bool m_supportLegacySchemas;

    public:
        //! Initializes an ImportOptions object with default settings:
        //!     - ImportOptions::DoSupplementation : true
        //!     - ImportOptions::UpdateExistingSchemas : false        
        ECDB_EXPORT ImportOptions ();

        //! Initializes an ImportOptions object
        //! @param[in] doSupplementation Normally true. If the list of schemas to be imported contain supplemental ECSchemas, use 
        //!                         them to supplement primary ECSchemas in the list of schemas to be imported.
        //!                         Otherwise, supplemental ECSchemas will be ignored
        //! @param[in] updateExistingSchemas Attempts to update any existing schema that is found in the list of schemas to import.
        //!                         @b WARNING: List of supported features:
        //!                           1. If a ECSchema already exist in ECDb and importing schema cache has same schema with minor version greater than existing ECSchema. 
        //!                              The ECSchema would be marked for upgrade.
        //!                           2. Add new ECClass to existing ECSchema
        //!                           3. Add new ECProperties to existing ECClass  
        //!                           4. Update only "DisplayLabel" and "Description" property of existing ECSchema.
        //!                           5. Update only "DisplayLabel" and "Description" property of existing ECClass.
        //!                           6. Update only "DisplayLabel" and "Description" property of existing ECProperty.
        //!                           7. If existing schema have different value of customAttribute for a container then it will be replaced with its new value.
        //!                          Updating of ECDb Mapping Hint custom attribute on existing ECClasses or ECProperties will be ignored with warning.
        //!                          Any other kind of change will cause operation to fail. After upgrade the schemas cached by ECDb has been cleared. 
        //!                          Any existing references to ECSchemas, ECClasses, or ECSqlStatements become invalid.  
        ECDB_EXPORT ImportOptions (bool doSupplementation, bool updateExistingSchemas);

        //! Gets a value indicating whether supplementation should be performed or not.
        //! If the list of schemas to be imported contain supplemental ECSchemas, use
        //! them to supplement primary ECSchemas in the list of schemas to be imported.
        //! Otherwise, supplemental ECSchemas will be ignored        
        ECDB_EXPORT bool DoSupplementation () const;

        //!Gets a value indicating whether existing ECSchemas should be updated or not.
        //!If true, %ECDb attempts to update any existing schema that is found in the list of schemas to import.
        //!
        //!@b WARNING: List of supported features:
        //!     1. If a ECSchema already exist in ECDb and importing schema cache has same schema with minor version greater than existing ECSchema. 
        //!        The ECSchema would be marked for upgrade.
        //!     2. Add new ECClass to existing ECSchema
        //!     3. Add new ECProperties to existing ECClass  
        //!     4. Update only "DisplayLabel" and "Description" property of existing ECSchema.
        //!     5. Update only "DisplayLabel" and "Description" property of existing ECClass.
        //!     6. Update only "DisplayLabel" and "Description" property of existing ECProperty.
        //!     7. If existing schema have different value of customAttribute for a container then it will be replaced with its new value.
        //!        Updating of ECDb Mapping Hint custom attribute on existing ECClasses or ECProperties will be ignored with warning.
        //!     Any other kind of change will cause operation to fail. After upgrade the schemas cached by ECDb has been cleared. 
        //!     Any existing references to ECSchemas, ECClasses, or ECSqlStatements become invalid.  
        ECDB_EXPORT bool UpdateExistingSchemas () const;

#if !defined (DOCUMENTATION_GENERATOR)
        //only to be used by publisher scenarios which have to support v8i legacy ECSchemas which do not comply to the current ECSchema design
        //standards
        ECDB_EXPORT void SetSupportLegacySchemas ();
        ECDB_EXPORT bool SupportLegacySchemas () const;
#endif
        };

private:
    ECDb& m_ecdb;
    ECDbMap& m_map;
    RefCountedPtr<ECDbSchemaReader> m_ecReader;
    RefCountedPtr<ECDbSchemaWriter> m_ecImporter;
    mutable BeMutex m_criticalSection;

    BentleyStatus BatchImportOrUpdateECSchemas (SchemaImportContext const&, bvector<ECN::ECSchemaCP>& importedSchemas, bvector<ECN::ECDiffPtr>&  diffs, ECN::ECSchemaCacheR, ImportOptions const&, bool addToReaderCache = false) const;
    BentleyStatus ImportECSchema (ECN::ECSchemaCR, bool addToReaderCache = false) const;
    BentleyStatus UpdateECSchema (ECN::ECDiffPtr&, ECN::ECSchemaCR) const;
    void ReportUpdateError (ECN::ECSchemaCR newSchema, ECN::ECSchemaCR existingSchema, Utf8CP reason) const;
    
    ECN::ECSchemaCP GetECSchema (ECN::ECSchemaId, bool ensureAllClassesLoaded) const;
    //! Ensure that all direct subclasses of @p ecClass are loaded. Subclasses of its subclasses are not loaded
    //! @param[in] ecClass ECClass whose direct subclasses should be loaded
    //! @return ::SUCCESS or ::ERROR
    BentleyStatus EnsureDerivedClassesExist (ECN::ECClassCR) const;
    //! Implementation of IECSchemaLocater
    virtual ECN::ECSchemaPtr _LocateSchema (ECN::SchemaKeyR, ECN::SchemaMatchType, ECN::ECSchemaReadContextR) override;

    //! Implementation of IECClassLocater
    virtual ECN::ECClassCP _LocateClass (Utf8CP schemaName, Utf8CP className) override;

public:
#if !defined (DOCUMENTATION_GENERATOR)
    ECDbSchemaManager(ECDb&, ECDbMap&);
    virtual ~ECDbSchemaManager ();
#endif

    //! Imports all @ref ECN::ECSchema "ECSchemas" contained by the @p schemaCache and all
    //! their referenced @ref ECN::ECSchema "ECSchemas" into the @ref ECDbFile "ECDb file".
    //! After importing the schemas, any pointers to the existing schemas should be discarded and
    //! they should be obtained as needed through the ECDbECSchemaManager API.
    //!
    //! @param[in] schemaCache Typically obtained from ECSchemaReadContext.GetCache() that contains the imported ECSchema and all of its referenced ECSchemas.
    //!                     If the referenced ECSchemas are known to have already been imported, they are not required, but it does no harm to include them again
    //!                     (the method detects that they are already imported, and simply skips them)
    //!                     All schemas should be read from single ECSchemaReadContext.  if any dublicate schema is found in schemaCache the function will return error.
    //! @param[in] options Schema import options
    //! @return BentleyStatus::SUCCESS or BentleyStatus::ERROR (error details are being logged)
    ECDB_EXPORT BentleyStatus ImportECSchemas (ECN::ECSchemaCacheR schemaCache, ImportOptions const& options = ImportOptions()) const;
    
    //! Checks whether the ECDb file contains the ECSchema with the specified name or not.
    //! @param[in] schemaName Name of the ECSchema to check for
    //! @return true if the ECDb file contains the ECSchema. false otherwise.
    ECDB_EXPORT bool ContainsECSchema(Utf8CP schemaName) const;

    //! Get an ECSchema by name
    //! @param[in] schemaName Name (not full name) of the ECSchema to retrieve
    //! @param[in] ensureAllClassesLoaded true, if all classes in the ECSchema should be proactively loaded into memory. false,
    //!                                   if they are loaded on-demand.
    //! @return The retrieved ECSchema or nullptr if not found
    ECDB_EXPORT ECN::ECSchemaCP GetECSchema (Utf8CP schemaName, bool ensureAllClassesLoaded = true) const;

    //! Gets all @ref ECN::ECSchema "ECSchemas" stored in the @ref ECDbFile "ECDb file"
    //! @param[out] schemas The retrieved list of ECSchemas
    //! @param[in] ensureAllClassesLoaded true, if all classes in the ECSchema should be proactively loaded into memory. false,
    //!                                   if they are loaded on-demand.
    //! @return BentleyStatus::SUCCESS or BentleyStatus::ERROR
    ECDB_EXPORT BentleyStatus GetECSchemas (ECSchemaList& schemas, bool ensureAllClassesLoaded = true) const;

#if !defined (DOCUMENTATION_GENERATOR)
    //replace following and it should return DbECSchemaKeys
    // Keys base functions
    ECDB_EXPORT BentleyStatus GetECSchemaKeys (ECSchemaKeys& keys) const;
    ECDB_EXPORT BentleyStatus GetECClassKeys (ECClassKeys& keys, Utf8CP schemaName) const;
#endif

    //! Gets the ECClass for the specified name.
    //! @param[in] schemaNameOrPrefix Name (not full name) or namespace prefix of the schema containing the class (@see @p resolveSchema)
    //! @param[in] className Name of the class to be retrieved
    //! @param[in] resolveSchema indicates whether @p schemaNameOrPrefix is a schema name or a schema prefix
    //! @return The retrieved ECClass or nullptr if not found
    ECDB_EXPORT ECN::ECClassCP GetECClass (Utf8CP schemaNameOrPrefix, Utf8CP className, ResolveSchema resolveSchema = ResolveSchema::BySchemaName) const;

    //! Gets the ECClass for the specified ECClassId.
    //! @param[in] ecClassId Id of the ECClass to retrieve
    //! @return The retrieved ECClass or nullptr if not found
    ECDB_EXPORT ECN::ECClassCP GetECClass (ECN::ECClassId ecClassId) const;

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
    ECN::ECClassId GetECClassId(Utf8CP schemaNameOrPrefix, Utf8CP className, ResolveSchema resolveSchema = ResolveSchema::BySchemaName) const { ECN::ECClassId id = ECN::ECClass::UNSET_ECCLASSID; TryGetECClassId(id, schemaNameOrPrefix, className, resolveSchema); return id; }

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
    ECDB_EXPORT ECN::ECDerivedClassesList const& GetDerivedECClasses (ECN::ECClassCR baseECClass) const;

#if !defined (DOCUMENTATION_GENERATOR)    
    //! For cases where we are working with an ECClass in a referenced ECSchema that is a duplicate of one already persisted
    //! and therefore doesn't have the persistent ECClassId set. Generally, we would prefer that the primary ECSchema had
    //! been deserialized using the persisted copies of the referenced ECSchema, but we cannot ensure that is always the case
    //! @param db must be an ECDb, but left as Db because that is what the callers actually have... needs refactoring for Graphite02
    //! @param ecClass The ECClass in the duplicate ECSchema. Its Id will be set (as well as returned)
    static ECN::ECClassId GetClassIdForECClassFromDuplicateECSchema(ECDbCR db, ECN::ECClassCR ecClass);

    //! For cases where we are working with an ECProperty in a referenced ECSchema that is a duplicate of one already persisted
    //! and therefore doesn't have the persistent ECPropertyId set. Generally, we would prefer that the primary ECSchema had
    //! been deserialized using the persisted copies of the referenced ECSchema, but we cannot ensure that is always the case
    //! @param db must be an ECDb, but left as Db because that is what the callers actually have... needs refactoring for Graphite02
    //! @param ecProperty. The ECProperty in the duplicate ECSchema. Its Id will be set (as well as returned)
    static ECN::ECPropertyId GetPropertyIdForECPropertyFromDuplicateECSchema(ECDbCR db, ECN::ECPropertyCR ecProperty);

    //! For cases where we are working with an ECSchema in a referenced ECSchema that is a duplicate of one already persisted
    //! and therefore doesn't have the persistent ECSchemaId set. Generally, we would prefer that the primary ECSchema had
    //! been deserialized using the persisted copies of the referenced ECSchema, but we cannot ensure that is always the case
    //! @param db must be an ECDb, but left as Db because that is what the callers actually have... needs refactoring for Graphite02
    //! @param ecSchema. The duplicate ECSchema. Its Id will be set (as well as returned)
    static ECN::ECSchemaId GetSchemaIdForECSchemaFromDuplicateECSchema(ECDbCR db, ECN::ECSchemaCR ecSchema);

    void ClearCache () const;
    ECDbCR GetECDb () const;
#endif
    };

typedef ECDbSchemaManager const& ECDbSchemaManagerCR;

END_BENTLEY_SQLITE_EC_NAMESPACE
