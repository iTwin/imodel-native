/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BeSQLite/ECDb/ECDbStore.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <BeSQLite/ECDb/ECDbTypes.h>
#include <BeSQLite/ECDb/ECDbSchemaManager.h>
#include <BeSQLite/ECDb/ECPersistence.h>
#include <BeSQLite/ECDb/ECSqlBuilder.h>
//__PUBLISH_SECTION_END__
#include <BeSQLite/ECDb/BeRepositoryBasedIdSequence.h>
//__PUBLISH_SECTION_START__

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

#if !defined (DOCUMENTATION_GENERATOR)
//=======================================================================================
//! @deprecated Use ECInstanceInserter, ECInstanceUpdater, ECInstanceDeleter instead.
//! @ingroup ECDbGroup
// @bsiclass                                               Ramanujam.Raman      06/2012
//+===============+===============+===============+===============+===============+======
enum PersistenceStatus
    {
    PERSISTENCE_Success                 = SUCCESS,
    PERSISTENCE_Error                   = 1,
    PERSISTENCE_ClassIsNotMapped        = 2,
    PERSISTENCE_StructIsNotDomainClass  = 3

    };
#endif 

struct ECDbStore;
typedef ECDbStore const& ECDbStoreCR;
typedef RefCountedPtr<ECDbStore> ECDbStorePtr;

//=======================================================================================
//! Main entry point of the @b %EC API of an @ref ECDbFile "ECDb file".
//! See @ref ECDbOverview for details.
//! @see @ref ECDbOverview, @ref ECDbCodeSamples
//! @ingroup ECDbGroup
// @bsiclass                                                 Ramanujam.Raman      03/2012
//+===============+===============+===============+===============+===============+======
struct ECDbStore : RefCountedBase
{
// The class is ref counted as it can be shared among different ECDb instances

public:
    enum class CacheType
    {
    Schema = 1,
    Sequences = 2,
    All = Schema | Sequences
    };

//__PUBLISH_SECTION_END__

private:
    static Utf8CP const ECINSTANCEIDSEQUENCE_BELOCALKEY;
    static Utf8CP const ECSCHEMAIDSEQUENCE_BELOCALKEY;
    static Utf8CP const ECCLASSIDSEQUENCE_BELOCALKEY;
    static Utf8CP const ECPROPERTYIDSEQUENCE_BELOCALKEY;

    ECDbR  m_ecDb;
    std::unique_ptr<ECDbMap> m_ecDbMap;
    std::unique_ptr<ECDbSchemaManager> m_ecDbSchemaManager;

    mutable BeRepositoryBasedIdSequence m_ecInstanceIdSequence;
    BeRepositoryBasedIdSequence m_ecSchemaIdSequence;
    BeRepositoryBasedIdSequence m_ecClassIdSequence;
    BeRepositoryBasedIdSequence m_ecPropertyIdSequence;

    mutable bmap<IClassMap const*, ECPersistencePtr> m_ecPersistenceCache;

    //! Initializes a new instance of the ECDbStore class.
    //! @param[in] db SQLite DB that needs to be accessed
    explicit ECDbStore (ECDbR db);
    ~ECDbStore();

    //class is non-copyable
    ECDbStore (ECDbStore const&);
    ECDbStore& operator= (ECDbStore const&);

    //! @deprecated Use ECInstanceInserter, ECInstanceUpdater, ECInstanceDeleter instead.
    ECPersistencePtr GetECPersistence (PersistenceStatus* status, IClassMap const& classMap) const;

public:
    //! Creates a new ref-counted instance of the ECDbStore class.
    //! @param[in] db SQLite DB that needs to be accessed
    //! @return created instance
    static ECDbStorePtr Create (ECDbR db);

    DbResult OnDbOpened ();
    DbResult OnDbCreated ();
    //! Called when the repository id of the underlying @ref ECDbFile "ECDb file" has changed.
    //! @param[in] newRepositoryId New repository id
    //! @return BE_SQLITE_OK on success, error code otherwise
    DbResult OnRepositoryIdChanged (BeRepositoryId newRepositoryId);

    //! Closes the ECDbStore
    void Close ();

// constructors are hidden from published API -> make it abstract in the published API
//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    //! Gets the schema manager for this @ref ECDbFile "ECDb file". With the schema manager clients can import @ref ECN::ECSchema "ECSchemas"
    //! into or retrieve @ref ECN::ECSchema "ECSchemas" or individual @ref ECN::ECClass "ECClasses" from the %ECDb file.
    //! @return Schema manager
    ECDB_EXPORT ECDbSchemaManagerCR GetSchemaManager() const;

    //! Gets the schema locator for schemas stored in this ECDb file.
    //! @return This ECDb file's schema locater
    ECDB_EXPORT ECN::IECSchemaLocaterR GetSchemaLocater () const;
    
    //! Gets the ECClass locator for ECClasses whose schemas are stored in this ECDb file.
    //! @return This ECDb file's ECClass locater
    ECDB_EXPORT ECN::IECClassLocaterR GetClassLocater () const;

    //! Clears the ECDb cache or portions of it
    //! @param[in] type Type of cache to be cleared
    //! @return If @type is or includes ECDbSchemaManager::CacheType::Sequences the method
    //! returns ERROR if one of the sequences has been modified in the current transaction and the transaction
    //! hasn't ended yet. SUCCESS otherwise.
    ECDB_EXPORT BentleyStatus ClearCache (CacheType type = CacheType::All) const;

#if !defined (DOCUMENTATION_GENERATOR)
    //! @deprecated Use ECInstanceInserter, ECInstanceUpdater, ECInstanceDeleter instead.
    //! Get an object for performing CRUD operations on EC data which are not yet supported through @ref ECSQLOverview.
    //! @remarks This API is the old way of doing CRUD with EC data in a ECDb file. It will be
    //! deprecated in the future as soon as the ECSQL support of ECDb (see @ref WorkingWithECDb) covers all features
    //! this old API had. So only use this API if you cannot accomplish the same with @ref ECSQLOverview.
    //! @param[out] status Optional error status (pass nullptr if you are not interested in the error status)
    //! @param[in] ecClass ECClass the operation targets
    //! @return An object to perform CRUD operations on EC data.
    ECDB_EXPORT ECPersistencePtr GetECPersistence (PersistenceStatus* status, ECN::ECClassCR ecClass) const;

    //! @deprecated No replacement api avaliable. 
    //! Optimzie db file space by replacing empty table with null view. Should be called if db will be used later for read only purpose.
    //! This funtion should be called before close the db or committing the top level transaction.
    //! @remarks This API will not be required in future version as it would smartly create and drop table as necessary. 
    //! @return BE_SQLITE_OK if successfull.
    ECDB_EXPORT DbResult ReplaceEmptyTablesWithEmptyViews () const;

    //! @deprecated No replacement api avaliable.  
    //! Deoptimize db file space by replace null views with tables. Should be called after ReplaceEmptyTablesWithEmptyViews() in order make EC data writable.
    //! This funtion should be called just after opening db and before any other operations on db. After calling this funtion ecdb.SaveChanges() should be call
    //! to commit top level transaction before making any other operation on the ECDb.
    //! @remarks This API will not be required in future version as it would smartly create and drop table as necessary. 
    //! @return BE_SQLITE_OK if successfull.
    ECDB_EXPORT DbResult ReplaceViewsWithEmptyTables () const;

#endif

//__PUBLISH_SECTION_END__
    
    //! Gets the ECDb backreference
    ECDbR GetECDbR() const;

    //! Gets the ECDbMap
    ECDbMapCR GetECDbMap() const;

    //! Gets the ECInstanceId sequence
    BeRepositoryBasedIdSequenceR GetECInstanceIdSequence ();

    //! Gets the ECSchemaId sequence
    BeRepositoryBasedIdSequenceR GetECSchemaIdSequence ();

    //! Gets the ECClassId sequence
    BeRepositoryBasedIdSequenceR GetECClassIdSequence ();

    //! Gets the ECPropertyId sequence
    BeRepositoryBasedIdSequenceR GetECPropertyIdSequence ();

    //! Gets all Id sequences used by ECDbStore as a list
    void GetIdSequences (bvector<BeRepositoryBasedIdSequenceCP>& idSequences) const;

//__PUBLISH_SECTION_START__
};

END_BENTLEY_SQLITE_EC_NAMESPACE
