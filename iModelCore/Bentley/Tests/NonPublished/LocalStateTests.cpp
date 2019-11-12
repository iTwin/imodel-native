/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include <Bentley/LocalState.h>

USING_NAMESPACE_BENTLEY

class RuntimeLocalStateTests : public ::testing::Test {};

/*-------------------------------------------------------------------------------------+
* @bsimethod                                    Vincas.Razma                    09/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RuntimeLocalStateTests, Ctor_Default_NoValues)
    {
    EXPECT_EQ(0, RuntimeLocalState().GetValues().size());
    }

/*-------------------------------------------------------------------------------------+
* @bsimethod                                    Vincas.Razma                    09/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RuntimeLocalStateTests, GetValue_NoValue_Empty)
    {
    RuntimeLocalState localState;
    EXPECT_STREQ("", localState.GetValue("Foo", "Boo").c_str());
    }

/*-------------------------------------------------------------------------------------+
* @bsimethod                                    Vincas.Razma                    09/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RuntimeLocalStateTests, GetValue_ExistingValue_ReturnsValue)
    {
    RuntimeLocalState localState;
    localState.SaveValue("Foo", "Boo", "42");
    EXPECT_STREQ("42", localState.GetValue("Foo", "Boo").c_str());
    }

/*-------------------------------------------------------------------------------------+
* @bsimethod                                    Vincas.Razma                    09/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RuntimeLocalStateTests, SaveValue_EmptyStr_RemovesValue)
    {
    RuntimeLocalState localState;
    localState.SaveValue("Foo", "Boo", "42");
    EXPECT_EQ(1, localState.GetValues().size());
    localState.SaveValue("Foo", "Boo", "");
    EXPECT_EQ(0, localState.GetValues().size());
    }


/*-------------------------------------------------------------------------------------+
* @bsimethod                                    Vincas.Razma                    09/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RuntimeLocalStateTests, SaveValue_DifferentKeys_StoresDifferentValues)
    {
    RuntimeLocalState localState;
    localState.SaveValue("Foo", "A", "ValueA");
    localState.SaveValue("Foo", "B", "ValueB");
    EXPECT_STREQ("ValueA", localState.GetValue("Foo", "A").c_str());
    EXPECT_STREQ("ValueB", localState.GetValue("Foo", "B").c_str());
    bpair<Utf8String, Utf8String> pairA {"Foo", "A"};
    bpair<Utf8String, Utf8String> pairB {"Foo", "B"};
    EXPECT_STREQ("ValueA", localState.GetValues()[pairA].c_str());
    EXPECT_STREQ("ValueB", localState.GetValues()[pairB].c_str());
    EXPECT_EQ(2, localState.GetValues().size());
    }

/*-------------------------------------------------------------------------------------+
* @bsimethod                                    Vincas.Razma                    09/18
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(RuntimeLocalStateTests, SaveValue_DifferentNamespaces_StoresDifferentValues)
    {
    RuntimeLocalState localState;
    localState.SaveValue("A", "Foo", "ValueA");
    localState.SaveValue("B", "Foo", "ValueB");
    EXPECT_STREQ("ValueA", localState.GetValue("A", "Foo").c_str());
    EXPECT_STREQ("ValueB", localState.GetValue("B", "Foo").c_str());
    bpair<Utf8String, Utf8String> pairA {"A", "Foo"};
    bpair<Utf8String, Utf8String> pairB {"B", "Foo"};
    EXPECT_STREQ("ValueA", localState.GetValues()[pairA].c_str());
    EXPECT_STREQ("ValueB", localState.GetValues()[pairB].c_str());
    EXPECT_EQ(2, localState.GetValues().size());
    }
