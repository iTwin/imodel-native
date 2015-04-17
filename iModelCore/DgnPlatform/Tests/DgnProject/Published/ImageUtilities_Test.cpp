/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/DgnProject/Published/ImageUtilities_Test.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <DgnPlatform/DgnCore/ImageUtilities.h>

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      04/2013
//---------------------------------------------------------------------------------------
TEST (ImageUtilities_Tests, Test1)
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

#if defined (COMMENT_OUT)
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      04/2013
//---------------------------------------------------------------------------------------
TEST (ImageUtilities_Tests, Test2)
    {
    BeFile pngFile;
    ASSERT_TRUE( pngFile.Open (L"D:\\tmp\\test.png", BeFileAccess::Read) == BeFileStatus::Success );

    bvector<uint8_t> imageData;
    ImageUtilities::RgbImageInfo info;
    ASSERT_TRUE( ImageUtilities::ReadImageFromPngFile (imageData, info, pngFile) == BSISUCCESS );

    BeFile pngFileOut;
    ASSERT_TRUE( pngFileOut.Create (L"D:\\tmp\\test1.pgn", /*createAlways*/true) == BeFileStatus::Success );
    ASSERT_TRUE( ImageUtilities::WriteImageToPngFile (pngFileOut, imageData, info) == BSISUCCESS );
    }
#endif
