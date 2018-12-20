/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/ProfileValidationTestCase.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ProfilesTestCase.h"

#define EXPECT_SUCCESS_Insert(createParams) EXPECT_EQ (DgnDbStatus::Success, InsertProfile (createParams))
#define EXPECT_FAIL_Insert(createParams) EXPECT_EQ (DgnDbStatus::ValidationFailed, InsertProfile (createParams))

/*---------------------------------------------------------------------------------**//**
* Test case for Profiles domain entity class validation tests.
* @param T - concrete Profiles entity class e.g. IShapeProfile.
* @bsiclass                                                                      12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
struct ProfileValidationTestCase : ProfilesTestCase
    {
public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                                     12/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    RefCountedPtr<T> CreateProfile (Profiles::Profile::CreateParams const& createParams)
        {
        typename T::CreateParams const* pParams = dynamic_cast<typename T::CreateParams const*> (&createParams);
        if (pParams == nullptr)
            BeAssert (false && "CreateParams must be of the class that is being created");

        return T::Create (*pParams);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                                     12/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    Dgn::DgnDbStatus InsertProfile (Profiles::Profile::CreateParams const& createParams)
        {
        Profiles::ProfilePtr profilePtr = CreateProfile (createParams);
        BeAssert (profilePtr.IsValid());

        Dgn::DgnDbStatus status;
        profilePtr->Insert (&status);
        if (status != Dgn::DgnDbStatus::Success)
            return status;

        // Perform an Update just to double check same validation is happenning on update.
        profilePtr->Update (&status);
        return status;
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                                     12/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void TestParameterToBeFiniteAndPositive (Profiles::Profile::CreateParams const& createParams, double& parameterToCheck,
                                             Utf8CP pParameterName, bool allowEqualToZero)
        {
        parameterToCheck = -1.0;
        EXPECT_FAIL_Insert (createParams) << pParameterName << " should be non negative.";

        parameterToCheck = 0;
        if (allowEqualToZero)
            EXPECT_SUCCESS_Insert (createParams) << pParameterName << " should be greater or equal to zero.";
        else
            EXPECT_FAIL_Insert (createParams) << pParameterName << " should be greater than zero.";

        parameterToCheck = std::numeric_limits<double>::signaling_NaN();
        EXPECT_FAIL_Insert (createParams) << pParameterName << " cannot be NaN.";

        parameterToCheck = std::numeric_limits<double>::infinity();
        EXPECT_FAIL_Insert (createParams) << pParameterName << " cannot be infinity.";

        parameterToCheck = std::numeric_limits<double>::infinity() * -1;
        EXPECT_FAIL_Insert (createParams) << pParameterName << " cannot be infinity.";
        }
    };