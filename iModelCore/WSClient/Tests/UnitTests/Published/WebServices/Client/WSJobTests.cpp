/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/UnitTests/Published/WebServices/Client/WSJobTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "WSJobTests.h"
#include <WebServices/Client/WSJob.h>

void WSJobTests::SetUp()
    {
    WSClientBaseTest::SetUpTestCase();
    BeTest::SetFailOnAssert(false);
    }

void WSJobTests::TearDown()
    {
    WSClientBaseTest::TearDownTestCase();
    BeTest::SetFailOnAssert(true);
    }

TEST_F(WSJobTests, IsValid_EmptyResponseContent_False)
    {
    HttpResponse response(HttpStatus(1), "", "", "");
    WSJob wsjob(response);

    EXPECT_FALSE(wsjob.IsValid());
    }

TEST_F(WSJobTests, IsValid_BadJobInstanceFormat_False)
    {
    HttpResponse response(HttpStatus(1), "", "", R"({ "instances" : [{ "Status" : "A" }] })");
    WSJob wsjob(response);

    EXPECT_FALSE(wsjob.IsValid());
    }

TEST_F(WSJobTests, IsValid_JobInstanceDoNotHaveJobStatus_False)
    {
    auto content = (R"({ "instances" :
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {
            "ScheduleTime": "2000-01-01T13:43:57.3126099Z",
            "ResponseStatusCode": 1,
            "ResponseContent": "Good-Content",
            "ResponseHeaders": "A:value\nB:value1"
            }
        }]
        })");
    HttpResponse response(HttpStatus(1), "", "", content);
    WSJob wsjob(response);

    EXPECT_FALSE(wsjob.IsValid());
    }

TEST_F(WSJobTests, IsValid_JobStatusIsNotString_False)
    {
    auto content = (R"({ "instances" :
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {
            "Status": 1,
            "ScheduleTime": "2000-01-01T13:43:57.3126099Z",
            "ResponseStatusCode": 1,
            "ResponseContent": "Good-Content",
            "ResponseHeaders": "A:value\nB:value1"
            }
        }]
        })");
    HttpResponse response(HttpStatus(1), "", "", content);
    WSJob wsjob(response);

    EXPECT_FALSE(wsjob.IsValid());
    }

TEST_F(WSJobTests, IsValid_JobInstanceHasNoResponseStatusCode_True)
    {
    auto content = (R"({ "instances" :
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {
            "Status": "Succeeded",
            "ScheduleTime": "2000-01-01T13:43:57.3126099Z",
            "ResponseContent": "Good-Content",
            "ResponseHeaders": "A:value\nB:value1"
         }
        }]
        })");
    HttpResponse response(HttpStatus(1), "", "", content);
    WSJob wsjob(response);

    EXPECT_TRUE(wsjob.IsValid());
    }

TEST_F(WSJobTests, IsValid_JobInstanceHasNoResponseContent_True)
    {
    auto content = (R"({ "instances" :
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {
            "Status": "Succeeded",
            "ScheduleTime": "2000-01-01T13:43:57.3126099Z",
            "ResponseStatusCode": 1,
            "ResponseHeaders": "A:value\nB:value1"
         }
        }]
        })");
    HttpResponse response(HttpStatus(1), "", "", content);
    WSJob wsjob(response);

    EXPECT_TRUE(wsjob.IsValid());
    }

TEST_F(WSJobTests, IsValid_JobInstanceHasNoResponseHeaders_True)
    {
    auto content = (R"({ "instances" :
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {
            "Status": "Succeeded",
            "ScheduleTime": "2000-01-01T13:43:57.3126099Z",
            "ResponseStatusCode": 1,
            "ResponseContent": "Good-Content"
         }
        }]
        })");
    HttpResponse response(HttpStatus(1), "", "", content);
    WSJob wsjob(response);

    EXPECT_TRUE(wsjob.IsValid());
    }

TEST_F(WSJobTests, IsValid_JobInstanceHasNoScheduleTime_False)
    {
    auto content = (R"({ "instances" :
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {
            "Status": "Succeeded",
            "ResponseStatusCode": 1,
            "ResponseContent": "Good-Content",
            "ResponseHeaders": "A:value\nB:value1"
         }
        }]
        })");
    HttpResponse response(HttpStatus(1), "", "", content);
    WSJob wsjob(response);

    EXPECT_FALSE(wsjob.IsValid());
    }

TEST_F(WSJobTests, IsValid_ScheduleTimeIsNotAString_False)
    {
    auto content = (R"({ "instances" :
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {
            "Status": "Succeeded",
            "ScheduleTime": 1,
            "ResponseStatusCode": 1,
            "ResponseContent": "Good-Content",
            "ResponseHeaders": "A:value\nB:value1"
         }
        }]
        })");
    HttpResponse response(HttpStatus(1), "", "", content);
    WSJob wsjob(response);

    EXPECT_FALSE(wsjob.IsValid());
    }


TEST_F(WSJobTests, IsValid_ScheduleTimeIsNotCorrectDateFormat_False)
    {
    auto content = (R"({ "instances" :
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {
            "Status": "Succeeded",
            "ScheduleTime": "Today",
            "ResponseStatusCode": 1,
            "ResponseContent": "Good-Content",
            "ResponseHeaders": "A:value\nB:value1"
         }
        }]
        })");
    HttpResponse response(HttpStatus(1), "", "", content);
    WSJob wsjob(response);

    EXPECT_FALSE(wsjob.IsValid());
    }

TEST_F(WSJobTests, IsValid_CorrectInstance_True)
    {
    auto content = (R"({ "instances" :
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {
            "Status": "Succeeded",
            "ScheduleTime": "2000-01-01T13:43:57.3126099Z",
            "ResponseStatusCode": 1,
            "ResponseContent": "Good-Content",
            "ResponseHeaders": "A:value\nB:value1"
         }
        }]
        })");
    HttpResponse response(HttpStatus(1), "", "", content);
    WSJob wsjob(response);

    EXPECT_TRUE(wsjob.IsValid());
    }


TEST_F(WSJobTests, HasResponseContent_EmptyResponseContent_False)
    {
    HttpResponse response(HttpStatus(1), "", "", "");
    WSJob wsjob(response);

    EXPECT_FALSE(wsjob.HasResponseContent());
    }

TEST_F(WSJobTests, HasResponseContent_BadJobInstanceFormat_False)
    {
    HttpResponse response(HttpStatus(1), "", "", R"({ "instances" : [{ "Status" : "A" }] })");
    WSJob wsjob(response);

    EXPECT_FALSE(wsjob.HasResponseContent());
    }

TEST_F(WSJobTests, HasResponseContent_JobInstanceDoNotHaveJobStatus_False)
    {
    auto content = (R"({ "instances" :
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {
            "ScheduleTime": "2000-01-01T13:43:57.3126099Z",
            "ResponseStatusCode": 1,
            "ResponseContent": "Good-Content",
            "ResponseHeaders": "A:value\nB:value1"
            }
        }]
        })");
    HttpResponse response(HttpStatus(1), "", "", content);
    WSJob wsjob(response);

    EXPECT_FALSE(wsjob.HasResponseContent());
    }

TEST_F(WSJobTests, HasResponseContent_JobInstanceHasNoResponseStatusCode_False)
    {
    auto content = (R"({ "instances" :
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {
            "Status": "Succeeded",
            "ScheduleTime": "2000-01-01T13:43:57.3126099Z",
            "ResponseContent": "Good-Content",
            "ResponseHeaders": "A:value\nB:value1"
         }
        }]
        })");
    HttpResponse response(HttpStatus(1), "", "", content);
    WSJob wsjob(response);

    EXPECT_FALSE(wsjob.HasResponseContent());
    }

TEST_F(WSJobTests, HasResponseContent_ResponseStatusCodeNotInt_False)
    {
    auto content = (R"({ "instances" :
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {
            "Status": "Succeeded",
            "ResponseStatusCode": "1",
            "ScheduleTime": "2000-01-01T13:43:57.3126099Z",
            "ResponseContent": "Good-Content",
            "ResponseHeaders": "A:value\nB:value1"
         }
        }]
        })");
    HttpResponse response(HttpStatus(1), "", "", content);
    WSJob wsjob(response);

    EXPECT_FALSE(wsjob.HasResponseContent());
    }

TEST_F(WSJobTests, HasResponseContent_JobInstanceHasNoResponseContent_True)
    {
    auto content = (R"({ "instances" :
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {
            "Status": "Succeeded",
            "ScheduleTime": "2000-01-01T13:43:57.3126099Z",
            "ResponseStatusCode": 1,
            "ResponseHeaders": "A:value\nB:value1"
         }
        }]
        })");
    HttpResponse response(HttpStatus(1), "", "", content);
    WSJob wsjob(response);

    EXPECT_TRUE(wsjob.HasResponseContent());
    }

TEST_F(WSJobTests, HasResponseContent_ResponseContentNotString_False)
    {
    auto content = (R"({ "instances" :
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {
            "Status": "Succeeded",
            "ScheduleTime": "2000-01-01T13:43:57.3126099Z",
            "ResponseStatusCode": 1,
            "ResponseContent": "1,
            "ResponseHeaders": "A:value\nB:value1"
         }
        }]
        })");
    HttpResponse response(HttpStatus(1), "", "", content);
    WSJob wsjob(response);

    EXPECT_FALSE(wsjob.HasResponseContent());
    }

TEST_F(WSJobTests, HasResponseContent_ResponseHeadersNotString_False)
    {
    auto content = (R"({ "instances" :
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {
            "Status": "Succeeded",
            "ScheduleTime": "2000-01-01T13:43:57.3126099Z",
            "ResponseStatusCode": 1,
            "ResponseHeaders": 1,
            "ResponseContent": "Good-Content"
         }
        }]
        })");
    HttpResponse response(HttpStatus(1), "", "", content);
    WSJob wsjob(response);

    EXPECT_FALSE(wsjob.HasResponseContent());
    }

TEST_F(WSJobTests, HasResponseContent_JobInstanceHasNoScheduleTime_False)
    {
    auto content = (R"({ "instances" :
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {
            "Status": "Succeeded",
            "ResponseStatusCode": 1,
            "ResponseContent": "Good-Content",
            "ResponseHeaders": "A:value\nB:value1"
         }
        }]
        })");
    HttpResponse response(HttpStatus(1), "", "", content);
    WSJob wsjob(response);

    EXPECT_FALSE(wsjob.HasResponseContent());
    }

TEST_F(WSJobTests, HasResponseContent_CorrectInstance_True)
    {
    auto content = (R"({ "instances" :
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {
            "Status": "Succeeded",
            "ScheduleTime": "2000-01-01T13:43:57.3126099Z",
            "ResponseStatusCode": 1,
            "ResponseContent": "Good-Content",
            "ResponseHeaders": "A:value\nB:value1"
         }
        }]
        })");
    HttpResponse response(HttpStatus(1), "", "", content);
    WSJob wsjob(response);

    EXPECT_TRUE(wsjob.HasResponseContent());
    }

TEST_F(WSJobTests, GetStatus_InstanceNotValid_ErrorString)
    {
    HttpResponse response(HttpStatus(1), "", "", R"({ "instances" : [{ "Status" : "A" }] })");
    WSJob wsjob(response);

    EXPECT_EQ(WSJob::Status::Failed, wsjob.GetStatus());
    }

TEST_F(WSJobTests, GetStatus_CorrectInstance_CorrectStatus)
    {
    auto content = (R"({ "instances" :
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {
            "Status": "Succeeded",
            "ScheduleTime": "2000-01-01T13:43:57.3126099Z",
            "ResponseStatusCode": 1,
            "ResponseContent": "Good-Content",
            "ResponseHeaders": "A:value\nB:value1"
         }
        }]
        })");
    HttpResponse response(HttpStatus(1), "", "", content);
    WSJob wsjob(response);

    EXPECT_EQ(WSJob::Status::Succeeded, wsjob.GetStatus());
    }

TEST_F(WSJobTests, GetStatus_CorrectInstanceStatusNotJobStatus_StatusFailed)
    {
    auto content = (R"({ "instances" :
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {
            "Status": "Somthing",
            "ScheduleTime": "2000-01-01T13:43:57.3126099Z",
            "ResponseStatusCode": 1,
            "ResponseContent": "Good-Content",
            "ResponseHeaders": "A:value\nB:value1"
         }
        }]
        })");
    HttpResponse response(HttpStatus(1), "", "", content);
    WSJob wsjob(response);

    EXPECT_EQ(WSJob::Status::Failed, wsjob.GetStatus());
    }

TEST_F(WSJobTests, GetScheduleTime_InstnaceNotValid_DateInvalid)
    {
    HttpResponse response(HttpStatus(1), "", "", R"({ "instances" : [{ "Status" : "A" }] })");
    WSJob wsjob(response);

    EXPECT_FALSE(wsjob.GetScheduleTime().IsValid());
    }

TEST_F(WSJobTests, GetScheduleTime_CorrectInstance_CorrectDate)
    {
    auto content = (R"({ "instances" :
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {
            "Status": "Succeeded",
            "ScheduleTime": "2000-01-01T13:43:57.313Z",
            "ResponseStatusCode": 1,
            "ResponseContent": "Good-Content",
            "ResponseHeaders": "A:value\nB:value1"
         }
        }]
        })");
    HttpResponse response(HttpStatus(1), "", "", content);
    WSJob wsjob(response);

    EXPECT_STREQ("2000-01-01T13:43:57.313Z", wsjob.GetScheduleTime().ToUtf8String().c_str());
    }

TEST_F(WSJobTests, GetResponse_InstanceNotValid_InvalidResponse)
    {
    HttpResponse response(HttpStatus(1), "", "", R"({ "instances" : [{ "Status" : "A" }] })");
    WSJob wsjob(response);

    EXPECT_FALSE(wsjob.GetResponse().IsSuccess()); //Better check
    }

TEST_F(WSJobTests, GetResponse_InstanceValidDontHaveJobContent_InvalidResponse)
    {
    auto content = (R"({ "instances" :
            [{
            "instanceId" : "A",
            "className" : "A",
            "schemaName" : "A",
            "properties" : {
                "Status": "Succeeded",
                "ScheduleTime": "2000-01-01T13:43:57.313Z"
             }
            }]
            })");
    HttpResponse response(HttpStatus(1), "", "", content);
    WSJob wsjob(response);

    EXPECT_FALSE(wsjob.GetResponse().IsSuccess()); //Better check
    }

TEST_F(WSJobTests, GetResponse_InstanceValidHasJobContent_CorrectResponse)
    {
    auto content = (R"({ "instances" :
        [{
        "instanceId" : "A",
        "className" : "A",
        "schemaName" : "A",
        "properties" : {
            "Status": "Succeeded",
            "ScheduleTime": "2000-01-01T13:43:57.313Z",
            "ResponseStatusCode": 3,
            "ResponseContent": "Good-Content",
            "ResponseHeaders": "A:value\nB:value1"
         }
        }]
        })");
    HttpResponse response(HttpStatus(1), "url", "", content);
    WSJob wsjob(response);

    EXPECT_STREQ("Good-Content", wsjob.GetResponse().GetBody().AsString().c_str());
    EXPECT_EQ(HttpStatus(3), wsjob.GetResponse().GetHttpStatus());
    EXPECT_EQ("url", wsjob.GetResponse().GetEffectiveUrl());
    EXPECT_EQ(ConnectionStatus::OK, wsjob.GetResponse().GetConnectionStatus());
    EXPECT_EQ(2, wsjob.GetResponse().GetHeaders().GetMap().size());
    EXPECT_STREQ("value", wsjob.GetResponse().GetHeaders().GetValue("A"));
    EXPECT_STREQ("value1", wsjob.GetResponse().GetHeaders().GetValue("B"));
    }
