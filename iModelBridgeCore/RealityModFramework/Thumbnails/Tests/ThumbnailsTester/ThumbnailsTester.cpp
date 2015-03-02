// ThumbnailsTester.cpp : main project file.

#include "stdafx.h"

#include <windows.h>
#include <vcclr.h>

#using "System.Drawing.dll"
#using "System.IO.dll"
using namespace System;
using namespace System::Drawing;
using namespace System::IO;

//WIP
//using namespace System::Runtime::InteropServices;

#include <iostream>
using namespace std;

#include <Bentley/Bentley.h>
#include <Bentley/RefCounted.h>
#include <Thumbnails/ThumbnailsAPI.h>

//-----------------------------------------------------------------------------
// This function print out the usage for the current program.
//-----------------------------------------------------------------------------
static void ShowUsage()
{
    // Check that we have the right number of parameters.
    cout << "ThumbnailTests --- " << __DATE__ << endl
        << endl
        << "Usage:  ThumbnailTests <drive:\\SourceFile[all raster or .pod]>  <drive:\\DestFile.[jpg, Png]>  <[Top|Front|Right|Iso]>" << endl
        << "Note:   <[Top|Front|Right|Iso]> option is relevant only for a point cloud" << endl
        << endl;
}

//WIP
//[System::Runtime::ExceptionServices::HandleProcessCorruptedStateExceptions]
//[System::Security::SecurityCritical]

int main(array<System::String ^> ^args)
{
    cout << "Copyright (c) Bentley Systems Inc, ThumbnailsTester " << "version 1.0 --- " << __DATE__ << endl;

    // Check that we have the right number of parameters.
    if (args->Length != 2 && args->Length != 3)
    {
        ShowUsage();
        exit(1);
    }

    for each(String^ arg in args)
        Console::WriteLine(arg);

    pin_ptr<const wchar_t>  pinFilename = PtrToStringChars(args[0]);
    HBITMAP ThumbnailBmp = NULL;
    StatusInt retCode(ERROR);

    uint32_t nbPts = 0;
    pin_ptr<double> pShape = nullptr;
	
    String^ Ext = Path::GetExtension(args[0]);
    Ext = Ext->ToUpper();
    if (Ext == ".POD")
    {
        String^ PCView = "";
        if (args->Length == 3)
        {
            PCView = args[2];
        }
        PCView = PCView->ToUpper();
        PointCloudView pointCloudView = PointCloudView::Top;    // Default
        if (PCView == "RIGHT")
        {
            pointCloudView = PointCloudView::Right;
        }
        else if (PCView == "FRONT")
        {
            pointCloudView = PointCloudView::Front;
        }
        else if (PCView == "ISO")
        {
            pointCloudView = PointCloudView::Iso;
        }
        retCode = ThumbnailsProvider::Get()->GetPointCloudThumbnail(&ThumbnailBmp, pinFilename, 256, 256, pointCloudView);
        pShape = ThumbnailsProvider::Get()->GetPointCloudFootprint(nbPts, pinFilename, 256, 256, pointCloudView);
    }
    else
    {
        retCode = ThumbnailsProvider::Get()->GetRasterThumbnail(&ThumbnailBmp, pinFilename, 256, 256);
        pShape = ThumbnailsProvider::Get()->GetRasterFootprint(nbPts, pinFilename);
    }
       

    // Save results.
    if (retCode == SUCCESS)
    {
        Bitmap^ bmp = Bitmap::FromHbitmap((IntPtr)ThumbnailBmp);
        DeleteObject(ThumbnailBmp);
        try
        {
            bmp->Save(args[1]);
        }
        catch (Exception^)
        {
            Console::WriteLine("There was a problem saving the thumbnail.");
        }
    }
    else
        Console::WriteLine("There was a problem input file (thumbnail).");

    if (pShape != nullptr)
    {
        try
        {
            String^ footprintFilename = args[1]->Substring(0, args[1]->LastIndexOf('.')) + ".txt";
            StreamWriter^ footprintFile = gcnew StreamWriter(footprintFilename);

            for (size_t i = 0; i < (nbPts * 2); i += 2)
                footprintFile->WriteLine("{0} {1}", pShape[i], pShape[i + 1]);

            footprintFile->Close();
        }
        catch (Exception^)
        {
            Console::WriteLine("There was a problem saving the footprint.");
        }     
    }
    else
        Console::WriteLine("There was a problem input file (footprint).");

    Console::WriteLine("Done.");

    return 0;
}
