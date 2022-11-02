/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "ECObjects/ECSchema.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

using ClassDeserializationVector = bvector<bpair<ECClassP, pugi::xml_node>>;

// =====================================================================================
// SchemaXmlReaderImpl class
// =====================================================================================
struct SchemaXmlReaderImpl
{
protected:
    pugi::xml_document& m_xmlDoc;
    ECSchemaReadContextR m_schemaContext;
    ECSchemaPtr m_conversionSchema;

    bool IsOpenPlantPidCircularReferenceSpecialCase(Utf8String& referencedECSchemaName, Utf8String& referencingECSchemaFullName);

    virtual bool ReadClassNode(ECClassP &ecClass, pugi::xml_node classNode, ECSchemaPtr& schemaOut) = 0;
    virtual SchemaReadStatus _ReadClassContentsFromXml(ECSchemaPtr& schemaOut, ClassDeserializationVector& classes);
    virtual SchemaReadStatus _ReadSchemaReferencesFromXml(ECSchemaPtr& schemaOut, pugi::xml_node schemaNode);

    virtual bool _IsSchemaChildElementNode(pugi::xml_node schemaNode, ECSchemaElementType childType, ECVersion xmlVersion = ECVersion::Latest);

protected:
    ECEntityClassP CreateEntityClass(ECSchemaPtr& schemaOut) {return new ECEntityClass(*schemaOut);}
    ECStructClassP CreateStructClass(ECSchemaPtr& schemaOut) {return new ECStructClass(*schemaOut);}
    ECCustomAttributeClassP CreateCustomAttributeClass(ECSchemaPtr& schemaOut) {return new ECCustomAttributeClass(*schemaOut);}
    ECRelationshipClassP CreateRelationshipClass(ECSchemaPtr& schemaOut) {return new ECRelationshipClass(*schemaOut);}

    // when `nameAttribute` is nullptr, it means the accessor is independent of its name argument and will be passed a nullptr
    template<typename Matcher, typename Accessor>
    CustomAttributeReadStatus ReadChildrenCustomAttributes(ECSchemaR parentSchema, pugi::xml_node parentNode, Utf8CP nameAttribute, Matcher matcher, Accessor accessor);

    template<typename T>
    CustomAttributeReadStatus ReadContainerCustomAttributes(ECSchemaR parentSchema, pugi::xml_node candidateNode, T& container);

public:
    SchemaXmlReaderImpl(ECSchemaReadContextR context, pugi::xml_document& xmlDoc) : m_schemaContext(context), m_xmlDoc(xmlDoc) {}
    virtual ~SchemaXmlReaderImpl() {}
    virtual SchemaReadStatus ReadSchemaReferencesFromXml(ECSchemaPtr& schemaOut, pugi::xml_node schemaNode) = 0;

    virtual SchemaReadStatus ReadClassStubsFromXml(ECSchemaPtr& schemaOut, pugi::xml_node schemaNode, ClassDeserializationVector& classes, int ecXmlVersionMajor);
    virtual SchemaReadStatus ReadClassContentsFromXml(ECSchemaPtr& schemaOut, ClassDeserializationVector&  classes) = 0;
    
    template<typename T>
    SchemaReadStatus ReadSchemaChildFromXml(ECSchemaPtr& schemaOut, pugi::xml_node schemaNode, ECSchemaElementType childType);

    // This method encompasses all Unit Type; Unit, Constant, InvertedUnit, Phenomenon and UnitSystem.
    // Needed because the ReadSchemaChildFromXml does not pass a name to the constructor, which is required
    // to construct any Unit type.
    template<typename T>
    SchemaReadStatus ReadUnitTypeFromXml(ECSchemaPtr& schemaOut, pugi::xml_node schemaNode, ECSchemaElementType unitType);

    CustomAttributeReadStatus DeserializeChildCustomAttributes(ECSchemaR contextSchema, pugi::xml_node schemaNode);

    void PopulateSchemaElementOrder(ECSchemaElementsOrder& elementOrder, pugi::xml_node schemaNode);
    virtual bool IsSchemaChildElementNode(pugi::xml_node schemaNode, ECSchemaElementType childType) {return _IsSchemaChildElementNode(schemaNode, childType);}
};

// =====================================================================================
// SchemaXMLReader class
// =====================================================================================
struct SchemaXmlReader
{
private:
    pugi::xml_document&     m_xmlDoc;
    ECSchemaReadContextR    m_schemaContext;

    SchemaReadStatus ReadSchemaContents(SchemaXmlReaderImpl* reader, ECSchemaPtr& schemaOut, pugi::xml_node schemaNode, uint32_t ecXmlMajorVersion, StopWatch& overallTimer);

public:
    SchemaXmlReader(ECSchemaReadContextR context, pugi::xml_document& xmlDoc) : m_schemaContext(context), m_xmlDoc(xmlDoc) {}
    SchemaReadStatus Deserialize(ECSchemaPtr& ecSchema, Utf8CP checksum = nullptr);

    static void SetErrorHandling(bool doAssert);

    static SchemaReadStatus ReadSchemaStub(SchemaKey& schemaKey, uint32_t& ecXmlMajorVersion, uint32_t& ecXmlMinorVersion, pugi::xml_node& schemaNode, pugi::xml_document& xmlDom);
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
    bool ReadClassNode(ECClassP &ecClass, pugi::xml_node classNode, ECSchemaPtr& schemaOut) override;

public:
    SchemaXmlReader2(ECSchemaReadContextR context, ECSchemaPtr schemaOut, pugi::xml_document& xmlDoc) : SchemaXmlReaderImpl(context, xmlDoc)
        {
        m_conversionSchema = context.LocateConversionSchemaFor(schemaOut->GetName().c_str(), schemaOut->GetVersionRead(), schemaOut->GetVersionMinor());
        }
    SchemaReadStatus ReadSchemaReferencesFromXml(ECSchemaPtr& schemaOut, pugi::xml_node schemaNode) override {return _ReadSchemaReferencesFromXml(schemaOut, schemaNode);}
    SchemaReadStatus ReadClassContentsFromXml(ECSchemaPtr& schemaOut, ClassDeserializationVector& classes) override {return _ReadClassContentsFromXml(schemaOut, classes);}

    bool IsSchemaChildElementNode(pugi::xml_node schemaNode, ECSchemaElementType childType) override {return _IsSchemaChildElementNode(schemaNode, childType, ECVersion::V2_0);}
};

// =====================================================================================
// SchemaXmlReader3 class
// =====================================================================================
struct SchemaXmlReader3 : SchemaXmlReaderImpl
{
protected:
    bool ReadClassNode(ECClassP &ecClass, pugi::xml_node classNode, ECSchemaPtr& schemaOut) override;

public:
    using SchemaXmlReaderImpl::SchemaXmlReaderImpl;  // reuse base constructor
    SchemaReadStatus ReadSchemaReferencesFromXml(ECSchemaPtr& schemaOut, pugi::xml_node schemaNode) override {return _ReadSchemaReferencesFromXml(schemaOut, schemaNode);}
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

        if (child.GetName() == "")
            {
            LOG.warningv("Schema Serialization Warning: An element was found to have an empty name and was ignored.");
            return SchemaWriteStatus::Success;
            }

        return child.WriteXml(m_xmlWriter, m_ecXmlVersion);
        }

public:
    SchemaXmlWriter(BeXmlWriterR xmlWriter, ECSchemaCR ecSchema, ECVersion ecXmlVersion = ECVersion::V2_0) : m_xmlWriter(xmlWriter), m_ecSchema(ecSchema), m_ecXmlVersion(ecXmlVersion) {}
    virtual SchemaWriteStatus Serialize(bool utf16 = false);
};

END_BENTLEY_ECOBJECT_NAMESPACE
