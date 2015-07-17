/*--------------------------------------------------------------------------------------+
|
|     $Source: Formats/PublicAPI/ImagePP.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__

#pragma once

/*__PUBLISH_SECTION_START__*/
TERRAINMODEL_TYPEDEFS (ImagePPConverter)
ADD_BENTLEY_TYPEDEFS (Bentley::TerrainModel, ImagePPConverter);

BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE

typedef RefCountedPtr<ImagePPConverter> ImagePPConverterPtr;


struct ImagePPConverter : RefCountedBase
    {
    private:
        WString m_filename;

        bool m_hasProperties;
        UInt64 m_widthInPixels;
        UInt64 m_heightInPixels;
        Bentley::GeoCoordinates::BaseGCSPtr m_gcs;

    private: ImagePPConverter (WCharCP filename);
    private: void GetImageProperties ();
    private: DTMStatusInt ImportAndTriangulateImageDtmObject (BcDTMPtr& dtm, double imageScaleFactor, WCharCP projectionKeyP, double unitConversionFactor, double elevationScaleFactor);

    public: BENTLEYDTMFORMATS_EXPORT BcDTMPtr ImportAndTriangulateImage (double imageScaleFactor, WCharCP projectionKeyP, double unitConversionFactor, double elevationScaleFactor);
    public: BENTLEYDTMFORMATS_EXPORT UInt64 GetWidth ();
    public: BENTLEYDTMFORMATS_EXPORT UInt64 GetHeight ();
    public: BENTLEYDTMFORMATS_EXPORT UInt64 GetNumberOfPixels ();
    public: BENTLEYDTMFORMATS_EXPORT Bentley::GeoCoordinates::BaseGCSPtr GetGCS ();
    public: BENTLEYDTMFORMATS_EXPORT static ImagePPConverterPtr Create (WCharCP filename);
    };

/*__PUBLISH_SECTION_END__*/
END_BENTLEY_TERRAINMODEL_NAMESPACE