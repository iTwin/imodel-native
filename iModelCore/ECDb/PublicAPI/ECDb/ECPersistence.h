/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECDb/ECPersistence.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#if !defined (DOCUMENTATION_GENERATOR)

#include <ECDb/ECInstanceId.h>
#include <ECObjects/ECSchema.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//======================================================================================
//! @deprecated See ECPersistence
//! @ingroup ECDbGroup
// @bsiclass                                                     Affan.Khan      04/2012
//+===============+===============+===============+===============+===============+======
enum InsertStatus
    {
    INSERT_Success                  = SUCCESS, //!< Success
    INSERT_Error                    = 1, //!< Error
    INSERT_FailedToGenerateSql      = 2, //!< Failed to generate SQL for the insert operation
    INSERT_InvalidInputInstance     = 3, //!< ECInstance passed to ECPersistence is invalid
    INSERT_SqliteError              = 4, //!< General Sqlite error
    INSERT_ConstraintViolation      = 5, //!< Sqlite constraint violation error
    INSERT_ECError                  = 6, //!< Error accessing ECInstance
    INSERT_MapError                 = 7, //!< General mapping error
    };

//======================================================================================
//! @deprecated See ECPersistence
//! @ingroup ECDbGroup
// @bsiclass                                                     Affan.Khan      04/2012
//+===============+===============+===============+===============+===============+======
enum UpdateStatus
    {
    UPDATE_Success                  = SUCCESS, //!< Success
    UPDATE_FailedToGenerateSql      = 1, //!< Failed to generate SQL for the update operation
    UPDATE_SqliteError              = 2, //!< General Sqlite error
    UPDATE_MismatchedInputInstance  = 3, //!< Class held by persistence is not the same as instance
    };


//======================================================================================
//! @deprecated See ECPersistence
//! @ingroup ECDbGroup
// @bsiclass                                                     Affan.Khan      04/2012
//+===============+===============+===============+===============+===============+======
enum DeleteStatus
    {
    DELETE_Success                  = SUCCESS, //!< Success
    DELETE_FailedToGenerateSql      = 1, //!< Failed to generate SQL for the delete operation
    DELETE_SqliteError              = 2 //!< General Sqlite error
    };

//+===============+===============+===============+===============+===============+======
//! @deprecated See ECPersistence
// @bsiclass                                                Ramanujam.Raman      09/2013
//+===============+===============+===============+===============+===============+======
struct ECDbDeleteHandler
    {
    virtual void _OnBeforeDelete (ECN::ECClassCR ecClass, ECInstanceId ecInstanceId, ECDbR ecDb) = 0;
    };

typedef ECDbDeleteHandler* ECDbDeleteHandlerP;
typedef ECDbDeleteHandler& ECDbDeleteHandlerR;

//__PUBLISH_SECTION_END__
struct IClassMap;

//__PUBLISH_SECTION_START__

//======================================================================================
//! @deprecated Use ECInstanceInserter, ECInstanceUpdater, or ECInstanceDeleter instead.
//!
//! API to insert or update @ref ECN::IECInstance "ECInstances" into / in an @ref ECDbFile "ECDb file".
//! @remarks This API is the old way of doing CRUD with EC data in an @ref ECDbFile "ECDb file". It will be
//! deprecated in the future as soon as the ECSQL support of ECDb (see @ref WorkingWithECDb) covers all features
//! this old API had. So only use this API if you cannot accomplish the same with @ref ECSQLOverview.
//! @ingroup ECDbGroup
// @bsiclass                                                     Casey.Mullen      12/2011
//+===============+===============+===============+===============+===============+======
struct ECPersistence : RefCountedBase
{
//__PUBLISH_SECTION_END__
friend struct ECDbStore;

private:
    ECDbR                        m_ecdb;
    BeRepositoryBasedIdSequenceR m_ecInstanceIdSequence;
    ECN::ECClassCR               m_ecClass;
    ECN::ECClassId               m_ecClassId;

    InstanceInserterPtr m_instanceInserter;
    InstanceDeleterPtr  m_instanceDeleter;
    InstanceUpdaterPtr  m_instanceUpdater;

    ECPersistence (IClassMap const& classMap, BeRepositoryBasedIdSequenceR ecInstanceIdSequence);

    bool IsValidInput (ECN::IECInstanceR ecInstance) const;

    //! Log the last error in the Db
    static void LogLastSqliteError (BeSQLiteDbR db);

    InstanceDeleterP GetInstanceDeleter();

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    //! Gets the ID of the ECClass this ECPersistence object works upon.
    //! @return ID of the ECClass this ECPersistence object works upon.
    ECDB_EXPORT ECN::ECClassId   GetClassId () const;

    //! Gets the the ECClass this ECPersistence object works upon.
    //! @return ECClass this ECPersistence object works upon.
    ECDB_EXPORT ECN::ECClassCR   GetClass() const;

    //! Inserts an instance into the @ref ECDbFile "ECDb file".
    //! @param[out] ecInstanceId ECInstanceId assigned to the newly imported instance. Can be nullptr
    //! @param[in]  instance     Instance to import
    //! @return ::INSERT_Success in case of success, error code otherwise
    ECDB_EXPORT InsertStatus    Insert (ECInstanceId* ecInstanceId, ECN::IECInstanceR instance);

    //! Updates the specified instance in the @ref ECDbFile "ECDb file".
    //! @note This method deletes the given instance first and then re-inserts it.
    //! @param[in]  instance     Instance to update
    //! @return ::UPDATE_Success in case of success, error code otherwise
    ECDB_EXPORT UpdateStatus    Update (ECN::IECInstanceR instance);

    //! Deletes the instance with the specified ECInstanceId.
    //! @param[in] ecInstanceId ECInstanceId of instance to delete
    //! @param[in] deleteHandler Handler invoked for every deleted instance. Can be nullptr. 
    //! @return ::DELETE_Success in case of success, error code otherwise
    //! @remarks Any relationships on the instance are also deleted. If these relationships have 
    //! embedding strength, child instances are also deleted. If these relationships have holding 
    //! strength, child instances are deleted only if the children don't have any parents left. 
    ECDB_EXPORT DeleteStatus    Delete (ECInstanceId ecInstanceId, ECDbDeleteHandlerP deleteHandler = nullptr);

    //! Deletes the instances which meet the specified SQLite SQL where criterion
    //! @param[out] nDeleted Number of instances deleted by this operation.
    //! @param[in] ecInstanceIdSet Set of instance ids to be deleted. 
    //! @param[in] deleteHandler Handler invoked for every deleted instance. Can be nullptr. 
    //! @return ::DELETE_Success in case of success, error code otherwise
    //! @remarks Any relationships on the instance are also deleted. If these relationships have 
    //! embedding strength, child instances are also deleted. If these relationships have holding 
    //! strength, child instances are deleted only if the children don't have any parents left. 
    DeleteStatus    Delete (int32_t* nDeleted, const ECInstanceIdSet& ecInstanceIdSet, ECDbDeleteHandlerP deleteHandler = nullptr);
    };

typedef RefCountedPtr<ECPersistence> ECPersistencePtr;
#endif

END_BENTLEY_SQLITE_EC_NAMESPACE
