// ThumbnailsTester.cpp : main project file.

#include "stdafx.h"
#include <windows.h>
#include <vcclr.h>

#using "System.Drawing.dll"
#using "System.IO.dll"
using namespace System;
using namespace System::Drawing;
using namespace System::IO;

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
    HBITMAP ThumbnailBmp;
    StatusInt RetCode;

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
        RetCode = ThumbnailsProvider::Get()->GetPointCloudThumbnail(&ThumbnailBmp, pinFilename, 256, 256, pointCloudView);
        }
    else
        RetCode = ThumbnailsProvider::Get()->GetRasterThumbnail(&ThumbnailBmp, pinFilename, 256, 256);

    if (RetCode == SUCCESS)
    {
        Bitmap^ bmp = Bitmap::FromHbitmap((IntPtr)ThumbnailBmp);
        DeleteObject(ThumbnailBmp);
        try
        {
            bmp->Save(args[1]);
        }
        catch (Exception^)
        {
            Console::WriteLine("There was a problem saving the file.");
        }
    }
    else
        Console::WriteLine("There was a problem input file.");
    
    Console::WriteLine("Done.");
    return 0;
}
