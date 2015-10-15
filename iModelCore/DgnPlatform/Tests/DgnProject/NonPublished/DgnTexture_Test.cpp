/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnTexture_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <numeric>
#include <DgnPlatform/DgnCore/DgnTexture.h>

USING_NAMESPACE_BENTLEY_SQLITE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnTexturesTest : public ::testing::Test
    {
private:
    ScopedDgnHost           m_host;
    DgnDbPtr                m_db;
protected:
    void SetupProject()
        {
        BeFileName filename = DgnDbTestDgnManager::GetOutputFilePath (L"textures.idgndb");
        BeFileName::BeDeleteFile (filename);

        CreateDgnDbParams params;
        params.SetOverwriteExisting (false);
        DbResult status;
        m_db = DgnDb::CreateDgnDb (&status, filename, params);
        ASSERT_TRUE (m_db != nullptr);
        ASSERT_EQ (BE_SQLITE_OK, status) << status;
        }

    DgnDbR      GetDb()
        {
        return *m_db;
        }

    DgnTexture::Data     MakeTextureData (DgnTexture::Format fmt, uint32_t w, uint32_t h)
        {
        // For the purposes of this test we really don't know/care about the raw texture data
        bvector<Byte> bytes (w*h);
        std::iota (bytes.begin(), bytes.end(), 0);
        return DgnTexture::Data (fmt, &bytes[0], bytes.size(), w, h);
        }

    void Compare(DgnTextureCR lhs, DgnTextureCR rhs)
        {
        EXPECT_EQ (lhs.GetTextureId(), rhs.GetTextureId());
        EXPECT_STREQ (lhs.GetTextureName().c_str(), rhs.GetTextureName().c_str());
        EXPECT_STREQ (lhs.GetDescription().c_str(), rhs.GetDescription().c_str());

        auto const& lhData = lhs.GetData();
        auto const& rhData = rhs.GetData();

        EXPECT_EQ (lhData.GetFormat(), rhData.GetFormat());
        EXPECT_EQ (lhData.GetFlags(), rhData.GetFlags());
        EXPECT_EQ (lhData.GetWidth(), rhData.GetWidth());
        EXPECT_EQ (lhData.GetHeight(), rhData.GetHeight());
        EXPECT_EQ (lhData.GetBytes().size(), rhData.GetBytes().size());
        EXPECT_EQ (0, memcmp (&lhData.GetBytes()[0], &rhData.GetBytes()[0], lhData.GetBytes().size()));
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgnTexturesTest, InsertQueryUpdateDelete)
    {
    SetupProject();
    DgnDbR db = GetDb();

    // Textures have names
    DgnTexture tx1(DgnTexture::CreateParams(db, "Texture1", MakeTextureData(DgnTexture::Format::JPEG, 2, 4)));
    DgnTextureCPtr pTx1 = tx1.Insert();
    ASSERT_TRUE(pTx1.IsValid());

    // Names must be unique
    DgnTexture tx1_duplicate(DgnTexture::CreateParams(db, "Texture1", MakeTextureData(DgnTexture::Format::JPEG, 4, 8)));
    EXPECT_FALSE(tx1_duplicate.Insert().IsValid());

    // ###TODO: unnamed textures

    DgnTexture tx2(DgnTexture::CreateParams(db, "Texture2", MakeTextureData(DgnTexture::Format::TIFF, 5, 5), "this is texture 2"));
    DgnTextureCPtr pTx2 = tx2.Insert();
    ASSERT_TRUE(pTx2.IsValid());

    // Persistent + local have equivalent values
    Compare(tx1, *pTx1);
    Compare(tx2, *pTx2);

    // Modify
    DgnTexturePtr tx2Edit = pTx2->MakeCopy<DgnTexture>();
    ASSERT_TRUE(tx2Edit.IsValid());
    Compare(*tx2Edit, tx2);

    tx2Edit->SetDescription("New description");
    EXPECT_EQ(DgnDbStatus::Success, tx2Edit->SetCode(DgnTexture::CreateTextureCode("Texture2Renamed", db)));
    tx2Edit->SetData(MakeTextureData(DgnTexture::Format::JPEG, 9, 18));

    DgnDbStatus status;
    pTx2 = tx2Edit->Update(&status);
    EXPECT_EQ(DgnDbStatus::Success, status);
    ASSERT_TRUE(pTx2.IsValid());
    Compare(*pTx2, *tx2Edit);

    // Purge persistent copies and check round-tripped values as expected
    pTx1 = nullptr;
    pTx2 = nullptr;
    EXPECT_TRUE(db.Memory().Purge(0));

    pTx1 = DgnTexture::QueryTexture(tx1.GetTextureId(), db);
    pTx2 = DgnTexture::QueryTexture(tx2.GetTextureId(), db);
    ASSERT_TRUE(pTx1.IsValid());
    ASSERT_TRUE(pTx2.IsValid());

    Compare(tx1, *pTx1);
    Compare(*pTx2, *tx2Edit);

    // Textures cannot be deleted, only purged
    EXPECT_EQ(DgnDbStatus::DeletionProhibited, pTx1->Delete());
    }

