/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECPersistence.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include <BeSQLite/ECDb/ECInstanceId.h>
#include <ECObjects/ECSchema.h>
#include "ClassMap.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

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
friend struct ECDb::Impl;

private:
    ECDbR                        m_ecdb;
    ECN::ECClassCR               m_ecClass;
    ECN::ECClassId               m_ecClassId;

    InstanceDeleterPtr  m_instanceDeleter;

    explicit ECPersistence (IClassMap const& classMap);

    //! Log the last error in the Db
    static void LogLastSqliteError (BeSQLiteDbR db);

    InstanceDeleterP GetInstanceDeleter();

public:
    //! Gets the ID of the ECClass this ECPersistence object works upon.
    //! @return ID of the ECClass this ECPersistence object works upon.
    ECN::ECClassId   GetClassId () const;

    //! Gets the the ECClass this ECPersistence object works upon.
    //! @return ECClass this ECPersistence object works upon.
    ECN::ECClassCR   GetClass() const;

    //! Deletes the instance with the specified ECInstanceId.
    //! @param[in] ecInstanceId ECInstanceId of instance to delete
    //! @param[in] deleteHandler Handler invoked for every deleted instance. Can be nullptr. 
    //! @return ::DELETE_Success in case of success, error code otherwise
    //! @remarks Any relationships on the instance are also deleted. If these relationships have 
    //! embedding strength, child instances are also deleted. If these relationships have holding 
    //! strength, child instances are deleted only if the children don't have any parents left. 
    DeleteStatus    Delete (ECInstanceId ecInstanceId, ECDbDeleteHandlerP deleteHandler = nullptr);

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

#define BINDING_NotBound -1

//=======================================================================================
//! @deprecated Only used by deprecated ECPersistence
//!
//! Holds binding metadata for mapping ECProperties to sql parameters or selected columns
//! in ECDbStatements
// @bsiclass                                                     Casey.Mullen      11/2012
//=======================================================================================
struct Binding
    {
    //! For looking up the bound/selected property in an IECInstance
    uint32_t              m_propertyIndex;

    //! Either a parameter index or column index, depending on whether the binding is used for parameters or selected columns
    //! May be BINDING_NotBound for unmapped columns
    int                 m_sqlIndex;

    //! The PropertyMap used to map, bind, and get values associated with this binding
    PropertyMapCR       m_propertyMap;

    //! The actual column that is bound
    DbColumnCP          m_column;

    //! Used to indicate which 'components' of a property are applicable
    uint16_t              m_componentMask;

    //! The enabler for which the m_propertyIndex is valid. Otherwise use propertyAccessString
    ECN::ECEnablerCR    m_enabler;

    //! primary constructor
    Binding (ECN::ECEnablerCR enabler, PropertyMapCR propertyMap, uint32_t propertyIndex, uint16_t componentMask, int sqlIndex, DbColumnCP column);

    //! Copy constructor for bvector
    Binding (Binding const& other);

    //! Needed for use in a bvector
    Binding const& operator= (Binding const& other);

    //! Binds the parameters of the statement with corresponding property values from the ecInstance
    static DbResult Bind (BeSQLiteStatementR statement, ECN::IECInstanceR ecInstance, Bindings const& parameterBindings);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
