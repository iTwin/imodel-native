/*--------------------------------------------------------------------------------------+
|
|     $Source: src/SchemaXml.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

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
};

// =====================================================================================
// SchemaXmlWriter class
// =====================================================================================
struct SchemaXmlWriter
    {
    private:
        struct  ECSchemaWriteContext
            {
            private:
            bool m_preserveElementOrder = false;

            public:
            bset<Utf8CP> m_alreadyWrittenClasses;
            bool GetPreserveElementOrder() { return m_preserveElementOrder; }
            void SetPreserveElementOrder(bool flag) { m_preserveElementOrder = flag; }
            };

        BeXmlWriterR m_xmlWriter;
        ECSchemaCR m_ecSchema;
        ECSchemaWriteContext m_context;
        int m_ecXmlVersionMajor;
        int m_ecXmlVersionMinor;

    protected:
        SchemaWriteStatus                   WriteSchemaReferences ();
        SchemaWriteStatus                   WriteClass (ECClassCR ecClass);
        SchemaWriteStatus                   WriteEnumeration(ECEnumerationCR ecEnumeration);
        SchemaWriteStatus                   WriteCustomAttributeDependencies (IECCustomAttributeContainerCR container);
        SchemaWriteStatus                   WritePropertyDependencies (ECClassCR ecClass);

    public:
        SchemaXmlWriter(BeXmlWriterR xmlWriter, ECSchemaCR ecSchema, int ecXmlVersionMajor = 2, int ecXmlVersionMinor = 0);
        virtual SchemaWriteStatus Serialize(bool utf16 = false);
    };


END_BENTLEY_ECOBJECT_NAMESPACE
