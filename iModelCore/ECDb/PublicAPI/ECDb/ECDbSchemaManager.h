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

        ECN::ECSchemaCP GetECSchema(ECN::ECSchemaId, bool loadSchemaEntities) const;
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
        //! @param[in] loadSchemaEntities true, if all ECClasses, ECEnumerations, KindOfQuantities in the ECSchema should be pro-actively loaded into memory. false,
        //!                                   if they are loaded on-demand.
        //! @return The retrieved ECSchema or nullptr if not found
        ECDB_EXPORT ECN::ECSchemaCP GetECSchema(Utf8CP schemaName, bool loadSchemaEntities = true) const;

        //! Gets all @ref ECN::ECSchema "ECSchemas" stored in the @ref ECDbFile "ECDb file"
        //! @param[in] loadSchemaEntities true, if all ECClasses, ECEnumerations, KindOfQuantities in the ECSchema should be pro-actively loaded into memory. false,
        //!                                   if they are loaded on-demand.
        //! @return Vector of all ECSchemas stored in the file
        ECDB_EXPORT bvector<ECN::ECSchemaCP> GetECSchemas(bool loadSchemaEntities = true) const;

        //! Gets the ECClass for the specified name.
        //! @param[in] schemaNameOrAlias Name (not full name) or alias of the schema containing the class (@see @p resolveSchema)
        //! @param[in] className Name of the class to be retrieved
        //! @param[in] resolveSchema indicates whether @p schemaNameOrAlias is a schema name or a schema alias
        //! @return The retrieved ECClass or nullptr if not found
        ECDB_EXPORT ECN::ECClassCP GetECClass(Utf8CP schemaNameOrAlias, Utf8CP className, ResolveSchema resolveSchema = ResolveSchema::BySchemaName) const;

        //! Gets the ECClass for the specified ECClassId.
        //! @param[in] ecClassId Id of the ECClass to retrieve
        //! @return The retrieved ECClass or nullptr if not found
        ECDB_EXPORT ECN::ECClassCP GetECClass(ECN::ECClassId ecClassId) const;

        //! Gets the ECClassId for the ECClass with the specified name.
        //! @param[out] id ECClassId of the requested ECClass.
        //! @param[in] schemaNameOrAlias Name (not full name) or alias of the schema containing the class (@see @p resolveSchema)
        //! @param[in] className Name of the class to be retrieved
        //! @param[in] resolveSchema indicates whether @p schemaNameOrAlias is a schema name or a schema alias
        //! @return true, if the %ECDb file has an ECClass with the given name, false, if the ECClass does not exist in the %ECDb file
        ECDB_EXPORT bool TryGetECClassId(ECN::ECClassId& id, Utf8CP schemaNameOrAlias, Utf8CP className, ResolveSchema resolveSchema = ResolveSchema::BySchemaName) const;

        //! Gets the ECClassId for the ECClass with the specified name.
        //! @param[in] schemaNameOrAlias Name (not full name) or namespace alias of the schema containing the class (@see @p resolveSchema)
        //! @param[in] className Name of the class to be retrieved
        //! @param[in] resolveSchema indicates whether @p schemaNameOrAlias is a schema name or a schema alias
        //! @return ECClassId of the requested ECClass. If the ECClass does not exist in the %ECDb file, an invalid class id is returned
        ECN::ECClassId GetECClassId(Utf8CP schemaNameOrAlias, Utf8CP className, ResolveSchema resolveSchema = ResolveSchema::BySchemaName) const { ECN::ECClassId id; TryGetECClassId(id, schemaNameOrAlias, className, resolveSchema); return id; }

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
        ECDB_EXPORT BentleyStatus CreateECClassViewsInDb() const;

#if !defined (DOCUMENTATION_GENERATOR)    
        void ClearCache() const;
        ECDbCR GetECDb() const;
#endif
    };

typedef ECDbSchemaManager const& ECDbSchemaManagerCR;


#if !defined (DOCUMENTATION_GENERATOR)
//=======================================================================================
//! Helper that provides information about ECClass Mappings
//=======================================================================================
struct ClassMappingInfoHelper
    {
private:
    ClassMappingInfoHelper();
    ~ClassMappingInfoHelper();

public:
    ECDB_EXPORT static BentleyStatus GetInfos(Json::Value&, ECDbCR, bool skipUnmappedClasses);
    ECDB_EXPORT static BentleyStatus GetInfos(Json::Value&, ECDbCR, Utf8CP schemaName, bool skipUnmappedClasses);
    ECDB_EXPORT static BentleyStatus GetInfo(Json::Value&, ECDbCR, Utf8CP schemaName, Utf8CP className);
    };

#endif
END_BENTLEY_SQLITE_EC_NAMESPACE
