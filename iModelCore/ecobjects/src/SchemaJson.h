/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

// =====================================================================================
// SchemaJsonWriter class
// =====================================================================================
struct SchemaJsonWriter
    {
    private:
        BeJsValue m_jsonRoot;
        ECSchemaCR m_ecSchema;

    protected:
        template<typename T>
        bool WriteSchemaItem(T const& item)
            {
            // Don't write any classes that aren't in the schema we're writing.
            if (&(item.GetSchema()) != &m_ecSchema)
                return true;

            if (item.GetName() == "")
                {
                LOG.warningv("Schema Serialization Warning: An item was found to have an empty name and was ignored.");
                return true;
                }

            auto itemObj = m_jsonRoot[ECJSON_SCHEMA_ITEMS_ATTRIBUTE][item.GetName()];
            return item.ToJson(itemObj, false, false);
            }

        bool WriteSchemaReferences();
        bool WriteClass(ECClassCR ecClass);
        bool WriteFormat(ECFormatCR format);

        bool WriteSchemaItems();

    public:
        SchemaJsonWriter(BeJsValue jsonRoot, ECSchemaCR ecSchema);
        virtual bool Serialize();
    };

END_BENTLEY_ECOBJECT_NAMESPACE
