/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/Published/Tasks/AsyncResultTests.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "AsyncResultTests.h"

#include <Bentley/Tasks/AsyncResult.h>

TEST_F (AsyncResultTests, Ctor_ErrorTypeOnly_IsSuccessFalse)
    {
    AsyncResult<void, Utf8String> result;
    EXPECT_FALSE (result.IsSuccess ());
    EXPECT_EQ ("", result.GetError ());
    }

TEST_F (AsyncResultTests, Ctor_ValueAndErrorTypes_IsSuccessFalse)
    {
    AsyncResult<Utf8String, Utf8String> result;
    EXPECT_FALSE (result.IsSuccess ());
    EXPECT_EQ ("", result.GetValue ());
    EXPECT_EQ ("", result.GetError ());
    }

TEST_F (AsyncResultTests, Error_VoidValueAndParameterAsyncResultWithDifferentValue_SameErrorPassed)
    {
    auto result1 = AsyncResult<Utf8String, Utf8String>::Error ("Error");
    auto result2 = AsyncResult<void, Utf8String>::Error (result1);

    EXPECT_FALSE (result2.IsSuccess ());
    EXPECT_EQ (&result1.GetError (), &result2.GetError ());
    }

TEST_F (AsyncResultTests, Error_ParameterAsyncResultWithDifferentValue_SameErrorPassed)
    {
    auto result1 = AsyncResult<int, Utf8String>::Error ("Error");
    auto result2 = AsyncResult<Utf8String, Utf8String>::Error (result1);

    EXPECT_FALSE (result2.IsSuccess ());
    EXPECT_EQ (&result1.GetError (), &result2.GetError ());
    }

TEST_F (AsyncResultTests, Error_ParameterAsyncResultWithVoidValue_SameErrorPassed)
    {
    auto result1 = AsyncResult<void, Utf8String>::Error ("Error");
    auto result2 = AsyncResult<Utf8String, Utf8String>::Error (result1);

    EXPECT_FALSE (result2.IsSuccess ());
    EXPECT_EQ (&result1.GetError (), &result2.GetError ());
    }

TEST_F (AsyncResultTests, IsSuccess_ConstructedWithSuccess_True)
    {
    auto result = AsyncResult<int, Utf8String>::Success (0);
    EXPECT_TRUE (result.IsSuccess ());
    }

TEST_F (AsyncResultTests, IsSuccess_ConstructedWithError_False)
    {
    auto result = AsyncResult<int, Utf8String>::Error ("Foo");
    EXPECT_FALSE (result.IsSuccess ());
    }

TEST_F (AsyncResultTests, GetValue_ConstructedWithSuccess_ReturnsSuccessValue)
    {
    auto result = AsyncResult<int, Utf8String>::Success (42);
    EXPECT_EQ (42, result.GetValue ());
    }

TEST_F (AsyncResultTests, GetValue_ConstructedWithError_ReturnsDefault)
    {
    auto result = AsyncResult<Utf8String, Utf8String>::Error ("Error");
    EXPECT_EQ ("", result.GetValue ());
    }
    
TEST_F (AsyncResultTests, GetValue_ConstructedWithErrorWithValue_ReturnsValue)
    {
    auto result = AsyncResult<Utf8String, Utf8String>::Error ("Error", "ErrorResultValue");
    EXPECT_EQ ("ErrorResultValue", result.GetValue ());
    }

TEST_F (AsyncResultTests, GetError_ConstructedWithSuccess_ReturnsDefault)
    {
    auto result = AsyncResult<int, Utf8String>::Success (42);
    EXPECT_EQ ("", result.GetError ());
    }

TEST_F (AsyncResultTests, GetError_ConstructedWithError_ReturnsError)
    {
    auto result = AsyncResult<Utf8String, Utf8String>::Error ("FatalError");
    EXPECT_EQ ("FatalError", result.GetError ());
    }

TEST_F (AsyncResultTests, SetSuccess_ConstructedWithError_MakesSuccessResult)
    {
    auto result = AsyncResult<int, Utf8String>::Error ("Foo");
    result.SetSuccess (42);
    EXPECT_TRUE (result.IsSuccess ());
    EXPECT_EQ (42, result.GetValue ());
    EXPECT_EQ ("", result.GetError ());
    }
    
TEST_F (AsyncResultTests, SetError_ConstructedWithSuccess_MakesErrorResult)
    {
    auto result = AsyncResult<Utf8String, Utf8String>::Success ("Success");
    result.SetError ("Boo");
    EXPECT_FALSE (result.IsSuccess ());
    EXPECT_EQ ("", result.GetValue ());
    EXPECT_EQ ("Boo", result.GetError ());
    }

TEST_F (AsyncResultTests, VoidValueIsSuccess_ConstructedWithSuccess_True)
    {
    auto result = AsyncResult<void, Utf8String>::Success ();
    EXPECT_TRUE (result.IsSuccess ());
    }

TEST_F (AsyncResultTests, VoidValueIsSuccess_ConstructedWithError_False)
    {
    auto result = AsyncResult<void, Utf8String>::Error ("Foo");
    EXPECT_FALSE (result.IsSuccess ());
    }

TEST_F (AsyncResultTests, VoidValueGetError_ConstructedWithSuccess_ReturnsDefault)
    {
    auto result = AsyncResult<void, Utf8String>::Success ();
    EXPECT_EQ ("", result.GetError ());
    }

TEST_F (AsyncResultTests, VoidValueGetError_ConstructedWithError_ReturnsError)
    {
    auto result = AsyncResult<Utf8String, Utf8String>::Error ("FatalError");
    EXPECT_EQ ("FatalError", result.GetError ());
    }

TEST_F (AsyncResultTests, VoidValueSetSuccess_ConstructedWithError_MakesSuccessResult)
    {
    auto result = AsyncResult<void, Utf8String>::Error ("Foo");
    result.SetSuccess ();
    EXPECT_TRUE (result.IsSuccess ());
    EXPECT_EQ ("", result.GetError ());
    }

TEST_F (AsyncResultTests, VoidValueSetError_ConstructedWithSuccess_MakesErrorResult)
    {
    auto result = AsyncResult<void, Utf8String>::Success ();
    result.SetError ("Boo");
    EXPECT_FALSE (result.IsSuccess ());
    EXPECT_EQ ("Boo", result.GetError ());
    }
