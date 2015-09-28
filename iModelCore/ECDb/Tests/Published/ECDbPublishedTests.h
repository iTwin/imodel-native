/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/Published/ECDbPublishedTests.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//this is just a wrapper header that avoids that all published ATPs have to add the below include.

#include "../NonPublished/PublicApi/NonPublished/ECDb/ECDbTestProject.h"
#include "../NonPublished/PublicApi/NonPublished/ECDb/ECDbTestFixture.h"

//exposes unpublished APIs for the published ATPs
#include "../BackDoor/PublicAPI/BackDoor/ECDb/Backdoor.h"
