/*--------------------------------------------------------------------------------------+
|
|     $Source: FormatsNET/ImagePP.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <TerrainModel\Formats\ImagePP.h>

BEGIN_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE

struct _ImagePPConverter_Unmanaged
    {
    Bentley::TerrainModel::ImagePPConverterPtr m_converter;
    };

public ref class ImagePPConverter
    {
    private: _ImagePPConverter_Unmanaged* m_unmanaged;

    private: ImagePPConverter (System::String^ filename);
    public: !ImagePPConverter ();
    public: ~ImagePPConverter ();
    public: static ImagePPConverter^ Create (System::String^ filename);
    public: Bentley::TerrainModelNET::DTM^ ImportAndTriangulateImage (double imageScaleFactor, System::String^ projectionKey, double unitConversionFactor, double elevationScaleFactor);
    public: property UInt64 Width { UInt64 get (); }
    public: property UInt64 Height { UInt64 get (); }
    public: property UInt64 NumberOfPixels { UInt64 get (); }

    public: property Bentley::GeoCoordinatesNET::BaseGCS^ GCS
        {
        Bentley::GeoCoordinatesNET::BaseGCS^ get ();
        }

    };

END_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE