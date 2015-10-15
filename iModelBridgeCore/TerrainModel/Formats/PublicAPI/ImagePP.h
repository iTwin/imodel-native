/*--------------------------------------------------------------------------------------+
|
|     $Source: Formats/PublicAPI/ImagePP.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__

#pragma once

/*__PUBLISH_SECTION_START__*/
TERRAINMODEL_TYPEDEFS (ImagePPConverter)

BEGIN_BENTLEY_TERRAINMODEL_NAMESPACE

typedef RefCountedPtr<ImagePPConverter> ImagePPConverterPtr;


struct ImagePPConverter : RefCountedBase
    {
    private:
        WString m_filename;

        bool m_hasProperties;
        uint64_t m_widthInPixels;
        uint64_t m_heightInPixels;
        BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSPtr m_gcs;

    private: ImagePPConverter (WCharCP filename);
    private: void GetImageProperties ();
    private: DTMStatusInt ImportAndTriangulateImageDtmObject (BcDTMPtr& dtm, double imageScaleFactor, WCharCP projectionKeyP, double unitConversionFactor, double elevationScaleFactor);

    public: BENTLEYDTMFORMATS_EXPORT BcDTMPtr ImportAndTriangulateImage (double imageScaleFactor, WCharCP projectionKeyP, double unitConversionFactor, double elevationScaleFactor);
    public: BENTLEYDTMFORMATS_EXPORT uint64_t GetWidth ();
    public: BENTLEYDTMFORMATS_EXPORT uint64_t GetHeight ();
    public: BENTLEYDTMFORMATS_EXPORT uint64_t GetNumberOfPixels ();
    public: BENTLEYDTMFORMATS_EXPORT BENTLEY_NAMESPACE_NAME::GeoCoordinates::BaseGCSPtr GetGCS ();
    public: BENTLEYDTMFORMATS_EXPORT static ImagePPConverterPtr Create (WCharCP filename);
    };

/*__PUBLISH_SECTION_END__*/
END_BENTLEY_TERRAINMODEL_NAMESPACE