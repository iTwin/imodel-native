/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "ECObjects/ECSchema.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

using ClassDeserializationVector = bvector<bpair<ECClassP, BeXmlNodeP>>;

// =====================================================================================
// SchemaXMLReader class
// =====================================================================================
struct SchemaXmlReader
{
private:
    BeXmlDomR               m_xmlDom;
    ECSchemaReadContextR    m_schemaContext;

public:
    SchemaXmlReader(ECSchemaReadContextR context, BeXmlDomR xmlDom) : m_schemaContext(context), m_xmlDom(xmlDom) {}
    SchemaReadStatus Deserialize(ECSchemaPtr& ecSchema, Utf8CP checksum = nullptr);

    static void SetErrorHandling(bool doAssert);

    static SchemaReadStatus ReadSchemaStub(SchemaKey& schemaKey, uint32_t& ecXmlMajorVersion, uint32_t& ecXmlMinorVersion, BeXmlNodeP& schemaNode, BeXmlDomR xmlDom);
};

// =====================================================================================
// SchemaXmlReaderImpl class
// =====================================================================================
struct SchemaXmlReaderImpl
{
protected:
    BeXmlDomR m_xmlDom;
    ECSchemaReadContextR m_schemaContext;
    ECSchemaPtr m_conversionSchema;
    bvector<Utf8String> m_droppedPrefixes;

    bool IsOpenPlantPidCircularReferenceSpecialCase(Utf8String& referencedECSchemaName, Utf8String& referencingECSchemaFullName);
    bool IsDgnV8DeliveredSchema(Utf8CP referencedECSchemaName);

    virtual bool ReadClassNode(ECClassP &ecClass, BeXmlNodeR classNode, ECSchemaPtr& schemaOut) = 0;
    virtual SchemaReadStatus _ReadClassContentsFromXml(ECSchemaPtr& schemaOut, ClassDeserializationVector& classes);
    virtual SchemaReadStatus _ReadSchemaReferencesFromXml(ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode);

    virtual bool _IsSchemaChildElementNode(BeXmlNodeR schemaNode, ECSchemaElementType childType, ECVersion xmlVersion = ECVersion::Latest);

protected:
    ECEntityClassP CreateEntityClass(ECSchemaPtr& schemaOut) {return new ECEntityClass(*schemaOut);}
    ECStructClassP CreateStructClass(ECSchemaPtr& schemaOut) {return new ECStructClass(*schemaOut);}
    ECCustomAttributeClassP CreateCustomAttributeClass(ECSchemaPtr& schemaOut) {return new ECCustomAttributeClass(*schemaOut);}
    ECRelationshipClassP CreateRelationshipClass(ECSchemaPtr& schemaOut) {return new ECRelationshipClass(*schemaOut);}

public:
    SchemaXmlReaderImpl(ECSchemaReadContextR context, BeXmlDomR xmlDom) : m_schemaContext(context), m_xmlDom(xmlDom) {}
    virtual ~SchemaXmlReaderImpl() {}
    virtual SchemaReadStatus ReadSchemaReferencesFromXml(ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode) = 0;

    virtual SchemaReadStatus ReadClassStubsFromXml(ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode, ClassDeserializationVector& classes, int ecXmlVersionMajor);
    virtual SchemaReadStatus ReadClassContentsFromXml(ECSchemaPtr& schemaOut, ClassDeserializationVector&  classes) = 0;
    
    template<typename T>
    SchemaReadStatus ReadSchemaChildFromXml(ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode, ECSchemaElementType childType);

    // This method encompasses all Unit Type; Unit, Constant, InvertedUnit, Phenomenon and UnitSystem.
    // Needed because the ReadSchemaChildFromXml does not pass a name to the constructor, which is required
    // to construct any Unit type.
    template<typename T>
    SchemaReadStatus ReadUnitTypeFromXml(ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode, ECSchemaElementType unitType);
    void PopulateSchemaElementOrder(ECSchemaElementsOrder& elementOrder, BeXmlNodeR schemaNode);
    virtual bool IsSchemaChildElementNode(BeXmlNodeR schemaNode, ECSchemaElementType childType) {return _IsSchemaChildElementNode(schemaNode, childType);}
};

// =====================================================================================
// SchemaXmlReader2 class
// =====================================================================================
struct SchemaXmlReader2 : SchemaXmlReaderImpl
{
private:
    void DetermineClassTypeAndModifier(Utf8StringCR className, ECSchemaPtr schemaOut, ECClassType& classType, ECClassModifier& classModifier, bool isCA, bool isStruct, bool isDomain, bool isSealed) const;
    ECClassModifier DetermineRelationshipClassModifier(Utf8StringCR className, bool isDomain) const;
    bool DropClassAttributeDefined(Utf8StringCR className) const;

protected:
    bool ReadClassNode(ECClassP &ecClass, BeXmlNodeR classNode, ECSchemaPtr& schemaOut) override;

public:
    SchemaXmlReader2(ECSchemaReadContextR context, ECSchemaPtr schemaOut, BeXmlDomR xmlDom) : SchemaXmlReaderImpl(context, xmlDom)
        {
        m_conversionSchema = context.LocateConversionSchemaFor(schemaOut->GetName().c_str(), schemaOut->GetVersionRead(), schemaOut->GetVersionMinor());
        }
    SchemaReadStatus ReadSchemaReferencesFromXml(ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode) override {return _ReadSchemaReferencesFromXml(schemaOut, schemaNode);}
    SchemaReadStatus ReadClassContentsFromXml(ECSchemaPtr& schemaOut, ClassDeserializationVector& classes) override {return _ReadClassContentsFromXml(schemaOut, classes);}

    bool IsSchemaChildElementNode(BeXmlNodeR schemaNode, ECSchemaElementType childType) override {return _IsSchemaChildElementNode(schemaNode, childType, ECVersion::V2_0);}
};

// =====================================================================================
// SchemaXmlReader3 class
// =====================================================================================
struct SchemaXmlReader3 : SchemaXmlReaderImpl
{
protected:
    bool ReadClassNode(ECClassP &ecClass, BeXmlNodeR classNode, ECSchemaPtr& schemaOut) override;

public:
    SchemaXmlReader3(ECSchemaReadContextR context, BeXmlDomR xmlDom) : SchemaXmlReaderImpl(context, xmlDom) {}
    SchemaReadStatus ReadSchemaReferencesFromXml(ECSchemaPtr& schemaOut, BeXmlNodeR schemaNode) override {return _ReadSchemaReferencesFromXml(schemaOut, schemaNode);}
    SchemaReadStatus ReadClassContentsFromXml(ECSchemaPtr& schemaOut, ClassDeserializationVector& classes) override {return _ReadClassContentsFromXml(schemaOut, classes);}
};

// =====================================================================================
// SchemaXmlWriter class
// =====================================================================================
struct SchemaXmlWriter
{
private:
    struct ECSchemaWriteContext
        {
        public:
            bset<Utf8CP> m_alreadyWrittenClasses;
        };

    BeXmlWriterR m_xmlWriter;
    ECSchemaCR m_ecSchema;
    ECSchemaWriteContext m_context;
    ECVersion m_ecXmlVersion;

protected:
    SchemaWriteStatus WriteSchemaReferences();
    SchemaWriteStatus WriteClass(ECClassCR ecClass);
    SchemaWriteStatus WriteCustomAttributeDependencies(IECCustomAttributeContainerCR container);
    SchemaWriteStatus WritePropertyDependencies(ECClassCR ecClass);

    template<typename T>
    SchemaWriteStatus WriteSchemaChild(T const& child)
        {
        if (&(child.GetSchema()) != &m_ecSchema)
            return SchemaWriteStatus::Success;

        return child.WriteXml(m_xmlWriter, m_ecXmlVersion);
        }

public:
    SchemaXmlWriter(BeXmlWriterR xmlWriter, ECSchemaCR ecSchema, ECVersion ecXmlVersion = ECVersion::V2_0) : m_xmlWriter(xmlWriter), m_ecSchema(ecSchema), m_ecXmlVersion(ecXmlVersion) {}
    virtual SchemaWriteStatus Serialize(bool utf16 = false);
};

END_BENTLEY_ECOBJECT_NAMESPACE
