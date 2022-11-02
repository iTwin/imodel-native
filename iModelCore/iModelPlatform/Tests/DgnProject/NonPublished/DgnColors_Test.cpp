
/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"
#include <DgnPlatform/ColorBook.h>

/*---------------------------------------------------------------------------------**//**
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnColorTests : public DgnDbTestFixture
{
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnColorTests, ColorBook)
    {
    SetupSeedProject();

    DgnElementId elementId;
    ColorDef color1(255, 254, 253);
    ColorDef color2(2, 3, 33);
    ColorDef color3(2, 3, 33); // It is legal to have two colors with the same RGB value
    ColorDef color4(4, 4, 44);
    ColorDef color5(55, 5, 55);
    ColorDef color6(6, 6, 66);

    if (true)
       {
        DgnDbR db = GetDgnDb();
        DefinitionModelR dictionary = db.GetDictionaryModel();
        ColorBook colorBook(dictionary, "TestBook1");

        colorBook.AddColor("Color1", color1);
        colorBook.AddColor("Color2", color2);
        colorBook.AddColor("Color3", color3);
        colorBook.AddColor("Color4", color4);
        colorBook.AddColor("Color Five", color5);
        colorBook.AddColor("6", color6);

        //Set Descripion of ColorBook
        ASSERT_EQ(DgnDbStatus::Success, colorBook.SetDescription("Descr"));
        ASSERT_EQ("Descr",colorBook.GetDescription());

        ASSERT_TRUE(colorBook.Insert().IsValid());

        ASSERT_EQ(color1, colorBook.GetColor("Color1"));
        ASSERT_EQ(color2, colorBook.GetColor("Color2"));
        ASSERT_EQ(color3, colorBook.GetColor("Color3"));
        ASSERT_EQ(color4, colorBook.GetColor("Color4"));
        ASSERT_EQ(color5, colorBook.GetColor("Color Five"));
        ASSERT_EQ(color6, colorBook.GetColor("6"));

#if 0
        int colorCount = 0;
        for (Utf8StringCR colorName : colorBook.GetColorNames())
        {
        ColorDef color = colorBook.GetColor(colorName.c_str());
        ASSERT_NE(0, color.GetValue());
        ++colorCount;
        }
        ASSERT_EQ(6, colorCount);

        ASSERT_STREQ("Color1", colorBook.FindColorName(color1).c_str());
        ASSERT_STREQ("Color4", colorBook.FindColorName(color4).c_str());
        ASSERT_STREQ("6", colorBook.FindColorName(color6).c_str());
        ASSERT_TRUE(colorBook.FindColorName(ColorDef(19, 19, 19)).empty());
#endif
        //Remove color from ColorBook
        colorBook.RemoveColor("6");
//        ASSERT_TRUE(colorBook.FindColorName(ColorDef(6, 6, 66)).empty());
        DgnDbStatus stat;
        //Update ColorBook
        DgnElementCPtr persistentEl = colorBook.UpdateAndGet(&stat);
        ASSERT_EQ(DgnDbStatus::Success, stat);
        ASSERT_TRUE(persistentEl.IsValid());
        elementId = persistentEl->GetElementId();
        db.SaveChanges();
    }
    //Check what stored in DB
    BeFileName fileName = m_db->GetFileName();
    m_db->CloseDb();
    m_db = nullptr;
    OpenDb(m_db, fileName, Db::OpenMode::ReadWrite, true);
    {
    auto colorBook=m_db->Elements().GetForEdit<ColorBook>(elementId);
#if 0
    int colorCount = 0;
    for (Utf8StringCR colorName : colorBook->GetColorNames())
    {
        ColorDef color = colorBook->GetColor(colorName.c_str());
        ASSERT_NE(0, color.GetValue());
        ++colorCount;
    }
    ASSERT_EQ(5, colorCount);
    ASSERT_TRUE(colorBook->FindColorName(ColorDef(6, 6, 66)).empty());
#endif

    ASSERT_EQ(color1, colorBook->GetColor("Color1"));
    ASSERT_EQ(color2, colorBook->GetColor("Color2"));
    ASSERT_EQ(color3, colorBook->GetColor("Color3"));
    ASSERT_EQ(color4, colorBook->GetColor("Color4"));
    ASSERT_EQ(color5, colorBook->GetColor("Color Five"));

    }
    auto colorBook = m_db->Elements().GetForEdit<ColorBook>(elementId);
    ASSERT_EQ(DgnDbStatus::Success, colorBook->Delete());
    DgnCode code=ColorBook::CreateCode(m_db->GetDictionaryModel(), "TestBook1");
    ASSERT_FALSE(m_db->Elements().QueryElementIdByCode(code).IsValid());
    }

#define VERIFY_HSV_TO_RGB(R,G,B ,H,S,V) { \
    HsvColorDef hsvColor = {H,S,V}; \
    ColorDef convertedRGB = ColorUtil::FromHSV(hsvColor); \
    EXPECT_TRUE(ColorDef(R, G, B) == convertedRGB)<<"Expected RGB : "<<R<<":"<<G<<":"<<B<<"\nConverted RGB : "<<(int)convertedRGB.GetRed()<<":"<<(int)convertedRGB.GetGreen()<<":"<<(int)convertedRGB.GetBlue(); \
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnColorTests, HSV_TO_RGB)
    {
    // Black
    VERIFY_HSV_TO_RGB(/*RGB*/0, 0, 0,/*HSV*/0, 0, 0);

    // White
    VERIFY_HSV_TO_RGB(/*RGB*/255, 255, 255,/*HSV*/0, 0, 100);

    // Red
    VERIFY_HSV_TO_RGB(/*RGB*/255, 0, 0,/*HSV*/0, 100, 100);

    // Yellow
    VERIFY_HSV_TO_RGB(/*RGB*/255, 255, 0,/*HSV*/60, 100, 100);

    // Magenta
    VERIFY_HSV_TO_RGB(/*RGB*/255, 0, 255,/*HSV*/300, 100, 100);

    // Gray
    VERIFY_HSV_TO_RGB(/*RGB*/128, 128, 128,/*HSV*/0, 0, 50);
    }

#define VERIFY_RGB_TO_HSV(R,G,B ,H,S,V) { \
    HsvColorDef expectedHsv = {H,S,V}; \
    HsvColorDef convertedHsv =  ColorUtil::ToHSV(ColorDef(R, G, B)); \
    EXPECT_EQ(expectedHsv.hue           , convertedHsv.hue); \
    EXPECT_EQ(expectedHsv.saturation    , convertedHsv.saturation); \
    EXPECT_EQ(expectedHsv.value         , convertedHsv.value); \
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnColorTests, RGB_TO_HSV)
    {
    // Black
    VERIFY_RGB_TO_HSV(/*RGB*/0, 0, 0,/*HSV*/0, 0, 0);

    // White
    VERIFY_RGB_TO_HSV(/*RGB*/255, 255, 255,/*HSV*/0, 0, 100);

    // Red
    VERIFY_RGB_TO_HSV(/*RGB*/255, 0, 0,/*HSV*/0, 100, 100);

    // Yellow
    VERIFY_RGB_TO_HSV(/*RGB*/255, 255, 0,/*HSV*/60, 100, 100);

    // Magenta
    VERIFY_RGB_TO_HSV(/*RGB*/255, 0, 255,/*HSV*/300, 100, 100);

    // Gray
    VERIFY_RGB_TO_HSV(/*RGB*/128, 128, 128,/*HSV*/0, 0, 50);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnColorTests, ToRgbFactor)
    {
    //White
    ColorDef cc(255, 255, 255);
    RgbFactor rgbf=ColorUtil::ToRgbFactor(cc);
    ASSERT_TRUE(1 == rgbf.red);
    ASSERT_TRUE(1 == rgbf.green);
    ASSERT_TRUE(1 == rgbf.blue);

    // Black
    ColorDef cc2(0, 0, 0);
    rgbf = ColorUtil::ToRgbFactor(cc2);
    ASSERT_TRUE(0 == rgbf.red);
    ASSERT_TRUE(0 == rgbf.green);
    ASSERT_TRUE(0 == rgbf.blue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnColorTests, FromRgbFactor)
    {
    RgbFactor rgbf;
    rgbf.red   = 0;
    rgbf.blue  = 0;
    rgbf.green = 0;
    ColorDef cc = ColorUtil::FromRgbFactor(rgbf);
    ASSERT_TRUE(0 == cc.GetRed());
    ASSERT_TRUE(0 == cc.GetGreen());
    ASSERT_TRUE(0 == cc.GetBlue());

    rgbf.red = 1;
    rgbf.blue = 1;
    rgbf.green = 1;
    cc = ColorUtil::FromRgbFactor(rgbf);
    ASSERT_TRUE(255 == cc.GetRed());
    ASSERT_TRUE(255 == cc.GetGreen());
    ASSERT_TRUE(255 == cc.GetBlue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnColorTests, ToFloatRgb)
    {
    //White
    ColorDef cc(255, 255, 255);
    FPoint3d fp = ColorUtil::ToFloatRgb(cc);
    ASSERT_TRUE(1 == fp.x);
    ASSERT_TRUE(1 == fp.y);
    ASSERT_TRUE(1 == fp.z);

    // Black
    ColorDef cc2(0, 0, 0);
    fp = ColorUtil::ToFloatRgb(cc2);
    ASSERT_TRUE(0 == fp.x);
    ASSERT_TRUE(0 == fp.y);
    ASSERT_TRUE(0 == fp.z);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnColorTests, FromFloatRgb)
    {
    FPoint3d fp;
    fp.x = 0;
    fp.y = 0;
    fp.z = 0;
    ColorDef cc = ColorUtil::FromFloatRgb(fp);
    ASSERT_TRUE(0 == cc.GetRed());
    ASSERT_TRUE(0 == cc.GetGreen());
    ASSERT_TRUE(0 == cc.GetBlue());

    fp.x = 1;
    fp.y = 1;
    fp.z = 1;
    cc = ColorUtil::FromFloatRgb(fp);
    ASSERT_TRUE(255 == cc.GetRed());
    ASSERT_TRUE(255 == cc.GetGreen());
    ASSERT_TRUE(255 == cc.GetBlue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnColorTests, AdjustForContrast)
    {
     //White
     ColorDef fg(255,255,255);
     //Black
     ColorDef bg(0, 0, 0);
     //Red
     ColorDef ac(255, 0, 0);
     ColorDef cc = ColorUtil::AdjustForContrast(fg,bg,ac);
     ASSERT_TRUE(fg == cc);

     //Same background and foreground color
     cc = ColorUtil::AdjustForContrast(fg,fg,ac);
     ASSERT_TRUE(ColorDef(179,179,179) == cc);
     //Maroon
     fg.SetColors(128, 0, 0,0);
     //Red
     bg.SetColors(255,0,0,0);
     //Yellow
     ac.SetColors(255, 255, 0, 0);
     cc = ColorUtil::AdjustForContrast(fg, bg, ac);
     ASSERT_TRUE(ColorDef(128, 0, 0) == cc);
    }
