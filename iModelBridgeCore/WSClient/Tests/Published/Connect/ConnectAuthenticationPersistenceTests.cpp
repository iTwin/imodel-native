/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/Connect/ConnectAuthenticationPersistenceTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ConnectAuthenticationPersistenceTests.h"

#include <WebServices/Connect/ConnectAuthenticationPersistence.h>
#include "StubLocalState.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES
using namespace ::testing;

TEST_F (ConnectAuthenticationPersistenceTests, GetCredentials_SetCredentialsCalledOnOtherPersistenceWithSameLocalState_ReturnsSameCredentials)
    {
    StubLocalState localState;
    ConnectAuthenticationPersistence p1 (&localState);
    ConnectAuthenticationPersistence p2 (&localState);

    p1.SetCredentials ({"A", "B"});
    EXPECT_EQ (Credentials ("A", "B"), p2.GetCredentials ());

    p1.SetCredentials ({"C", "D"});
    EXPECT_EQ (Credentials ("C", "D"), p2.GetCredentials ());
    }
