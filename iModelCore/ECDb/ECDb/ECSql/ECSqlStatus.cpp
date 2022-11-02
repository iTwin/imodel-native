/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//******************* ECSqlStatus ************************************
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
//static
const ECSqlStatus ECSqlStatus::Success;
//static
const ECSqlStatus ECSqlStatus::Error = ECSqlStatus(Status::Error);
//static
const ECSqlStatus ECSqlStatus::InvalidECSql = ECSqlStatus(Status::InvalidECSql);


END_BENTLEY_SQLITE_EC_NAMESPACE
