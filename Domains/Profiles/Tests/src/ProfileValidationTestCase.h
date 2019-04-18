/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ProfilesTestCase.h"

/*---------------------------------------------------------------------------------**//**
* Test case for Profiles domain section entity class validation tests.
* @param T - concrete Profiles entity class e.g. IShapeProfile.
* @bsiclass                                                                      12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T>
struct ProfileValidationTestCase : ProfilesTestCase
    {
public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                                     01/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    RefCountedPtr<T> CreateProfile (typename T::CreateParams const& createParams)
        {
        return typename T::Create (createParams);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                                     01/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    Dgn::DgnDbStatus InsertAndUpdateElement (typename T::CreateParams const& createParams)
        {
        return ProfilesTestCase::InsertAndUpdateElement<T> (createParams);
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                                     12/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    void TestParameterToBeFiniteAndPositive (typename T::CreateParams const& createParams, double& parameterToCheck,
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

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                                                     01/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    void TestParameterToBeFiniteAndPositive (typename T::CreateParams const& createParams, Angle& parameterToCheck,
                                             Utf8CP pParameterName, bool allowEqualToZero)
        {
        if (sizeof (Angle) != sizeof (double) || Angle::FromRadians (123.456).Radians() != 123.456)
            FAIL() << "Assuming that Angle is based on radians.";

        double& angle = (double&)parameterToCheck;
        TestParameterToBeFiniteAndPositive (createParams, angle, pParameterName, allowEqualToZero);
        }
    };
