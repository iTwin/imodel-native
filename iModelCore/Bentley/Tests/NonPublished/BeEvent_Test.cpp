/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include <Bentley/BeEvent.h>

USING_NAMESPACE_BENTLEY

class BeEvents : public ::testing::Test {};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeEvents, Simple)
    {
    BeEvent<int> event;
    int s1, s2, s3;
    s1 = s2 = s3 = 0;
    auto cancel1 = event.AddListener([&s1](int i) {
        s1 = i;
        });
    auto cancel2 = event.AddListener([&s2](int i) {
        s2 = i;
        });

    ASSERT_EQ(event.Count(), 2);
    event.RaiseEvent(100);
    ASSERT_EQ(s1, 100);
    ASSERT_EQ(s2, 100);
    ASSERT_EQ(s3, 0);

    event.RaiseEvent(500);
    ASSERT_EQ(s1, 500);
    ASSERT_EQ(s2, 500);
    ASSERT_EQ(s3, 0);

    auto cancel3 = event.AddListener([&s3](int i) {
        s3 = i;
        });

    ASSERT_EQ(event.Count(), 3);
    event.RaiseEvent(600);
    ASSERT_EQ(s1, 600);
    ASSERT_EQ(s2, 600);
    ASSERT_EQ(s3, 600);

    cancel1();
    ASSERT_EQ(event.Count(), 2);

    event.RaiseEvent(700);
    ASSERT_EQ(s1, 600);
    ASSERT_EQ(s2, 700);
    ASSERT_EQ(s3, 700);

    cancel2();
    cancel3();
    ASSERT_EQ(event.Count(), 0);

    event.RaiseEvent(1);
    ASSERT_EQ(s1, 600);
    ASSERT_EQ(s2, 700);
    ASSERT_EQ(s3, 700);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeEvents, Once)
    {
    BeEvent<int> event;
    int s1, s2;
    s1 = s2 = 0;
    auto cancel1 = event.AddListener([&s1](int i) {
        s1 = i;
        });
    auto cancel2 = event.AddOnce([&s2](int i) {
        s2 = i;
        });

    ASSERT_EQ(event.Count(), 2);

    event.RaiseEvent(10);
    ASSERT_EQ(event.Count(), 1);
    ASSERT_EQ(s1, 10);
    ASSERT_EQ(s2, 10);

    event.RaiseEvent(20);
    ASSERT_EQ(s1, 20);
    ASSERT_EQ(s2, 10);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeEvents, Scope)
    {
    BeEvent<int> event;

    auto cancel = event.AddListener([](int) {});
    if (true) {
        BeEventScope scope;
        scope.Add(event.AddListener([](int) {}));
        scope.Add(event.AddListener([](int) {}));
        scope.Add(event.AddListener([](int) {}));
        ASSERT_EQ(event.Count(), 4);
    }
    ASSERT_EQ(event.Count(), 1);
    cancel();
    ASSERT_EQ(event.Count(), 0);

    cancel = event.AddListener([](int) {});
    BeEventScope scope;
    event.AddListener(scope, [](int) {});
    event.AddListener(scope, [](int) {});
    event.AddListener(scope, [](int) {});
    ASSERT_EQ(event.Count(), 4);
    scope.CancelAll();
    ASSERT_EQ(event.Count(), 1);
    cancel();
    ASSERT_EQ(event.Count(), 0);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(BeEvents, DefaultTraits)
    {
    BeEvent<> event;
    auto cancel1 = event.AddListener([]() {});
    auto cancel2 = event.AddOnce([]() {});
    ASSERT_EQ(event.Count(), 2);
    }