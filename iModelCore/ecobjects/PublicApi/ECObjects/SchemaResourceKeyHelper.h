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
        ECOBJECTS_EXPORT static Utf8String GetSchemaDisplayLabelKey(Utf8CP schemaLegacyName, Utf8CP invariant);
        ECOBJECTS_EXPORT static Utf8String GetSchemaDescriptionKey(Utf8CP schemaLegacyName, Utf8CP invariant);
        //! Can be used to obtain a resource key for any named Type Display Label (ECClass,ECEnumeration,KindOfQuantity)
        ECOBJECTS_EXPORT static Utf8String GetTypeDisplayLabelKey(Utf8CP schemaLegacyName, Utf8CP typeName, Utf8CP invariant);
        //! Can be used to obtain a resource key for any named Type Description (ECClass,ECEnumeration,KindOfQuantity)
        ECOBJECTS_EXPORT static Utf8String GetTypeDescriptionKey(Utf8CP schemaLegacyName, Utf8CP typeName, Utf8CP invariant);
        //! Can be used to obtain a resource key for any named Child Display Label inside a type (ECProperty,ECEnumerator)
        ECOBJECTS_EXPORT static Utf8String GetTypeChildDisplayLabelKey(Utf8CP schemaLegacyName, Utf8CP typeName, Utf8CP childName, Utf8CP invariant);
        //! Can be used to obtain a resource key for any named Child Description inside a type (ECProperty,ECEnumerator)
        ECOBJECTS_EXPORT static Utf8String GetTypeChildDescriptionKey(Utf8CP schemaLegacyName, Utf8CP typeName, Utf8CP childName, Utf8CP invariant);
        ECOBJECTS_EXPORT static Utf8String GetRelationshipSourceRoleLabelKey(Utf8CP schemaLegacyName, Utf8CP className, Utf8CP invariant);
        ECOBJECTS_EXPORT static Utf8String GetRelationshipTargetRoleLabelKey(Utf8CP schemaLegacyName, Utf8CP className, Utf8CP invariant);
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
