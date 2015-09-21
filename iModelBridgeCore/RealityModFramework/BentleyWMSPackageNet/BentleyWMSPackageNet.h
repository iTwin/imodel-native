/*--------------------------------------------------------------------------------------+
|
|     $Source: BentleyWMSPackageNet/BentleyWMSPackageNet.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/BeFileName.h>
#include <RealityPackage/RealityDataPackage.h>

#using  <mscorlib.dll>

using RealityPackage::RealityDataPackage;
using namespace System::Collections::Generic;


// Forward declaration
namespace RealityDataPackageWrapper { ref class ImageryGroupNet; }
namespace RealityDataPackageWrapper { ref class ModelGroupNet; }
namespace RealityDataPackageWrapper { ref class PinnedGroupNet; }
namespace RealityDataPackageWrapper { ref class TerrainGroupNet; }
namespace RealityDataPackageWrapper { ref class RealityDataSourceNet; }

namespace RealityDataPackageWrapper
    {   
    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote               6/2015
    //=====================================================================================
    public ref class RealityDataPackageNet
        {
        public:
            //! Create package.
            static void Create(System::String^  location, 
                               System::String^  name,
                               System::String^  description,
                               System::String^  copyright,
                               List<double>^    regionOfInterest,
                               ImageryGroupNet^ imageryData,
                               ModelGroupNet^   modelData,
                               PinnedGroupNet^  pinnedData,
                               TerrainGroupNet^ terrainData);
        
        private:
            RealityDataPackageNet();
            ~RealityDataPackageNet();
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote               9/2015
    //=====================================================================================
    public ref class ImageryGroupNet
        {
        public:
            //! Create imagery group.
            static ImageryGroupNet^ Create();

            void AddData(RealityDataSourceNet^ imageryData);

            List<RealityDataSourceNet^>^ GetData() { return m_imageryDataList; }

        private:
            ImageryGroupNet();
            ~ImageryGroupNet();

            List<RealityDataSourceNet^>^ m_imageryDataList;
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote               9/2015
    //=====================================================================================
    public ref class ModelGroupNet
        {
        public:
            //! Create model group.
            static ModelGroupNet^ Create();

            void AddData(RealityDataSourceNet^ modelData);

            List<RealityDataSourceNet^>^ GetData() { return m_modelDataList; }

        private:
            ModelGroupNet();
            ~ModelGroupNet();

            List<RealityDataSourceNet^>^ m_modelDataList;
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote               9/2015
    //=====================================================================================
    public ref class PinnedGroupNet
        {
        public:
            //! Create pinned group.
            static PinnedGroupNet^ Create();

            void AddData(RealityDataSourceNet^ pinnedData);

            List<RealityDataSourceNet^>^ GetData() { return m_pinnedDataList; }

        private:
            PinnedGroupNet();
            ~PinnedGroupNet();

            List<RealityDataSourceNet^>^ m_pinnedDataList;
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote               9/2015
    //=====================================================================================
    public ref class TerrainGroupNet
        {
        public:
            //! Create terrain group.
            static TerrainGroupNet^ Create();

            void AddData(RealityDataSourceNet^ terrainData);

            List<RealityDataSourceNet^>^ GetData() { return m_terrainDataList; }

        private:
            TerrainGroupNet();
            ~TerrainGroupNet();

            List<RealityDataSourceNet^>^ m_terrainDataList;
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote               9/2015
    //=====================================================================================
    public ref class RealityDataSourceNet
        {
        public:
            //! Create generic data source.
            static RealityDataSourceNet^ Create(System::String^ url,
                                                System::String^ type,
                                                System::String^ copyright);



            System::String^ GetUrl() { return m_url; }
            System::String^ GetSourceType() { return m_type; }
            System::String^ GetCopyright() { return m_copyright; }
            System::String^ GetXmlFragment() { return m_xmlFragment; }
            
        protected:
            RealityDataSourceNet(System::String^ url,
                                 System::String^ type,
                                 System::String^ copyright);

            ~RealityDataSourceNet();

            System::String^ m_xmlFragment;

        private:
            System::String^ m_url;
            System::String^ m_type;
            System::String^ m_copyright;  
            
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote               5/2015
    //=====================================================================================
    public ref class WmsSourceNet : public RealityDataSourceNet
        {
        public:
            static WmsSourceNet^ Create(System::String^    url,
                                        System::String^    copyright,
                                        double             bboxMinX,
                                        double             bboxMinY,
                                        double             bboxMaxX,
                                        double             bboxMaxY,
                                        System::String^    version,
                                        System::String^    layers,
                                        System::String^    csType,
                                        System::String^    csLabel,
                                        size_t             metaWidth,
                                        size_t             metaHeight,
                                        System::String^    styles,
                                        System::String^    format,
                                        System::String^    vendorSpecific,
                                        bool               isTransparent);

        private:
            WmsSourceNet(System::String^   url,
                         System::String^   copyright,         
                         double            bboxMinX,
                         double            bboxMinY,
                         double            bboxMaxX,
                         double            bboxMaxY,
                         System::String^   version,
                         System::String^   layers,
                         System::String^   csType,
                         System::String^   csLabel,
                         size_t            metaWidth,
                         size_t            metaHeight,
                         System::String^   styles,
                         System::String^   format,
                         System::String^   vendorSpecific,
                         bool              isTransparent);

            ~WmsSourceNet();
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote               9/2015
    //=====================================================================================
    public ref class UsgsSourceNet : public RealityDataSourceNet
        {
        public:
            static UsgsSourceNet^ Create(System::String^        url, 
                                         System::String^        copyright,
                                         System::String^        dataType,
                                         System::String^        dataLocation,
                                         List<System::String^>^ sisterFiles,
                                         System::String^        metadata);

        private:
            UsgsSourceNet(System::String^           url, 
                          System::String^           copyright,
                          System::String^           dataType, 
                          System::String^           dataLocation,
                          List<System::String^>^    sisterFiles,
                          System::String^           metadata);

            ~UsgsSourceNet();
        };    
    }
