/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityServicesNet/RealityPackageNet/OsmSourceNet.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <RealityPackage/RealityPackage.h>
#include <RealityPackage/OsmSource.h>

namespace RealityPackageNet
    {
    //=====================================================================================
    //! @bsiclass                                   Jean-Francois.Cote              10/2016
    //=====================================================================================
    public ref class OsmResourceNet
        {
        public:
            //! Create OsmResource with all the required information. 
            static OsmResourceNet^ Create(System::Collections::Generic::List<double>^ bbox);

            //! Get/Set the alternate url list. 
            System::Collections::Generic::List<System::String^>^ GetAlternateUrlList();
            void SetAlternateUrlList(System::Collections::Generic::List<System::String^>^ urlList);

            //! Xml fragment.
            System::String^ ToXml();

        protected:
            OsmResourceNet(System::Collections::Generic::List<double>^ bbox);
            ~OsmResourceNet();
            !OsmResourceNet();

        private:
            RealityPackage::OsmResourcePtr* m_pOsmResource;
        };
    }