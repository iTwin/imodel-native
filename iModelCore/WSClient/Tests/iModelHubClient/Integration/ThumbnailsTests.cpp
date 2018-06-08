/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/ThumbnailsTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "iModelTestsBase.h"

USING_NAMESPACE_BENTLEY_IMODELHUB
USING_NAMESPACE_BENTLEY_IMODELHUB_UNITTESTS

static const Utf8CP s_iModelName = "ThumbnailsTests";

/*======================================================================================+
* @bsiclass                                     Andrius.Zonys                   04/2018
+===============+===============+===============+===============+===============+======*/
struct ThumbnailsTests : public iModelTestsBase
    {
    static bvector<Utf8String> s_versionIds;
    static bvector<Thumbnail::Size> s_thumbnailSizes;

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Andrius.Zonys                   04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void SetUpTestCase()
        {
        iModelTestsBase::SetUpTestCase();

        iModelHubHelpers::AcquireAndAddChangeSets(s_client, s_info, 3);
        for (int i = 1; i <= 3; i++)
            {
            VersionInfoPtr version;
            iModelHubHelpers::CreateNamedVersion(version, s_connection, Utf8PrintfString("ThumbnailsTests%d", i), i);
            s_versionIds.push_back(version->GetId());
            }
        s_thumbnailSizes = { Thumbnail::Size::Small, Thumbnail::Size::Large };
        }

    /*--------------------------------------------------------------------------------------+
    * @bsimethod                                    Andrius.Zonys                   04/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static void TearDownTestCase()
        {
        iModelTestsBase::TearDownTestCase();
        }
    };
bvector<Utf8String> ThumbnailsTests::s_versionIds;
bvector<Thumbnail::Size> ThumbnailsTests::s_thumbnailSizes;

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Andrius.Zonys                   04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ThumbnailsTests, GetiModelThumbnail)
    {
    for each (Thumbnail::Size size in s_thumbnailSizes)
        {
        ThumbnailImageResult result;
        int retryCount = 5;
        for (int i = 0; i <= retryCount; i++)
            {
            result = s_client->GetiModelThumbnail(s_projectId, s_info->GetId(), size)->GetResult();
            if (result.IsSuccess() || i == retryCount)
                break;
            BeThreadUtilities::BeSleep(3000);
            }
        EXPECT_TRUE(result.IsSuccess());
        EXPECT_TRUE(result.GetValue().IsValid());
        EXPECT_TRUE(result.GetValue().GetByteStream().HasData());
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Andrius.Zonys                   04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ThumbnailsTests, GetThumbnailsById)
    {
    for each (Thumbnail::Size size in s_thumbnailSizes)
        {
        ThumbnailsManagerCR thumbnailsManager = s_connection->GetThumbnailsManager();
        bvector<Utf8String> thumbnailsIds;
        int retryCount = 10;
        for (int i = 0; i <= retryCount; i++)
            {
            thumbnailsIds = thumbnailsManager.GetAllThumbnailsIds(size)->GetResult().GetValue();
            if (4 == thumbnailsIds.size() || i == retryCount)
                break;
            BeThreadUtilities::BeSleep(3000);
            }
        EXPECT_EQ(4, thumbnailsIds.size());

        for each (Utf8String thumbnailId in thumbnailsIds)
            {
            ThumbnailImageResult result = thumbnailsManager.GetThumbnailById(thumbnailId, size)->GetResult();
            EXPECT_TRUE(result.IsSuccess());
            EXPECT_TRUE(result.GetValue().IsValid());
            EXPECT_TRUE(result.GetValue().GetByteStream().HasData());
            }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                    Andrius.Zonys                   04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ThumbnailsTests, GetThumbnailsByVersionId)
    {
    for each (Thumbnail::Size size in s_thumbnailSizes)
        {
        ThumbnailsManagerCR thumbnailsManager = s_connection->GetThumbnailsManager();
        for each (Utf8String versionId in s_versionIds)
            {
            ThumbnailImageResult result;
            int retryCount = 5;
            for (int i = 0; i <= retryCount; i++)
                {
                result = thumbnailsManager.GetThumbnailByVersionId(versionId, size)->GetResult();
                if (result.IsSuccess() || i == retryCount)
                    break;
                BeThreadUtilities::BeSleep(3000);
                }
            EXPECT_TRUE(result.IsSuccess());
            EXPECT_TRUE(result.GetValue().IsValid());
            EXPECT_TRUE(result.GetValue().GetByteStream().HasData());
            }
        }
    }