/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include <Bentley/BeTest.h>
#include "SMUnitTestUtil.h"
#include <ScalableMesh/IScalableMeshQuery.h>

using namespace ScalableMesh;

class IScalableMeshMeshFlagsTest : public ::testing::Test
    {
    protected:

        IScalableMeshMeshFlagsPtr m_flags = nullptr;

    public:
    virtual void SetUp() 
        {
        m_flags = IScalableMeshMeshFlags::Create();
        ASSERT_TRUE(m_flags.IsValid());
        }
    virtual void TearDown() 
        {
        m_flags = nullptr;
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IScalableMeshMeshFlagsTest, LoadClips)
    {
    ASSERT_FALSE(m_flags->ShouldLoadClips());
    m_flags->SetLoadClips(true);
    EXPECT_TRUE(m_flags->ShouldLoadClips());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IScalableMeshMeshFlagsTest, LoadTexture)
    {
    ASSERT_FALSE(m_flags->ShouldLoadTexture());
    m_flags->SetLoadTexture(true);
    EXPECT_TRUE(m_flags->ShouldLoadTexture());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IScalableMeshMeshFlagsTest, LoadIndices)
    {
    ASSERT_TRUE(m_flags->ShouldLoadIndices());
    m_flags->SetLoadIndices(false);
    EXPECT_FALSE(m_flags->ShouldLoadIndices());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IScalableMeshMeshFlagsTest, LoadGraph)
    {
    ASSERT_FALSE(m_flags->ShouldLoadGraph());
    m_flags->SetLoadGraph(true);
    EXPECT_TRUE(m_flags->ShouldLoadGraph());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IScalableMeshMeshFlagsTest, SaveToCache)
    {
    ASSERT_FALSE(m_flags->ShouldSaveToCache());
    m_flags->SetSaveToCache(true);
    EXPECT_TRUE(m_flags->ShouldSaveToCache());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IScalableMeshMeshFlagsTest, PrecomputeBoxes)
    {
    ASSERT_FALSE(m_flags->ShouldPrecomputeBoxes());
    m_flags->SetPrecomputeBoxes(true);
    EXPECT_TRUE(m_flags->ShouldPrecomputeBoxes());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IScalableMeshMeshFlagsTest, ClipsToShow)
    {
    ASSERT_FALSE(m_flags->ShouldUseClipsToShow());

    bset<uint64_t> insertedClips, storedClips;
    m_flags->GetClipsToShow(insertedClips);
    ASSERT_TRUE(insertedClips.empty());

    bvector<uint64_t> ids = { 1,2,3 };
    insertedClips.insert(ids.begin(), ids.end());
    m_flags->SetClipsToShow(insertedClips, false);

    ASSERT_FALSE(m_flags->ShouldInvertClips());

    m_flags->GetClipsToShow(storedClips);
    ASSERT_FALSE(storedClips.empty());
    EXPECT_TRUE(storedClips.size() == insertedClips.size());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Richard.Bois      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(IScalableMeshMeshFlagsTest, ClipsToShowWithInvertedClips)
    {
    ASSERT_FALSE(m_flags->ShouldUseClipsToShow());

    bset<uint64_t> insertedClips, storedClips;
    m_flags->GetClipsToShow(insertedClips);
    ASSERT_TRUE(insertedClips.empty());

    bvector<uint64_t> ids = { 1,2,3 };
    insertedClips.insert(ids.begin(), ids.end());
    m_flags->SetClipsToShow(insertedClips, true);

    ASSERT_TRUE(m_flags->ShouldInvertClips());

    m_flags->GetClipsToShow(storedClips);
    ASSERT_FALSE(storedClips.empty());
    EXPECT_TRUE(storedClips.size() == insertedClips.size());
    }
