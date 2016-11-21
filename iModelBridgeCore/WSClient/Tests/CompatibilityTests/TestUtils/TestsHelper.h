/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/CompatibilityTests/TestUtils/TestsHelper.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Client/ClientInfo.h>

#include <prg.h>
static Utf8CP BUILD_VERSION = REL_V "." MAJ_V "." MIN_V "." SUBMIN_V;

USING_NAMESPACE_BENTLEY_WEBSERVICES

ClientInfoPtr StubValidClientInfo();