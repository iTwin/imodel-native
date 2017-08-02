/*--------------------------------------------------------------------------------------+
|
|     $Source: src/SchemaXml.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

// =====================================================================================
// SchemaXMLReader class
// =====================================================================================
struct SchemaXmlReader
{
private:
    BeXmlDomR               m_xmlDom;
    ECSchemaReadContextR    m_schemaContext;

public:
    SchemaXmlReader(ECSchemaReadContextR context, BeXmlDomR xmlDom);
    SchemaReadStatus Deserialize(ECSchemaPtr& ecSchema, uint32_t checkSum);

    static void SetErrorHandling(bool doAssert);

    static SchemaReadStatus ReadSchemaStub(SchemaKey& schemaKey, uint32_t& ecXmlMajorVersion, uint32_t& ecXmlMinorVersion, BeXmlNodeP& schemaNode, BeXmlDomR xmlDom);
};

// =====================================================================================
// SchemaXmlWriter class
// =====================================================================================
struct SchemaXmlWriter
    {
    private:
        struct  ECSchemaWriteContext
            {
            public:
            bset<Utf8CP> m_alreadyWrittenClasses;
            };

        BeXmlWriterR m_xmlWriter;
        ECSchemaCR m_ecSchema;
        ECSchemaWriteContext m_context;
        ECVersion m_ecXmlVersion;

    protected:
        SchemaWriteStatus                   WriteSchemaReferences ();
        SchemaWriteStatus                   WriteClass (ECClassCR ecClass);
        SchemaWriteStatus                   WriteEnumeration(ECEnumerationCR ecEnumeration);
        SchemaWriteStatus                   WriteKindOfQuantity(KindOfQuantityCR kindOfQuantity);
        SchemaWriteStatus                   WritePropertyCategory(PropertyCategoryCR propertyCategory);
        SchemaWriteStatus                   WriteCustomAttributeDependencies (IECCustomAttributeContainerCR container);
        SchemaWriteStatus                   WritePropertyDependencies (ECClassCR ecClass);

    public:
        SchemaXmlWriter(BeXmlWriterR xmlWriter, ECSchemaCR ecSchema, ECVersion ecXmlVersion = ECVersion::V2_0);
        virtual SchemaWriteStatus Serialize(bool utf16 = false);
    };


END_BENTLEY_ECOBJECT_NAMESPACE
