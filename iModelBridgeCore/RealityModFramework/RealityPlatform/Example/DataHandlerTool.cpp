/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/Example/DataHandlerTool.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <msclr/marshal.h>
#include <RealityPlatform/RealityDataHandler.h>

#using "System.Drawing.dll"

#define THUMBNAIL_WIDTH     512
#define THUMBNAIL_HEIGHT    512

using namespace msclr::interop;
using namespace System;
using namespace System::Drawing;
using namespace System::IO;

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    3/2016
//
// Check that we have the right number of parameters and that they are valid.
//-------------------------------------------------------------------------------------
bool ValidateParameters(array<String^>^ params)
    {
    if (params->Length != 2 && params->Length != 3)
        return false;

    return true;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    3/2016
//-------------------------------------------------------------------------------------
bool IsURL(String^ filename)
    {
    return filename->Contains("http");
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         	    3/2016
//-------------------------------------------------------------------------------------
int main(array<String^>^ args)
    {
    // WMS GetMap Url
    // http://ogi.state.ok.us/geoserver/wms?VERSION=1.1.1&REQUEST=GetMap&SERVICE=WMS&LAYERS=ogi:okcounties,ogi:okcities&SRS=EPSG:4326&BBOX=-104.5005,32.7501,-94.01,37.20&WIDTH=800&HEIGHT=300&FORMAT=image/png&STYLES=&TRANSPARENT=TRUE
    // http://giswebservices.massgis.state.ma.us/geoserver/wms?VERSION=1.1.1&REQUEST=GetMap&SERVICE=WMS&LAYERS=massgis:GISDATA.TOWNS_POLYM,massgis:GISDATA.NAVTEQRDS_ARC,massgis:GISDATA.NAVTEQRDS_ARC_INT&SRS=EPSG:26986&BBOX=232325.38526025353,898705.3447384972,238934.49648710093,903749.1401484597&WIDTH=570&HEIGHT=435&FORMAT=image/png&STYLES=Black_Lines,GISDATA.NAVTEQRDS_ARC::ForOrthos,GISDATA.NAVTEQRDS_ARC_INT::Default&TRANSPARENT=TRUE

    Console::Title = "DataHandlerTool";

    // Validate parameters.
    if (!ValidateParameters(args))
        {
        Console::WriteLine("Invalid parameters.");
        Console::WriteLine("Usage:  DataHandlerTool <drive:\\SourceFile[all raster or .pod]>  <drive:\\DestFile.[jpg, Png]>  <[Top|Front|Right|Iso]>");
        Console::WriteLine("Note:   <[Top|Front|Right|Iso]> option is relevant only for a point cloud");
        Console::WriteLine();
        Console::WriteLine("Press any key to exit.");
        Console::ReadKey();
        exit(1);
        }

    // Parse parameters.
    marshal_context ctx;
    Utf8String inFilename = ctx.marshal_as<Utf8CP>(args[0]);
    Utf8String outFilename = ctx.marshal_as<Utf8CP>(args[1]);
    Utf8String view = "";
    if (args->Length == 3)
        {
        view = ctx.marshal_as<Utf8CP>(args[2]);
        }

    Console::Write("Input: ");
    Console::WriteLine(args[0]);

    //-------------------------------------------------------------------------------------
    //                                      HANDLER
    //-------------------------------------------------------------------------------------
    RealityDataPtr dataHandler = nullptr;
    String^ ext = Path::GetExtension(args[0])->ToUpper();

    // Pointcloud
    if (ext == ".POD")
        {
        String^ PCView = "";
        if (args->Length == 3)
            {
            PCView = args[2]->ToUpper();
            }
        
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
        dataHandler = RealityPlatform::PointCloudData::Create(inFilename.c_str(), pointCloudView);
        }
    // WMS
    else if (IsURL(args[0]))
        {
        Console::WriteLine("WMS data detected.");
        Console::Write("Creating handler... ");

        dataHandler = RealityPlatform::WmsData::Create(inFilename.c_str());
        if (!dataHandler.IsValid())
            {
            Console::WriteLine("FAILED");
            exit(0);
            }
        }
    // Raster
    else
        {
        dataHandler = RealityPlatform::RasterData::Create(inFilename.c_str());
        }

    Console::WriteLine("SUCCESS");

    //-------------------------------------------------------------------------------------
    //                                      THUMBNAIL
    //-------------------------------------------------------------------------------------
    Console::Write("Creating thumbnail... ");

    StatusInt   retCode = ERROR;
    bvector<System::Byte> data;
    
    retCode = dataHandler->GetThumbnail(data, THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT);
    if (SUCCESS != retCode || data.empty())
        {
        Console::WriteLine("FAILED");
        exit(0);
        }

    // Save results.
    uint32_t bufferSize = static_cast<uint32_t>(data.size());
    array<System::Byte>^ buffer = gcnew array<System::Byte>(bufferSize);
    for (uint32_t i = 0; i < bufferSize; ++i)
        buffer[i] = data[i];

    MemoryStream^ stream = gcnew MemoryStream(buffer);
    Bitmap^ bmp = gcnew Bitmap(stream);
    try
        {
        bmp->Save(args[1]);
        }
    catch (Exception^)
        {
        Console::WriteLine("FAILED");
        exit(0);
        }

    Console::WriteLine("SUCCESS");
    Console::Write("Output:");
    Console::WriteLine(args[1]);

    //-------------------------------------------------------------------------------------
    //                                      FOOTPRINT
    //-------------------------------------------------------------------------------------
    /*
    DRange2d    pShape;

    retCode = dataHandler->GetFootprint(&pShape);

    // Save results.
    if (SUCCESS == retCode)
        {
        cout << "GetFootprint SUCCESS." << endl;
        try
            {
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
            Console::WriteLine("Footprint error.");
            }
        }
    */
    Console::WriteLine("Done.");

    return 0;
    }
