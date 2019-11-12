/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <RealityPlatform/RealityPlatformAPI.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=====================================================================================
//! Type used to represent Open Street Map specific resources.
//! @bsiclass                                   Jean-Francois.Cote              10/2015
//=====================================================================================
struct OsmResource : public RefCountedBase
    {
    public:
        //! Create OsmResource with all the required information. 
        REALITYDATAPLATFORM_EXPORT static OsmResourcePtr Create(DRange2dCR bbox);
        REALITYDATAPLATFORM_EXPORT static OsmResourcePtr CreateFromXml(Utf8CP xmlFragment);

        //! Get/Set the list of OSM specific alternate URLs to use if the default one is not responding.
        REALITYDATAPLATFORM_EXPORT const bvector<Utf8String>&   GetAlternateUrlList() const;
        REALITYDATAPLATFORM_EXPORT void                         SetAlternateUrlList(const bvector<Utf8String>& urlList);

        //! Generate a xml fragment.
        REALITYDATAPLATFORM_EXPORT void ToXml(Utf8StringR xmlFragment) const;

    private:
        OsmResource(DRange2dCR bbox);
        ~OsmResource();

        bvector<Utf8String> m_alternateUrlList;     
        DRange2d m_bbox;                            // Bbox to append to the alternate urls.
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE