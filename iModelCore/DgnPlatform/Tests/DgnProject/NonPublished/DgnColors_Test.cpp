/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/NonPublished/DgnColors_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnTrueColor.h>
#include <DgnPlatform/ColorUtil.h>

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Umar.Hayat   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct DgnColorTests : public DgnDbTestFixture
{
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnColorTests, TrueColors)
    {
    DgnDbR db = GetDgnDb();

    DgnTrueColor color1(DgnTrueColor::CreateParams(db, ColorDef(255, 254, 253), "TestName1", "TestBook1"));
    EXPECT_TRUE(color1.Insert().IsValid());
    DgnTrueColorId colorId = color1.GetColorId();
    EXPECT_TRUE(colorId.IsValid());

    DgnTrueColor color2(DgnTrueColor::CreateParams(db, ColorDef(2, 3, 33), "Color2"));
    EXPECT_TRUE(color2.Insert().IsValid());
    auto colorId2 = color2.GetColorId();
    EXPECT_TRUE(colorId2.IsValid());

    // It is legal to have two colors with the same RGB value
    DgnTrueColor color3(DgnTrueColor::CreateParams(db, ColorDef(2,3,33), "Color3"));
    EXPECT_TRUE(color3.Insert().IsValid());
    auto colorId3 = color3.GetColorId();
    EXPECT_TRUE(colorId3.IsValid());

    // It is not legal to have two colors with the same book+color name
    DgnTrueColor color3_dup(DgnTrueColor::CreateParams(db, ColorDef(2,3,33), "Color3"));
    EXPECT_FALSE(color3_dup.Insert().IsValid());

    DgnTrueColor color1_dup(DgnTrueColor::CreateParams(db, ColorDef(5,54,3), "TestName1", "TestBook1"));
    EXPECT_FALSE(color1_dup.Insert().IsValid());

    DgnTrueColor color4(DgnTrueColor::CreateParams(db, ColorDef(4,3,33), "Color4"));
    EXPECT_TRUE(color4.Insert().IsValid());
    auto colorId4 = color4.GetColorId();
    EXPECT_TRUE(colorId4.IsValid());

    // Color count in TestBook1
    EXPECT_EQ(1, DgnTrueColor::QueryCount(db, "TestBook1"));

    EXPECT_EQ(4, DgnTrueColor::QueryCount(db));

    int i=0;
    for (auto& it : DgnTrueColor::MakeIterator(db))
        {
        if (it.GetId() == color1.GetColorId())
            {
            ++i;
            EXPECT_TRUE(it.GetColorDef() == color1.GetColorDef());
            EXPECT_TRUE(it.GetName() == color1.GetName());
            EXPECT_TRUE(it.GetBook() == color1.GetBook());
            }
        else if (it.GetId() == color2.GetColorId())
            {
            ++i;
            EXPECT_TRUE(it.GetColorDef() == color2.GetColorDef());
            EXPECT_TRUE(it.GetName() == color2.GetName());
            EXPECT_TRUE(it.GetBook() == color2.GetBook());
            }
        else if (it.GetId() == color3.GetColorId())
            {
            ++i;
            EXPECT_TRUE(it.GetColorDef() == color3.GetColorDef());
            EXPECT_TRUE(it.GetName() == color3.GetName());
            EXPECT_TRUE(it.GetBook() == color3.GetBook());
            }
        else if (it.GetId() == color4.GetColorId())
            {
            ++i;
            EXPECT_TRUE(it.GetColorDef() == color4.GetColorDef());
            EXPECT_TRUE(it.GetName() == color4.GetName());
            EXPECT_TRUE(it.GetBook() == color4.GetBook());
            }
        else
            EXPECT_TRUE(false); // too many entries in iterator
        }

    EXPECT_TRUE(4 == i);

    DgnTrueColorId matchingColorId = DgnTrueColor::FindMatchingColor(color1.GetColorDef(), db);
    EXPECT_TRUE(matchingColorId.IsValid());
    EXPECT_TRUE(color1.GetColorId() == matchingColorId);

    DgnTrueColorCPtr toFind = DgnTrueColor::QueryColor(colorId, db);
    EXPECT_TRUE(toFind.IsValid());
    EXPECT_TRUE(toFind->GetColorId() == color1.GetColorId());
    EXPECT_TRUE(toFind->GetColorDef() == color1.GetColorDef());
    EXPECT_TRUE(toFind->GetName() == color1.GetName());
    EXPECT_TRUE(toFind->GetBook() == color1.GetBook());

    toFind = DgnTrueColor::QueryColorByName("TestName1", "TestBook1", db);
    EXPECT_TRUE(toFind.IsValid());
    EXPECT_TRUE(toFind->GetColorId() == color1.GetColorId());
    EXPECT_TRUE(toFind->GetColorDef() == color1.GetColorDef());
    EXPECT_TRUE(toFind->GetName() == color1.GetName());
    EXPECT_TRUE(toFind->GetBook() == color1.GetBook());

    // No match Case
    EXPECT_FALSE(DgnTrueColor::FindMatchingColor(ColorDef(120, 120, 120), db).IsValid());
    // Color with same definition
    EXPECT_TRUE(DgnTrueColor::FindMatchingColor(ColorDef(2, 3, 33), db).IsValid());

    // Cannot update or delete
    auto cpColor4 = DgnTrueColor::QueryColor(colorId4, db);
    ASSERT_TRUE(cpColor4.IsValid());
    EXPECT_EQ(DgnDbStatus::DeletionProhibited, cpColor4->Delete());
    auto pColor4 = cpColor4->MakeCopy<DgnTrueColor>();
    DgnDbStatus updateStat;
    EXPECT_FALSE(pColor4->Update(&updateStat).IsValid());
    EXPECT_EQ(DgnDbStatus::WrongElement, updateStat);
    }

#define VERIFY_HSV_TO_RGB(R,G,B ,H,S,V) { \
    HsvColorDef hsvColor = {H,S,V}; \
    ColorDef convertedRGB = ColorUtil::FromHSV(hsvColor); \
    EXPECT_TRUE(ColorDef(R, G, B) == convertedRGB)<<"Expected RGB : "<<R<<":"<<G<<":"<<B<<"\nConverted RGB : "<<(int)convertedRGB.GetRed()<<":"<<(int)convertedRGB.GetGreen()<<":"<<(int)convertedRGB.GetBlue(); \
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Umar.Hayat                   01/16
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
* @bsimethod                                    Umar.Hayat                   01/16
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
* @bsimethod                                    Ridha.Malik                   08/16
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
* @bsimethod                                    Ridha.Malik                   08/16
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
* @bsimethod                                    Ridha.Malik                   08/16
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
* @bsimethod                                    Ridha.Malik                   08/16
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
* @bsimethod                                    Ridha.Malik                   08/16
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
