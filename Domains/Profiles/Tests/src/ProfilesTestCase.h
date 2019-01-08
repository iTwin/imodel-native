/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/src/ProfilesTestCase.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley\BeTest.h>
#include <DgnView\DgnViewLib.h>
#include <Profiles\ProfilesApi.h>

#define EXPECT_SUCCESS_Insert(createParams) EXPECT_EQ (DgnDbStatus::Success, InsertAndUpdateElement (createParams))
#define EXPECT_FAIL_Insert(createParams) EXPECT_EQ (DgnDbStatus::ValidationFailed, InsertAndUpdateElement (createParams))

/*---------------------------------------------------------------------------------**//**
* Base class for Profiles domain test cases.
* @bsiclass                                                                      11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
struct ProfilesTestCase : testing::Test
    {
private:
    Dgn::DgnDbPtr m_dbPtr;
    Dgn::DefinitionModelPtr m_definitionModelPtr;
    Dgn::PhysicalModelPtr m_physicalModelPtr;
    Dgn::DgnCategoryId m_categoryId;

protected:
    ProfilesTestCase();
    virtual ~ProfilesTestCase();

    Dgn::DgnDb& GetDb();
    Dgn::DgnModel& GetModel();
    Dgn::PhysicalModel& GetPhysicalModel();
    Dgn::DgnCategoryId GetCategoryId();

    /*---------------------------------------------------------------------------------**//**
    * Create and insert an entity class instance.
    * @bsimethod                                                                     01/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename T>
    RefCountedPtr<T> InsertElement (typename T::CreateParams const& createParams, Dgn::DgnDbStatus* pStatus = nullptr)
        {
        RefCountedPtr<T> instancePtr = typename T::Create (createParams);
        BeAssert (instancePtr.IsValid());

        Dgn::DgnDbStatus status;
        if (pStatus == nullptr)
            pStatus = &status;

        instancePtr->Insert (pStatus);
        if (*pStatus != Dgn::DgnDbStatus::Success)
            return nullptr;

        return instancePtr;
        }

    /*---------------------------------------------------------------------------------**//**
    * Insert and update an element, returning the status of these operations.
    * @bsimethod                                                                     01/2019
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename T>
    Dgn::DgnDbStatus InsertAndUpdateElement (typename T::CreateParams const& createParams)
        {
        Dgn::DgnDbStatus status;
        RefCountedPtr<T> instancePtr = InsertElement<T> (createParams, &status);
        if (status != Dgn::DgnDbStatus::Success)
            return status;

        // Perform an Update just to double check same validation is happenning on update.
        instancePtr->Update (&status);
        return status;
        }
    };
