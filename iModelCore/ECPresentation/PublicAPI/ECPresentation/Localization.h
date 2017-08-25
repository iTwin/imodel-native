/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/Localization.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once 
//__PUBLISH_SECTION_START__

#include <ECPresentation/ECPresentation.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

#define PRESENTATION_LOCALIZEDSTRING_FORMAT           "@%s:%s@"
#define PRESENTATION_LOCALIZEDSTRING(ns,id)           BeStringUtilities::Join(bvector<Utf8CP>{"@", ns, ":", id, "@"})

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                08/2015
+===============+===============+===============+===============+===============+======*/
struct ILocalizationProvider
{
protected:
    virtual bool _GetString(Utf8StringCR key, Utf8StringR localizedValue) const = 0;
public:
    ECPRESENTATION_EXPORT bool GetString(Utf8StringCR key, Utf8StringR localizedValue) const;
    Utf8String GetString(Utf8StringCR key) const {Utf8String str; return GetString(key, str) ? str : key;}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE