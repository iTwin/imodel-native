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

#include <assert.h>
#include <Bentley/Bentley.h>
#include <Bentley/RefCounted.h>
#include <Properties/PropertiesAPI.h>

// For now we are only dealing with bounding boxes (5 points, first = last) and points are stored in an array of doubles (coordX, coordY).
#define FOOTPRINT_SIZE 10

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

//WIP
//[System::Runtime::ExceptionServices::HandleProcessCorruptedStateExceptions]
//[System::Security::SecurityCritical]

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
            {
                // {:R}: Round-trip format specifier used to ensure that a numeric value that is converted to a string will be 
                //       parsed back into the same numeric value.
                // {:G17}: In some cases, Double values formatted with the "R" standard numeric format string do not successfully 
                //         round-trip if compiled using the /platform:x64 or /platform:anycpu switches and run on 64-bit systems. 
                //         To work around this problem, you can format Double values by using the "G17" standard numeric format string.
                footprintFile->WriteLine("{0:G17} {1:G17}", pShape[i], pShape[i + 1]);
            }  

            footprintFile->Close();

#ifndef NDEBUG
            // Make sure footprint is a close shape, i.e., first point is equal to last point.
            StreamReader^ footprintReader = gcnew StreamReader(footprintFilename);
            String^ firstPt = footprintReader->ReadLine();
            String^ lastPt = nullptr;
            while (footprintReader->Peek() >= 0)
            {
                lastPt = footprintReader->ReadLine();
            }
            //Debug.Assert();
            if (!firstPt->Equals(lastPt))
            {
                Console::WriteLine("ERROR: Footprint not valid.");
                assert(false);
            }       
            footprintReader->Close();
#endif

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
