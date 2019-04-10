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

    ByteStream testImage(height * width * 3);
    Byte* p=testImage.GetDataP();
    for (uint8_t y = 0; y<height; ++y)
        {
        for (uint8_t x = 0; x<width; ++x)
            {
            *p++ = (y%256); // R
            *p++ = (x%256); // G
            *p++ = (0x33);  // B
            }
        }

    Image image(width, height, std::move(testImage), Image::Format::Rgb);
    ASSERT_TRUE(image.IsValid());

    ImageSource pngImg(image, ImageSource::Format::Png);
    ASSERT_TRUE(pngImg.IsValid());
    ASSERT_TRUE(pngImg.GetFormat()==ImageSource::Format::Png);

    Image image2(pngImg, Image::Format::Rgb);
    ASSERT_TRUE(image2.IsValid());

    ASSERT_EQ(width, image2.GetWidth());
    ASSERT_EQ(height, image2.GetHeight());
    ASSERT_TRUE(Image::Format::Rgb == image2.GetFormat());

    ASSERT_EQ(image2.GetByteStream().GetSize(), image.GetByteStream().GetSize());
    // image -> PNG -> image should be lossless
    ASSERT_TRUE(0==memcmp(image.GetByteStream().GetData(), image2.GetByteStream().GetData(), image2.GetByteStream().GetSize())); 

    Image image3(pngImg, Image::Format::Rgba);
    ASSERT_EQ(width, image3.GetWidth());
    ASSERT_EQ(height, image3.GetHeight());
    ASSERT_TRUE(Image::Format::Rgba == image3.GetFormat());

    // we started with Rgb and converted to Rgba. Make sure the data is the same and we get 0xff for alpha
    Byte const* src=image.GetByteStream().GetData();
    Byte const* dst=image3.GetByteStream().GetData();
    for (uint8_t y = 0; y<height; ++y)
        {
        for (uint8_t x = 0; x<width; ++x)
            {
            ASSERT_TRUE(*src++ == *dst++);
            ASSERT_TRUE(*src++ == *dst++);
            ASSERT_TRUE(*src++ == *dst++);
            ASSERT_TRUE(0xff == *dst++);
            }
        }

    ImageSource png2(image3, ImageSource::Format::Png);
    ASSERT_TRUE(png2.IsValid());
    ASSERT_TRUE(png2.GetFormat()==ImageSource::Format::Png);
    ASSERT_TRUE(png2.GetSize().x == image3.GetWidth());
    ASSERT_TRUE(png2.GetSize().y == image3.GetHeight());

    Image image4(png2, Image::Format::Rgb);
    ASSERT_TRUE(image4.IsValid());
    ASSERT_EQ(width, image4.GetWidth());
    ASSERT_EQ(height, image4.GetHeight());
    ASSERT_TRUE(Image::Format::Rgb == image2.GetFormat());
    ASSERT_EQ(image4.GetByteStream().GetSize(), image.GetByteStream().GetSize());
    ASSERT_TRUE( 0==memcmp(image.GetByteStream().GetData(), image4.GetByteStream().GetData(), image.GetByteStream().GetSize()) ); 

    Image image5(pngImg, Image::Format::Rgba);
    ASSERT_EQ(width, image5.GetWidth());
    ASSERT_EQ(height, image5.GetHeight());
    ASSERT_TRUE(Image::Format::Rgba == image3.GetFormat());
    ASSERT_EQ(image5.GetByteStream().GetSize(), image3.GetByteStream().GetSize());
    ASSERT_TRUE(0==memcmp(image5.GetByteStream().GetData(), image3.GetByteStream().GetData(), image3.GetByteStream().GetSize())); 

    Image image_png = Image::FromPng(pngImg.GetByteStream().GetData(), pngImg.GetByteStream().GetSize(), Image::Format::Rgb);
    ASSERT_TRUE(image_png.IsValid());
    ASSERT_EQ(width, image_png.GetWidth());
    ASSERT_EQ(height, image_png.GetHeight());
    ASSERT_EQ(image_png.GetByteStream().GetSize(), image.GetByteStream().GetSize());
    ASSERT_TRUE(0 == memcmp(image_png.GetByteStream().GetData(), image.GetByteStream().GetData(), image.GetByteStream().GetSize()));
    Byte const* src1 = image.GetByteStream().GetData();
    Byte const* dst1 = image_png.GetByteStream().GetData();
    for (uint8_t y = 0; y<height; ++y)
    {
        for (uint8_t x = 0; x<width; ++x)
        {
            ASSERT_TRUE(*src1++ == *dst1++);
            ASSERT_TRUE(*src1++ == *dst1++);
            ASSERT_TRUE(*src1++ == *dst1++);
        }
    }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Umar.Hayat      11/2015
//---------------------------------------------------------------------------------------
TEST (ImageUtilities_Tests, JPG)
    {
    uint32_t width = 100;
    uint32_t height = 200;

    ByteStream testImage(height * width * 3);
    Byte* p=testImage.GetDataP();

    for (uint8_t y = 0; y<height; ++y)
        {
        for (uint8_t x = 0; x<width; ++x)
            {
            *p++ = (y%256); // R
            *p++ = (x%256); // G
            *p++ = (0x33);  // B
            }
        }

    Image image(width, height, std::move(testImage), Image::Format::Rgb);
    ImageSource jpgImg(image, ImageSource::Format::Jpeg, 100);
    ASSERT_TRUE(jpgImg.IsValid());
    ASSERT_TRUE(jpgImg.GetFormat()==ImageSource::Format::Jpeg);

    Image image2(jpgImg, Image::Format::Rgb);
    ASSERT_TRUE(image2.IsValid());

    ASSERT_EQ(width, image2.GetWidth());
    ASSERT_EQ(height, image2.GetHeight());
    ASSERT_TRUE(Image::Format::Rgb == image2.GetFormat());

    Image image3(jpgImg, Image::Format::Rgba);
    ASSERT_EQ(width, image3.GetWidth());
    ASSERT_EQ(height, image3.GetHeight());
    ASSERT_TRUE(Image::Format::Rgba == image3.GetFormat());

    // the outcome of an image -> Jpeg -> image isn't guaranteed to be the same, but check that we get the 
    // same bytes from our Rgb and Rgba conversions
    Byte const* src=image2.GetByteStream().GetData();
    Byte const* dst=image3.GetByteStream().GetData();
    for (uint8_t y = 0; y<height; ++y)
        {
        for (uint8_t x = 0; x<width; ++x)
            {
            ASSERT_TRUE(*src++ == *dst++);
            ASSERT_TRUE(*src++ == *dst++);
            ASSERT_TRUE(*src++ == *dst++);
            ASSERT_TRUE(0xff == *dst++);
            }
        }
    Image image_jpeg = Image::FromJpeg(jpgImg.GetByteStream().GetData(), jpgImg.GetByteStream().GetSize(), Image::Format::Rgb, Image::BottomUp::No);
    ASSERT_TRUE(image_jpeg.IsValid());
    }
