/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityServicesNet/RealityPackageNet/RealityDataSourceNet.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <RealityPackage/RealityDataPackage.h>


namespace RealityPackageNet
    {   
    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              10/2016
    //=====================================================================================
    public ref class UriNet
        {
        public:
            //! Get/Set the source type.
            static UriNet^ Create(System::String^ resourceIdentifier);
            static UriNet^ Create(System::String^ source, System::String^ fileInCompound);

            //! Get the first part of the identifier e.g. the full path of the document.
            System::String^ GetSource();

            //! Get the later part of the identifier e.g. the file in the compound document. Null if the source is not a compound document.
            System::String^ GetFileInCompound();

            //! Get the complete identifier as a string.
            System::String^ ToStr();

        protected:
            UriNet() {}
            UriNet(System::String^ resourceIdentifier);
            UriNet(System::String^ source, System::String^ fileInCompound);
            ~UriNet();
            !UriNet();

        private:
            RealityPackage::UriPtr* m_pUri;
        };


    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              10/2016
    //=====================================================================================
    public ref class RealityDataSourceNet
        {
        public:
            static RealityDataSourceNet^ Create(UriNet^ uri, System::String^ type);

            //! Get/Set the source uri. It could be a full URL or a path relative to the package file.
            //! ex: "http://www.Bentley.com/logo.jpg"
            //!     "./imagery/road.jpg"
            UriNet^ GetUri();
            void SetUri(UriNet^ uri);

            //! Get/Set the source type.
            System::String^ GetSourceType();
            void SetSourceType(System::String^ type);

            //! Get/Set the id. Might be empty.
            System::String^ GetId();
            void SetId(System::String^ id);

            //! Get/Set the copyright. Might be empty.
            System::String^ GetCopyright();
            void SetCopyright(System::String^ copyright);

            //! Get/Set the provider. Might be empty.
            System::String^ GetProvider();
            void SetProvider(System::String^ provider);

            //! Get/Set the size in kilobytes. Default to 0.
            int GetSize();
            void SetSize(int sizeInKB);

            //! Get/Set the metadata. Might be empty.
            System::String^ GetMetadata();
            void SetMetadata(System::String^ metadata);

            //! Indicates the type of metadata. This is a string.
            System::String^ GetMetadataType();
            void SetMetadataType(System::String^ metadataType);

            //! Indicates the default geographic coordinate system keyname. May be empty.
            System::String^ GetGeoCS();
            void SetGeoCS(System::String^ geoCS);

            //! Specifies the no data value if it is not integrally part of the format. May be empty.
            System::String^ GetNoDataValue();
            void SetNoDataValue(System::String^ nodatavalue);

            //! Get/Set the sister files. Might be empty.
            System::Collections::Generic::List<System::String^>^ GetSisterFiles();
            void SetSisterFiles(System::Collections::Generic::List<System::String^>^ sisterFiles);

            //! Get the element name.
            System::String^ GetElementName();

        protected:
            RealityDataSourceNet() {}
            RealityDataSourceNet(UriNet^ uri, System::String^ type);
            virtual ~RealityDataSourceNet();
            !RealityDataSourceNet();
   
        private:
            RealityPackage::RealityDataSourcePtr* m_pSource;
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              10/2016
    //=====================================================================================
    public ref class WmsDataSourceNet : public RealityDataSourceNet
        {
        public:
            static WmsDataSourceNet^ Create(System::String^ uri);
            static WmsDataSourceNet^ Create(UriNet^ uri);

            //! Get/Set The source data.
            //! The string used here should represent a xml fragment containing all the nodes/infos required for WMS processing.
            //! You can take a look at PublicApi/RealityPlatform/WmsSource.h for more details on the structure of a WmsMapSettings object.
            System::String^ GetMapSettings();
            void SetMapSettings(System::String^ mapSettings);

        protected:
            WmsDataSourceNet() {}
            WmsDataSourceNet(System::String^ uri);
            WmsDataSourceNet(UriNet^ uri);
            virtual ~WmsDataSourceNet();
            !WmsDataSourceNet();

        private:
            RealityPackage::WmsDataSourcePtr* m_pSource;
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              10/2016
    //=====================================================================================
    /*
    public ref class OsmDataSourceNet : public RealityDataSourceNet
        {
        public:
            static OsmDataSourceNet^ Create(System::String^ uri, double bboxMinX, double bboxMinY, double bboxMaxX, double bboxMaxY);

            //! Get/Set The source data.
            //! The string used here should represent a xml fragment containing all the nodes/infos required for OSM processing.
            //! You can take a look at PublicApi/RealityPlatform/OsmSource.h for more details on the structure of a OsmResource object.
            System::String^ GetOsmResource();
            void SetOsmResource(System::String^ osmResource);

        protected:
            OsmDataSourceNet() {}
            OsmDataSourceNet(System::String^ uri, double bboxMinX, double bboxMinY, double bboxMaxX, double bboxMaxY);
            virtual ~OsmDataSourceNet();
            !OsmDataSourceNet();

        private:
            RealityPackage::OsmDataSourcePtr* m_pSource;
        };
    */

    }
