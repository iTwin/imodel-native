/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <numeric>
#include <DgnPlatform/DgnTexture.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_RENDER

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnTexturesTest : public DgnDbTestFixture
{
    DgnDbR GetDb(){ return *m_db; }
    ImageSource MakeTextureData(ImageSource::Format fmt, uint32_t w, uint32_t h)
        {
        // For the purposes of this test we really don't know/care about the raw texture data
        ByteStream bytes(w*h*3);
        memset(bytes.GetDataP(), 33, bytes.GetSize());
        Image image(w,h,std::move(bytes), Image::Format::Rgb);
        return ImageSource(image, fmt);
        }

    void Compare(DgnTextureCR lhs, DgnTextureCR rhs)
        {
        EXPECT_EQ (lhs.GetTextureId(), rhs.GetTextureId());
        EXPECT_STREQ (lhs.GetTextureName().c_str(), rhs.GetTextureName().c_str());
        EXPECT_STREQ (lhs.GetDescription().c_str(), rhs.GetDescription().c_str());

        auto const& lhData = lhs.GetImageSource();
        auto const& rhData = rhs.GetImageSource();

        EXPECT_EQ (lhs.GetFlags(), rhs.GetFlags());
        EXPECT_EQ (lhs.GetWidth(), rhs.GetWidth());
        EXPECT_EQ (lhs.GetHeight(), rhs.GetHeight());
        EXPECT_EQ (lhData.GetFormat(), rhData.GetFormat());
        EXPECT_EQ (lhData.GetByteStream().GetSize(), rhData.GetByteStream().GetSize());
        EXPECT_EQ (0, memcmp (lhData.GetByteStream().GetData(), rhData.GetByteStream().GetData(), lhData.GetByteStream().GetSize()));
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgnTexturesTest, InsertQueryUpdateDelete)
    {
    SetupSeedProject();
    DgnDbR db = GetDb();
    DefinitionModelR dictionary = db.GetDictionaryModel();

    // Textures have names
    DgnTexture tx1(DgnTexture::CreateParams(dictionary, "Texture1", MakeTextureData(ImageSource::Format::Jpeg, 2, 4), 2,4));
    DgnTextureCPtr pTx1 = tx1.Insert();
    ASSERT_TRUE(pTx1.IsValid());

    // Names must be unique
    DgnTexture tx1_duplicate(DgnTexture::CreateParams(dictionary, "Texture1", MakeTextureData(ImageSource::Format::Jpeg, 4, 8), 4, 8));
    EXPECT_FALSE(tx1_duplicate.Insert().IsValid());

    DgnTexture tx2(DgnTexture::CreateParams(dictionary, "Texture2", MakeTextureData(ImageSource::Format::Png, 5, 5), 5,5,"this is texture 2"));
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
    EXPECT_EQ(DgnDbStatus::Success, tx2Edit->SetCode(DgnTexture::CreateCode(dictionary, "Texture2Renamed")));
    tx2Edit->SetImageSource(MakeTextureData(ImageSource::Format::Jpeg, 9, 18), 9, 18);

    DgnDbStatus status;
    pTx2 = tx2Edit->Update(&status);
    EXPECT_EQ(DgnDbStatus::Success, status);
    ASSERT_TRUE(pTx2.IsValid());
    Compare(*pTx2, *tx2Edit);

    // Purge persistent copies and check round-tripped values as expected
    pTx1 = nullptr;
    pTx2 = nullptr;
    db.Elements().ClearCache();

    pTx1 = DgnTexture::Get(db, tx1.GetTextureId());
    pTx2 = DgnTexture::Get(db, tx2.GetTextureId());
    ASSERT_TRUE(pTx1.IsValid());
    ASSERT_TRUE(pTx2.IsValid());

    Compare(tx1, *pTx1);
    Compare(*pTx2, *tx2Edit);

    // Textures cannot be deleted, only purged
    EXPECT_EQ(DgnDbStatus::DeletionProhibited, pTx1->Delete());

    // Unnamed textures are supported
    DgnTexture unnamed1(DgnTexture::CreateParams(dictionary, "", MakeTextureData(ImageSource::Format::Jpeg, 2, 2), 2,2));
    EXPECT_TRUE(unnamed1.GetTextureName().empty());
    EXPECT_TRUE(unnamed1.GetCode().IsValid());
    EXPECT_TRUE(unnamed1.GetCode().IsEmpty());
    EXPECT_TRUE(unnamed1.Insert().IsValid());

    // Multiple unnamed textures can exist
    DgnTexture unnamed2(DgnTexture::CreateParams(dictionary, "", MakeTextureData(ImageSource::Format::Jpeg, 4, 4), 4,4));
    EXPECT_TRUE(unnamed2.Insert().IsValid());

    // Can't query unnamed texture by name
    EXPECT_FALSE(DgnTexture::QueryTextureId(dictionary, "").IsValid());

    // Can query unnamed textures by ID
    auto pUnnamed1 = DgnTexture::Get(db, unnamed1.GetTextureId()),
         pUnnamed2 = DgnTexture::Get(db, unnamed2.GetTextureId());
    ASSERT_TRUE(pUnnamed1.IsValid());
    ASSERT_TRUE(pUnnamed2.IsValid());
    EXPECT_EQ(pUnnamed1->GetTextureId(), unnamed1.GetTextureId());
    EXPECT_EQ(pUnnamed2->GetTextureId(), unnamed2.GetTextureId());
    }
