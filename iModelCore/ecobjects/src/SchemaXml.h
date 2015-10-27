/*--------------------------------------------------------------------------------------+
|
|     $Source: src/SchemaXml.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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

    bool IsOpenPlantPidCircularReferenceSpecialCase(Utf8String& referencedECSchemaName, Utf8String& referencingECSchemaFullName);

protected:
    SchemaReadStatus ReadSchemaReferencesFromXml(ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode);

    typedef bvector<bpair<ECClassP, BeXmlNodeP> >  ClassDeserializationVector;
    SchemaReadStatus                    ReadClassStubsFromXml (ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode, ClassDeserializationVector& classes);
    SchemaReadStatus                    ReadClassContentsFromXml (ECSchemaPtr& schemaOut, ClassDeserializationVector&  classes);

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
            bset<Utf8CP> m_alreadyWrittenClasses;
            };

        BeXmlWriterR m_xmlWriter;
        ECSchemaCR m_ecSchema;
        ECSchemaWriteContext m_context;

    protected:
        SchemaWriteStatus                   WriteSchemaReferences ();
        SchemaWriteStatus                   WriteClass (ECClassCR ecClass);
        SchemaWriteStatus                   WriteCustomAttributeDependencies (IECCustomAttributeContainerCR container);
        SchemaWriteStatus                   WritePropertyDependencies (ECClassCR ecClass);

    public:
        SchemaXmlWriter(BeXmlWriterR xmlWriter, ECSchemaCR ecSchema);
        virtual SchemaWriteStatus Serialize();
    };


END_BENTLEY_ECOBJECT_NAMESPACE
