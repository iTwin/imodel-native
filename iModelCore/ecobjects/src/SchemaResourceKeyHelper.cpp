/*--------------------------------------------------------------------------------------+
|
|     $Source: src/SchemaResourceKeyHelper.cpp $
|
|   $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

static uint32_t const FNV_32_PRIME = 0x01000193;
Utf8String SchemaResourceKeyHelper::ComputeHash(Utf8CP str)
    {
    if (Utf8String::IsNullOrEmpty(str))
        return "";

    size_t len = strlen(str) * sizeof(Utf8Char);
    unsigned char *bp = (unsigned char *) str;   /* start of buffer */
    unsigned char *be = bp + len;        /* beyond end of buffer */

    uint32_t hashValue = 0;
    while (bp < be)
        {
        /* multiply by the 32 bit FNV magic prime mod 2^32 */
        hashValue *= FNV_32_PRIME;
        /* xor the bottom with the current octet */
        hashValue ^= (uint32_t) *bp++;
        }

    return Utf8PrintfString("%02x", hashValue);
    }

Utf8String SchemaResourceKeyHelper::GetSchemaDisplayLabelKey(ECSchemaCR ecSchema)
    {
    return GetSchemaDisplayLabelKey(ecSchema.GetName().c_str(), ecSchema.GetInvariantDisplayLabel().c_str());
    }

Utf8String SchemaResourceKeyHelper::GetSchemaDescriptionKey(ECSchemaCR ecSchema)
    {
    return GetSchemaDescriptionKey(ecSchema.GetName().c_str(), ecSchema.GetInvariantDescription().c_str());
    }

Utf8String SchemaResourceKeyHelper::GetTypeDisplayLabelKey(ECClassCR ecClass)
    {
    return GetTypeDisplayLabelKey(ecClass.GetSchema().GetName().c_str(),
                                  ecClass.GetName().c_str(),
                                  ecClass.GetInvariantDisplayLabel().c_str());
    }

Utf8String SchemaResourceKeyHelper::GetTypeDisplayLabelKey(ECEnumerationCR ecEnumeration)
    {
    return GetTypeDisplayLabelKey(ecEnumeration.GetSchema().GetName().c_str(),
                                  ecEnumeration.GetName().c_str(),
                                  ecEnumeration.GetInvariantDisplayLabel().c_str());
    }

Utf8String SchemaResourceKeyHelper::GetTypeDescriptionKey(ECClassCR ecClass)
    {
    return GetTypeDescriptionKey(ecClass.GetSchema().GetName().c_str(),
                                 ecClass.GetName().c_str(),
                                 ecClass.GetInvariantDescription().c_str());
    }

Utf8String SchemaResourceKeyHelper::GetTypeDescriptionKey(ECEnumerationCR ecEnumeration)
    {
    return GetTypeDescriptionKey(ecEnumeration.GetSchema().GetName().c_str(),
                                 ecEnumeration.GetName().c_str(),
                                 ecEnumeration.GetInvariantDescription().c_str());
    }

Utf8String SchemaResourceKeyHelper::GetTypeChildDisplayLabelKey(ECPropertyCR ecProperty)
    {
    return GetTypeChildDisplayLabelKey(ecProperty.GetClass().GetSchema().GetName().c_str(),
                                       ecProperty.GetClass().GetName().c_str(),
                                       ecProperty.GetName().c_str(),
                                       ecProperty.GetInvariantDisplayLabel().c_str());
    }

Utf8String SchemaResourceKeyHelper::GetTypeChildDescriptionKey(ECPropertyCR ecProperty)
    {
    return GetTypeChildDescriptionKey(ecProperty.GetClass().GetSchema().GetName().c_str(),
                                      ecProperty.GetClass().GetName().c_str(),
                                      ecProperty.GetName().c_str(),
                                      ecProperty.GetInvariantDescription().c_str());
    }

Utf8String SchemaResourceKeyHelper::GetTypeChildDisplayLabelKey(ECEnumeratorCR ecEnumerator)
    {
    Utf8String enumeratorName;
    if (ecEnumerator.IsInteger())
        enumeratorName.Sprintf("%d", ecEnumerator.GetInteger());
    else
        enumeratorName = ecEnumerator.GetString();

    return GetTypeChildDisplayLabelKey(ecEnumerator.GetEnumeration().GetSchema().GetName().c_str(),
                                       ecEnumerator.GetEnumeration().GetName().c_str(),
                                       enumeratorName.c_str(),
                                       ecEnumerator.GetInvariantDisplayLabel().c_str());
    }


Utf8String SchemaResourceKeyHelper::GetRelationshipSourceRoleLabelKey(ECRelationshipClassCR relClass, Utf8CP invariant)
    {
    return GetRelationshipSourceRoleLabelKey(
        relClass.GetSchema().GetName().c_str(), relClass.GetName().c_str(), invariant);
    }

Utf8String SchemaResourceKeyHelper::GetRelationshipTargetRoleLabelKey(ECRelationshipClassCR relClass, Utf8CP invariant)
    {
    return GetRelationshipTargetRoleLabelKey(
        relClass.GetSchema().GetName().c_str(), relClass.GetName().c_str(), invariant);
    }


END_BENTLEY_ECOBJECT_NAMESPACE

