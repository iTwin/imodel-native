/*--------------------------------------------------------------------------------------+
|
|     $Source: src/SchemaJson.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

// =====================================================================================
// SchemaJsonWriter class
// =====================================================================================
struct SchemaJsonWriter
    {
    private:
        Json::Value& m_jsonRoot;
        ECSchemaCR m_ecSchema;

    protected:
        SchemaWriteStatus WriteSchemaReferences();
        SchemaWriteStatus WriteClass(ECClassCR ecClass);
        SchemaWriteStatus WriteEnumeration(ECEnumerationCR ecEnumeration);
        SchemaWriteStatus WriteKindOfQuantity(KindOfQuantityCR kindOfQuantity);
        SchemaWriteStatus WritePropertyCategory(PropertyCategoryCR propertyCategory);
        SchemaWriteStatus WriteSchemaChildren();

    public:
        SchemaJsonWriter(Json::Value& jsonRoot, ECSchemaCR ecSchema);
        virtual SchemaWriteStatus Serialize();
    };

END_BENTLEY_ECOBJECT_NAMESPACE
