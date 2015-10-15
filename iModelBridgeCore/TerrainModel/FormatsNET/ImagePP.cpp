/*--------------------------------------------------------------------------------------+
|
|     $Source: FormatsNET/ImagePP.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "ImagePP.h"

BEGIN_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE

ImagePPConverter::ImagePPConverter (System::String^ filename)
    {
    pin_ptr<const wchar_t> p = PtrToStringChars (filename);
    m_unmanaged = new _ImagePPConverter_Unmanaged ();
    m_unmanaged->m_converter = BENTLEY_NAMESPACE_NAME::TerrainModel::ImagePPConverter::Create (p);
    }

ImagePPConverter::!ImagePPConverter ()
    {
    ImagePPConverter::~ImagePPConverter ();
    }

ImagePPConverter::~ImagePPConverter ()
    {
    if (m_unmanaged)
        {
        delete m_unmanaged;
        m_unmanaged = nullptr;
        }
    }

ImagePPConverter^ ImagePPConverter::Create (System::String^ filename)
    {
    return gcnew ImagePPConverter (filename);
    }

BENTLEY_NAMESPACE_NAME::TerrainModelNET::DTM^ ImagePPConverter::ImportAndTriangulateImage (double imageScaleFactor, System::String^ projectionKey, double unitConversionFactor, double elevationScaleFactor)
    {
    pin_ptr<const wchar_t> p = PtrToStringChars (projectionKey);
    BcDTMPtr dtm = m_unmanaged->m_converter->ImportAndTriangulateImage (imageScaleFactor, p, unitConversionFactor, elevationScaleFactor);
    return dtm.IsValid () ? BENTLEY_NAMESPACE_NAME::TerrainModelNET::DTM::FromHandle ((System::IntPtr)dtm.get ()) : nullptr;
    }

uint64_t ImagePPConverter::Width::get()
    {
    return m_unmanaged->m_converter->GetWidth ();
    }

uint64_t ImagePPConverter::Height::get ()
    {
    return m_unmanaged->m_converter->GetHeight();
    }

uint64_t ImagePPConverter::NumberOfPixels::get ()
    {
    return m_unmanaged->m_converter->GetNumberOfPixels ();
    }

Bentley::GeoCoordinatesNET::BaseGCS^ ImagePPConverter::GCS::get ()
    {
    Bentley::GeoCoordinates::BaseGCSPtr gcs = m_unmanaged->m_converter->GetGCS ();

    if (gcs.IsValid ())
        return gcnew Bentley::GeoCoordinatesNET::BaseGCS (gcs.get ());
    return nullptr;
    }
END_BENTLEY_TERRAINMODELNET_FORMATS_NAMESPACE