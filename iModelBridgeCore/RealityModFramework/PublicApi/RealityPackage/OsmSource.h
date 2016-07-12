/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPackage/OsmSource.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <RealityPackage/RealityPackage.h>

BEGIN_BENTLEY_REALITYPACKAGE_NAMESPACE

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote              10/2015
//=====================================================================================
struct OsmResource : public RefCountedBase
    {
public:
    //! Create OsmResource with all the required information. 
    REALITYPACKAGE_EXPORT static OsmResourcePtr Create(DRange2dCR bbox);
    REALITYPACKAGE_EXPORT static OsmResourcePtr CreateFromXml(Utf8CP xmlFragment);

    //! Get/Set the alternate url list. 
    REALITYPACKAGE_EXPORT const bvector<Utf8String>&   GetAlternateUrlList() const;
    REALITYPACKAGE_EXPORT void                         SetAlternateUrlList(const bvector<Utf8String>& urlList);

    //! Xml fragment.
    REALITYPACKAGE_EXPORT void ToXml(Utf8StringR xmlFragment) const;

private:
    OsmResource(DRange2dCR bbox);
    ~OsmResource();

    bvector<Utf8String> m_alternateUrlList;     //! List of alternate urls to use if the default one is not responding.  
    DRange2d m_bbox;                            //! Bbox to append to the alternate urls.
    };

END_BENTLEY_REALITYPACKAGE_NAMESPACE