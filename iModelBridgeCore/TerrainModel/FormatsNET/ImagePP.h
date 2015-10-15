/*--------------------------------------------------------------------------------------+
|
|     $Source: FormatsNET/ImagePP.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <TerrainModel\Formats\ImagePP.h>

BEGIN_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE

struct _ImagePPConverter_Unmanaged
    {
    BENTLEY_NAMESPACE_NAME::TerrainModel::ImagePPConverterPtr m_converter;
    };

public ref class ImagePPConverter
    {
    private: _ImagePPConverter_Unmanaged* m_unmanaged;

    private: ImagePPConverter (System::String^ filename);
    public: !ImagePPConverter ();
    public: ~ImagePPConverter ();
    public: static ImagePPConverter^ Create (System::String^ filename);
    public: BENTLEY_NAMESPACE_NAME::TerrainModelNET::DTM^ ImportAndTriangulateImage (double imageScaleFactor, System::String^ projectionKey, double unitConversionFactor, double elevationScaleFactor);
    public: property uint64_t Width { uint64_t get (); }
    public: property uint64_t Height { uint64_t get (); }
    public: property uint64_t NumberOfPixels { uint64_t get (); }

    public: property Bentley::GeoCoordinatesNET::BaseGCS^ GCS
        {
        Bentley::GeoCoordinatesNET::BaseGCS^ get ();
        }

    };

END_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE