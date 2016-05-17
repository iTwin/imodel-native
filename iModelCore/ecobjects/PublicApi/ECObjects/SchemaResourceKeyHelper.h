/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/SchemaResourceKeyHelper.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/
#include "ECSchema.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*=================================================================================**//**
Resolves Resource Keys for localizable things in Schemas.
@bsiclass
+===============+===============+===============+===============+===============+======*/
struct SchemaResourceKeyHelper
    {
    public:
        static Utf8String GetDisplayLabelKey(Utf8CP key, Utf8CP invariant)
            {
            return Utf8PrintfString("%s.DisplayLabel:%s", key, ComputeHash(invariant).c_str());
            }

        static Utf8String GetDescriptionKey(Utf8CP key, Utf8CP invariant)
            {
            return Utf8PrintfString("%s.Description:%s", key, ComputeHash(invariant).c_str());
            }

        static Utf8String GetSchemaDisplayLabelKey(Utf8CP schemaName, Utf8CP invariant)
            {
            return GetDisplayLabelKey(schemaName, invariant);
            }

        static Utf8String GetSchemaDescriptionKey(Utf8CP schemaName, Utf8CP invariant)
            {
            return GetDescriptionKey(schemaName, invariant);
            }

        static Utf8String GetTypeKey(Utf8CP schemaName, Utf8CP typeName)
            {
            return Utf8PrintfString("%s:%s", schemaName, typeName);
            }

        //! Can be used to obtain a resource key for any named Type Display Label (ECClass,ECEnumeration,KindOfQuantity)
        static Utf8String GetTypeDisplayLabelKey(Utf8CP schemaName, Utf8CP typeName, Utf8CP invariant)
            {
            return GetDisplayLabelKey(GetTypeKey(schemaName, typeName).c_str(), invariant);
            }

        //! Can be used to obtain a resource key for any named Type Description (ECClass,ECEnumeration,KindOfQuantity)
        static Utf8String GetTypeDescriptionKey(Utf8CP schemaName, Utf8CP typeName, Utf8CP invariant)
            {
            return GetDescriptionKey(GetTypeKey(schemaName, typeName).c_str(), invariant);
            }

        static Utf8String GetTypeChildKey(Utf8CP schemaName, Utf8CP typeName, Utf8CP childName)
            {
            return Utf8PrintfString("%s:%s", GetTypeKey(schemaName, typeName), childName);
            }

        //! Can be used to obtain a resource key for any named Child Display Label inside a type (ECProperty,ECEnumerator)
        static Utf8String GetTypeChildDisplayLabelKey(Utf8CP schemaName, Utf8CP typeName, Utf8CP childName, Utf8CP invariant)
            {
            return GetDisplayLabelKey(GetTypeChildKey(schemaName, typeName, childName).c_str(), invariant);
            }

        //! Can be used to obtain a resource key for any named Child Description inside a type (ECProperty,ECEnumerator)
        static Utf8String GetTypeChildDescriptionKey(Utf8CP schemaName, Utf8CP typeName, Utf8CP childName, Utf8CP invariant)
            {
            return GetDescriptionKey(GetTypeChildKey(schemaName, typeName, childName).c_str(), invariant);
            }

        static Utf8String GetRelationshipSourceRoleLabelKey(Utf8CP schemaName, Utf8CP className, Utf8CP invariant)
            {
            return Utf8PrintfString("%s.SourceRoleLabel:%s", GetTypeKey(schemaName, className), ComputeHash(invariant).c_str());
            }

        static Utf8String GetRelationshipTargetRoleLabelKey(Utf8CP schemaName, Utf8CP className, Utf8CP invariant)
            {
            return Utf8PrintfString("%s.TargetRoleLabel:%s", GetTypeKey(schemaName, className), ComputeHash(invariant).c_str());
            }

        ECOBJECTS_EXPORT static Utf8String ComputeHash(Utf8CP invariant);
        ECOBJECTS_EXPORT static Utf8String GetSchemaDisplayLabelKey(ECSchemaCR ecSchema);
        ECOBJECTS_EXPORT static Utf8String GetSchemaDescriptionKey(ECSchemaCR ecSchema);
        ECOBJECTS_EXPORT static Utf8String GetTypeDisplayLabelKey(ECClassCR ecClass);
        ECOBJECTS_EXPORT static Utf8String GetTypeDisplayLabelKey(ECEnumerationCR ecEnumeration);
        ECOBJECTS_EXPORT static Utf8String GetTypeDescriptionKey(ECClassCR ecClass);
        ECOBJECTS_EXPORT static Utf8String GetTypeDescriptionKey(ECEnumerationCR ecEnumeration);
        ECOBJECTS_EXPORT static Utf8String GetTypeChildDisplayLabelKey(ECPropertyCR ecProperty);
        ECOBJECTS_EXPORT static Utf8String GetTypeChildDescriptionKey(ECPropertyCR ecProperty);
        ECOBJECTS_EXPORT static Utf8String GetTypeChildDisplayLabelKey(ECEnumeratorCR ecEnumerator);
        ECOBJECTS_EXPORT static Utf8String GetRelationshipSourceRoleLabelKey(ECRelationshipClassCR relClass, Utf8CP invariant);
        ECOBJECTS_EXPORT static Utf8String GetRelationshipTargetRoleLabelKey(ECRelationshipClassCR relClass, Utf8CP invariant);
    };

END_BENTLEY_ECOBJECT_NAMESPACE
