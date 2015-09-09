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
namespace BentleyRealityDataPackage { ref class WmsMapInfoNet; }
namespace BentleyRealityDataPackage { ref class UsgsSourceNet; }

namespace BentleyRealityDataPackage
    {   
    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote               6/2015
    //=====================================================================================
    public ref class RealityDataPackageNet
        {
        public:
            //! Create package.
            static void Create(System::String^ location, 
                               System::String^ name,
                               List<double>^ regionOfInterest,
                               List<WmsMapInfoNet^>^ wmsMapInfoList,
                               List<UsgsSourceNet^>^ usgsSourceList);
        
        private:
            RealityDataPackageNet();
            ~RealityDataPackageNet();
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote               5/2015
    //=====================================================================================
    public ref class WmsMapInfoNet
        {
        public:
            static WmsMapInfoNet^ Create(System::String^ copyright,
                                         System::String^ url,
                                         double bboxMinX,
                                         double bboxMinY,
                                         double bboxMaxX,
                                         double bboxMaxY,
                                         System::String^ version,
                                         System::String^ layers,
                                         System::String^ csType,
                                         System::String^ csLabel,
                                         size_t metaWidth,
                                         size_t metaHeight,
                                         System::String^ styles,
                                         System::String^ format,
                                         System::String^ vendorSpecific,
                                         bool isTransparent);

            System::String^ GetUrl() { return m_url; }
            System::String^ GetCopyright() { return m_copyright; }
            System::String^ GetXml() { return m_xmlFragment; }

        private:
            WmsMapInfoNet(System::String^ copyright,
                          System::String^ url,
                          double bboxMinX,
                          double bboxMinY,
                          double bboxMaxX,
                          double bboxMaxY,
                          System::String^ version,
                          System::String^ layers,
                          System::String^ csType,
                          System::String^ csLabel,
                          size_t metaWidth,
                          size_t metaHeight,
                          System::String^ styles,
                          System::String^ format,
                          System::String^ vendorSpecific,
                          bool isTransparent);

            ~WmsMapInfoNet();

            System::String^ m_url;
            System::String^ m_copyright;
            System::String^ m_xmlFragment;
        };

    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote               9/2015
    //=====================================================================================
    public ref class UsgsSourceNet
        {
        public:
            static UsgsSourceNet^ Create(System::String^ copyright,
                                         System::String^ url,
                                         System::String^ dataType,
                                         System::String^ dataLocation,
                                         List<System::String^>^ sisterFiles,
                                         System::String^ metadata);

            System::String^ GetUrl() { return m_url; }
            System::String^ GetCopyright() { return m_copyright; }
            System::String^ GetXml() { return m_xmlFragment; }

        private:
            UsgsSourceNet(System::String^ copyright, 
                          System::String^ url, 
                          System::String^ dataType, 
                          System::String^ dataLocation,
                          List<System::String^>^ sisterFiles,
                          System::String^ metadata);

            ~UsgsSourceNet();

            System::String^ m_url;
            System::String^ m_copyright;
            System::String^ m_xmlFragment;
        };
    }
