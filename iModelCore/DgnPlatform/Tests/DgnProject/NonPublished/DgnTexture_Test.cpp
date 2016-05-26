/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnTexture_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/BlankDgnDbTestFixture.h"
#include <numeric>
#include <DgnPlatform/DgnTexture.h>

USING_NAMESPACE_BENTLEY_SQLITE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnTexturesTest : public BlankDgnDbTestFixture
    {
    DgnTexture::Data MakeTextureData(DgnTexture::Format fmt, uint32_t w, uint32_t h)
        {
        // For the purposes of this test we really don't know/care about the raw texture data
        bvector<Byte> bytes (w*h);
        std::iota (bytes.begin(), bytes.end(), 0);
        return DgnTexture::Data (fmt, &bytes[0], (uint32_t) bytes.size(), w, h);
        }

    void Compare(DgnTextureCR lhs, DgnTextureCR rhs)
        {
        EXPECT_EQ (lhs.GetTextureId(), rhs.GetTextureId());
        EXPECT_STREQ (lhs.GetTextureName().c_str(), rhs.GetTextureName().c_str());
        EXPECT_STREQ (lhs.GetDescription().c_str(), rhs.GetDescription().c_str());

        auto const& lhData = lhs.GetTextureData();
        auto const& rhData = rhs.GetTextureData();

        EXPECT_EQ (lhData.GetFormat(), rhData.GetFormat());
        EXPECT_EQ (lhData.GetFlags(), rhData.GetFlags());
        EXPECT_EQ (lhData.GetWidth(), rhData.GetWidth());
        EXPECT_EQ (lhData.GetHeight(), rhData.GetHeight());
        EXPECT_EQ (lhData.GetSize(), rhData.GetSize());
        EXPECT_EQ (0, memcmp (lhData.GetData(), rhData.GetData(), lhData.GetSize()));
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgnTexturesTest, InsertQueryUpdateDelete)
    {
    SetupProject(L"textures.ibim");
    DgnDbR db = GetDb();

    // Textures have names
    DgnTexture tx1(DgnTexture::CreateParams(db, "Texture1", MakeTextureData(DgnTexture::Format::JPEG, 2, 4)));
    DgnTextureCPtr pTx1 = tx1.Insert();
    ASSERT_TRUE(pTx1.IsValid());

    // Names must be unique
    DgnTexture tx1_duplicate(DgnTexture::CreateParams(db, "Texture1", MakeTextureData(DgnTexture::Format::JPEG, 4, 8)));
    EXPECT_FALSE(tx1_duplicate.Insert().IsValid());

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
    EXPECT_EQ(DgnDbStatus::Success, tx2Edit->SetCode(DgnTexture::CreateTextureCode("Texture2Renamed")));
    tx2Edit->SetTextureData(MakeTextureData(DgnTexture::Format::JPEG, 9, 18));

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

    // Unnamed textures are supported
    DgnTexture unnamed1(DgnTexture::CreateParams(db, "", MakeTextureData(DgnTexture::Format::JPEG, 2, 2)));
    EXPECT_TRUE(unnamed1.Insert().IsValid());

    // Multiple unnamed textures can exist
    DgnTexture unnamed2(DgnTexture::CreateParams(db, "", MakeTextureData(DgnTexture::Format::JPEG, 4, 4)));
    EXPECT_TRUE(unnamed2.Insert().IsValid());

    // Can't query unnamed texture by name
    EXPECT_FALSE(DgnTexture::QueryTextureId("", db).IsValid());

    // Can query unnamed textures by ID
    auto pUnnamed1 = DgnTexture::QueryTexture(unnamed1.GetTextureId(), db),
         pUnnamed2 = DgnTexture::QueryTexture(unnamed2.GetTextureId(), db);
    ASSERT_TRUE(pUnnamed1.IsValid());
    ASSERT_TRUE(pUnnamed2.IsValid());
    EXPECT_EQ(pUnnamed1->GetTextureId(), unnamed1.GetTextureId());
    EXPECT_EQ(pUnnamed2->GetTextureId(), unnamed2.GetTextureId());
    }

