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

//=======================================================================================
// @bsistruct                                                   Paul.Connelly   11/17
//=======================================================================================
struct TestFixture : public ::testing::Test
{
protected:
    ScopedDgnHost   m_host;
    DgnDbPtr        m_db;

    void TearDown() override { SaveDb(); }
public:
    // Create a blank DgnDb to hold data to be published
    void SetupDb(WCharCP filenameWithoutExtension);

    // Get the DgnDb produced by SetupDb()
    DgnDbR GetDgnDb() { BeAssert(m_db.IsValid()); return *m_db; }

    // Close the DgnDb.
    void CloseDb() { GetDgnDb().CloseDb(); }

    // Save all changes to the DgnDb. You generally want to do this before publishing tiles.
    void SaveDb()
        {
        if (m_db.IsValid() && m_db->IsDbOpen() && !m_db->IsReadonly())
            m_db->SaveChanges();
        }

    // If a test obtains ref-counted pointers to DgnElements, and does not explicitly set all of them to null before the test terminates,
    // DgnDb will assert. Pass your test function/lambda to this function to ensure any DgnElementPtrs within it are released before termination.
    template<typename T> void ExecuteTest(T testFunc) { testFunc(); }
};

