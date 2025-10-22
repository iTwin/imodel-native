/*--------------------------------------------------------------------------------------+       22
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"

/**
 *
 */
struct FontTests : public DgnDbTestFixture {
};

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

  EXPECT_FALSE(dbFonts.FindId(FontType::TrueType, "Karla").IsValid());

  BeSQLite::Statement stmt;
  stmt.Prepare(*m_db, "SELECT Id,Name FROM dgn_Font");
  EXPECT_EQ(DbResult::BE_SQLITE_ROW, stmt.Step());
  auto fontIdInt = stmt.GetValueUInt64(0);
  EXPECT_NE(fontIdInt, 0);
  EXPECT_EQ(stmt.GetValueText(1), "Karla");

  dbFonts.Invalidate();

  auto fontId = dbFonts.FindId(FontType::TrueType, "Karla");
  EXPECT_TRUE(fontId.IsValid());
  EXPECT_EQ(&fbFont, &FontManager::GetFallbackFont(FontType::TrueType));
  EXPECT_NE(&fbFont, &dbFonts.FindFont(fontId));
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
