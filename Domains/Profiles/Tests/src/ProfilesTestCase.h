/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include <Bentley\BeTest.h>
#include <DgnView\DgnViewLib.h>
#include <Profiles\ProfilesApi.h>

#define TESTS_EPSILON 0.000000000000001

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

protected:
    ProfilesTestCase();
    virtual ~ProfilesTestCase();

    Dgn::DgnDb& GetDb();
    Dgn::DefinitionModel& GetModel();

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

    /*---------------------------------------------------------------------------------**//**
    * Create and insert a DgnModel.
    * @bsimethod                                                                     11/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    template<typename T_Model, typename T_Partition>
    RefCountedPtr<T_Model> InsertDgnModel (Utf8CP pPartitionName)
        {
        Dgn::SubjectCPtr rootSubjectPtr = GetDb().Elements().GetRootSubject();

        RefCountedPtr<T_Partition> partitionPtr = typename T_Partition::Create (*rootSubjectPtr, pPartitionName);
        GetDb().BriefcaseManager().AcquireForElementInsert (*partitionPtr);

        Dgn::DgnDbStatus status;
        partitionPtr->Insert (&status);
        if (status != Dgn::DgnDbStatus::Success)
            return nullptr;

        RefCountedPtr<T_Model> modelPtr = typename T_Model::Create (*partitionPtr);
        status = modelPtr->Insert();
        if (status != Dgn::DgnDbStatus::Success)
            return nullptr;

        return modelPtr;
        }

    Utf8String GetFailureMessage (Utf8CP message, CharCP file, size_t lineNo);
    };

