/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <Bentley/BeDirectoryIterator.h>

USING_NAMESPACE_BENTLEY_RENDER

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
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

#if defined(RUN_TRANSPARENCY_TESTS)
// This test was run to verify that ImageSource::SupportsTransparency produces correct results for all of the PNG files at http://www.schaik.com/pngsuite/pngsuite_trn_png.html
// and http://www.schaik.com/pngsuite/pngsuite_bas_png.html. It requires access to those files, hence it is not run by default.
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ImageSource, SupportsTransparency_SampleImages)
    {
    auto test = [](Utf8CP directory, bool expectTransparency)
        {
        BeFileName dirname(Utf8String("d:\\\\png\\\\") + directory);
        BeFileName filename;
        bool isDir;
        for (BeDirectoryIterator iter(dirname); SUCCESS == iter.GetCurrentEntry(filename, isDir); iter.ToNext())
            {
            if (isDir || !BeFileName::GetExtension(filename).EqualsI(L"png"))
                continue;

            BeFile file;
            ASSERT_EQ(BeFileStatus::Success, file.Open(filename, BeFileAccess::Read));

            ByteStream bytes;
            ASSERT_EQ(BeFileStatus::Success, file.ReadEntireFile(bytes));
            ImageSource src(ImageSource::Format::Png, std::move(bytes));
            EXPECT_TRUE(src.IsValid());

            EXPECT_EQ(src.SupportsTransparency(), expectTransparency);
        };
    };

    test("opaque", false);
    test("transp", true);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST(ImageSource, SupportsTransparency)
    {
    // Encode a 1x1 image. If alpha <= 255, use RGBA and set A=alpha; else use RGB.
    auto test = [](uint16_t alpha, bool expectTransparency)
        {
        auto wantAlpha = alpha <= 255;
        ByteStream bytes(wantAlpha ? 4 : 3);
        bytes[0] = bytes[1] = bytes[2] = 0xff;
        if (wantAlpha)
            bytes[3] = static_cast<uint8_t>(alpha);

        Image image(1, 1, std::move(bytes), wantAlpha ? Image::Format::Rgba : Image::Format::Rgb);
        ASSERT_TRUE(image.IsValid());

        ImageSource jpeg(image, ImageSource::Format::Jpeg);
        EXPECT_TRUE(jpeg.IsValid());
        EXPECT_FALSE(jpeg.SupportsTransparency());

        ImageSource png(image, ImageSource::Format::Png);
        EXPECT_TRUE(png.IsValid());
        EXPECT_EQ(png.SupportsTransparency(), expectTransparency);
        };

    // When we encode a PNG from an RGBA image we always enable alpha channel - even if no transparent pixels exist.
    test(0, true);
    test(0x7f, true);
    test(0xff, true);

    // When we encode a PNG from an RGB image we never enable alpha channel, because we know no transparent pixels exist.
    test(0xffff, false);
    }
