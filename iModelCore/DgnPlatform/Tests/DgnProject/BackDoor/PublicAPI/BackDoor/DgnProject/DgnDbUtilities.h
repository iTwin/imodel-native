/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/BackDoor/PublicAPI/BackDoor/DgnProject/DgnDbUtilities.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__

#include "BackDoor.h"
#include <ECDb/ECDbTypes.h>
#include <ECDb/ECDbApi.h>
#include <ECObjects/ECSchema.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnDb.h>

BEGIN_DGNDB_UNIT_TESTS_NAMESPACE

//=======================================================================================
//! DgnDb utilities
//=======================================================================================
struct DgnDbUtilities : NonCopyableClass
{
public:
    //! Insert a relationship (by name) using keys to identify the source and target instances
    //! @param[out] rkey The ID of the relationship instance that was created, if successful
    //! @param[in,out] stmt The statement that is used to perform the insert. Reuse the same statement in order to cache the costly preparation.
    //! @param[in] db the database to query
    //! @param[in] relationshipName the fully-qualified ECSql relationship class name
    //! @param[in] sourceKey the key of the source instance
    //! @param[in] targetKey the key of the target instance
    static BeSQLite::EC::ECSqlStatus InsertRelationship (BeSQLite::EC::ECInstanceKey& rkey, BeSQLite::EC::ECSqlStatement& stmt, BeSQLite::EC::ECDbR db, Utf8CP relationshipName, BeSQLite::EC::ECInstanceKeyCR sourceKey, BeSQLite::EC::ECInstanceKeyCR targetKey);
    
    //! Query for the key of the source instance give a relationship name and a target key.
    //! @param[in] db the database to query
    //! @param[in] relationshipName the fully-qualified ECSql relationship class name
    //! @param[in] targetKey the key of the target instance
    static BeSQLite::EC::ECInstanceKey QueryRelationshipSourceFromTarget (BeSQLite::EC::ECDbR db, Utf8CP relationshipName, BeSQLite::EC::ECInstanceKeyCR targetKey);
};

END_DGNDB_UNIT_TESTS_NAMESPACE
