/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/UsgsSource.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <RealityPlatform/RealityPlatformAPI.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote               9/2015
//=====================================================================================
struct UsgsSource : public RefCountedBase
    {
public:
    //! Create UsgsSource with all the required information. 
    REALITYDATAPLATFORM_EXPORT static UsgsSourcePtr Create(Utf8CP url, Utf8CP dataType, Utf8CP metadata);

    //! Get/Set the url. 
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetUrl() const;
    REALITYDATAPLATFORM_EXPORT void         SetUrl(Utf8CP url);

    //! Get/Set the data source type.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetDataType() const;
    REALITYDATAPLATFORM_EXPORT void         SetDataType(Utf8CP type);

    //! Get/Set the location of the main file.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetDataLocation() const;
    REALITYDATAPLATFORM_EXPORT void         SetDataLocation(Utf8CP locationInCompound);

    //! Get/Set the sister files.
    REALITYDATAPLATFORM_EXPORT const bvector<Utf8String>&   GetSisterFiles() const;
    REALITYDATAPLATFORM_EXPORT void                         SetSisterFiles(const bvector<Utf8String>& sisterFiles);

    //! Get/Set the metadata. 
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetMetadata() const;
    REALITYDATAPLATFORM_EXPORT void         SetMetadata(Utf8CP metadata);

    //! Xml fragment.
    REALITYDATAPLATFORM_EXPORT void ToXml(Utf8StringR xmlFragment) const;
    REALITYDATAPLATFORM_EXPORT void FromXml(Utf8CP xmlFragment);

private:
    UsgsSource(Utf8CP url, Utf8CP dataType, Utf8CP metadata);
    ~UsgsSource();

    Utf8String          m_url;                  //! Url to zip folder.
    Utf8String          m_dataType;             //! Type of the main file.
    Utf8String          m_dataLocation;         //! Location of the main file in the folder. Include filename with extension.
    bvector<Utf8String> m_sisterFiles;          //! List of sister files.
    Utf8String          m_metadata;             //! Location of the metadata file in the folder. Include filename with xml extension.   
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE