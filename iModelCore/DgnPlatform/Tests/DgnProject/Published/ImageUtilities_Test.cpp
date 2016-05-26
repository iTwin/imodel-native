/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/DgnProject/Published/ImageUtilities_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"

USING_NAMESPACE_BENTLEY_RENDER

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      04/2013
//---------------------------------------------------------------------------------------
TEST (ImageUtilities_Tests, Png)
    {
    uint32_t width = 100;
    uint32_t height = 200;

    ByteStream testImage(height * width * 4);
    Byte* p=testImage.GetDataP();
    for (uint8_t y = 0; y<height; ++y)
        {
        for (uint8_t x = 0; x<width; ++x)
            {
            *p++ = (y%256); // R
            *p++ = (x%256); // G
            *p++ = (0xff);  // B
            *p++ = (0xff);  // A
            }
        }

    Image image(width, height, Image::Format::Rgba, std::move(testImage));
    ASSERT_TRUE(image.IsValid());

    ImageSource pngImg(ImageSource::Format::Png, image);
    ASSERT_TRUE(pngImg.IsValid());
    ASSERT_TRUE(pngImg.HasAlpha());
    ASSERT_FALSE(pngImg.IsBGR());
    ASSERT_TRUE(pngImg.IsTopDown());
    ASSERT_TRUE(pngImg.GetFormat()==ImageSource::Format::Png);

    Image image2(pngImg);
    ASSERT_TRUE(image2.IsValid());

    ASSERT_EQ(width, image2.GetWidth());
    ASSERT_EQ(height, image2.GetHeight());
    ASSERT_TRUE(Image::Format::Rgba == image2.GetFormat());

    ASSERT_EQ(image2.GetByteStream().GetSize(), image.GetByteStream().GetSize());
    ASSERT_TRUE( 0==memcmp(image.GetByteStream().GetData(), image2.GetByteStream().GetData(), image2.GetByteStream().GetSize()) ); // Since our input was RGBA, there was no transformation on the way out to the file.
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Umar.Hayat      11/2015
//---------------------------------------------------------------------------------------
TEST (ImageUtilities_Tests, JPG)
    {
    uint32_t width = 100;
    uint32_t height = 200;

    ByteStream testImage(height * width * 4);
    Byte* p=testImage.GetDataP();

    for (uint8_t y = 0; y<height; ++y)
        {
        for (uint8_t x = 0; x<width; ++x)
            {
            *p++ =  (255-y%256); // R
            *p++ = ((x % 256) ^ 0xFF ); // G
            *p++ = (0xff);  // B
            }
        }

    Image image(width, height, Image::Format::Rgb, std::move(testImage));

    ImageSource jpgImg(ImageSource::Format::Jpeg, image, 100);
    ASSERT_TRUE(jpgImg.IsValid());
    ASSERT_FALSE(jpgImg.HasAlpha());
    ASSERT_FALSE(jpgImg.IsBGR());
    ASSERT_TRUE(jpgImg.IsTopDown());
    ASSERT_TRUE(jpgImg.GetFormat()==ImageSource::Format::Jpeg);

    Image image2(jpgImg);
    ASSERT_TRUE(image2.IsValid());

    ASSERT_EQ(width, image2.GetWidth());
    ASSERT_EQ(height, image2.GetHeight());
    ASSERT_TRUE(Image::Format::Rgb == image2.GetFormat());

    // Why image is not same , qaulity was 100 so it should transform or change any thing
    //ASSERT_TRUE(imageRead == testImage); // Since our input was RGBA, there was no transformation on the way out to the file.
    }
