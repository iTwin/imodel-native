/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/Localization.h>
#include <BeSQLite/L10N.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ILocalizationProvider::GetString(Utf8StringCR locale, Utf8StringCR key, Utf8StringR localizedValue) const {return _GetString(locale, key, localizedValue);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SQLangLocalizationProvider::ParseKey(Utf8StringR ns, Utf8StringR id, Utf8StringCR key)
    {
    size_t pos;
    if (Utf8String::npos == (pos = key.GetNextToken(ns, ":", 0)))
        return ERROR;

    key.GetNextToken(id, ":", pos);
    return (!ns.empty() && !id.empty()) ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool SQLangLocalizationProvider::_GetString(Utf8StringCR, Utf8StringCR key, Utf8StringR localizedValue) const
    {
    Utf8String ns, id;
    if (SUCCESS != ParseKey(ns, id, key))
        return false;

    bool hasString = false;
    localizedValue = L10N::GetString(BeSQLite::L10N::NameSpace(ns.c_str()), BeSQLite::L10N::StringId(id.c_str()), &hasString);
    return hasString;
    }