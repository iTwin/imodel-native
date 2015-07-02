// RealityDataHandlerTester.cpp : main project file.

#include "stdafx.h"

#include <assert.h>
#include <iostream>
#include <windows.h>

#include <Bentley/Bentley.h>
#include <Bentley/RefCounted.h>
#include <RealityPlatform/RealityDataHandler.h>
#include <RealityPlatform/WMSCapabilities.h>

#include <vcclr.h>

#using "System.Drawing.dll"
#using "System.IO.dll"
using namespace System;
using namespace System::Drawing;
using namespace System::IO;
using namespace std;

USING_BENTLEY_NAMESPACE_REALITYPLATFORM
USING_BENTLEY_NAMESPACE_WMSPARSER

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

bool IsURL(String^ filename)
    {
    return filename->Contains("http");
    }


int main(array<System::String ^> ^args)
    {
    /* WMSCAPABILITIES PARSER TESTER */
    //WCharCP url = L"http://schemas.opengis.net/wms/1.3.0/capabilities_1_3_0.xml";
    //WCharCP url = L"http://wms.gsfc.nasa.gov/cgi-bin/goes-wms.cgi?SERVICE=WMS&REQUEST=GetCapabilities";
    //WCharCP url = L"http://paikkatieto.ymparisto.fi/arcgis/services/INSPIRE/SYKE_Hydrografia/MapServer/WmsServer?SERVICE=WMS&REQUEST=GetCapabilities";
    //WCharCP url = L"http://coastwatch.pfeg.noaa.gov/erddap/wms/erdMBsstd3day/request?SERVICE=WMS&REQUEST=GetCapabilities";
    //WCharCP url = L"http://webmap.ornl.gov/ogcbroker/wms?SERVICE=WMS&REQUEST=GetCapabilities";
    //WCharCP url = L"http://gdr.ess.nrcan.gc.ca/wmsconnector/com.esri.wms.Esrimap/energy_e?SERVICE=WMS&REQUEST=GetCapabilities";
    //WCharCP url = L"http://sampleserver1.arcgisonline.com/ArcGIS/services/Specialty/ESRI_StatesCitiesRivers_USA/MapServer/WMSServer?service=WMS&request=GetCapabilities";
    //WCharCP url = L"http://tilecache.osgeo.org/wms-c/tilecache.py?SERVICE=WMS&REQUEST=GetCapabilities";
    //WCharCP url = L"http://ws.carmen.developpement-durable.gouv.fr/WMS/11/Carte_stations_hydro_region?language=fre&SERVICE=WMS&REQUEST=GetCapabilities";
    //WCharCP url = L"http://spatial.dcenr.gov.ie/wmsconnector/com.esri.wms.Esrimap/INFOMAR?SERVICE=WMS&REQUEST=GetCapabilities";
    //WMSParserStatus status;
    //WMSCapabilitiesPtr capabilities = WMSCapabilities::CreateAndReadFromUrl(status, url);

    // WMS GetMap Url
    // http://ogi.state.ok.us/geoserver/wms?VERSION=1.1.1&REQUEST=GetMap&SERVICE=WMS&LAYERS=ogi:okcounties,ogi:okcities&SRS=EPSG:4326&BBOX=-104.5005,32.7501,-94.01,37.20&WIDTH=800&HEIGHT=300&FORMAT=image/png&STYLES=&TRANSPARENT=TRUE
    // http://giswebservices.massgis.state.ma.us/geoserver/wms?VERSION=1.1.1&REQUEST=GetMap&SERVICE=WMS&LAYERS=massgis:GISDATA.TOWNS_POLYM,massgis:GISDATA.NAVTEQRDS_ARC,massgis:GISDATA.NAVTEQRDS_ARC_INT&SRS=EPSG:26986&BBOX=232325.38526025353,898705.3447384972,238934.49648710093,903749.1401484597&WIDTH=570&HEIGHT=435&FORMAT=image/png&STYLES=Black_Lines,GISDATA.NAVTEQRDS_ARC::ForOrthos,GISDATA.NAVTEQRDS_ARC_INT::Default&TRANSPARENT=TRUE





    
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
    pin_ptr<const wchar_t> inFilename       = PtrToStringChars(args[0]);    // Required
    pin_ptr<const wchar_t> outFilenameCP    = PtrToStringChars(args[1]);    // Required
    String^                outFilename      = args[1];                      // Required
    pin_ptr<const wchar_t> view             = nullptr;                      // Optional
    if (args->Length == 3)
        {
        view = PtrToStringChars(args[2]);
        }

    //-------------------------------------------------------------------------------------
    //                                      HANDLER
    //-------------------------------------------------------------------------------------
    RealityDataHandlerPtr dataHandler = nullptr;
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
        dataHandler = RealityPlatform::PointCloudDataHandler::Create(inFilename, pointCloudView);
        }
    // WMS
    else if (IsURL(args[0]))
        {
        dataHandler = RealityPlatform::WMSDataHandler::Create(inFilename, outFilenameCP);
        }
    // Raster
    else
        {
        dataHandler = RealityPlatform::RasterDataHandler::Create(inFilename);
        }
        
    //-------------------------------------------------------------------------------------
    //                                      THUMBNAIL
    //-------------------------------------------------------------------------------------
    StatusInt   retCode = ERROR;
    HBITMAP     thumbnailBmp = NULL;

    retCode = dataHandler->GetThumbnail(&thumbnailBmp);

    // Save results.
    if (SUCCESS == retCode && thumbnailBmp)
        {
        cout << "GetThumbnail SUCCESS." << endl;
        Bitmap^ bmp = Bitmap::FromHbitmap((IntPtr)thumbnailBmp);
        DeleteObject(thumbnailBmp);
        try
            {
            bmp->Save(outFilename);
            }
        catch (Exception^)
            {
            Console::WriteLine("Thumbnail error.");
            }
        }

    //-------------------------------------------------------------------------------------
    //                                      FOOTPRINT
    //-------------------------------------------------------------------------------------
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
    
    Console::WriteLine("Done.");

    return 0;
    }
