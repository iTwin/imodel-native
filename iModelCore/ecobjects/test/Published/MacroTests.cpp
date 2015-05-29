
/*--------------------------------------------------------------------------------------+
|
|     $Source: test/Published/MacroTests.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../ECObjectsTestPCH.h"
#include "../TestFixture/TestFixture.h"
using namespace BentleyApi::ECN;

BEGIN_BENTLEY_ECN_TEST_NAMESPACE

void ExplicitlyDisableAssertsWithoutReenabling(bool assertsEnabledWhenInvoked)
    {
    EXPECT_EQ (assertsEnabledWhenInvoked, !AssertDisabler::AreAssertsDisabled());

    DISABLE_ASSERTS
    EXPECT_FALSE (!AssertDisabler::AreAssertsDisabled());
        {
        DISABLE_ASSERTS
        EXPECT_FALSE (!AssertDisabler::AreAssertsDisabled());
            {
            DISABLE_ASSERTS
            EXPECT_FALSE (!AssertDisabler::AreAssertsDisabled());
                {
                DISABLE_ASSERTS
                EXPECT_FALSE (!AssertDisabler::AreAssertsDisabled());
                }
            }
        }
    EXPECT_FALSE (!AssertDisabler::AreAssertsDisabled());
    }

void ExplicitlyDisableAssertsAndThrowException(bool assertsEnabledWhenInvoked)
    {
    EXPECT_EQ (assertsEnabledWhenInvoked, !AssertDisabler::AreAssertsDisabled());
    DISABLE_ASSERTS
    EXPECT_FALSE (!AssertDisabler::AreAssertsDisabled());
        {   
        DISABLE_ASSERTS
        EXPECT_FALSE (!AssertDisabler::AreAssertsDisabled());
            {
            DISABLE_ASSERTS
            EXPECT_FALSE (!AssertDisabler::AreAssertsDisabled());
            }
        }
    throw 0;
    }

TEST(AssertTest, VerifyAssertGuardScenarios)
    {
    EXPECT_TRUE (!AssertDisabler::AreAssertsDisabled());    

    {
    DISABLE_ASSERTS
    EXPECT_FALSE (!AssertDisabler::AreAssertsDisabled());
    }    
    EXPECT_TRUE (!AssertDisabler::AreAssertsDisabled());

    {
    DISABLE_ASSERTS
        {
        DISABLE_ASSERTS
        EXPECT_FALSE (!AssertDisabler::AreAssertsDisabled());
        }    
    EXPECT_FALSE (!AssertDisabler::AreAssertsDisabled());
    }
    EXPECT_TRUE (!AssertDisabler::AreAssertsDisabled());

    // since we enter this method with asserts enabled we expect on return that asserts will be enabled
    ExplicitlyDisableAssertsWithoutReenabling(true);    
    EXPECT_TRUE (!AssertDisabler::AreAssertsDisabled());

    {
    DISABLE_ASSERTS
        {
        DISABLE_ASSERTS
        // since we enter this method with asserts disabled twice we expect on return that asserts will be disabled and need to be enabled twice to really re-enable them
        ExplicitlyDisableAssertsWithoutReenabling(false);    
        EXPECT_FALSE (!AssertDisabler::AreAssertsDisabled());
        }    
    EXPECT_FALSE (!AssertDisabler::AreAssertsDisabled());
    }    
    EXPECT_TRUE (!AssertDisabler::AreAssertsDisabled());

    try
        {
        ExplicitlyDisableAssertsAndThrowException (true);
// ExplicitlyDisableAssertsAndThrowException currently always throws and we're not testing the compiler.
//  Perhaps it should've done an assert to verify that it 
//        FAIL() << "ExplicitlyDisableAssertsAndThrowException should have thrown an exception";
        }
    catch (...) {}
    EXPECT_TRUE (!AssertDisabler::AreAssertsDisabled());

    {
    DISABLE_ASSERTS
        {
        DISABLE_ASSERTS
        try
            {
            ExplicitlyDisableAssertsAndThrowException (false);
// ExplicitlyDisableAssertsAndThrowException currently always throws and we're not testing the compiler.
//  Perhaps it should've done an assert to verify that it 
//            FAIL() << "ExplicitlyDisableAssertsAndThrowException should have thrown an exception";
            }
        catch (...) {}
        EXPECT_FALSE (!AssertDisabler::AreAssertsDisabled());
        }       
    EXPECT_FALSE (!AssertDisabler::AreAssertsDisabled());
    }    
    EXPECT_TRUE (!AssertDisabler::AreAssertsDisabled());
    }

/********* PRECONDITION checks **********/

int UtilizePreConditionMacro (int number, void* pointer)
    {
    PRECONDITION (number > 45, -1);
    PRECONDITION (NULL != pointer, -2);
    return 0;
    }

TEST(AssertTest, ExpectErrorForPreConditionCheckWhenNumberIsOutOfRange)
    {    
    DISABLE_ASSERTS
    EXPECT_EQ (-1, UtilizePreConditionMacro (5, (void*)0xBADF));
    }

TEST(AssertTest, ExpectErrorForPreConditionCheckWhenPointerIsNull)
    {    
    DISABLE_ASSERTS
    EXPECT_EQ (-2, UtilizePreConditionMacro (50, NULL));
    }

TEST(AssertTest, ExpectSuccessForPreConditionCheck)
    {    
    EXPECT_EQ (0, UtilizePreConditionMacro (50, (void*)0xBADF));
    }

/********* POSTCONDITION checks **********/

int UtilizePostConditionMacroWithError (int number)
    {
    int x = number * 2;
    x -= 1;
    POSTCONDITION (x == number + number, -1);
    return 0;
    }

TEST(AssertTest, ExpectErrorForPostConditionCheck)
    {    
    DISABLE_ASSERTS
    EXPECT_EQ (-1, UtilizePostConditionMacroWithError (5));
    }

int UtilizePostConditionMacroWithSuccess (int number)
    {
    int x = number * 2;
    POSTCONDITION (x = number + number, -1);
    return 0;
    }

TEST(AssertTest, ExpectSuccessForPostConditionCheck)
    {    
    EXPECT_EQ (0, UtilizePostConditionMacroWithSuccess (5));
    }


/********* EXPECTED_CONDITION check **********/

TEST(AssertTest, ExpectFalseForExpectedConditionCheck)
    {    
    DISABLE_ASSERTS
    int n = 5;
    EXPECT_FALSE (EXPECTED_CONDITION(n==10));
    }

TEST(AssertTest, ExpectTrueForExpectedConditionCheck)
    {    
    int n = 10;
    EXPECT_TRUE (EXPECTED_CONDITION(n==10));
    }


END_BENTLEY_ECN_TEST_NAMESPACE