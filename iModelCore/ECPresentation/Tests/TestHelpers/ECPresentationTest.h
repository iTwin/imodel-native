/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/TestHelpers/ECPresentationTest.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <ECPresentation/ECPresentation.h>
#include <Bentley/BeTest.h>
#include <Bentley/BeTimeUtilities.h>
#include <Logging/bentleylogging.h>

#define BEGIN_ECPRESENTATIONTESTS_NAMESPACE BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE namespace Tests {
#define END_ECPRESENTATIONTESTS_NAMESPACE   } END_BENTLEY_ECPRESENTATION_NAMESPACE
#define USING_NAMESPACE_ECPRESENTATIONTESTS using namespace BentleyApi::ECPresentation::Tests;

BEGIN_ECPRESENTATIONTESTS_NAMESPACE

//===================================================================================
// @bsiclass                                Grigas.Petraitis                03/13
//===================================================================================
struct BeAssertIgnoreContext
{
    static bool s_value;
    bool m_previousValue;
    BeAssertIgnoreContext() : m_previousValue(s_value)
    {
        s_value = false;
        BeTest::SetFailOnAssert(false);
    }
    ~BeAssertIgnoreContext()
    {
        BeTest::SetFailOnAssert(m_previousValue);
        s_value = m_previousValue;
    }
};
#define IGNORE_BE_ASSERT() BeAssertIgnoreContext _ignore_be_asserts;

END_ECPRESENTATIONTESTS_NAMESPACE
