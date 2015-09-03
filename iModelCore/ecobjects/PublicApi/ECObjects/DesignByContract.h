/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/DesignByContract.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <assert.h>
#include <stdarg.h>
/*__PUBLISH_SECTION_START__*/

#ifndef DesignByContract_H_
#define DesignByContract_H_

/// @cond BENTLEY_SDK_Internal
#include <ECObjects/ECObjects.h>

//! This class is utilzed by the macros defined in this header file.  No calling code should typically ever need to use this class directly.
struct AssertDisabler
{
/*__PUBLISH_SECTION_END__*/
private:
    static int s_globalIgnoreCount;

public:
/*__PUBLISH_SECTION_START__*/
    ECOBJECTS_EXPORT static bool AreAssertsDisabled (void);
    ECOBJECTS_EXPORT AssertDisabler(void);
    ECOBJECTS_EXPORT ~AssertDisabler(void);
};

    //! Utilize this macro to disable asserts that may occur within a codeblock.
    //! The intent is that this macro will only ever be used by ATPs when testing failure scenarios.  No delivered code should ever utilize this macro.
    //! This macro can only be used once within a codeblock and will disable any assert that may occur within the context of that codeblock.
    //! Usage within Nested codeblocks and method calls are valid.
    //! Let's assume the following method exists in a library
    //! \code
    //! StatusInt SetHour (int hour)
    //!      {
    //!      PRECONDITION (0 <= hour && hour <= 23, ERROR);
    //!      m_hour = hour;
    //!      return SUCCESS;
    //!      }
    //! \endcode
    //! Aside from logging and returning the specified ERROR, the PRECONDITION macro will assert in debug builds when argument <= 0.  Now let's assume you wanted
    //! to write a test case that validated this expected failure condition.  You would encounter problems in a debug build because the assertion dialog would display and
    //! interrupt a test that was designed to target this failure.  You can DISABLE_ASSERTS in the context of executing this method from your test in order to prevent such
    //! scenario from occuring.
    //! \code
    //!   void TestFailureAndSuccessScenarioInAFunctionThatWillAssert
    //!       {
    //!         {
    //!         DISABLE_ASSERTS
    //!         EXPECT_EQ (ERROR, ExpectArgumentGreaterThenZero (-1));
    //!         }
    //!       EXPECT_EQ (SUCCESS, ExpectArgumentGreaterThenZero (-1));
    //!       };
    //! \endcode
    #define DISABLE_ASSERTS           AssertDisabler assertDisabler;

/*__PUBLISH_SECTION_END__*/

#ifdef  NDEBUG
    #define ASSERT_FALSE_IF_NOT_DISABLED(_Message) (void)0
    #define DATA_ASSERT_FALSE_IF_NOT_DISABLED(_Message) (void)0
#else
    //! Avoid direct use of this macro.  It is only intended for use by other macros defined in this file.
    // Forces an assert with the specified message as long as asserts are enabled.  No expression is evaluated.
    #define ASSERT_FALSE_IF_NOT_DISABLED(_Message)    (void)((AssertDisabler::AreAssertsDisabled()) || (BeAssert(_Message), 0))
    #define DATA_ASSERT_FALSE_IF_NOT_DISABLED(_Message)    (void)((AssertDisabler::AreAssertsDisabled()) || (BeDataAssert(_Message), 0))
#endif

#if defined(NDEBUG) && !defined (LOG_ASSERT_IN_PRODUCTION_CODE)
    #define LOG_ASSERT_FAILURE(_LogMessage, ...) (void)0
#else
    #define LOG_ASSERT_FAILURE(_LogMessage, ...) LogFailureMessage(_LogMessage, ## __VA_ARGS__)
#endif

//! Avoid direct use of this function.  It is only intended for use by macros defined in this file.
ECOBJECTS_EXPORT void LogFailureMessage (WCharCP message, ...);

//! Avoid direct use of this macro.  It is only intended for use by other macros defined in this file.
#define LOG_ASSERT_RETURN(_Expression, _ErrorStatus, _LogMessage, ...)           \
        {                                                           \
        LOG_ASSERT_FAILURE (_LogMessage, ## __VA_ARGS__);           \
        ASSERT_FALSE_IF_NOT_DISABLED(_Expression);                  \
        return _ErrorStatus;                                        \
        }

//! Avoid direct use of this macro.  It is only intended for use by other macros defined in this file.
#define EXPECT_CONDITION_LOG_ASSERT_RETURN(_Expression, _ErrorStatus, _LogMessage, ...)       \
    {\
    if (!(_Expression))                                                         \
        {                                                                       \
        LOG_ASSERT_RETURN(_Expression, _ErrorStatus, _LogMessage, ## __VA_ARGS__)            \
        }\
    }

//! This macro should be utilized in published API methods to enforce any restrictions on the parameters of the method and/or data members as a way to
//! ensure the method is to behave correctly, PRIOR to running the code in the method.  It is in accordance with the Bertrand Meyer "Design By Contract" methodology.
//! If _Expression does not evaluate to true the macro will log the details of the precondition violation, assert and return the specified error code.
//! The assertion will only occur in debug builds.  Further, the assertion will only occur as long as they have not been disabled using the DISABLE_ASSERTS macros which allows
//! for creation of tests to validate failure cases without being aborted by the standard assert behavior.
//! Example:
//! \code
//! StatusInt SetHour (int hour)
//!      {
//!      PRECONDITION (0 <= hour && hour <= 23, ERROR);
//!      m_hour = hour;
//!      return SUCCESS;
//!      }
//! \endcode
#define PRECONDITION(_Expression, _ErrorStatus)             \
        EXPECT_CONDITION_LOG_ASSERT_RETURN(_Expression, _ErrorStatus, \
        L"Precondition failed: %ls\n  method: %ls\n  file: %ls\n  line: %i\n", \
        WString(#_Expression, BentleyCharEncoding::Utf8).c_str(), WString(__FUNCTION__, BentleyCharEncoding::Utf8).c_str(), WString(__FILE__, BentleyCharEncoding::Utf8).c_str(), __LINE__)


//! This macro should be utilized in published API methods to enforce the post-conditions.  The post-conditions of a method are a series of assertions near the
//! end of the method that check that the method actually did what it said it would do. It's a double-check on the method's implementation.
//! A DGN specific example would be: the level cache defines a method to remove a level. That method should ensure that the level is really gone before it returns. If the
//! contract calls for the method to NULL out the input level, then the post-condition should check that, too.  If that sounds like a lot of redundancy, it is!
//! Post-conditions catch you when you make a change and introduce a bug.  It is in accordance with the Bertrand Meyer "Design By Contract" methodology.
//! If _Expression does not evaluate to true the macro will log the details of the postcondition violation, assert and return the specified error code.
//! The assertion will only occur in debug builds.  Further, the assertion will only occur as long as they have not been disabled using the DISABLE_ASSERTS macros which allows
//! for creation of tests to validate failure cases without being aborted by the standard assert behavior.
//! Example:
//! \code
//! StatusInt IncrementClockByNumberOfHour (int numberOfHours)
//!      {
//!      // code to increment hour
//!      // .....
//!
//!      POSTCONDITION (0 <= m_hour && m_hour <= 23, ERROR);
//!      return SUCCESS;
//!      }
//! \endcode
#define POSTCONDITION(_Expression, _ErrorStatus)            \
        EXPECT_CONDITION_LOG_ASSERT_RETURN(_Expression, _ErrorStatus, \
            L"Postcondition failed: %ls\n  method: %ls\n  file: %ls\n  line: %i\n", \
            WString(#_Expression, BentleyCharEncoding::Utf8).c_str(), WString(__FUNCTION__, BentleyCharEncoding::Utf8).c_str(), WString(__FILE__, BentleyCharEncoding::Utf8).c_str(), __LINE__)


//! This macro should be utilized to check that an expected condition is true.  If the condition evaluates to false the macro will log and BeAssert leaving it to the caller
//! to return an error code or take any additional action.
//! The assertion will only occur in debug builds.  Further, the assertion will only occur as long as they have not been disabled using the DISABLE_ASSERTS macro which allows
//! for creation of tests to validate failure cases without being aborted by the standard assert behavior.
//! Example:
//! \code
//!     If (!EXPECTED_CONDITION (x < 3))
//!         {
//!         // free something I malloced
//!         // set outputs to 0
//!         return MyStatus::ERROR_ItFailed;
//!         }
//! \endcode
#define EXPECTED_CONDITION(_Expression)     ( (_Expression) \
    || (LOG_ASSERT_FAILURE(L"Expected condition failed: %ls\n  method: %ls\n  file: %ls\n  line: %i\n", WString(#_Expression, BentleyCharEncoding::Utf8).c_str(), WString(__FUNCTION__, BentleyCharEncoding::Utf8).c_str(), WString(__FILE__, BentleyCharEncoding::Utf8).c_str(), __LINE__), 0) \
    || (ASSERT_FALSE_IF_NOT_DISABLED (_Expression), 0) )

//! An inverted form of EXPECTED_CONDITION if you prefer checking for the positive of an expression when writing your code.
//! @see EXPECTED_CONDITION
//! @code
//!     if (UNEXPECTED_CONDITION(x >= 3))
//!         {
//!         // free something I malloced
//!         // set outputs to 0
//!         return MyStatus::ERROR_ItFailed;
//!         }
//! @endcode
#define UNEXPECTED_CONDITION(_Expression) !EXPECTED_CONDITION(!(_Expression))

//! This macro should be utilized to check that an expected data condition is true.  If the data condition evaluates to false the macro will log and BeAssert leaving it to the caller
//! to return an error code or take any additional action.
//! The assertion will only occur in debug builds.  Further, the assertion will only occur as long as they have not been disabled using the DISABLE_ASSERTS macro which allows
//! for creation of tests to validate failure cases without being aborted by the standard assert behavior.
//! Example:
//! \code
//!     If (!EXPECTED_DATA_CONDITION (x < 3))
//!         {
//!         // free something I malloced
//!         // set outputs to 0
//!         return MyStatus::ERROR_ItFailed;
//!         }
//! \endcode
#define EXPECTED_DATA_CONDITION(_Expression)     ( (_Expression) \
    || (LOG_ASSERT_FAILURE(L"Expected condition failed: %ls\n  method: %ls\n  file: %ls\n  line: %i\n", WString(#_Expression, BentleyCharEncoding::Utf8).c_str(), WString(__FUNCTION__, BentleyCharEncoding::Utf8).c_str(), WString(__FILE__, BentleyCharEncoding::Utf8).c_str(), __LINE__), 0) \
    || (DATA_ASSERT_FALSE_IF_NOT_DISABLED (_Expression), 0) )

//! An inverted form of EXPECTED_DATA_CONDITION if you prefer checking for the positive of an expression when writing your code.
//! @see EXPECTED_DATA_CONDITION
//! @code
//!     if (UNEXPECTED_DATA_CONDITION(x >= 3))
//!         {
//!         // free something I malloced
//!         // set outputs to 0
//!         return MyStatus::ERROR_ItFailed;
//!         }
//! @endcode
#define UNEXPECTED_DATA_CONDITION(_Expression) !EXPECTED_DATA_CONDITION(!(_Expression))

#ifdef NDEBUG
    #define DEBUG_EXPECT(_Expression) (void)0
    #define DEBUG_FAIL(_Message) (void)0
#else
    #define DEBUG_EXPECT(_Expression)    EXPECTED_CONDITION(_Expression)
    #define DEBUG_FAIL(_Message)         EXPECTED_CONDITION(false && _Message)
#endif

/*__PUBLISH_SECTION_START__*/

/// @endcond BENTLEY_SDK_Internal

#endif // DesignByContract_H_
