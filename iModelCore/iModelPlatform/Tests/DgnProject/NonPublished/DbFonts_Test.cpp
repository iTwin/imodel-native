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
