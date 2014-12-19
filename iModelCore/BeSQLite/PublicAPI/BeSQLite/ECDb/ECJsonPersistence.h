/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/BeSQLite/ECDb/ECJsonPersistence.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#if !defined (DOCUMENTATION_GENERATOR)

#include <BeSQLite/ECDb/ECDbTypes.h>
#include <BeSQLite/ECDb/ECPersistence.h>
#include <BeSQLite/ECDb/ECInstanceFinder.h> // For ECInstanceKeyMultiMap
#include <json/json.h>

ECDB_TYPEDEFS (ECJsonInserter);

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*=================================================================================**//**
//! @deprecated Use JsonReader, JsonInserter, JsonUpdater, JsonDeleter instead
//! Base class for CRUD operations using JSON as input
* @bsiclass                                                 Ramanujam.Raman      02/2013
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE ECJsonPersistence : NonCopyableClass
{
protected:
    ECDbR m_ecDb;
    ECN::ECClassId m_ecClassId;
    ECN::ECClassP m_ecClass;
    bool m_initialized;
    ECPersistencePtr m_ecPersistence;

protected:
    ECJsonPersistence (ECDbR ecDb, ECN::ECClassId ecClassId);
    virtual StatusInt _Initialize ();

};


/*=================================================================================**//**
//! @deprecated Use JsonInserter instead
//! Insert JSON values into the Db.
* @bsiclass                                                 Ramanujam.Raman      02/2013
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE ECJsonInserter : ECJsonPersistence
{
//__PUBLISH_SECTION_END__
private:
    //! Retrieves the ECInstanceId from an ECInstance
    //! @param[out] ecInstanceId Retrieved ECInstanceId
    //! @param[in] instance ECInstance to retrieve ECInstanceId from
    //! @return true in case of success, false otherwise
    static bool GetECInstanceIdFromECInstance (ECInstanceId& ecInstanceId, ECN::IECInstanceCR instance);
//__PUBLISH_SECTION_START__

public:
    //! Construct an inserter for the specified class. 
    //! @param ecDb [in] ECDb
    //! @param ecClassId [in] ECClassId indicating the class of the instance that needs to be inserted. 
    //! @remarks Holds some cached state to speed up future inserts of the same class. Keep the 
    //! inserter around when inserting many instances of the same class. 
    ECDB_EXPORT ECJsonInserter (ECDbR ecDb, ECN::ECClassId ecClassId);

    //! Inserts the instance and updates the $ECInstanceId field with the new ECInstanceId
    ECDB_EXPORT StatusInt Insert (Json::Value& jsonValue);

    //! Insert an instance created from the specified jsonValue
    //! @param[out] instanceId the ECInstanceId assigned to the new instance
    //! @param[in] jsonValue the instance data
    //! @return SUCCESS if insert is successful
    ECDB_EXPORT StatusInt Insert (ECInstanceId* instanceId, RapidJsonValueCR jsonValue);
};

/*=================================================================================**//**
//! @deprecated Use JsonUpdater instead
//! Update the Db through JSON values
* @bsiclass                                                 Ramanujam.Raman      02/2013
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE ECJsonUpdater : ECJsonPersistence
{
public:
    //! Construct an updater for the specified class. 
    //! @param ecDb [in] ECDb
    //! @param ecClassId [in] ECClassId indicating the class of the instance that needs to be updated
    //! @remarks Holds some cached state to speed up future updates of the same class. Keep the 
    //! inserter around when updating many instances of the same class. 
    ECDB_EXPORT ECJsonUpdater (ECDbR ecDb, ECN::ECClassId ecClassId);

    //! Updates the instance identified by the $ECInstanceId field
    ECDB_EXPORT StatusInt Update (const Json::Value& jsonValue);

    //! Update an instance from the specified jsonValue
    //! @param[in] instanceId the ECInstanceId of the instance to update
    //! @param[in] jsonValue the instance data
    //! @return SUCCESS if update is successful
    ECDB_EXPORT StatusInt Update (const ECInstanceId& instanceId, RapidJsonValueCR jsonValue);
};

/*=================================================================================**//**
//! @deprecated Use JsonDeleter instead
//! Delete rows in the Db
* @bsiclass                                                 Ramanujam.Raman      02/2013
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE ECJsonDeleter : ECJsonPersistence
{
public:
    //! Construct an deleter for the specified class. 
    //! @param ecDb [in] ECDb
    //! @param ecClassId [in] ECClassId indicating the class of the instance that needs to be deleted
    //! @remarks Holds some cached state to speed up future deletes of the same class. Keep the 
    //! deleter around when deleting many instances of the same class. 
    ECDB_EXPORT ECJsonDeleter (ECDbR ecDb, ECN::ECClassId ecClassId);

    //! Deletes the instance identified by the supplied ECInstanceId
    ECDB_EXPORT StatusInt Delete (ECInstanceId ecInstanceId);
};

END_BENTLEY_SQLITE_EC_NAMESPACE

#endif
//__PUBLISH_SECTION_END__

