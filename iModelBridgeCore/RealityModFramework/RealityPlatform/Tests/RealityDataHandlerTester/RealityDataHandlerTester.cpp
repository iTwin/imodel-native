// RealityDataHandlerTester.cpp : main project file.

#include "stdafx.h"

#include <assert.h>
#include <iostream>
#include <windows.h>

#include <Bentley/Bentley.h>
#include <Bentley/RefCounted.h>
#include <RealityPlatform/RealityDataHandler.h>

#include <vcclr.h>

#using "System.Drawing.dll"
#using "System.IO.dll"
using namespace System;
using namespace System::Drawing;
using namespace System::IO;
using namespace std;

//-----------------------------------------------------------------------------
// This function print out the usage for the current program.
//-----------------------------------------------------------------------------
static void ShowUsage()
    {
    // Check that we have the right number of parameters.
    cout << "PropertiesTests --- " << __DATE__ << endl
        << endl
        << "Usage:  PropertiesTests <drive:\\SourceFile[all raster or .pod]>  <drive:\\DestFile.[jpg, Png]>  <[Top|Front|Right|Iso]>" << endl
        << "Note:   <[Top|Front|Right|Iso]> option is relevant only for a point cloud" << endl
        << endl;
    }


int main(array<System::String ^> ^args)
    {
    cout << "Copyright (c) Bentley Systems Inc, PropertiesTester " << "version 1.0 --- " << __DATE__ << endl;


    // Check that we have the right number of parameters.
    if (args->Length != 2 && args->Length != 3)
        {
        ShowUsage();
        exit(1);
        }
    
    for each(String^ arg in args)
        Console::WriteLine(arg);

    // Parse parameters.
    pin_ptr<const wchar_t> inFilename   = PtrToStringChars(args[0]);    // Required
    String^                outFilename  = args[1];                      // Required
    pin_ptr<const wchar_t> view         = nullptr;                      // Optional
    if (args->Length == 3)
        {
        view = PtrToStringChars(args[2]);
        }


    StatusInt       retCode             = ERROR;
    HBITMAP         thumbnailBmp        = NULL;
    DRange2d        pShape;
    RealityDataHandlerPtr dataHandler   = nullptr;

    // Create handler.
    String^ ext = Path::GetExtension(args[0]);
    //&&JFC Replace ToUpper() by another function that do the same but more safely (can't remember the name at this moment).
    ext = ext->ToUpper();
    if (ext == ".POD")
        {
        String^ PCView = "";
        if (args->Length == 3)
            {
            PCView = args[2];
            }
        PCView = PCView->ToUpper();
        RealityPlatform::PointCloudView pointCloudView = RealityPlatform::PointCloudView::Top;    // Default
        if (PCView == "RIGHT")
            {
            pointCloudView = RealityPlatform::PointCloudView::Right;
            }
        else if (PCView == "FRONT")
            {
            pointCloudView = RealityPlatform::PointCloudView::Front;
            }
        else if (PCView == "ISO")
            {
            pointCloudView = RealityPlatform::PointCloudView::Iso;
            }
        dataHandler = RealityPlatform::PointCloudDataHandler::Create(inFilename, pointCloudView);
        }
    else
        dataHandler = RealityPlatform::RasterDataHandler::Create(inFilename);

    // Create properties. 
    retCode = dataHandler->GetThumbnail(&thumbnailBmp);
    retCode = dataHandler->GetFootprint(&pShape);

    // Save results.
    if (retCode == SUCCESS)
        {
        Bitmap^ bmp = Bitmap::FromHbitmap((IntPtr)thumbnailBmp);
        DeleteObject(thumbnailBmp);
        try
            {
            // Thumbnail
            bmp->Save(outFilename);

            // Footprint
            String^ footprintFilename = outFilename->Substring(0, outFilename->LastIndexOf('.')) + ".txt";
            StreamWriter^ footprintFile = gcnew StreamWriter(footprintFilename);
            DPoint2dP pPts = new DPoint2d();
            pShape.Get4Corners(pPts);

            // For now we are only dealing with bboxes.
            for (size_t i = 0; i < 4; ++i)
                {
                // {:R}: Round-trip format specifier used to ensure that a numeric value that is converted to a string will be 
                //       parsed back into the same numeric value.
                // {:G17}: In some cases, Double values formatted with the "R" standard numeric format string do not successfully 
                //         round-trip if compiled using the /platform:x64 or /platform:anycpu switches and run on 64-bit systems. 
                //         To work around this problem, you can format Double values by using the "G17" standard numeric format string.
                footprintFile->WriteLine("{0:G17} {1:G17}", pPts[i].x, pPts[i].y);
                }
            // Close shape (first point = last point)
            footprintFile->WriteLine("{0:G17} {1:G17}", pPts[0].x, pPts[0].y);

            footprintFile->Close();
            }
        catch (Exception^)
            {
            Console::WriteLine("There was a problem saving file.");
            }
        }
    else
        Console::WriteLine("There was a problem input file.");

    Console::WriteLine("Done.");

    return 0;

    }
