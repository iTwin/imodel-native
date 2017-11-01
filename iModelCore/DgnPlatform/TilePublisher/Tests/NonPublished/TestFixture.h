/*--------------------------------------------------------------------------------------+
|
|     $Source: TilePublisher/Tests/NonPublished/TestFixture.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeTest.h>
#include <DgnPlatform/DgnPlatformAPI.h>
#include <TilePublisher/CesiumPublisher.h>
#include <DgnPlatform/TileReader.h>
#include <DgnPlatform/TileWriter.h>
#include <DgnPlatform/UnitTests/DgnDbTestUtils.h>
#include <UnitTests/BackDoor/DgnPlatform/ScopedDgnHost.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_TILETREE
USING_NAMESPACE_TILETREE_IO

#define SEED_FILE_NAME L"TilePublisherSeed.ibim"

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/17
//=======================================================================================
struct TestFixture : public ::testing::Test
{
public:
    DgnDbPtr    m_db;
};

