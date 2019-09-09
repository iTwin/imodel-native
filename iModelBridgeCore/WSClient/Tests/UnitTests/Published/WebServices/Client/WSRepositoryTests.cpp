/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "../../Utils/WebServicesTestsHelper.h"
#include <WebServices/Client/WSRepository.h>

struct WSRepositoryTests : WSClientBaseTest {};

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryTests, IsValid_DefaultCtor_False)
    {
    EXPECT_FALSE(WSRepository().IsValid());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryTests, Ctor_Default_EmptyValues)
    {
    WSRepository repo;
    EXPECT_FALSE(repo.IsValid());
    EXPECT_EQ("", repo.GetDescription());
    EXPECT_EQ("", repo.GetId());
    EXPECT_EQ("", repo.GetLabel());
    EXPECT_EQ("", repo.GetLocation());
    EXPECT_EQ("", repo.GetPluginId());
    EXPECT_EQ("", repo.GetServerUrl());
    EXPECT_EQ(0, repo.GetMaxUploadSize());
    EXPECT_EQ(BeVersion(), repo.GetPluginVersion());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(WSRepositoryTests, Ctor_FormSerialized_HasSameValues)
    {
    WSRepository src;
    src.SetDescription("A");
    src.SetId("B");
    src.SetLabel("C");
    src.SetLocation("D");
    src.SetPluginId("E");
    src.SetServerUrl("F");
    src.SetPluginVersion(BeVersion(1, 2, 3, 4));
    src.SetServiceVersion(BeVersion(5, 6, 7, 8));
    src.SetMaxUploadSize(9);

    WSRepository repo(src.ToString());
    EXPECT_TRUE(repo.IsValid());

    EXPECT_EQ("A", repo.GetDescription());
    EXPECT_EQ("B", repo.GetId());
    EXPECT_EQ("C", repo.GetLabel());
    EXPECT_EQ("D", repo.GetLocation());
    EXPECT_EQ("E", repo.GetPluginId());
    EXPECT_EQ("F", repo.GetServerUrl());
    EXPECT_EQ(BeVersion(1, 2, 3, 4), repo.GetPluginVersion());
    EXPECT_EQ(BeVersion(5, 6, 7, 8), repo.GetServiceVersion());
    EXPECT_EQ(9, repo.GetMaxUploadSize());
    }
