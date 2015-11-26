/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/OsmSource.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <RealityPlatform/RealityPlatformAPI.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              10/2015
//=====================================================================================
struct OsmResource : public RefCountedBase
    {
public:
    //! Create OsmResource with all the required information. 
    REALITYDATAPLATFORM_EXPORT static OsmResourcePtr Create(DRange2dCR bbox);
    REALITYDATAPLATFORM_EXPORT static OsmResourcePtr CreateFromXml(Utf8CP xmlFragment);

    //! Get/Set the alternate url list. 
    REALITYDATAPLATFORM_EXPORT const bvector<Utf8String>&   GetAlternateUrlList() const;
    REALITYDATAPLATFORM_EXPORT void                         SetAlternateUrlList(const bvector<Utf8String>& urlList);

    //! Xml fragment.
    REALITYDATAPLATFORM_EXPORT void ToXml(Utf8StringR xmlFragment) const;

private:
    OsmResource(DRange2dCR bbox);
    ~OsmResource();

    bvector<Utf8String> m_alternateUrlList;     //! List of alternate urls to use if the default one is not responding.  
    DRange2d m_bbox;                            //! Bbox to append to the alternate urls.
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE