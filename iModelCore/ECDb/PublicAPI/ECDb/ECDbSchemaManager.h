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
    uint32_t m_versionMiddle;
    uint32_t m_versionMinor;
public:
    ECSchemaKey (ECN::ECSchemaId ecSchemaId, Utf8CP name, uint32_t versionMajor, uint32_t versionMiddle, uint32_t versionMinor, Utf8CP displayLabel)
        : m_ecSchemaId (ecSchemaId), m_name (name), m_versionMajor (versionMajor), m_versionMiddle(versionMiddle), m_versionMinor (versionMinor), m_displayLabel (displayLabel) {}

    ECN::ECSchemaId GetECSchemaId () const {BeAssert(m_ecSchemaId != 0ULL); return m_ecSchemaId;}
    uint32_t GetVersionMajor () const {return m_versionMajor;}
    uint32_t GetVersionMiddle() const { return m_versionMiddle; }
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

        public:
            //! Initializes an ImportOptions object with default settings:
            //!     - ImportOptions::DoSupplementation : true
            ECDB_EXPORT ImportOptions();

            //! Initializes an ImportOptions object
            //! @param[in] doSupplementation Normally true. If the list of schemas to be imported contain supplemental ECSchemas, use 
            //!                         them to supplement primary ECSchemas in the list of schemas to be imported.
            //!                         Otherwise, supplemental ECSchemas will be ignored
            ECDB_EXPORT explicit ImportOptions(bool doSupplementation);

            //! Gets a value indicating whether supplementation should be performed or not.
            //! If the list of schemas to be imported contain supplemental ECSchemas, use
            //! them to supplement primary ECSchemas in the list of schemas to be imported.
            //! Otherwise, supplemental ECSchemas will be ignored        
            ECDB_EXPORT bool DoSupplementation() const;
        };

private:
    ECDb& m_ecdb;
    ECDbMap& m_map;
    RefCountedPtr<ECDbSchemaReader> m_ecReader;
    mutable BeMutex m_criticalSection;

    BentleyStatus BatchImportECSchemas (SchemaImportContext const&, ECN::ECSchemaCacheR, ImportOptions const&) const;
    
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
    //!                     All schemas should be read from a single ECN::ECSchemaReadContext. 
    //!                     If any dublicates are found in @p schemaCache an error will returned.
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

    //! Gets the ECEnumeration for the specified name.
    //! @param[in] schemaName Name (not full name) of the schema containing the ECEnumeration
    //! @param[in] enumName Name of the ECEnumeration to be retrieved
    //! @return The retrieved ECEnumeration or nullptr if not found
    ECDB_EXPORT ECN::ECEnumerationCP GetECEnumeration(Utf8CP schemaName, Utf8CP enumName) const;

    //! Creates or updates views in the ECDb file to visualize the EC content as ECClasses and ECProperties rather than tables and columns.
    //! This can help debugging the EC data, especially when ECClasses and ECProperties share tables and columns or are spread across multiple tables.
    //! @note The views are strickly intended for developers for debugging purpose only. They should not be used in application code. 
    //! No code should depend on these views.
    //! @return SUCCESS OR ERROR
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

    void ClearCache () const;
    ECDbCR GetECDb () const;
#endif
    };

typedef ECDbSchemaManager const& ECDbSchemaManagerCR;

END_BENTLEY_SQLITE_EC_NAMESPACE
