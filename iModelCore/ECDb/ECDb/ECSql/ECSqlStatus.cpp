/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlStatus.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//******************* ECSqlStatus ************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle   06/2013
//---------------------------------------------------------------------------------------
//static
const ECSqlStatus ECSqlStatus::Success;
//static
const ECSqlStatus ECSqlStatus::Error = ECSqlStatus(Status::Error);
//static
const ECSqlStatus ECSqlStatus::InvalidECSql = ECSqlStatus(Status::InvalidECSql);


END_BENTLEY_SQLITE_EC_NAMESPACE
