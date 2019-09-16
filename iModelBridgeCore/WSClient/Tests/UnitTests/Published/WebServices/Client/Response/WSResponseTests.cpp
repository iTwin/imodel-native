/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "../../../Utils/WebServicesTestsHelper.h"
#include <WebServices/Client/Response/WSResponse.h>

struct WSResponseTests : WSClientBaseTest {};

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 Simonas.Mulevicius 08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSResponseTests, Ctor_WhenNoParametersAreGiven_ShouldSetParametersToDefaultValues)
    {
    //Arrange and Act
    WSResponse response = WSResponse();

    //Assert
    EXPECT_FALSE(response.IsModified());
    EXPECT_STREQ("", response.GetETag().c_str());
    EXPECT_STREQ("", response.GetActivityId().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 Simonas.Mulevicius 08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSResponseTests, Ctor_WhenResponseIsModified_ShouldSetCorrespondingParameters)
    {
    //Arrange 
    auto isModified = true;

    //Act
    WSResponse response = WSResponse(isModified);

    //Assert
    EXPECT_EQ(isModified, response.IsModified());
    EXPECT_STREQ("", response.GetETag().c_str());
    EXPECT_STREQ("", response.GetActivityId().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 Simonas.Mulevicius 08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSResponseTests, Ctor_WhenResponseStatusIsOkAndResponseHasSpecifiedEtag_ShouldSetCorrespondingParameters)
    {
    //Arrange 
    HttpStatus status = HttpStatus::OK;
    Utf8String eTag = "specifiedEtag";

    //Act
    WSResponse response = WSResponse(status, eTag);

    //Assert
    EXPECT_TRUE(response.IsModified());
    EXPECT_STREQ(eTag.c_str(), response.GetETag().c_str());
    EXPECT_STREQ("", response.GetActivityId().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 Simonas.Mulevicius 08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSResponseTests, Ctor_WhenErrorOccursAndResponseHasSpecifiedEtag_ShouldSetCorrespondingParameters)
    {
    //Arrange 
    HttpStatus status = HttpStatus::InternalServerError;
    Utf8String eTag = "specifiedEtag";

    //Act
    WSResponse response = WSResponse(status, eTag);

    //Assert
    EXPECT_FALSE(response.IsModified());
    EXPECT_STREQ(eTag.c_str(), response.GetETag().c_str());
    EXPECT_STREQ("", response.GetActivityId().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 Simonas.Mulevicius 08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSResponseTests, Ctor_WhenResponseIsModifiedAndHasEtag_ShouldSetCorrespondingParameters)
    {
    //Arrange 
    bool IsModified = true;
    Utf8String eTag = "specifiedEtag";

    // Act
    WSResponse response = WSResponse(IsModified, eTag);

    //Assert
    EXPECT_EQ(IsModified, response.IsModified());
    EXPECT_STREQ(eTag.c_str(), response.GetETag().c_str());
    EXPECT_STREQ("", response.GetActivityId().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 Simonas.Mulevicius 08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSResponseTests, SetActivityId_WhenActivityIdIsEmpty_ShouldSetEmptyActivityId)
    {
    //Arrange 
    WSResponse response = WSResponse();
    Utf8String activityId = "";

    //Act
    response.SetActivityId(activityId);

    //Assert
    EXPECT_STREQ("", response.GetActivityId().c_str());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                 Simonas.Mulevicius 08/2019
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSResponseTests, SetActivityId_WhenActivityIdIsSpecified_ShouldSetSpecifiedActivityId)
    {
    //Arrange 
    WSResponse response = WSResponse();
    Utf8String activityId = "specifiedActivityId";

    //Act
    response.SetActivityId(activityId);

    //Assert
    EXPECT_STREQ(activityId.c_str(), response.GetActivityId().c_str());
    }