/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/DgnProject/Published/ImageUtilities_Test.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <DgnPlatform/ImageUtilities.h>

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      04/2013
//---------------------------------------------------------------------------------------
TEST (ImageUtilities_Tests, Png)
    {
    ByteStream testImage;
    RgbImageInfo info;
    info.m_isBGR = false;
    info.m_hasAlpha = true;
    info.m_isTopDown = true;
    info.m_width = 100;
    info.m_height = 200;

    testImage.Resize(info.m_height * info.m_width * 4);
    Byte* p=testImage.GetDataP();
    for (uint8_t y = 0; y<info.m_height; ++y)
        {
        for (uint8_t x = 0; x<info.m_width; ++x)
            {
            *p++ = (y%256); // R
            *p++ = (x%256); // G
            *p++ = (0xff);  // B
            *p++ = (0xff);  // A
            }
        }

    BeFileName pngFileName;
    BeTest::GetHost().GetOutputRoot(pngFileName);
    pngFileName.AppendToPath(L"ImageUtilities_Tests.png");
    BeFile pngFile;
    ASSERT_TRUE( pngFile.Create(pngFileName, /*createAlways*/true) == BeFileStatus::Success );

    ASSERT_TRUE( info.WriteImageToPngFile(pngFile, testImage) == BSISUCCESS);

    ByteStream imageRead;
    RgbImageInfo infoRead;
    pngFile.Close();
    ASSERT_TRUE( pngFile.Open(pngFileName, BeFileAccess::Read) == BeFileStatus::Success );
    ASSERT_TRUE( infoRead.ReadImageFromPngFile(imageRead, pngFile) == BSISUCCESS );
    ASSERT_EQ( infoRead.m_width, info.m_width );
    ASSERT_EQ( infoRead.m_height, info.m_height );
    ASSERT_EQ( infoRead.m_hasAlpha, info.m_hasAlpha );
    ASSERT_TRUE( infoRead.m_isTopDown ); // PNG is always top-down
    ASSERT_TRUE( 0==memcmp(imageRead.GetDataP(), testImage.GetDataP(), imageRead.GetSize()) ); // Since our input was RGBA, there was no transformation on the way out to the file.
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Umar.Hayat      11/2015
//---------------------------------------------------------------------------------------
TEST (ImageUtilities_Tests, PngReadFromBuffer)
    {
    ByteStream testImage;
    RgbImageInfo info;
    info.m_isBGR = false;
    info.m_hasAlpha = true;
    info.m_isTopDown = true;
    info.m_width = 100;
    info.m_height = 200;

    testImage.Resize(info.m_height * info.m_width * 4);
    Byte* p=testImage.GetDataP();
    for (uint8_t y = 0; y<info.m_height; ++y)
        {
        for (uint8_t x = 0; x<info.m_width; ++x)
            {
            *p++ = (y%256); // R
            *p++ = (x%256); // G
            *p++ = (0xff);  // B
            *p++ = (0xff);  // A
            }
        }

    BeFileName pngFileName;
    BeTest::GetHost().GetOutputRoot(pngFileName);
    pngFileName.AppendToPath(L"ImageUtilities_Tests.png");
    BeFile pngFile;
    ASSERT_TRUE( pngFile.Create(pngFileName, /*createAlways*/true) == BeFileStatus::Success );

    ASSERT_TRUE( info.WriteImageToPngFile(pngFile, testImage) == BSISUCCESS);

    Render::Image imageRead;
    ::RgbImageInfo infoRead;
    pngFile.Close();
    ASSERT_TRUE( pngFile.Open(pngFileName, BeFileAccess::Read) == BeFileStatus::Success );
    bvector<Byte> readBuffer;
    ASSERT_TRUE(BeFileStatus::Success == pngFile.ReadEntireFile(readBuffer));
    ASSERT_TRUE(0 != readBuffer.size());
    ASSERT_TRUE(infoRead.ReadImageFromPngBuffer(imageRead, &readBuffer[0], readBuffer.size()) == BSISUCCESS);
    ASSERT_EQ( infoRead.m_width, info.m_width );
    ASSERT_EQ( infoRead.m_height, info.m_height );
    ASSERT_EQ( infoRead.m_hasAlpha, info.m_hasAlpha );
    ASSERT_TRUE( infoRead.m_isTopDown ); // PNG is always top-down
    ASSERT_TRUE( 0==memcmp(imageRead.GetByteStream().GetDataP(), testImage.GetDataP(), imageRead.GetByteStream().GetSize()) ); // Since our input was RGBA, there was no transformation on the way out to the file.
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Umar.Hayat      11/2015
//---------------------------------------------------------------------------------------
TEST (ImageUtilities_Tests, JPG)
    {
    ByteStream testImage;
    RgbImageInfo info;
    info.m_isBGR = false;
    info.m_hasAlpha = false;
    info.m_isTopDown = true;
    info.m_width = 100;
    info.m_height = 200;

    testImage.Resize(info.m_height * info.m_width * 3);
    Byte* p=testImage.GetDataP();

    for (uint8_t y = 0; y<info.m_height; ++y)
        {
        for (uint8_t x = 0; x<info.m_width; ++x)
            {
            *p++ =  (255-y%256); // R
            *p++ = ((x % 256) ^ 0xFF ); // G
            *p++ = (0xff);  // B
            }
        }

    BeFileName pngFileName;
    BeTest::GetHost().GetOutputRoot(pngFileName);
    pngFileName.AppendToPath(L"ImageUtilities_JPGTests.jpg");
    BeFile pngFile;
    ASSERT_TRUE( pngFile.Create(pngFileName, /*createAlways*/true) == BeFileStatus::Success );
    
    bvector<uint8_t> jpegData;
    ASSERT_TRUE(SUCCESS == info.WriteImageToJpgBuffer(jpegData, testImage, 100));
    ASSERT_TRUE(BeFileStatus::Success == pngFile.Write(NULL, &jpegData[0], (uint32_t)jpegData.size()));

    Render::Image imageRead;
    RgbImageInfo infoRead = info;
    pngFile.Close();
    ASSERT_TRUE(pngFile.Open(pngFileName, BeFileAccess::Read) == BeFileStatus::Success);
    bvector<Byte> readBuffer;
    ASSERT_TRUE(BeFileStatus::Success == pngFile.ReadEntireFile(readBuffer));
    ASSERT_TRUE(0 != readBuffer.size());
    ASSERT_TRUE(infoRead.ReadImageFromJpgBuffer(imageRead, &readBuffer[0], readBuffer.size()) == BSISUCCESS);
    ASSERT_EQ(infoRead.m_width, info.m_width);
    ASSERT_EQ(infoRead.m_height, info.m_height);
    ASSERT_EQ(infoRead.m_hasAlpha, info.m_hasAlpha);
    ASSERT_TRUE(infoRead.m_isTopDown); // PNG is always top-down
    EXPECT_EQ(imageRead.GetByteStream().GetSize(), testImage.GetSize());
    // Why image is not same , qaulity was 100 so it should transform or change any thing
    //ASSERT_TRUE(imageRead == testImage); // Since our input was RGBA, there was no transformation on the way out to the file.
    }
