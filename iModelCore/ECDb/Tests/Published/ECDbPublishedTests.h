/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/ECDB/Published/ECDbPublishedTests.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//this is just a wrapper header that avoids that all published ATPs have to add the below include.

#include <UnitTests/NonPublished/ECDb/ECDbTestProject.h>

//exposes unpublished APIs for the published ATPs
#include <UnitTests/BackDoor/ECDb/Backdoor.h>
