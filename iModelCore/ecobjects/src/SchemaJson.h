/*--------------------------------------------------------------------------------------+
|
|     $Source: src/SchemaJson.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
        template<typename T>
        bool WriteSchemaItem(T const& item)
            {
            // Don't write any classes that aren't in the schema we're writing.
            if (&(item.GetSchema()) != &m_ecSchema)
                return true;

            Json::Value& itemObj = m_jsonRoot[ECJSON_SCHEMA_ITEMS_ATTRIBUTE][item.GetName()];
            return item.ToJson(itemObj, false, false);
            }

        bool WriteSchemaReferences();
        bool WriteClass(ECClassCR ecClass);
        bool WriteFormat(ECFormatCR format);

        bool WriteSchemaItems();

    public:
        SchemaJsonWriter(Json::Value& jsonRoot, ECSchemaCR ecSchema);
        virtual bool Serialize();
    };

END_BENTLEY_ECOBJECT_NAMESPACE
