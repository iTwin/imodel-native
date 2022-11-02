/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

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
    //! Query for the key of the source instance give a relationship name and a target key.
    //! @param[in] db the database to query
    //! @param[in] relationshipName the fully-qualified ECSql relationship class name
    //! @param[in] targetKey the key of the target instance
    static BeSQLite::EC::ECInstanceKey QueryRelationshipSourceFromTarget (BeSQLite::EC::ECDbR db, Utf8CP relationshipName, BeSQLite::EC::ECInstanceKeyCR targetKey);
};

END_DGNDB_UNIT_TESTS_NAMESPACE
