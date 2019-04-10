/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityServicesNet/RealityPackageNet/WmsSourceNet.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <RealityPlatform/RealityPlatformAPI.h>
#include <RealityPlatform/WMSSource.h>

namespace RealityPackageNet
    {
    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              10/2016
    //=====================================================================================
    public ref class WmsMapSettingsNet
        {
        public:
            //! Create WMS MapRequest info with all the required information. 
            static WmsMapSettingsNet^ Create(System::String^ uri, System::Collections::Generic::List<double>^ bbox, System::String^ version, System::String^ layers, System::String^ csType, System::String^ csLabel);

            //! Get/Set the server uri. 
            System::String^ GetUri();
            void SetUri(System::String^ uri);

            //! Get/Set the bounding box (bboxMinX, bboxMinY, bboxMaxX, bboxMaxY).
            System::Collections::Generic::List<double>^ GetBBox();
            void SetBBox(System::Collections::Generic::List<double>^ bbox);

            //! Get/Set the width of the window in pixels.
            //! The pixel ratio should be equal to the 'BoundingBox' ratio to avoid any distorsion.
            //! Sub-Resolutions will be generated from this size.
            uint32_t GetMetaWidth();
            void SetMetaWidth(uint32_t width);

            //! Get/Set the height of the window in pixels.
            //! The pixel ratio should be equal to the 'BoundingBox' ratio to avoid any distorsion.
            //! Sub-Resolutions will be generated from this size.
            uint32_t GetMetaHeight();
            void SetMetaHeight(uint32_t height);

            //! Get/Set the WMS version. 
            System::String^ GetVersion();
            void SetVersion(System::String^ version);

            //! Get/Set the layer list. 
            System::String^ GetLayers();
            void SetLayers(System::String^ layers);

            //! Get/Set the style list. 
            System::String^ GetStyles();
            void SetStyles(System::String^ styles);

            //! Get/Set the coordinate system type attribute. 
            //! CRS for version 1.3, SRS for 1.1.1 and below.
            System::String^ GetCoordSysType();
            void SetCoordSysType(System::String^ csType);

            //! Get/Set the coordinate system. 
            System::String^ GetCoordSysLabel();
            void SetCoordSysLabel(System::String^ csLabel);

            //! Get/Set the output format. 
            System::String^ GetFormat();
            void SetFormat(System::String^ format);

            //! Get/Set the vendor specific parameters.
            //! Unparsed, vendor specific parameters that will be appended to the request.
            System::String^ GetVendorSpecific();
            void SetVendorSpecific(System::String^ vendorSpecific);

            //! Get/Set the transparency. 
            bool IsTransparent();
            void SetTransparency(bool isTransparent);

            //! Xml fragment.
            System::String^ ToXml();

        protected:
            WmsMapSettingsNet(System::String^ uri, System::Collections::Generic::List<double>^ bbox, System::String^ version, System::String^ layers, System::String^ csType, System::String^ csLabel);
            ~WmsMapSettingsNet();
            !WmsMapSettingsNet();

        private:
            RealityPlatform::WmsMapSettingsPtr* m_pWmsMapSettings;
        };
    }