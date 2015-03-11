// ThumbnailsTester.cpp : main project file.

#include "stdafx.h"

//#include <windows.h>
//#include <vcclr.h>
#include "PackageECTest.cpp"

// #using "System.Drawing.dll"
// #using "System.IO.dll"
// using namespace System;
// using namespace System::Drawing;
// using namespace System::IO;

//WIP
//using namespace System::Runtime::InteropServices;

// #include <iostream>
// using namespace std;

#include <Bentley/Bentley.h>
#include <Bentley/RefCounted.h>
//#include <Properties/PropertiesAPI.h>



// For now we are only dealing with bounding boxes (5 points, first = last) and points are stored in an array of doubles (coordX, coordY).
#define FOOTPRINT_SIZE 10

//-----------------------------------------------------------------------------
// This function print out the usage for the current program.
//-----------------------------------------------------------------------------
// static void ShowUsage()
// {
//     // Check that we have the right number of parameters.
//     cout << "PropertiesTests --- " << __DATE__ << endl
//         << endl
//         << "Usage:  PropertiesTests <drive:\\SourceFile[all raster or .pod]>  <drive:\\DestFile.[jpg, Png]>  <[Top|Front|Right|Iso]>" << endl
//         << "Note:   <[Top|Front|Right|Iso]> option is relevant only for a point cloud" << endl
//         << endl;
// }

//WIP
//[System::Runtime::ExceptionServices::HandleProcessCorruptedStateExceptions]
//[System::Security::SecurityCritical]

int main(int argc, char * argv[])
{
  //  cout << "Copyright (c) Bentley Systems Inc, PropertiesTester " << "version 1.0 --- " << __DATE__ << endl;

    DoIt();
#if 0
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

    // Create properties. 
    StatusInt       retCode         = ERROR;
    HBITMAP         thumbnailBmp    = NULL;
    pin_ptr<double> pShape          = nullptr;

    retCode = IPropertiesProvider::Create(inFilename, view)->GetThumbnail(&thumbnailBmp);
    pShape  = IPropertiesProvider::Create(inFilename, view)->GetFootprint();

    // Save results.
    if (retCode == SUCCESS)
    {
        Bitmap^ bmp = Bitmap::FromHbitmap((IntPtr)thumbnailBmp);
        DeleteObject(thumbnailBmp);
        try
        {
            bmp->Save(outFilename);
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
            String^ footprintFilename = outFilename->Substring(0, outFilename->LastIndexOf('.')) + ".txt";
            StreamWriter^ footprintFile = gcnew StreamWriter(footprintFilename);

            for (size_t i = 0; i < FOOTPRINT_SIZE; i += 2)
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
    #endif
    return 0;

}
