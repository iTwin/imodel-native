/*--------------------------------------------------------------------------------------+
|
|     $Source: src/SchemaResourceKeyHelper.cpp $
|
|   $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

typedef uint32_t Fnv32_t;
#define FNV_32_PRIME ((Fnv32_t)0x01000193)
Utf8String SchemaResourceKeyHelper::ComputeHash(Utf8CP str)
    {
    if (Utf8String::IsNullOrEmpty(str))
        return "";
    
    return "";
    //Disabled hashes for the moment until tests and tools are adjusted
    //size_t len = strlen(str) * sizeof(Utf8Char);
    //unsigned char *bp = (unsigned char *) str;   /* start of buffer */
    //unsigned char *be = bp + len;        /* beyond end of buffer */

    //uint32_t hashValue = 0;
    //while (bp < be)
    //    {
    //    /* multiply by the 32 bit FNV magic prime mod 2^32 */
    //    hashValue *= FNV_32_PRIME;
    //    /* xor the bottom with the current octet */
    //    hashValue ^= (uint32_t) *bp++;
    //    }

    //return Utf8PrintfString("%02x", hashValue);
    }

// Standard:Schema.04.02.DisplayLabel:[Hash]
Utf8String SchemaResourceKeyHelper::GetSchemaDisplayLabelKey(Utf8CP schemaLegacyName, Utf8CP invariant)
    {
    return Utf8PrintfString("Standard:%s.DisplayLabel:%s", schemaLegacyName, ComputeHash(invariant).c_str());
    }

// Standard:Schema.04.02.Description:[Hash]
Utf8String SchemaResourceKeyHelper::GetSchemaDescriptionKey(Utf8CP schemaLegacyName, Utf8CP invariant)
    {
    return Utf8PrintfString("Standard:%s.Description:%s", schemaLegacyName, ComputeHash(invariant).c_str());
    }

// Standard:Schema.04.02:Class.DisplayLabel:[Hash]
Utf8String SchemaResourceKeyHelper::GetTypeDisplayLabelKey(Utf8CP schemaLegacyName, Utf8CP typeName, Utf8CP invariant)
    {
    return Utf8PrintfString("Standard:%s:%s.DisplayLabel:%s", schemaLegacyName, typeName, ComputeHash(invariant).c_str());
    }

// Standard:Schema.04.02:Class.Description:[Hash]
Utf8String SchemaResourceKeyHelper::GetTypeDescriptionKey(Utf8CP schemaLegacyName, Utf8CP typeName, Utf8CP invariant)
    {
    return Utf8PrintfString("Standard:%s:%s.Description:%s", schemaLegacyName, typeName, ComputeHash(invariant).c_str());
    }

// Standard:Schema.04.02:Class:Property.DisplayLabel:[Hash]
Utf8String SchemaResourceKeyHelper::GetTypeChildDisplayLabelKey(Utf8CP schemaLegacyName, Utf8CP typeName, Utf8CP childName, Utf8CP invariant)
    {
    return Utf8PrintfString("Standard:%s:%s:%s.DisplayLabel:%s", schemaLegacyName, typeName, childName, ComputeHash(invariant).c_str());
    }

// Standard:Schema.04.02:Class:Property.Description:[Hash]
Utf8String SchemaResourceKeyHelper::GetTypeChildDescriptionKey(Utf8CP schemaLegacyName, Utf8CP typeName, Utf8CP childName, Utf8CP invariant)
    {
    return Utf8PrintfString("Standard:%s:%s:%s.Description:%s", schemaLegacyName, typeName, childName, ComputeHash(invariant).c_str());
    }

// Standard:Schema.04.02:RelClass.SourceRoleLabel:[Hash]
Utf8String SchemaResourceKeyHelper::GetRelationshipSourceRoleLabelKey(Utf8CP schemaLegacyName, Utf8CP className, Utf8CP invariant)
    {
    return Utf8PrintfString("Standard:%s:%s.SourceRoleLabel:%s", schemaLegacyName, className, ComputeHash(invariant).c_str());
    }

// Standard:Schema.04.02:RelClass.TargetRoleLabel:[Hash]
Utf8String SchemaResourceKeyHelper::GetRelationshipTargetRoleLabelKey(Utf8CP schemaLegacyName, Utf8CP className, Utf8CP invariant)
    {
    return Utf8PrintfString("Standard:%s:%s.TargetRoleLabel:%s", schemaLegacyName, className, ComputeHash(invariant).c_str());
    }


Utf8String SchemaResourceKeyHelper::GetSchemaDisplayLabelKey(ECSchemaCR ecSchema)
    {
    return GetSchemaDisplayLabelKey(ecSchema.GetLegacyFullSchemaName().c_str(), ecSchema.GetInvariantDisplayLabel().c_str());
    }

Utf8String SchemaResourceKeyHelper::GetSchemaDescriptionKey(ECSchemaCR ecSchema)
    {
    return GetSchemaDescriptionKey(ecSchema.GetLegacyFullSchemaName().c_str(), ecSchema.GetInvariantDescription().c_str());
    }

Utf8String SchemaResourceKeyHelper::GetTypeDisplayLabelKey(ECClassCR ecClass)
    {
    return GetTypeDisplayLabelKey(ecClass.GetSchema().GetLegacyFullSchemaName().c_str(),
                                  ecClass.GetName().c_str(),
                                  ecClass.GetInvariantDisplayLabel().c_str());
    }

Utf8String SchemaResourceKeyHelper::GetTypeDisplayLabelKey(ECEnumerationCR ecEnumeration)
    {
    return GetTypeDisplayLabelKey(ecEnumeration.GetSchema().GetLegacyFullSchemaName().c_str(),
                                  ecEnumeration.GetName().c_str(),
                                  ecEnumeration.GetInvariantDisplayLabel().c_str());
    }

Utf8String SchemaResourceKeyHelper::GetTypeDescriptionKey(ECClassCR ecClass)
    {
    return GetTypeDescriptionKey(ecClass.GetSchema().GetLegacyFullSchemaName().c_str(),
                                 ecClass.GetName().c_str(),
                                 ecClass.GetInvariantDescription().c_str());
    }

Utf8String SchemaResourceKeyHelper::GetTypeDescriptionKey(ECEnumerationCR ecEnumeration)
    {
    return GetTypeDescriptionKey(ecEnumeration.GetSchema().GetLegacyFullSchemaName().c_str(),
                                 ecEnumeration.GetName().c_str(),
                                 ecEnumeration.GetInvariantDescription().c_str());
    }

Utf8String SchemaResourceKeyHelper::GetTypeChildDisplayLabelKey(ECPropertyCR ecProperty)
    {
    return GetTypeChildDisplayLabelKey(ecProperty.GetClass().GetSchema().GetLegacyFullSchemaName().c_str(),
                                       ecProperty.GetClass().GetName().c_str(),
                                       ecProperty.GetName().c_str(),
                                       ecProperty.GetInvariantDisplayLabel().c_str());
    }

Utf8String SchemaResourceKeyHelper::GetTypeChildDescriptionKey(ECPropertyCR ecProperty)
    {
    return GetTypeChildDescriptionKey(ecProperty.GetClass().GetSchema().GetLegacyFullSchemaName().c_str(),
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

    return GetTypeChildDisplayLabelKey(ecEnumerator.GetEnumeration().GetSchema().GetLegacyFullSchemaName().c_str(),
                                       ecEnumerator.GetEnumeration().GetName().c_str(),
                                       enumeratorName.c_str(),
                                       ecEnumerator.GetInvariantDisplayLabel().c_str());
    }


Utf8String SchemaResourceKeyHelper::GetRelationshipSourceRoleLabelKey(ECRelationshipClassCR relClass, Utf8CP invariant)
    {
    return GetRelationshipSourceRoleLabelKey(
        relClass.GetSchema().GetLegacyFullSchemaName().c_str(), relClass.GetName().c_str(), invariant);
    }

Utf8String SchemaResourceKeyHelper::GetRelationshipTargetRoleLabelKey(ECRelationshipClassCR relClass, Utf8CP invariant)
    {
    return GetRelationshipTargetRoleLabelKey(
        relClass.GetSchema().GetLegacyFullSchemaName().c_str(), relClass.GetName().c_str(), invariant);
    }


END_BENTLEY_ECOBJECT_NAMESPACE

