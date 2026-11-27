/*--------------------------------------------------------------------------------------+       22
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <zlib/zlib.h>

/**
 *
 */
struct FontTests : public DgnDbTestFixture {
};

using TestShxGlyph = std::pair<uint16_t, bvector<Byte>>;

static void appendUInt16(ByteStreamR data, uint16_t value) {
    data.Append(static_cast<Byte>(value & 0xff));
    data.Append(static_cast<Byte>(value >> 8));
}

static ByteStream createLocaleShx(bvector<TestShxGlyph> const& glyphs, uint16_t finalDeclaredSize = 0) {
    static Utf8CP header = "AutoCAD-86 shapes 1.0\r\n\x1a";
    ByteStream data;
    data.Append(reinterpret_cast<ByteCP>(header), static_cast<uint32_t>(strlen(header)));

    appendUInt16(data, 0);
    appendUInt16(data, UINT16_MAX);
    appendUInt16(data, static_cast<uint16_t>(glyphs.size()));

    for (size_t i = 0; i < glyphs.size(); ++i) {
        appendUInt16(data, glyphs[i].first);
        uint16_t dataSize = static_cast<uint16_t>(glyphs[i].second.size());
        if (finalDeclaredSize != 0 && i == glyphs.size() - 1)
            dataSize = finalDeclaredSize;
        appendUInt16(data, dataSize);
    }

    for (auto const& glyph : glyphs)
        data.Append(glyph.second.data(), static_cast<uint32_t>(glyph.second.size()));

    while (data.size() < 40)
        data.Append(static_cast<Byte>(0));

    return data;
}

static DbFontP embedTestShx(FontDbR fontDb, Utf8CP name, ByteStreamCR data) {
    bvector<FontFace> faces;
    faces.emplace_back(FontFace(FontType::Shx, name));
    if (SUCCESS != fontDb.EmbedFont(faces, data, false))
        return nullptr;

    return fontDb.FindFont(FontType::Shx, name);
}

void testFont(DbFont& font) {
    auto glyphA = font.FindGlyphCP('A', FaceStyle::Regular);
    auto glyphb = font.FindGlyphCP('b', FaceStyle::Regular);
    EXPECT_TRUE(nullptr != glyphA);
    EXPECT_TRUE(nullptr != glyphb);
    EXPECT_TRUE(glyphA != glyphb);
    EXPECT_TRUE(glyphA == font.FindGlyphCP('A', FaceStyle::Bold)); // there is no bold font, use regular
    EXPECT_TRUE(glyphA == font.FindGlyphCP('A', FaceStyle::Italic)); // there is no italic font, use regular
    EXPECT_TRUE(glyphA == font.FindGlyphCP('A', FaceStyle::BoldItalic)); // there is no bolditalic font, use regular
    auto curve = glyphA->GetCurveVector();
    EXPECT_TRUE(curve.IsValid());
    curve = glyphb->GetCurveVector();
    EXPECT_TRUE(curve.IsValid());
}

void testDefaultFont(FontType fontType) {
    auto& font = FontManager::GetFallbackFont(fontType);
    EXPECT_TRUE(fontType == font.GetType());
    EXPECT_TRUE(font.IsFallback());
    testFont(font);
}

TEST_F(FontTests, DisposeFontMangerOnCloseDb) {
    SetupSeedProject();
    struct ErrorLogger : NativeLogging::Logger {
        std::vector<Utf8String> m_messages;
        void LogMessage(Utf8CP category, NativeLogging::SEVERITY sev, Utf8CP msg) override {
            if (sev == NativeLogging::SEVERITY::LOG_ERROR)
                m_messages.push_back(msg);
        }
        bool IsSeverityEnabled(Utf8CP category, NativeLogging::SEVERITY sev) override{
            return sev == NativeLogging::SEVERITY::LOG_ERROR ;
        }
    };

    auto dbFileName = m_db->GetFileName();

    BeFileName ttfFontPath;
    ASSERT_TRUE(SUCCESS == DgnDbTestDgnManager::FindTestData(ttfFontPath, L"Fonts\\Karla-Regular.ttf", __FILE__));
    // Since we testing for BlobIO that is only called for uncompress fonts.
    TrueTypeFile ttFile(ttfFontPath.GetNameUtf8().c_str(), false);
    ASSERT_FALSE(ttFile.m_compress) << "Expecting uncompress";
    ASSERT_TRUE(ttFile.Embed(m_db->Fonts().m_fontDb));
    EXPECT_TRUE(DbResult::BE_SQLITE_OK == m_db->SaveChanges("Font embedded"));
    m_db = nullptr;

    OpenDb(m_db, dbFileName, Db::OpenMode::ReadWrite);
    auto id = m_db->Fonts().GetId(FontType::TrueType, "Karla");
    ASSERT_TRUE(id.IsValid());
    auto& font = static_cast<TrueTypeFont&>(m_db->Fonts().FindFont(id));

    // Following is called to trigger GetReader() which open BlobIO handle which
    // is left open when m_db->CloseDb() is called without the patch in this PR.
    ASSERT_FALSE(font.ComputeGlyphIndices("Hello, World", false, false).empty()) << "This trigger FindReader()";

    ErrorLogger logger;
    NativeLogging::Logging::SetLogger(&logger);
    BeTest::SetFailOnAssert(false);
    // We do not get any return result from following so we have
    // depend on LOG.errorv() messages to see if sqlite3_close() has failed in ~DbFile()
    m_db->CloseDb();
    BeTest::SetFailOnAssert(true);
    NativeLogging::Logging::SetLogger(nullptr);

    ASSERT_EQ(logger.m_messages.size(), 0) << "No error should be logged with patch";

    // Without the patch we expect following as first error message
    // due to open blobio handle as FontManager is not destroyed in
    // CloseDb() rather destroyed in ~DgnDb() which happen later.
    if (logger.m_messages.size()>0) {
        // blobio are just statement with no SQL but has hardcoded program
        ASSERT_TRUE(logger.m_messages[0].Equals("Statement not closed: ''"));
    }

    if (logger.m_messages.size()>1) {
        // This is second message we get once db fail to close.
        ASSERT_TRUE(logger.m_messages[1].StartsWith("Cannot close database"));
    }
}

/**
 * Test embedding of truetype files
 */
TEST_F(FontTests, EmbedTTFont) {
    SetupSeedProject();

    auto& dbFonts = m_db->Fonts();
    auto& fontDb = dbFonts.m_fontDb;

    BeFileName ttfFontPath;
    ASSERT_TRUE(SUCCESS == DgnDbTestDgnManager::FindTestData(ttfFontPath, L"Fonts\\Karla-Regular.ttf", __FILE__));
    TrueTypeFile ttFile(ttfFontPath.GetNameUtf8().c_str(), true);
    ASSERT_TRUE(ttFile.Embed(fontDb));

    auto id = dbFonts.GetId(FontType::TrueType, "Karla");
    ASSERT_TRUE(id.IsValid());

    ASSERT_TRUE(SUCCESS == DgnDbTestDgnManager::FindTestData(ttfFontPath, L"Fonts\\Sitka.ttc", __FILE__));
    TrueTypeFile sitka(ttfFontPath.GetNameUtf8().c_str(), false); // don't compress this font to test direct reading from FontDb
    ASSERT_TRUE(sitka.Embed(fontDb));
    EXPECT_TRUE(DbResult::BE_SQLITE_OK == m_db->SaveChanges("Font embedded"));

    auto& font = dbFonts.FindFont(id);
    ASSERT_TRUE(font.GetName().EqualsI("karla"));
    testFont(font);

    auto& ttFonts = fontDb.GetMap(FontType::TrueType);
    testFont(*ttFonts.Find("sitka small")); // font lookup is case insensitive
    testFont(*ttFonts.Find("sitka Text"));
    testFont(*ttFonts.Find("Sitka Subheading"));
    testFont(*ttFonts.Find("SITKA HEADING"));
    testFont(*ttFonts.Find("Sitka Banner"));

    CharCP dne = "Does Not Exist";
    auto dneId = dbFonts.InsertFont(FontType::TrueType, dne);
    ASSERT_TRUE(dneId.IsValid());
    auto& dneFont = dbFonts.FindFont(dneId);
    ASSERT_TRUE(dneFont.GetType() == FontType::TrueType); // should be fallback truetype font.
    ASSERT_TRUE(dneFont.GetName().EqualsI("FallbackTrueType"));
    ASSERT_TRUE(dneFont.IsFallback());

    auto dneId2 = dbFonts.FindId(FontType::TrueType, dne); // looking up a not-found font by name should work
    ASSERT_TRUE(dneId == dneId2);
}

TEST_F(FontTests, LazyCache) {
  SetupSeedProject();

  auto& dbFonts = m_db->Fonts();
  EXPECT_EQ(&dbFonts.m_fontDb.m_db, m_db.get());

  auto& fbFont = FontManager::GetFallbackFont(FontType::TrueType);
  auto invalidId = dbFonts.FindId(FontType::TrueType, "Karla");
  EXPECT_FALSE(invalidId.IsValid());
  EXPECT_EQ(&dbFonts.FindFont(invalidId), &fbFont);

  BeFileName ttfFontPath;
  ASSERT_TRUE(SUCCESS == DgnDbTestDgnManager::FindTestData(ttfFontPath, L"Fonts\\Karla-Regular.ttf", __FILE__));
  TrueTypeFile ttFile(ttfFontPath.GetNameUtf8().c_str(), true);
  ASSERT_TRUE(ttFile.Embed(dbFonts.m_fontDb));
  EXPECT_TRUE(DbResult::BE_SQLITE_OK == m_db->SaveChanges("Font embedded"));

  // Embedding a font only writes to the be_Props table - it doesn't allocate a FontId in the dgn_Font table.
  // The core-backend API does both by default.
  EXPECT_FALSE(dbFonts.FindId(FontType::TrueType, "Karla").IsValid());
  // GetId (unlike FindId) will allocate a new FontId in the dgn_Font table if one doesn't already exist.
  // If it allocates a FontId, it also invalidates the font cache.
  auto fontId = dbFonts.GetId(FontType::TrueType, "Karla");
  EXPECT_TRUE(fontId.IsValid());
  EXPECT_EQ(fontId.GetValue(), dbFonts.FindId(FontType::TrueType, "Karla").GetValue());

  // The font cache was invalidated when we allocated a Font Id. It should now find our newly-embedded font.
  EXPECT_NE(&fbFont, &dbFonts.FindFont(fontId));

  // The fallback font should remain the same after invalidating the cache.
  EXPECT_EQ(&fbFont, &FontManager::GetFallbackFont(FontType::TrueType));
}

/**
 * test embedding SHX fonts
 */
TEST_F(FontTests, EmbedSHxFont) {
    SetupSeedProject();

    DgnDbFonts& dbFonts = m_db->Fonts();

    BeFileName shxFilepath;
    ASSERT_TRUE(SUCCESS == DgnDbTestDgnManager::FindTestData(shxFilepath, L"Fonts\\Cdm.shx", __FILE__));

    ByteStream data;
    BeFile shxFile;
    EXPECT_TRUE(BeFileStatus::Success == shxFile.Open(shxFilepath, BeFileAccess::Read));
    shxFile.ReadEntireFile(data);
    shxFile.Close();

    bvector<FontFace> faces;
    faces.emplace_back(FontFace(FontType::Shx, "Cdm"));

    EXPECT_TRUE(SUCCESS == dbFonts.m_fontDb.EmbedFont(faces, data, false));
    EXPECT_TRUE(SUCCESS != dbFonts.m_fontDb.EmbedFont(faces, data, false)); // should not import if already there

    auto font = dbFonts.m_fontDb.FindFont(FontType::Shx, "Cdm");
    EXPECT_TRUE(nullptr != font);
    testFont(*font);
}

TEST_F(FontTests, RejectMalformedShxSubShapes) {
    SetupSeedProject();
    FontDbR fontDb = m_db->Fonts().m_fontDb;

    {
        ByteStream data = createLocaleShx({{'A', {0, 7, 'A', 0}}});
        DbFontP font = embedTestShx(fontDb, "SelfCycle", data);
        ASSERT_NE(nullptr, font);
        DbGlyphCP glyph = font->FindGlyphCP('A', FaceStyle::Regular);
        ASSERT_NE(nullptr, glyph);
        EXPECT_TRUE(glyph->GetCurveVector().IsValid());
    }

    {
        ByteStream data = createLocaleShx({
            {'A', {0, 7, 'B', 0}},
            {'B', {0, 7, 'A', 0}},
        });
        DbFontP font = embedTestShx(fontDb, "MutualCycle", data);
        ASSERT_NE(nullptr, font);
        DbGlyphCP glyph = font->FindGlyphCP('A', FaceStyle::Regular);
        ASSERT_NE(nullptr, glyph);
        EXPECT_TRUE(glyph->GetCurveVector().IsValid());
    }

    {
        bvector<TestShxGlyph> glyphs;
        bvector<uint16_t> chainIds;
        for (uint16_t id = 1; id <= 64; ++id)
            chainIds.push_back(id);
        for (uint16_t id = 66; id <= 71; ++id)
            chainIds.push_back(id);

        glyphs.emplace_back('A', bvector<Byte>{0, 0x10, 7, static_cast<Byte>(chainIds[0]), 2, 0});
        for (size_t i = 0; i < chainIds.size(); ++i) {
            if (i + 1 == chainIds.size())
                glyphs.emplace_back(chainIds[i], bvector<Byte>{0, 0x10, 2, 0});
            else
                glyphs.emplace_back(chainIds[i], bvector<Byte>{0, 0x10, 7, static_cast<Byte>(chainIds[i + 1]), 2, 0});
        }

        ByteStream data = createLocaleShx(glyphs);
        DbFontP font = embedTestShx(fontDb, "ExcessiveDepth", data);
        ASSERT_NE(nullptr, font);
        DbGlyphCP glyph = font->FindGlyphCP('A', FaceStyle::Regular);
        ASSERT_NE(nullptr, glyph);
        CurveVectorPtr curves = glyph->GetCurveVector();
        ASSERT_TRUE(curves.IsValid());
        DRange3d range;
        ASSERT_TRUE(curves->GetRange(range));
        EXPECT_DOUBLE_EQ(64.0, range.high.x);
    }

    {
        ByteStream data = createLocaleShx({
            {'A', {0, 7, 'B', 7, 'B', 0}},
            {'B', {0, 0x10, 2, 0}},
        });
        DbFontP font = embedTestShx(fontDb, "RepeatedSubShape", data);
        ASSERT_NE(nullptr, font);
        DbGlyphCP glyph = font->FindGlyphCP('A', FaceStyle::Regular);
        ASSERT_NE(nullptr, glyph);
        CurveVectorPtr curves = glyph->GetCurveVector();
        ASSERT_TRUE(curves.IsValid());
        DRange3d range;
        ASSERT_TRUE(curves->GetRange(range));
        EXPECT_DOUBLE_EQ(2.0, range.high.x);
    }

    {
        ByteStream data = createLocaleShx({{'A', {0, 0}}}, UINT16_MAX);
        DbFontP font = embedTestShx(fontDb, "TruncatedRecord", data);
        ASSERT_NE(nullptr, font);
        EXPECT_EQ(nullptr, font->FindGlyphCP('A', FaceStyle::Regular));
    }

    {
        ByteStream data = createLocaleShx({{'A', {0, 0x10, 2, 8, 1}}});
        DbFontP font = embedTestShx(fontDb, "TruncatedOpcode", data);
        ASSERT_NE(nullptr, font);
        DbGlyphCP glyph = font->FindGlyphCP('A', FaceStyle::Regular);
        ASSERT_NE(nullptr, glyph);
        CurveVectorPtr curves = glyph->GetCurveVector();
        ASSERT_TRUE(curves.IsValid());
        DRange3d range;
        ASSERT_TRUE(curves->GetRange(range));
        EXPECT_DOUBLE_EQ(1.0, range.high.x);
    }

    {
        bvector<Byte> glyphData = {0};
        glyphData.insert(glyphData.end(), 4096, 14);
        glyphData.push_back(0x10);
        glyphData.push_back(2);
        glyphData.push_back(0);

        ByteStream data = createLocaleShx({{'A', std::move(glyphData)}});
        DbFontP font = embedTestShx(fontDb, "VerticalChain", data);
        ASSERT_NE(nullptr, font);
        DbGlyphCP glyph = font->FindGlyphCP('A', FaceStyle::Regular);
        ASSERT_NE(nullptr, glyph);
        EXPECT_TRUE(glyph->GetCurveVector().IsValid());
    }

    {
        ByteStream data = createLocaleShx({
            {'A', {0, 9, 1}},
            {'B', {0, 13, 1, 1}},
        });
        DbFontP font = embedTestShx(fontDb, "TruncatedSequences", data);
        ASSERT_NE(nullptr, font);
        DbGlyphCP glyphA = font->FindGlyphCP('A', FaceStyle::Regular);
        DbGlyphCP glyphB = font->FindGlyphCP('B', FaceStyle::Regular);
        ASSERT_NE(nullptr, glyphA);
        ASSERT_NE(nullptr, glyphB);
        EXPECT_TRUE(glyphA->GetCurveVector().IsValid());
        EXPECT_TRUE(glyphB->GetCurveVector().IsValid());
    }
}

/** tests for FontManager */
TEST_F(FontTests, DefaultFonts) {
    SetupSeedProject();
    testDefaultFont(FontType::TrueType);
    testDefaultFont(FontType::Rsc);
    testDefaultFont(FontType::Shx);
}

/** tests for embedding system fonts */
TEST_F(FontTests, EmbeddingSystemFonts) {
    SetupSeedProject();
    auto& dbFonts = m_db->Fonts();
    auto& fontDb = dbFonts.m_fontDb;

    SystemTrueTypeFont fontThatShouldBeFound("Arial", true);
#if defined (BENTLEY_WIN32)
    EXPECT_TRUE(fontThatShouldBeFound.Embed(fontDb));
#else
    EXPECT_FALSE(fontThatShouldBeFound.Embed(fontDb)); // non-windows can't embed "system" fonts
#endif

    SystemTrueTypeFont fontThatShouldNOTBeFound("I Don't Exist", true);
    EXPECT_FALSE(fontThatShouldNOTBeFound.Embed(fontDb));
}

TEST_F(FontTests, IsRscFontData)
    {
    // Compressed FONT060 data from RscFonts.itwin-workspace, uncompressed size 1040 bytes
    const Byte compressedData[375] = {
        0x78, 0x5E, 0xCD, 0xD3, 0x4D, 0x4E, 0xC2, 0x40, 0x14, 0xC0, 0xF1, 0x37, 0x2D, 0xB5, 0xD4, 0xDA,
        0x00, 0x22, 0x01, 0x82, 0x04, 0x8D, 0x1A, 0x63, 0x8C, 0x11, 0x35, 0x7E, 0xC5, 0x18, 0xBF, 0x13,
        0x96, 0x2E, 0xDD, 0xFA, 0x81, 0x2B, 0xA3, 0x89, 0x61, 0xCF, 0x01, 0xDC, 0x78, 0x09, 0x0E, 0x60,
        0xE2, 0x2D, 0xBC, 0x82, 0x87, 0x70, 0xE9, 0x06, 0xF0, 0xFF, 0x4A, 0x11, 0x6D, 0xBA, 0x70, 0xE9,
        0x90, 0x5F, 0x67, 0xFA, 0x66, 0xDA, 0x37, 0x33, 0x4C, 0x4B, 0xA2, 0x65, 0x52, 0xA6, 0xA5, 0x42,
        0x3D, 0x25, 0x29, 0xC9, 0xCA, 0x84, 0xA4, 0x89, 0x88, 0x9C, 0x5B, 0x1A, 0x11, 0xF9, 0x40, 0xC3,
        0x88, 0xB4, 0xEC, 0x70, 0xB0, 0x79, 0xE7, 0x72, 0x24, 0x3D, 0xAE, 0x6A, 0x50, 0x02, 0x1C, 0xC7,
        0x62, 0x2E, 0xB1, 0x93, 0x58, 0x4C, 0xC7, 0x9E, 0xC6, 0x62, 0xA4, 0x91, 0xB3, 0x58, 0x8C, 0x74,
        0x72, 0x99, 0x90, 0xE3, 0x2A, 0x21, 0xC7, 0x75, 0x42, 0x8E, 0x9B, 0x84, 0x1C, 0xCD, 0x84, 0x1C,
        0xDD, 0xFE, 0xEF, 0x75, 0xE8, 0xDD, 0x45, 0xB4, 0xA6, 0x79, 0xEA, 0xA1, 0x46, 0xD4, 0xBE, 0x8B,
        0xEA, 0xE7, 0xA8, 0xD6, 0xA1, 0x7F, 0x19, 0xF7, 0x1A, 0xAE, 0x5B, 0xE4, 0x33, 0x7A, 0x77, 0x49,
        0x37, 0x12, 0x96, 0x38, 0xE1, 0x3E, 0xB6, 0xF9, 0x69, 0xD1, 0xD6, 0x70, 0x5F, 0xFF, 0xDB, 0x98,
        0x9D, 0x70, 0xBE, 0x3A, 0x6F, 0x9D, 0xB3, 0xCE, 0xB8, 0xF7, 0x63, 0xE6, 0x6D, 0xE2, 0x2E, 0x7B,
        0xF7, 0x22, 0x8F, 0xF2, 0x26, 0x4F, 0xE8, 0xD0, 0xEE, 0xD0, 0x6A, 0x4B, 0x13, 0xEB, 0x52, 0x67,
        0xC1, 0x86, 0x17, 0x18, 0xCE, 0x91, 0x05, 0x93, 0x12, 0xB1, 0x61, 0x78, 0x9D, 0x05, 0x1B, 0x29,
        0x98, 0x31, 0x11, 0x07, 0x26, 0x4D, 0x0C, 0x0E, 0x5C, 0x98, 0x71, 0xEE, 0xE1, 0xC2, 0x83, 0xC9,
        0x72, 0x0F, 0x07, 0x2E, 0x3C, 0xF8, 0x08, 0x90, 0x81, 0x99, 0xA1, 0x1F, 0x0E, 0x5C, 0x78, 0xF0,
        0x11, 0x20, 0x83, 0x1C, 0xF2, 0x28, 0xA0, 0x88, 0x32, 0x2A, 0xA8, 0xA2, 0x06, 0x73, 0xC8, 0xF3,
        0x70, 0xE0, 0xC2, 0x83, 0x8F, 0x00, 0x19, 0xE4, 0x90, 0x47, 0x01, 0x45, 0x94, 0x51, 0x41, 0x15,
        0x35, 0xCC, 0x62, 0x0E, 0x0B, 0x58, 0xC4, 0x12, 0x96, 0xB1, 0x82, 0x55, 0xAC, 0x61, 0x03, 0x9B,
        0xD8, 0xC6, 0x2E, 0xF6, 0xB0, 0x8F, 0x03, 0x68, 0xD1, 0xF3, 0x66, 0xF4, 0xA0, 0x7C, 0x17, 0x23,
        0x9C, 0x8B, 0xA8, 0x77, 0x10, 0xF4, 0xF9, 0x1A, 0xBB, 0xFD, 0x6E, 0x7F, 0x38, 0x44, 0xFF, 0x95,
        0xD1, 0x97, 0x39, 0x7A, 0x92, 0xAD, 0x4E, 0x28, 0x69, 0xB9, 0x7D, 0xB8, 0x6F, 0xD5, 0xB7, 0xEA,
        0x61, 0xDF, 0x17, 0x02, 0xDC, 0x64, 0xC9
        };
    uLong dataSize, uncompressedSize = 1040;
    bvector<Byte> data;
    data.resize(uncompressedSize);
    EXPECT_EQ(Z_OK, uncompress(data.data(), &dataSize, compressedData, sizeof(compressedData)));
    EXPECT_EQ(dataSize, uncompressedSize);

    EXPECT_TRUE(RscFont::IsRscFontData(data.data(), data.size()));
    }
