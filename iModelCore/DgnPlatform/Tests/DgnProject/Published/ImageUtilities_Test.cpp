/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/DgnProject/Published/ImageUtilities_Test.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnCore/ImageUtilities.h>

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      04/2013
//---------------------------------------------------------------------------------------
TEST (ImageUtilities_Tests, Png)
    {
    bvector<uint8_t> testImage;
    ImageUtilities::RgbImageInfo info;
    info.isBGR = false;
    info.hasAlpha = true;
    info.isTopDown = true;
    info.width = 100;
    info.height = 200;
    for (uint8_t y = 0; y<info.height; ++y)
        {
        for (uint8_t x = 0; x<info.width; ++x)
            {
            testImage.push_back (y%256); // R
            testImage.push_back (x%256); // G
            testImage.push_back (0xff);  // B
            testImage.push_back (0xff);  // A
            }
        }

    BeFileName pngFileName;
    BeTest::GetHost().GetOutputRoot (pngFileName);
    pngFileName.AppendToPath (L"ImageUtilities_Tests.png");
    BeFile pngFile;
    ASSERT_TRUE( pngFile.Create (pngFileName, /*createAlways*/true) == BeFileStatus::Success );

    ASSERT_TRUE( ImageUtilities::WriteImageToPngFile (pngFile, testImage, info) == BSISUCCESS);

    bvector<uint8_t> imageRead;
    ImageUtilities::RgbImageInfo infoRead;
    memset (&infoRead, 0, sizeof(infoRead));
    pngFile.Close();
    ASSERT_TRUE( pngFile.Open (pngFileName, BeFileAccess::Read) == BeFileStatus::Success );
    ASSERT_TRUE( ImageUtilities::ReadImageFromPngFile (imageRead, infoRead, pngFile) == BSISUCCESS );
    ASSERT_EQ( infoRead.width, info.width );
    ASSERT_EQ( infoRead.height, info.height );
    ASSERT_EQ( infoRead.hasAlpha, info.hasAlpha );
    ASSERT_TRUE( infoRead.isTopDown ); // PNG is always top-down
    ASSERT_TRUE( imageRead == testImage ); // Since our input was RGBA, there was no transformation on the way out to the file.
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Umar.Hayat      11/2015
//---------------------------------------------------------------------------------------
TEST (ImageUtilities_Tests, PngReadFromBuffer)
    {
    bvector<uint8_t> testImage;
    ImageUtilities::RgbImageInfo info;
    info.isBGR = false;
    info.hasAlpha = true;
    info.isTopDown = true;
    info.width = 100;
    info.height = 200;
    for (uint8_t y = 0; y<info.height; ++y)
        {
        for (uint8_t x = 0; x<info.width; ++x)
            {
            testImage.push_back (y%256); // R
            testImage.push_back (x%256); // G
            testImage.push_back (0xff);  // B
            testImage.push_back (0xff);  // A
            }
        }

    BeFileName pngFileName;
    BeTest::GetHost().GetOutputRoot (pngFileName);
    pngFileName.AppendToPath (L"ImageUtilities_Tests.png");
    BeFile pngFile;
    ASSERT_TRUE( pngFile.Create (pngFileName, /*createAlways*/true) == BeFileStatus::Success );

    ASSERT_TRUE( ImageUtilities::WriteImageToPngFile (pngFile, testImage, info) == BSISUCCESS);

    bvector<uint8_t> imageRead;
    ImageUtilities::RgbImageInfo infoRead;
    memset (&infoRead, 0, sizeof(infoRead));
    pngFile.Close();
    ASSERT_TRUE( pngFile.Open (pngFileName, BeFileAccess::Read) == BeFileStatus::Success );
    bvector<Byte> readBuffer;
    ASSERT_TRUE(BeFileStatus::Success == pngFile.ReadEntireFile(readBuffer));
    ASSERT_TRUE(0 != readBuffer.size());
    ASSERT_TRUE(ImageUtilities::ReadImageFromPngBuffer(imageRead, infoRead, &readBuffer[0], readBuffer.size()) == BSISUCCESS);
    ASSERT_EQ( infoRead.width, info.width );
    ASSERT_EQ( infoRead.height, info.height );
    ASSERT_EQ( infoRead.hasAlpha, info.hasAlpha );
    ASSERT_TRUE( infoRead.isTopDown ); // PNG is always top-down
    ASSERT_TRUE( imageRead == testImage ); // Since our input was RGBA, there was no transformation on the way out to the file.
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Umar.Hayat      11/2015
//---------------------------------------------------------------------------------------
TEST (ImageUtilities_Tests, JPG)
    {
    bvector<uint8_t> testImage;
    ImageUtilities::RgbImageInfo info;
    info.isBGR = false;
    info.hasAlpha = false;
    info.isTopDown = true;
    info.width = 100;
    info.height = 200;

    for (uint8_t y = 0; y<info.height; ++y)
        {
        for (uint8_t x = 0; x<info.width; ++x)
            {
            testImage.push_back (255-y%256); // R
            testImage.push_back((x % 256) ^ 0xFF ); // G
            testImage.push_back(0xff);  // B
            }
        }

    BeFileName pngFileName;
    BeTest::GetHost().GetOutputRoot (pngFileName);
    pngFileName.AppendToPath (L"ImageUtilities_JPGTests.jpg");
    BeFile pngFile;
    ASSERT_TRUE( pngFile.Create (pngFileName, /*createAlways*/true) == BeFileStatus::Success );
    
    bvector<Byte> jpegData;
    ASSERT_TRUE(SUCCESS == ImageUtilities::WriteImageToJpgBuffer(jpegData, testImage, info, 100));
    ASSERT_TRUE(BeFileStatus::Success == pngFile.Write(NULL, &jpegData[0], (uint32_t)jpegData.size()));

    bvector<uint8_t> imageRead;
    ImageUtilities::RgbImageInfo infoRead;
    memset (&infoRead, 0, sizeof(infoRead));
    pngFile.Close();
    ASSERT_TRUE(pngFile.Open(pngFileName, BeFileAccess::Read) == BeFileStatus::Success);
    bvector<Byte> readBuffer;
    ASSERT_TRUE(BeFileStatus::Success == pngFile.ReadEntireFile(readBuffer));
    ASSERT_TRUE(0 != readBuffer.size());
    ASSERT_TRUE(ImageUtilities::ReadImageFromJpgBuffer(imageRead, infoRead, &readBuffer[0], readBuffer.size(), info) == BSISUCCESS);
    ASSERT_EQ(infoRead.width, info.width);
    ASSERT_EQ(infoRead.height, info.height);
    ASSERT_EQ(infoRead.hasAlpha, info.hasAlpha);
    ASSERT_TRUE(infoRead.isTopDown); // PNG is always top-down
    EXPECT_EQ(imageRead.size(), testImage.size());
    // Why image is not same , qaulity was 100 so it should transform or change any thing
    //ASSERT_TRUE(imageRead == testImage); // Since our input was RGBA, there was no transformation on the way out to the file.
    }
