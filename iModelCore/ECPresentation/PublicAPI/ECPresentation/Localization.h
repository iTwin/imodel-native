/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/Localization.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once 
//__PUBLISH_SECTION_START__

#include <ECPresentation/ECPresentation.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

#define PRESENTATION_LOCALIZEDSTRING_FORMAT           "@%s:%s@"
#define PRESENTATION_LOCALIZEDSTRING(ns,id)           BeStringUtilities::Join(bvector<Utf8CP>{"@", ns, ":", id, "@"})

/*=================================================================================**//**
//! Gets a localized string using provided key in format `namespace_id:string_id`
* @bsiclass                                     Grigas.Petraitis                08/2015
+===============+===============+===============+===============+===============+======*/
struct ILocalizationProvider
{
protected:
    virtual bool _GetString(Utf8StringCR locale, Utf8StringCR key, Utf8StringR localizedValue) const = 0;
public:
    virtual ~ILocalizationProvider() {}
    ECPRESENTATION_EXPORT bool GetString(Utf8StringCR locale, Utf8StringCR key, Utf8StringR localizedValue) const;
    Utf8String GetString(Utf8StringCR locale, Utf8StringCR key) const {Utf8String str; return GetString(locale, key, str) ? str : key;}
    template<typename TStr> Utf8String GetString(Utf8StringCR locale, bvector<TStr> const& keyParts) const
        {
        Utf8String key = BeStringUtilities::Join(keyParts, ":");
        Utf8String str; 
        return GetString(locale, key, str) ? str : key;
        }
};

/*=================================================================================**//**
//! Gets localized strings using L10N API.
//! Note: this localization provider doesn't handle different locales - it just uses
//! whatever strings the active sqlang contains.
* @bsiclass                                     Grigas.Petraitis                08/2015
+===============+===============+===============+===============+===============+======*/
struct EXPORT_VTABLE_ATTRIBUTE SQLangLocalizationProvider : ILocalizationProvider
{
private:
    static StatusInt ParseKey(Utf8StringR ns, Utf8StringR id, Utf8StringCR key);
protected:
    ECPRESENTATION_EXPORT bool _GetString(Utf8StringCR, Utf8StringCR key, Utf8StringR localizedValue) const override;
};

END_BENTLEY_ECPRESENTATION_NAMESPACE