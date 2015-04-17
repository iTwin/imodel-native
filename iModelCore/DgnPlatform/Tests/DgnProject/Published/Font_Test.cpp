/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/DgnProject/Published/Font_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeTest.h>
#include <Bentley/BeStringUtilities.h>
#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnHandlers/ScopedDgnHost.h>
#include <DgnPlatform/DgnCore/DgnRscFont.h>

// Republish API:           bb re DgnPlatform:PublishedApi
// Rebuild API:             bb re DgnPlatformDll
// Republish seed files:    bb re UnitTests_Documents
// Rebuild test:            bb re DgnProjectUnitTests BeGTestExe
// All code:                bb re DgnPlatform:PublishedApi DgnPlatformDll DgnProjectUnitTests BeGTestExe
// Run test:                %SrcRoot%BeGTest\RunTests.py -ax64 --gtest_filter="DgnFontTests.*"

USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_SQLITE

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2010
//---------------------------------------------------------------------------------------
TEST (DgnFontTests, CompressRscFractions)
    {
    ScopedDgnHost autoDgnHost;
    WString compressedString;
    DgnDbTestDgnManager tdm (L"fonts.dgn.i.idgndb", __FILE__, Db::OPEN_Readonly);
    DgnRscFontCP engineeringFont = static_cast<DgnRscFontCP>(DgnFontManager::FindFont ("ENGINEERING", DgnFontType::Rsc, tdm.GetDgnProjectP()));
    ASSERT_TRUE (NULL != engineeringFont);
        
    // Engineering actually has some fraction glyphs, so be a little more thorough...
    engineeringFont->CompressFractions  (compressedString,  NULL);          EXPECT_EQ (0, compressedString.size ())                     << L"ENGINEERING did not correctly compress a string.";
    engineeringFont->CompressFractions  (compressedString,  L"");           EXPECT_EQ (0, compressedString.size ())                     << L"ENGINEERING did not correctly compress a string.";
    engineeringFont->CompressFractions  (compressedString,  L"A");          EXPECT_STREQ (L"A", compressedString.c_str ())              << L"ENGINEERING did not correctly compress a string.";
    engineeringFont->CompressFractions  (compressedString,  L"ABC");        EXPECT_STREQ (L"ABC", compressedString.c_str ())            << L"ENGINEERING did not correctly compress a string.";
    engineeringFont->CompressFractions  (compressedString,  L"123");        EXPECT_STREQ (L"123", compressedString.c_str ())            << L"ENGINEERING did not correctly compress a string.";
    engineeringFont->CompressFractions  (compressedString,  L"A/B");        EXPECT_STREQ (L"A/B", compressedString.c_str ())            << L"ENGINEERING did not correctly compress a string.";
    engineeringFont->CompressFractions  (compressedString,  L"1/2");        EXPECT_STREQ (L"\xbd", compressedString.c_str ())           << L"ENGINEERING did not correctly compress a string.";
    engineeringFont->CompressFractions  (compressedString,  L"1/16");       EXPECT_STREQ (L"\xe107", compressedString.c_str ())         << L"ENGINEERING did not correctly compress a string.";
    engineeringFont->CompressFractions  (compressedString,  L"1/");         EXPECT_STREQ (L"1/", compressedString.c_str ())             << L"ENGINEERING did not correctly compress a string.";
    engineeringFont->CompressFractions  (compressedString,  L"/2");         EXPECT_STREQ (L"/2", compressedString.c_str ())             << L"ENGINEERING did not correctly compress a string.";
    engineeringFont->CompressFractions  (compressedString,  L"1/A");        EXPECT_STREQ (L"1/A", compressedString.c_str ())            << L"ENGINEERING did not correctly compress a string.";
    engineeringFont->CompressFractions  (compressedString,  L"A1/2");       EXPECT_STREQ (L"A\xbd", compressedString.c_str ())          << L"ENGINEERING did not correctly compress a string.";
    engineeringFont->CompressFractions  (compressedString,  L"1/2B");       EXPECT_STREQ (L"\xbd" L"B", compressedString.c_str ())       << L"ENGINEERING did not correctly compress a string.";
    engineeringFont->CompressFractions  (compressedString,  L"AB1/2CD");    EXPECT_STREQ (L"AB\xbd" L"CD", compressedString.c_str ())    << L"ENGINEERING did not correctly compress a string.";
    engineeringFont->CompressFractions  (compressedString,  L"A1/3B");      EXPECT_STREQ (L"A1/3B", compressedString.c_str ())          << L"ENGINEERING did not correctly compress a string.";
    engineeringFont->CompressFractions  (compressedString,  L"1/2/2010");   EXPECT_STREQ (L"1/2/2010", compressedString.c_str ())       << L"ENGINEERING did not correctly compress a string.";
    engineeringFont->CompressFractions  (compressedString,  L"1-13/64\"");  EXPECT_STREQ (L"1-\xe125" L"\"", compressedString.c_str ())  << L"ENGINEERING did not correctly compress a string.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2010
//---------------------------------------------------------------------------------------
TEST (DgnFontTests, ExpandRscFractions)
    {
    ScopedDgnHost autoDgnHost;
    WString expandedString;        
    DgnDbTestDgnManager tdm (L"fonts.dgn.i.idgndb", __FILE__, Db::OPEN_Readonly);
    DgnRscFontCP engineeringFont = static_cast<DgnRscFontCP>(DgnFontManager::FindFont ("ENGINEERING", DgnFontType::Rsc, tdm.GetDgnProjectP()));
    ASSERT_TRUE (NULL != engineeringFont);
       
    // Engineering actually has some fraction glyphs, so be a little more thorough...
    // Note that passing a WChar means we need to send Unicode which it will then convert to locale and look up; don't pass the actual RSC fraction code.
    engineeringFont->ExpandFractions    (expandedString,    NULL);              EXPECT_EQ (0, expandedString.size ())                   << L"ENGINEERING did not correctly expand a string.";
    engineeringFont->ExpandFractions    (expandedString,    L"");               EXPECT_EQ (0, expandedString.size ())                   << L"ENGINEERING did not correctly expand a string.";
    engineeringFont->ExpandFractions    (expandedString,    L"A");              EXPECT_STREQ (L"A", expandedString.c_str ())            << L"ENGINEERING did not correctly expand a string.";
    engineeringFont->ExpandFractions    (expandedString,    L"ABC");            EXPECT_STREQ (L"ABC", expandedString.c_str ())          << L"ENGINEERING did not correctly expand a string.";
    engineeringFont->ExpandFractions    (expandedString,    L"\xbd");           EXPECT_STREQ (L"1/2", expandedString.c_str ())          << L"ENGINEERING did not correctly expand a string.";
    engineeringFont->ExpandFractions    (expandedString,    L"A\xbd" L"B");      EXPECT_STREQ (L"A1/2B", expandedString.c_str ())        << L"ENGINEERING did not correctly expand a string.";
    engineeringFont->ExpandFractions    (expandedString,    L"1-\xA6" L"\"");    EXPECT_STREQ (L"1-13/64\"", expandedString.c_str ())    << L"ENGINEERING did not correctly expand a string.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2010
//---------------------------------------------------------------------------------------
TEST (DgnFontTests, CompressEscapeSequences)
    {
    ScopedDgnHost autoDgnHost;
    WString expandedString;
    DgnDbTestDgnManager tdm (L"fonts.dgn.i.idgndb", __FILE__, Db::OPEN_Readonly);          
    DgnFontCP arialFont = DgnFontManager::FindFont ("Courier New", DgnFontType::TrueType, tdm.GetDgnProjectP());
    ASSERT_TRUE (NULL != arialFont);

    arialFont->CompressEscapeSequences  (expandedString,    NULL);              EXPECT_EQ (0, expandedString.size ())           << L"Arial did not correctly compress a string.";
    arialFont->CompressEscapeSequences  (expandedString,    L"");               EXPECT_EQ (0, expandedString.size ())           << L"Arial did not correctly compress a string.";
    arialFont->CompressEscapeSequences  (expandedString,    L"A");              EXPECT_STREQ (L"A", expandedString.c_str ())    << L"Arial did not correctly compress a string.";
    arialFont->CompressEscapeSequences  (expandedString,    L"123");            EXPECT_STREQ (L"123", expandedString.c_str ())  << L"Arial did not correctly compress a string.";
    arialFont->CompressEscapeSequences  (expandedString,    L"\\123");          EXPECT_STREQ (L"{", expandedString.c_str ())    << L"Arial did not correctly compress a string.";
    arialFont->CompressEscapeSequences  (expandedString,    L"A\\66C");         EXPECT_STREQ (L"ABC", expandedString.c_str ())  << L"Arial did not correctly compress a string.";
    arialFont->CompressEscapeSequences  (expandedString,    L"\\");             EXPECT_STREQ (L"\\", expandedString.c_str ())   << L"Arial did not correctly compress a string.";
    arialFont->CompressEscapeSequences  (expandedString,    L"\\\\");           EXPECT_STREQ (L"\\", expandedString.c_str ())   << L"Arial did not correctly compress a string.";
    arialFont->CompressEscapeSequences  (expandedString,    L"\\\\66");         EXPECT_STREQ (L"\\66", expandedString.c_str ()) << L"Arial did not correctly compress a string.";
    arialFont->CompressEscapeSequences  (expandedString,    L"\\\\\\66");       EXPECT_STREQ (L"\\B", expandedString.c_str ())  << L"Arial did not correctly compress a string.";
    arialFont->CompressEscapeSequences  (expandedString,    L"\\65\\66\\67");   EXPECT_STREQ (L"ABC", expandedString.c_str ())  << L"Arial did not correctly compress a string.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2010
//---------------------------------------------------------------------------------------
TEST(DgnFontTests, CompressEscapeSequencesRsc)
    {
    ScopedDgnHost autoDgnHost;
    WString expandedString;
    DgnDbTestDgnManager tdm (L"fonts.dgn.i.idgndb", __FILE__, Db::OPEN_Readonly);  
    DgnFontCP engineeringFont = DgnFontManager::FindFont ("ENGINEERING", DgnFontType::Rsc, tdm.GetDgnProjectP());
    ASSERT_TRUE (NULL != engineeringFont);        
    engineeringFont->CompressEscapeSequences (expandedString, L"\\134"); EXPECT_STREQ (L"\x215d", expandedString.c_str ()) << L"ENGINEERING did not correctly compress a string.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Jeff.Marker     11/2010
//---------------------------------------------------------------------------------------
TEST (DgnFontTests, ExpandEscapeSequences)
    {
    ScopedDgnHost autoDgnHost;
    WString expandedString;
    DgnDbTestDgnManager tdm (L"fonts.dgn.i.idgndb", __FILE__, Db::OPEN_Readonly);    
    DgnFontCP arialFont = DgnFontManager::FindFont ("Courier New", DgnFontType::TrueType, tdm.GetDgnProjectP());
    ASSERT_TRUE (NULL != arialFont);

    arialFont->ExpandEscapeSequences    (expandedString,    NULL);      EXPECT_EQ (0, expandedString.size ())               << L"Arial did not correctly expand a string.";
    arialFont->ExpandEscapeSequences    (expandedString,    L"");       EXPECT_EQ (0, expandedString.size ())               << L"Arial did not correctly expand a string.";
    arialFont->ExpandEscapeSequences    (expandedString,    L"A");      EXPECT_STREQ (L"A", expandedString.c_str ())        << L"Arial did not correctly expand a string.";
    arialFont->ExpandEscapeSequences    (expandedString,    L"123");    EXPECT_STREQ (L"123", expandedString.c_str ())      << L"Arial did not correctly expand a string.";
    arialFont->ExpandEscapeSequences    (expandedString,    L"\\");     EXPECT_STREQ (L"\\\\", expandedString.c_str ())     << L"Arial did not correctly expand a string.";
    arialFont->ExpandEscapeSequences    (expandedString,    L"A\\B");   EXPECT_STREQ (L"A\\\\B", expandedString.c_str ())   << L"Arial did not correctly expand a string.";
    arialFont->ExpandEscapeSequences    (expandedString,    L"\\A");    EXPECT_STREQ (L"\\\\A", expandedString.c_str ())    << L"Arial did not correctly expand a string.";
    arialFont->ExpandEscapeSequences    (expandedString,    L"B\\");    EXPECT_STREQ (L"B\\\\", expandedString.c_str ())    << L"Arial did not correctly expand a string.";
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Julija.Suboc     05/2012
//---------------------------------------------------------------------------------------
TEST(DgnFontTests, GetFonts)
    {
    ScopedDgnHost autoDgnHost;
    DgnDbTestDgnManager tdm (L"fonts.dgn.i.idgndb", __FILE__, Db::OPEN_Readonly);    
    DgnFontVariant variants [] = {DGNFONTVARIANT_Rsc, DGNFONTVARIANT_TrueType, 
    DGNFONTVARIANT_ShxShape, DGNFONTVARIANT_ShxUni};
    size_t extractedFontCount = 0;
    for(int i = 0; i < 4; i++)
        {
        T_DgnFontCPs fontList = DgnFontManager::GetFonts(variants[i], tdm.GetDgnProjectP()); 
        extractedFontCount += fontList.size();
        }
    T_DgnFontCPs allFontList = DgnFontManager::GetFonts(DGNFONTVARIANT_DontCare, tdm.GetDgnProjectP()); 
    size_t totalFonts = allFontList.size();
    EXPECT_EQ(totalFonts, extractedFontCount)<<"Font count does not match.";
    EXPECT_EQ(DGNFONTVARIANT_Invalid, DGNFONTVARIANT_DontCare);    
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Julija.Suboc     05/2012
//---------------------------------------------------------------------------------------
TEST(DgnFontTests, GetTypeVariantAndName)
    {
    ScopedDgnHost autoDgnHost;
    DgnDbTestDgnManager tdm (L"fonts.dgn.i.idgndb", __FILE__, Db::OPEN_Readonly); 
    Utf8CP fontName= "ENGINEERING";
    DgnFontType fontType = DgnFontType::Rsc;
    //Find Rsc fond and check if name and type matches
    DgnFontCP font = DgnFontManager::FindFont(fontName, fontType, tdm.GetDgnProjectP()); 
    ASSERT_TRUE (NULL != font)<<"Font was not found";
    ASSERT_TRUE(fontType == font->GetType())<<"Type does not match";
    ASSERT_EQ(DGNFONTVARIANT_Rsc, font->GetVariant())<<"Variant does not match";
    Utf8String extractedName =  font->GetName();
    int cmp = extractedName.CompareTo(fontName);
    ASSERT_EQ(0, cmp)<<"Name does not match";
    ASSERT_FALSE(font->IsMissing());
    ASSERT_FALSE(font->IsLastResort());

    //Find and check trueType font
    Utf8CP fontName2= "Courier New";
    DgnFontType fontType2 = DgnFontType::TrueType;
    font = DgnFontManager::FindFont(fontName2, fontType2, tdm.GetDgnProjectP()); 
    ASSERT_TRUE (NULL != font)<<"Font was not found";
    ASSERT_TRUE(fontType2 == font->GetType())<<"Type does not match";
    ASSERT_EQ(DGNFONTVARIANT_TrueType, font->GetVariant())<<"Variant does not match";
    Utf8String extractedName2 =  font->GetName();
    cmp = extractedName2.CompareTo(fontName2);
    ASSERT_EQ(0, cmp)<<"Name does not match";
    ASSERT_FALSE(font->IsMissing());
    ASSERT_FALSE(font->IsLastResort());
    Utf8String fd; 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Julija.Suboc     05/2012
//---------------------------------------------------------------------------------------
TEST(DgnFontTests, ConvertFontVariantToType)
    {
    ScopedDgnHost autoDgnHost;
    DgnFontType type = DgnFontManager::ConvertFontVariantToType(DGNFONTVARIANT_Rsc);
    EXPECT_TRUE(DgnFontType::Rsc == type);
    type = DgnFontManager::ConvertFontVariantToType(DGNFONTVARIANT_TrueType);
    EXPECT_TRUE(DgnFontType::TrueType == type);
    type = DgnFontManager::ConvertFontVariantToType(DGNFONTVARIANT_Rsc);
    EXPECT_TRUE(DgnFontType::Rsc == type);
    type = DgnFontManager::ConvertFontVariantToType(DGNFONTVARIANT_ShxShape);
    EXPECT_TRUE(DgnFontType::Shx == type);
    type = DgnFontManager::ConvertFontVariantToType(DGNFONTVARIANT_ShxUni);
    EXPECT_TRUE(DgnFontType::Shx == type);
    //When passing Invalid or DontCare variant, invalid type should be returned. 
    // This would normally trigger an assertion failure.
    BeTest::SetFailOnAssert (false);
    type = DgnFontManager::ConvertFontVariantToType(DGNFONTVARIANT_Invalid);
    EXPECT_TRUE(DgnFontType::None == type);
    type = DgnFontManager::ConvertFontVariantToType(DGNFONTVARIANT_DontCare);
    EXPECT_TRUE(DgnFontType::None == type);
    BeTest::SetFailOnAssert (true);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Julija.Suboc     05/2012
//---------------------------------------------------------------------------------------
TEST(DgnFontTests, GetFontAndConvertFontVariantToType)
    {
    ScopedDgnHost autoDgnHost;
    DgnDbTestDgnManager tdm (L"fonts.dgn.i.idgndb", __FILE__, Db::OPEN_Readonly); 
    Utf8CP fontName= "ENGINEERING";
    DgnFontType fontType = DgnFontType::Rsc;
    //Find Rsc fond and check if name and type matches
    DgnFontCP font = DgnFontManager::FindFont(fontName, fontType, tdm.GetDgnProjectP()); 
    ASSERT_TRUE (NULL != font)<<"Font was not found";
     DgnFontType type = DgnFontManager::ConvertFontVariantToType(font->GetVariant());
    EXPECT_TRUE(fontType == type);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Julija.Suboc     05/2012
//---------------------------------------------------------------------------------------
TEST(DgnFontTests, ResolveFonts)
    {
    ScopedDgnHost autoDgnHost;
    DgnDbTestDgnManager tdm (L"fonts.dgn.i.idgndb", __FILE__, Db::OPEN_Readonly); 

    //Resolve some fonts in fonts.dgn.i.dgndb
    DgnFontCP font = &DgnFontManager::ResolveFont(3, *tdm.GetDgnProjectP(), DGNFONTVARIANT_Rsc);
    Utf8String extractedFontName = font->GetName();
    int cmp = extractedFontName.CompareTo("ENGINEERING");
    EXPECT_EQ(0, cmp)<<"Expected font: ENGINEERING, extracted: "<<extractedFontName;
    EXPECT_EQ(DGNFONTVARIANT_Rsc, font->GetVariant());

    font = &DgnFontManager::ResolveFont(41, *tdm.GetDgnProjectP(), DGNFONTVARIANT_Rsc);
    extractedFontName = font->GetName();
    cmp = extractedFontName.CompareTo("ARCHITECTURAL");
    EXPECT_EQ(0, cmp)<<"Expected font: ARCHITECTURAL, extracted: "<<extractedFontName;
    EXPECT_EQ(DGNFONTVARIANT_Rsc, font->GetVariant());

    font = &DgnFontManager::ResolveFont(1024, *tdm.GetDgnProjectP(), DGNFONTVARIANT_TrueType);
    extractedFontName = font->GetName();
    cmp = extractedFontName.CompareTo("Courier New");
    EXPECT_EQ(0, cmp)<<"Expected font: Courier New, extracted: "<<extractedFontName;
    EXPECT_EQ(DGNFONTVARIANT_TrueType, font->GetVariant());
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Julija.Suboc     05/2012
//---------------------------------------------------------------------------------------
TEST(DgnFontTests, CreateDgnFontKey)
    {
    DgnFontType type = DgnFontType::Rsc;
    Utf8String name = "TestKey";
    DgnFontKey key = DgnFontKey(type, name);
    ASSERT_TRUE(type == key.m_type)<<"Key was created with other type";
    int cmp = name.CompareTo(key.m_name);
    ASSERT_EQ(0,  cmp)<<"Key was created with other name than expected";
    }
