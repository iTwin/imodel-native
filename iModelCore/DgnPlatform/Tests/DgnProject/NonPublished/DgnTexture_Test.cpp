/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnTexture_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <numeric>

USING_NAMESPACE_BENTLEY_SQLITE

#define EXPECT_STR_EQ(X,Y) { if ((X).empty() || (Y).empty()) { EXPECT_EQ ((X).empty(), (Y).empty()); } else { EXPECT_EQ ((X), (Y)); } }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnTexturesTest : public ::testing::Test
    {
private:
    ScopedDgnHost           m_host;
    DgnDbPtr                m_db;
protected:
    typedef DgnTextures TX;

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

    TX::TextureData     MakeTextureData (TX::Format fmt, uint32_t w, uint32_t h)
        {
        // For the purposes of this test we really don't know/care about the raw texture data
        bvector<Byte> bytes (w*h);
        std::iota (bytes.begin(), bytes.end(), 0);
        return TX::TextureData (fmt, &bytes[0], bytes.size(), w, h);
        }

    bool                Insert (TX::Texture& tx)
        {
        return m_db->Textures().Insert (tx).IsValid();
        }

    struct TextureEntry : TX::Texture
        {
        DgnTextureId        m_textureId;

        TextureEntry (TX::Iterator::Entry const& entry)
            : TX::Texture (TX::TextureData (entry.GetFormat(), entry.GetDataBytes(), entry.GetDataSize(), entry.GetWidth(), entry.GetHeight(), entry.GetFlags()),
                                    entry.GetName(), entry.GetDescr()),
              m_textureId (entry.GetId()) { }

        DgnTextureId    GetId() const { return m_textureId; }
        };

    template<typename T, typename U> void Compare (T const& lhs, U const& rhs)
        {
        EXPECT_EQ (lhs.GetId(), rhs.GetId());
        EXPECT_STR_EQ (lhs.GetName(), rhs.GetName());
        EXPECT_STR_EQ (lhs.GetDescription(), rhs.GetDescription());

        auto const& lhData = lhs.GetData();
        auto const& rhData = rhs.GetData();

        EXPECT_EQ (lhData.GetFormat(), rhData.GetFormat());
        EXPECT_EQ (lhData.GetFlags(), rhData.GetFlags());
        EXPECT_EQ (lhData.GetWidth(), rhData.GetWidth());
        EXPECT_EQ (lhData.GetHeight(), rhData.GetHeight());
        EXPECT_EQ (lhData.GetData().size(), rhData.GetData().size());
        EXPECT_EQ (0, memcmp (&lhData.GetData()[0], &rhData.GetData()[0], lhData.GetData().size()));
        }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F (DgnTexturesTest, InsertUpdateIterate)
    {
    SetupProject();
    DgnDbR db = GetDb();

    DgnTextures& txs = db.Textures();
    bvector<TX::Texture> expectedTxs;

    // A texture can have a name...
    TX::Texture tx (MakeTextureData (TX::Format::JPEG, 2, 4), "Texture1");
    EXPECT_TRUE (Insert (tx));
    expectedTxs.push_back (tx);

    // ...or no name
    tx = TX::Texture (MakeTextureData (TX::Format::PNG, 3, 2), nullptr, "unnamed texture");
    EXPECT_TRUE (Insert (tx));
    expectedTxs.push_back (tx);

    // Names must be unique...
    tx = TX::Texture (MakeTextureData (TX::Format::RAW, 1, 1), "Texture1");
    EXPECT_FALSE (Insert (tx));

    // ...but multiple unnamed textures can exist
    tx = TX::Texture (MakeTextureData (TX::Format::TIFF, 5, 5), nullptr, "another unnamed texture");
    EXPECT_TRUE (Insert (tx));
    expectedTxs.push_back (tx);

    // Test iteration of textures in the db
    auto iter = txs.MakeIterator();
    EXPECT_EQ (iter.QueryCount(), expectedTxs.size());

    size_t nTxsFound = 0;
    for (auto& entry : iter)
        {
        nTxsFound++;
        auto match = std::find_if (expectedTxs.begin(), expectedTxs.end(), [&](TX::Texture const& arg) { return arg.GetId() == entry.GetId(); });
        ASSERT_FALSE (match == expectedTxs.end());
        Compare (TextureEntry (entry), *match);
        }

    EXPECT_EQ (nTxsFound, expectedTxs.size());

    // Test query by name and ID
    for (auto const& expectedTx : expectedTxs)
        {
        auto txId = expectedTx.GetId();
        if (!expectedTx.GetName().empty())
            {
            auto roundTrippedTxId = txs.QueryTextureId (expectedTx.GetName());
            EXPECT_EQ (roundTrippedTxId, txId);
            }

        tx = txs.Query (txId);
        EXPECT_TRUE (tx.IsValid());
        Compare (expectedTx, tx);
        }

    // Test modification
    tx = expectedTxs.back();
    tx.SetDescription ("updated description");
    tx.SetData (MakeTextureData (TX::Format::PNG, 12, 2));
    EXPECT_EQ (DgnDbStatus::Success, txs.Update (tx));
    TX::Texture updatedTx = txs.Query (tx.GetId());
    EXPECT_TRUE (updatedTx.IsValid());
    Compare (updatedTx, tx);
    }



