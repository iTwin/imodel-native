/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/SchemaLocalizedStrings.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <ECObjects/ECInstance.h>
//#include "apr_sha1.h"

EC_TYPEDEFS(SchemaLocalizedStrings);

BEGIN_BENTLEY_ECOBJECT_NAMESPACE


struct SchemaLocalizedStrings
    {
private:
    bmap<WString, WString> m_localizedStrings;
    bool m_empty;

    WStringCR GetLocalizedString(WCharCP labelKey, WStringCR invariantString) const;
    WString ComputeHash(WStringCR invariantString) const;

    bool TryConstructStringMaps(bmap<WString, bpair<size_t, WString> >& caStrings, ECSchemaCP localizationSupplemental);

    static bool TryGetStringValue(IECInstanceCR instance, WStringR value, WCharCP accessString);
    static bool TryGetBoolValue(IECInstanceCR instance, bool & value, WCharCP accessString);
    static ECObjectsStatus ParseCaKeyString(WStringR containerAccessor,WStringR caSchemaName, WStringR caClassName, WStringR propertyAccessor, WStringCR keyString, size_t prefixLength, size_t atIndex);
    static ECObjectsStatus ParseContainerAccessor(WStringR className, WStringR relEndPoint, WStringR propertyName, WStringCR containerAccessor);
    static IECCustomAttributeContainerP GetContainer(WStringCR containerAccessor, ECSchemaR primarySchema);
    static IECCustomAttributeContainerP GetClassContainer(WStringCR className, WStringCR relEndPoint, ECSchemaR primarySchema);
    static IECCustomAttributeContainerP GetPropertyContainer(WStringCR className, WStringCR propertyName, ECSchemaR primarySchema);

public:
    SchemaLocalizedStrings() { m_empty = true; }
    //! Constructs a map of localized strings from the input localization supplemental schema
    //! @param[in]  localizationSupplemental  The supplemental schema containing the localized strings.
    ECOBJECTS_EXPORT SchemaLocalizedStrings(ECSchemaCP localizationSupplemental, ECSchemaR primarySchema);

    ECOBJECTS_EXPORT WStringCR GetSchemaDisplayLabel(ECSchemaCP ecSchema, WStringCR invariantDisplayLabel) const;
    ECOBJECTS_EXPORT WStringCR GetSchemaDescription(ECSchemaCP ecSchema, WStringCR invariantDescription) const;
    ECOBJECTS_EXPORT WStringCR GetClassDisplayLabel(ECClassCP ecClass, WStringCR invariantDisplayLabel) const;
    ECOBJECTS_EXPORT WStringCR GetClassDescription(ECClassCP ecClass, WStringCR invariantDescription) const;
    ECOBJECTS_EXPORT WStringCR GetPropertyDisplayLabel(ECPropertyCP ecProperty, WStringCR invariantDisplayLabel) const;
    ECOBJECTS_EXPORT WStringCR GetPropertyDescription(ECPropertyCP ecProperty, WStringCR invariantDescription) const;
    ECOBJECTS_EXPORT WStringCR GetRelationshipSourceRoleLabel(ECRelationshipClassCP relClass, WStringCR invariantRoleLabel) const;
    ECOBJECTS_EXPORT WStringCR GetRelationshipTargetRoleLabel(ECRelationshipClassCP relClass, WStringCR invariantRoleLabel) const;

    ECOBJECTS_EXPORT static bool IsLocalizationSupplementalSchema(ECSchemaCP schema);
    ECOBJECTS_EXPORT static WString GetLocaleFromSupplementalSchema(ECSchemaCP schema);
    };


END_BENTLEY_ECOBJECT_NAMESPACE
