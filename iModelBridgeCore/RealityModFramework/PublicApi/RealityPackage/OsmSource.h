/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPackage/OsmSource.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <RealityPackage/RealityPackage.h>

BEGIN_BENTLEY_REALITYPACKAGE_NAMESPACE

//=====================================================================================
//! Type used to represent Open Street Map specific resources.
//! @bsiclass                                   Jean-Francois.Cote              10/2015
//=====================================================================================
struct OsmResource : public RefCountedBase
    {
    public:
        //! Create OsmResource with all the required information. 
        REALITYPACKAGE_EXPORT static OsmResourcePtr Create(DRange2dCR bbox);
        REALITYPACKAGE_EXPORT static OsmResourcePtr CreateFromXml(Utf8CP xmlFragment);

        //! Get/Set the list of OSM specific alternate URLs to use if the default one is not responding.
        REALITYPACKAGE_EXPORT const bvector<Utf8String>&   GetAlternateUrlList() const;
        REALITYPACKAGE_EXPORT void                         SetAlternateUrlList(const bvector<Utf8String>& urlList);

        //! Generate a xml fragment.
        REALITYPACKAGE_EXPORT void ToXml(Utf8StringR xmlFragment) const;

    private:
        OsmResource(DRange2dCR bbox);
        ~OsmResource();

        bvector<Utf8String> m_alternateUrlList;     
        DRange2d m_bbox;                            // Bbox to append to the alternate urls.
    };

END_BENTLEY_REALITYPACKAGE_NAMESPACE