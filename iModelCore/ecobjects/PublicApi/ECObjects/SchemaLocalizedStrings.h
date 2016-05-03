/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/SchemaLocalizedStrings.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <ECObjects/ECInstance.h>
#include <ECObjects/SchemaResourceKeyHelper.h>
//#include "apr_sha1.h"

EC_TYPEDEFS(SchemaLocalizedStrings);

BEGIN_BENTLEY_ECOBJECT_NAMESPACE


struct SchemaLocalizedStrings
    {
private:
    bmap<Utf8String, Utf8String> m_localizedStrings;
    bool m_empty;

    Utf8StringCR GetLocalizedString(Utf8CP labelKey, Utf8StringCR invariantString) const;

    bool TryConstructStringMaps(bmap<Utf8String, bpair<size_t, Utf8String> >& caStrings, ECSchemaCP localizationSupplemental);

    static bool TryGetStringValue(IECInstanceCR instance, Utf8StringR value, Utf8CP accessString);
    static bool TryGetBoolValue(IECInstanceCR instance, bool & value, Utf8CP accessString);
    static ECObjectsStatus ParseCaKeyString(Utf8StringR containerAccessor, Utf8StringR caSchemaName, Utf8StringR caClassName, Utf8StringR propertyAccessor, Utf8StringCR keyString, size_t prefixLength, size_t atIndex);
    static ECObjectsStatus ParseContainerAccessor(Utf8StringR className, Utf8StringR relEndPoint, Utf8StringR propertyName, Utf8StringCR containerAccessor);
    static IECCustomAttributeContainerP GetContainer(Utf8StringCR containerAccessor, ECSchemaR primarySchema);
    static IECCustomAttributeContainerP GetClassContainer(Utf8StringCR className, Utf8StringCR relEndPoint, ECSchemaR primarySchema);
    static IECCustomAttributeContainerP GetPropertyContainer(Utf8StringCR className, Utf8StringCR propertyName, ECSchemaR primarySchema);

public:
    SchemaLocalizedStrings() { m_empty = true; }
    //! Constructs a map of localized strings from the input localization supplemental schema
    //! @param[in]  localizationSupplemental  The supplemental schema containing the localized strings.
    ECOBJECTS_EXPORT SchemaLocalizedStrings(ECSchemaCP localizationSupplemental, ECSchemaR primarySchema);

    ECOBJECTS_EXPORT Utf8StringCR GetSchemaDisplayLabel(ECSchemaCP ecSchema, Utf8StringCR invariantDisplayLabel) const;
    ECOBJECTS_EXPORT Utf8StringCR GetSchemaDescription(ECSchemaCP ecSchema, Utf8StringCR invariantDescription) const;
    ECOBJECTS_EXPORT Utf8StringCR GetClassDisplayLabel(ECClassCP ecClass, Utf8StringCR invariantDisplayLabel) const;
    ECOBJECTS_EXPORT Utf8StringCR GetEnumerationDisplayLabel(ECEnumerationCR ecEnumeration, Utf8StringCR invariantDisplayLabel) const;
    ECOBJECTS_EXPORT Utf8StringCR GetClassDescription(ECClassCP ecClass, Utf8StringCR invariantDescription) const;
    ECOBJECTS_EXPORT Utf8StringCR GetEnumerationDescription(ECEnumerationCR ecEnumeration, Utf8StringCR invariantDescription) const;
    ECOBJECTS_EXPORT Utf8StringCR GetPropertyDisplayLabel(ECPropertyCP ecProperty, Utf8StringCR invariantDisplayLabel) const;
    ECOBJECTS_EXPORT Utf8StringCR GetPropertyDescription(ECPropertyCP ecProperty, Utf8StringCR invariantDescription) const;
    ECOBJECTS_EXPORT Utf8StringCR GetEnumeratorDisplayLabel(ECEnumeratorCR ecEnumerator, Utf8StringCR invariantDisplayLabel) const;
    ECOBJECTS_EXPORT Utf8StringCR GetRelationshipSourceRoleLabel(ECRelationshipClassCP relClass, Utf8StringCR invariantRoleLabel) const;
    ECOBJECTS_EXPORT Utf8StringCR GetRelationshipTargetRoleLabel(ECRelationshipClassCP relClass, Utf8StringCR invariantRoleLabel) const;

    ECOBJECTS_EXPORT static bool IsLocalizationSupplementalSchema(ECSchemaCP schema);
    ECOBJECTS_EXPORT static Utf8String GetLocaleFromSupplementalSchema(ECSchemaCP schema);
    };


END_BENTLEY_ECOBJECT_NAMESPACE
