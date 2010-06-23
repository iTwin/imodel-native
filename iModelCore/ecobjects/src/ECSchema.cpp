/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/ECSchema.cpp $
|    $RCSfile: file.tpl,v $
|   $Revision: 1.10 $
|       $Date: 2005/11/07 15:38:45 $
|     $Author: EarlinLutz $
|
|  $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                 
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchema::~ECSchema
(
)
    {
    // NEEDSWORK make sure everything is destroyed
    Logger::GetLogger()->debugv (L"~~~~ Destroying ECSchema: %s\n", GetName().c_str());
    ClassMap::iterator          classIterator = m_classMap.begin();
    ClassMap::const_iterator    classEnd = m_classMap.end();        
    Logger::GetLogger()->debugv(L"     Freeing memory for %d classes\n", m_classMap.size());
    while (classIterator != classEnd)
        {
        ECClassP ecClass = classIterator->second;        
        classIterator = m_classMap.erase(classIterator);        
        delete ecClass;
        }

    assert (m_classMap.empty());

    memset (this, 0xececdead, sizeof(this));
    /*
    for (ECSchemaReferenceVector::iterator sit = m_referencedSchemas.begin(); sit != m_referencedSchemas.end(); sit++)
        {
        CECSchemaReference & schemaRef = *sit;
        if (NULL != schemaRef.m_pECSchema)
            delete schemaRef.m_pECSchema; //needswork: are we sure that something else isn't holding it... we need a DgnECManager
        }
    m_referencedSchemas.clear();*/
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bwstring const& ECSchema::GetName
(
) const
    {
    return m_name;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetName
(
bwstring const& name
)
    {        
    //NEEDSWORK name needs to be validated
    m_name = name;        
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bwstring const& ECSchema::GetNamespacePrefix
(
) const
    {        
    return m_namespacePrefix;    
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetNamespacePrefix
(
bwstring const& namespacePrefix
)
    {        
    m_namespacePrefix = namespacePrefix;  
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bwstring const& ECSchema::GetDescription
(
) const
    {
    return m_description;        
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetDescription
(
bwstring const& description
)
    {        
    m_description = description;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bwstring const& ECSchema::GetDisplayLabel
(
) const
    {
    return (m_displayLabel.empty()) ? Name : m_displayLabel;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetDisplayLabel
(
bwstring const& displayLabel
)
    {        
    m_displayLabel = displayLabel;
    return ECOBJECTS_STATUS_Success;
    }
    
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchema::GetIsDisplayLabelDefined
(
) const
    {
    return (!m_displayLabel.empty());        
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 ECSchema::GetVersionMajor
(
) const
    {
    return m_versionMajor;        
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetVersionMajor
(
const UInt32 versionMajor
)
    {        
    m_versionMajor = versionMajor;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 ECSchema::GetVersionMinor
(
) const
    {
    return m_versionMinor;        
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetVersionMinor
(
const UInt32 versionMinor
)
    {        
    m_versionMinor = versionMinor;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassP ECSchema::GetClassP
(
bwstring const& name
) const
    {
    ClassMap::const_iterator  classIterator;
    classIterator = m_classMap.find (name.c_str());
    
    if ( classIterator != m_classMap.end() )
        return classIterator->second;
    else
        return NULL;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::AddClass 
(
ECClassP&                 pClass
)
    {
    std::pair < stdext::hash_map<const wchar_t *, ECClassP>::iterator, bool > resultPair;
    resultPair = m_classMap.insert (std::pair<const wchar_t *, ECClassP> (pClass->Name.c_str(), pClass));
    if (resultPair.second == false)
        {
        Logger::GetLogger()->warningv (L"Can not create class '%s' because it already exists in the schema", pClass->Name.c_str());
        delete pClass;
        pClass = NULL;        
        return ECOBJECTS_STATUS_NamedItemAlreadyExists;
        }

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::CreateClass 
(
ECClassP&                 pClass, 
bwstring const&     name
)
    {
    pClass = new ECClass(*this);
    ECObjectsStatus status = pClass->SetName (name);
    if (ECOBJECTS_STATUS_Success != status)
        return status;

    return AddClass (pClass);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::CreateRelationshipClass 
(
ECRelationshipClassP&     pClass, 
bwstring const&     name
)
    {
    pClass = new ECRelationshipClass(*this);
    ECObjectsStatus status = pClass->SetName (name);
    if (ECOBJECTS_STATUS_Success != status)
        return status;

    std::pair < stdext::hash_map<const wchar_t *, ECClassP>::iterator, bool > resultPair;
    resultPair = m_classMap.insert (std::pair<const wchar_t *, ECClassP> (pClass->Name.c_str(), pClass));
    if (resultPair.second == false)
        {
        delete pClass;
        pClass = NULL;
        Logger::GetLogger()->warningv (L"Can not create relationship class '%s' because it already exists in the schema", name.c_str());
        return ECOBJECTS_STATUS_NamedItemAlreadyExists;
        }

    return ECOBJECTS_STATUS_Success;
    }

#define     ECSCHEMA_VERSION_FORMAT_EXPLAINATION L" Format must be MM.mm where MM is major version and mm is minor version.\n"
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::ParseVersionString 
(
UInt32&                 versionMajor, 
UInt32&                 versionMinor, 
bwstring const&     versionString
)
    {
    versionMajor = DEFAULT_VERSION_MAJOR;
    versionMinor = DEFAULT_VERSION_MINOR;
    if (versionString.empty())
        return ECOBJECTS_STATUS_Success;

    const wchar_t * version = versionString.c_str();
    const wchar_t * theDot = wcschr (version, L'.');
    if (NULL == theDot)
        {
        Logger::GetLogger()->errorv (L"Invalid ECSchema Version String: '%s' does not contain a '.'!" ECSCHEMA_VERSION_FORMAT_EXPLAINATION, versionString.c_str());
        return ECOBJECTS_STATUS_ParseError;
        }

    size_t majorLen = theDot - version;
    if (majorLen < 1 || majorLen > 3)
        {
        Logger::GetLogger()->errorv (L"Invalid ECSchema Version String: '%s' does not have 1-3 numbers before the '.'!" ECSCHEMA_VERSION_FORMAT_EXPLAINATION, versionString.c_str());
        return ECOBJECTS_STATUS_ParseError;
        }

    size_t minorLen = wcslen (theDot) - 1;
    if (minorLen < 1 || minorLen > 3)
        {
        Logger::GetLogger()->errorv (L"Invalid ECSchema Version String: '%s' does not have 1-3 numbers after the '.'!" ECSCHEMA_VERSION_FORMAT_EXPLAINATION, versionString.c_str());
        return ECOBJECTS_STATUS_ParseError;
        }

    wchar_t * end = NULL;    
    versionMajor = wcstoul (version, &end, 10);
    if (version == end)
        {
        Logger::GetLogger()->errorv (L"Invalid ECSchema Version String: '%s' The characters before the '.' must be numeric!" ECSCHEMA_VERSION_FORMAT_EXPLAINATION, versionString.c_str());
        return ECOBJECTS_STATUS_ParseError;
        }

    versionMinor = wcstoul (&theDot[1], &end, 10);
    if (&theDot[1] == end)
        {
        Logger::GetLogger()->errorv (L"Invalid ECSchema Version String: '%s' The characters after the '.' must be numeric!" ECSCHEMA_VERSION_FORMAT_EXPLAINATION, versionString.c_str());
        return ECOBJECTS_STATUS_ParseError;
        }

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetVersionFromString 
(
bwstring const& versionString
)
    {
    UInt32 versionMajor;
    UInt32 versionMinor;
    ECObjectsStatus status;
    if ((ECOBJECTS_STATUS_Success != (status = ParseVersionString (versionMajor, versionMinor, versionString))) ||         
        (ECOBJECTS_STATUS_Success != (status = this->SetVersionMajor (versionMajor))) ||
        (ECOBJECTS_STATUS_Success != (status = this->SetVersionMinor (versionMinor))))
        return status;
    else
        return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen   04/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECSchema::SchemasMatch
(
SchemaMatchType matchType,
const wchar_t * soughtName,
UInt32          soughtMajor,
UInt32          soughtMinor,
const wchar_t * candidateName,
UInt32          candidateMajor,
UInt32          candidateMinor
)
    {
    if (wcsicmp (candidateName, soughtName))
        return false;
    
    return ((matchType == SCHEMAMATCHTYPE_Latest) ||
            (matchType == SCHEMAMATCHTYPE_LatestCompatible && candidateMajor == soughtMajor && candidateMinor >= soughtMinor) ||
            (matchType == SCHEMAMATCHTYPE_Exact            && candidateMajor == soughtMajor && candidateMinor == soughtMinor));
    }
    
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::CreateSchema
(
ECSchemaPtr&              schemaOut, 
bwstring const&     schemaName
)
    {    
    if (!NameValidator::Validate(schemaName))
        return ECOBJECTS_STATUS_InvalidName;

    schemaOut = new ECSchema();
    return schemaOut->SetName (schemaName);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaP ECSchema::GetSchemaByNamespacePrefixP
(
bwstring const&     namespacePrefix
) const
    {
    if ((namespacePrefix.length() == 0) || (namespacePrefix == m_namespacePrefix))
        return (ECSchemaP)this;

    // lookup referenced schema by prefix
    ECSchemaReferenceList::const_iterator schemaIterator;
    for (schemaIterator = m_refSchemaList.begin(); schemaIterator != m_refSchemaList.end(); schemaIterator++)
        {
        if (0 == namespacePrefix.compare ((*schemaIterator)->m_namespacePrefix))
            return (*schemaIterator).get();
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bwstring const* ECSchema::ResolveNamespacePrefix
(
ECSchemaCR    schema
) const
    {
    if (&schema == this)
        return &EMPTY_STRING;

    stdext::hash_map<ECSchemaP, const bwstring *>::const_iterator schemaIterator = m_referencedSchemaNamespaceMap.find((ECSchemaP) &schema);
    if (schemaIterator != m_referencedSchemaNamespaceMap.end())
        {
        return schemaIterator->second;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassContainerCR ECSchema::GetClasses
(
) const
    {
    return m_classContainer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
const ECSchemaReferenceList& ECSchema::GetReferencedSchemas
(
) const
    {
    return m_refSchemaList;
    }
  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::AddReferencedSchema
(
Bentley::EC::ECSchemaPtr refSchema
)
    {
    ECSchemaReferenceList::const_iterator schemaIterator;
    for (schemaIterator = m_refSchemaList.begin(); schemaIterator != m_refSchemaList.end(); schemaIterator++)
        {
        if (*schemaIterator == refSchema)
            return ECOBJECTS_STATUS_NamedItemAlreadyExists;
        }
            
    m_refSchemaList.push_back(refSchema);
    m_referencedSchemaNamespaceMap.insert(std::pair<ECSchemaP, const bwstring *> (refSchema.get(), &(refSchema->NamespacePrefix)));
    return ECOBJECTS_STATUS_Success;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::RemoveReferencedSchema
(
Bentley::EC::ECSchemaPtr refSchema
)
    {
    ECSchemaReferenceList::const_iterator schemaIterator;
    for (schemaIterator = m_refSchemaList.begin(); schemaIterator != m_refSchemaList.end(); schemaIterator++)
        {
        if (*schemaIterator == refSchema)
            {
//            m_refSchemaList.erase(schemaIterator);
//            return ECOBJECTS_STATUS_Success;
            break;
            }
        }
    if (m_refSchemaList.end() == schemaIterator)
        return ECOBJECTS_STATUS_SchemaNotFound;
        
    // Can only remove the reference if nothing actually references it.
    
    ECSchemaP foundSchemaP = (*schemaIterator).get();
    for each (ECClassP ecClass in Classes)
        {
        // First, check each base class to see if the base class uses that schema
        for each (ECClassP baseClass in ecClass->BaseClasses)
            {
            if ((ECSchemaP) &(baseClass->Schema) == foundSchemaP)
                {
                return ECOBJECTS_STATUS_SchemaInUse;
                }
            }
            
        // If it is a relationship class, check the constraints to make sure the constraints don't use that schema
        ECRelationshipClassP relClass = dynamic_cast<ECRelationshipClassP>(ecClass);
        if (NULL != relClass)
            {
            for each (ECClassP target in relClass->Target.Classes)
                {
                if ((ECSchemaP) &(target->Schema) == foundSchemaP)
                    {
                    return ECOBJECTS_STATUS_SchemaInUse;
                    }
                }
            for each (ECClassP source in relClass->Source.Classes)
                {
                if ((ECSchemaP) &(source->Schema) == foundSchemaP)
                    {
                    return ECOBJECTS_STATUS_SchemaInUse;
                    }
                }
            }
            
        // And make sure that there are no struct types from another schema
        for each (ECPropertyP prop in ecClass->GetProperties(false))
            {
            ECClassCP typeClass;
            if (prop->IsStruct)
                {
                typeClass = &(prop->GetAsStructProperty()->Type);
                }
            else if (prop->IsArray)
                {
                typeClass = prop->GetAsArrayProperty()->StructElementType;
                }
            else
                {
                typeClass = NULL;
                }
            if (NULL == typeClass)
                continue;
            if (this->Name.compare(typeClass->Schema.Name) == 0 && this->VersionMajor == typeClass->Schema.VersionMajor &&
                this->VersionMinor == typeClass->Schema.VersionMinor)
                continue;
            return ECOBJECTS_STATUS_SchemaInUse;
            }
        }

    m_refSchemaList.erase(schemaIterator);        
    return ECOBJECTS_STATUS_Success;
    }
    
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaDeserializationStatus ECSchema::ReadClassStubsFromXml
(
MSXML2::IXMLDOMNode& schemaNode,
ClassDeserializationVector&  classes
)
    {
    SchemaDeserializationStatus status = SCHEMA_DESERIALIZATION_STATUS_Success;

    // Create ECClass Stubs (no attributes or properties)
    size_t classElementLength = wcslen (EC_CLASS_ELEMENT);
    MSXML2::IXMLDOMNodeListPtr xmlNodeListPtr = schemaNode.selectNodes (EC_NAMESPACE_PREFIX L":" EC_CLASS_ELEMENT L" | " EC_NAMESPACE_PREFIX L":" EC_RELATIONSHIP_CLASS_ELEMENT);
    _bstr_t baseName;    
    ECClassP pClass;
    ECRelationshipClassP pRelationshipClass;
    MSXML2::IXMLDOMNodePtr xmlNodePtr;
    while (NULL != (xmlNodePtr = xmlNodeListPtr->nextNode()))
        {        
        baseName = xmlNodePtr->baseName;
        
        if (baseName.length() == classElementLength)
            {            
            pClass = new ECClass (*this);
            pRelationshipClass = NULL;
            }
        else
            {            
            pRelationshipClass = new ECRelationshipClass (*this);            
            pClass = pRelationshipClass;
            }

        if (SCHEMA_DESERIALIZATION_STATUS_Success != (status = pClass->ReadXmlAttributes(xmlNodePtr)))
            return status;           

        if (ECOBJECTS_STATUS_Success != this->AddClass (pClass))
            return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml;

        if (NULL == pRelationshipClass)
            Logger::GetLogger()->tracev (L"    Created ECClass Stub: %s\n", pClass->Name.c_str());
        else
            Logger::GetLogger()->tracev (L"    Created Relationship ECClass Stub: %s\n", pClass->Name.c_str());

        classes.push_back (std::make_pair (pClass, xmlNodePtr));
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
 - Expects class stubs have already been read and created.  They are stored in the vector passed into this method.
 - Expects referenced schemas have been resolved and deserialized so that base classes & structs in other schemas can be located.
 - Reads the contents of each XML node cached in the classes vector and populates the in-memory EC:ECClass with
   base classes, properties & relationship endpoints.
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaDeserializationStatus ECSchema::ReadClassContentsFromXml
(
ClassDeserializationVector&  classes
)
    {
    SchemaDeserializationStatus status = SCHEMA_DESERIALIZATION_STATUS_Success;

    ClassDeserializationVector::const_iterator  classesStart, classesEnd, classesIterator;
    ECClassP pClass;
    MSXML2::IXMLDOMNodePtr xmlNodePtr;
    for(classesStart = classes.begin(), classesEnd = classes.end(), classesIterator = classesStart; classesIterator != classesEnd; classesIterator++)
        {
        pClass = classesIterator->first;
        xmlNodePtr = classesIterator->second;
        status = pClass->ReadXmlContents (xmlNodePtr);
        if (SCHEMA_DESERIALIZATION_STATUS_Success != status)
            return status;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaDeserializationStatus ECSchema::ReadSchemaReferencesFromXml
(
MSXML2::IXMLDOMNode& schemaNode, 
const bvector<IECSchemaLocatorP> * schemaLocators, 
const bvector<const wchar_t *> * schemaPaths,
void * schemaContext
)
    {
    SchemaDeserializationStatus status = SCHEMA_DESERIALIZATION_STATUS_Success;
    bool needToDelete = false;
    SchemaMap *underConstruction = NULL;
    if (NULL == schemaContext)
        {
        underConstruction = new SchemaMap();
        needToDelete = true;
        }
    else
        {
        underConstruction = static_cast<SchemaMap *>(schemaContext);
        if (NULL == underConstruction)
            {
            underConstruction = new SchemaMap();
            needToDelete = true;
            }
        }
        
    wchar_t version[10];
    swprintf(version, 10, L".%02d.%02d", m_versionMajor, m_versionMinor);
    bwstring *versionString = new bwstring(m_name + version);
    underConstruction->insert(std::pair<const wchar_t *, ECSchemaP>(versionString->c_str(), this));
    
    m_referencedSchemaNamespaceMap.clear();

    MSXML2::IXMLDOMNodeListPtr xmlNodeListPtr = schemaNode.selectNodes (EC_NAMESPACE_PREFIX L":" EC_SCHEMAREFERENCE_ELEMENT);
    MSXML2::IXMLDOMNodePtr xmlNodePtr;
    
    while (NULL != (xmlNodePtr = xmlNodeListPtr->nextNode()))
        {
        MSXML2::IXMLDOMNamedNodeMapPtr nodeAttributesPtr = xmlNodePtr->attributes;
        MSXML2::IXMLDOMNodePtr attributePtr;

        if (NULL == (attributePtr = nodeAttributesPtr->getNamedItem (SCHEMAREF_NAME_ATTRIBUTE)))
            {
            Logger::GetLogger()->errorv (L"Invalid ECSchemaXML: %s element must contain a " SCHEMAREF_NAME_ATTRIBUTE L" attribute\n", (const wchar_t *)xmlNodePtr->baseName);
            if (needToDelete)
                delete underConstruction;
            return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml;
            }
            
        bwstring schemaName = (const wchar_t*) attributePtr->text;

        if (NULL == (attributePtr = nodeAttributesPtr->getNamedItem (SCHEMAREF_PREFIX_ATTRIBUTE)))
            {
            Logger::GetLogger()->errorv (L"Invalid ECSchemaXML: %s element must contain a " SCHEMAREF_PREFIX_ATTRIBUTE L" attribute\n", (const wchar_t *)xmlNodePtr->baseName);
            if (needToDelete)
                delete underConstruction;
            return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml;
            }
        bwstring prefix = (const wchar_t*) attributePtr->text;

        if (NULL == (attributePtr = nodeAttributesPtr->getNamedItem (SCHEMAREF_VERSION_ATTRIBUTE)))
            {
            Logger::GetLogger()->errorv (L"Invalid ECSchemaXML: %s element must contain a " SCHEMAREF_VERSION_ATTRIBUTE L" attribute\n", (const wchar_t *)xmlNodePtr->baseName);
            if (needToDelete)
                delete underConstruction;
            return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml;
            }
        bwstring versionString = (const wchar_t*) attributePtr->text;

        UInt32 versionMajor;
        UInt32 versionMinor;
        if (ECOBJECTS_STATUS_Success != ParseVersionString (versionMajor, versionMinor, versionString.c_str()))
            {
            Logger::GetLogger()->errorv (L"Invalid ECSchemaXML: unable to parse version string for referenced schema %s.", schemaName.c_str());
            if (needToDelete)
                delete underConstruction;
            return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml;
            }
            
        // If the schema (uselessly) references itself, just skip it
        if (m_name.compare(schemaName) == 0)
            continue;

        ECSchemaPtr referencedSchema = LocateSchema(schemaLocators, schemaPaths, schemaName, versionMajor, versionMinor, underConstruction);
        if (!referencedSchema.IsValid())
            {
            Logger::GetLogger()->errorv(L"Unable to locate referenced schema %s.%02d.%02d", schemaName.c_str(), versionMajor, versionMinor);
            if (needToDelete)
                delete underConstruction;
            return SCHEMA_DESERIALIZATION_STATUS_ReferencedSchemaNotFound;
            }
        ECSchemaPtr newPtr(referencedSchema);
        AddReferencedSchema(newPtr);
        }
    if (needToDelete)
        delete underConstruction;
    else
        underConstruction->erase(versionString->c_str());
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr ECSchema::LocateSchema
(    
const bvector<IECSchemaLocatorP> * schemaLocators, 
const bvector<const wchar_t *> * schemaPaths,
const bwstring & name,
UInt32& versionMajor,
UInt32& versionMinor,
void*   schemaContext
)
    {
    SchemaMap* schemasUnderConstruction = (SchemaMap*)schemaContext;

    // First check if there is a circular reference and this reference has already started to be de-serialized farther upstream
    bwstring fullName(name);
    wchar_t version[10];
    swprintf(version, L".%02d.%02d", versionMajor, versionMinor);
    fullName += version;
    SchemaMap::const_iterator finder = schemasUnderConstruction->find(fullName.c_str());
    if (finder != schemasUnderConstruction->end())
        {
        return finder->second;
        }
    
    ECSchemaPtr schemaPtr;
    if (NULL != schemaLocators)
        {
        bvector<IECSchemaLocatorP>::const_iterator locator;
        for (locator = schemaLocators->begin(); locator != schemaLocators->end(); locator++)
            {
            IECSchemaLocatorP schemaLocator = *locator;
            if (NULL != schemaLocator)
                schemaPtr = schemaLocator->LocateSchema(name.c_str(), versionMajor, versionMinor, SCHEMAMATCHTYPE_LatestCompatible, schemasUnderConstruction);
            if (schemaPtr.IsValid())
                return schemaPtr;
            }
        }
        
    if (NULL != schemaPaths)
        {
        schemaPtr = LocateSchemaByPath(schemaLocators, schemaPaths, name, versionMajor, versionMinor, schemasUnderConstruction);
        if (schemaPtr.IsValid())
            return schemaPtr;
        }
        
    // try in standard path locations for the schema
    
    bvector<const wchar_t *> standardPaths;
    bwstring dllPath = ECFileUtilities::GetDllPath();
    if (0 == dllPath.length())
        return NULL;
        
    standardPaths.push_back(dllPath.c_str());
    
    wchar_t schemaPath[_MAX_PATH];
    wchar_t generalPath[_MAX_PATH];
    wchar_t libraryPath[_MAX_PATH];
    
    swprintf(schemaPath, _MAX_PATH, L"%s\\Schemas", dllPath.c_str());
    swprintf(generalPath, _MAX_PATH, L"%s\\Schemas\\General", dllPath.c_str());
    swprintf(libraryPath, _MAX_PATH, L"%s\\Schemas\\LibraryUnits", dllPath.c_str());
    standardPaths.push_back(schemaPath);
    standardPaths.push_back(generalPath);
    standardPaths.push_back(libraryPath);
    return LocateSchemaByPath(schemaLocators, &standardPaths, name, versionMajor, versionMinor, schemasUnderConstruction);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr ECSchema::LocateSchemaByPath
(
const bvector<IECSchemaLocatorP> * schemaLocators, 
const bvector<const wchar_t *> * schemaPaths,
const bwstring & name,
UInt32& versionMajor,
UInt32& versionMinor,
SchemaMap * schemasUnderConstruction
)
    {
    ECSchemaPtr schemaOut;
    bvector<const wchar_t *>::const_iterator path;
    wchar_t versionString[24];
    swprintf(versionString, 24, L".%02d.*.ecschema.xml", versionMajor);
    bwstring schemaName = name;
    schemaName += versionString;

    for (path = schemaPaths->begin(); path != schemaPaths->end(); path++)
        {
        bwstring schemaPath = *path;
        if (schemaPath[schemaPath.length() - 1] != '\\')
            schemaPath += '\\';
        schemaPath += schemaName;

        ECFileNameIterator *fileList = new ECFileNameIterator(schemaPath.c_str());
        wchar_t filePath[MAX_PATH];
        if (SUCCESS != fileList->GetNextFileName(filePath))
            continue;

        if (ECSchema::ReadXmlFromFile (schemaOut, filePath, schemaLocators, schemaPaths, schemasUnderConstruction) != SCHEMA_DESERIALIZATION_STATUS_Success)
            continue;
        
        return schemaOut;
        }
    return schemaOut;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaDeserializationStatus ECSchema::ReadXml
(
ECSchemaPtr&                        schemaOut, 
MSXML2::IXMLDOMDocument2&           pXmlDoc, 
const bvector<IECSchemaLocatorP> * schemaLocators, 
const bvector<const wchar_t *> * schemaPaths,
void * schemaContext
)
    {            
    SchemaDeserializationStatus status = SCHEMA_DESERIALIZATION_STATUS_Success;
    
    pXmlDoc.setProperty("SelectionNamespaces", L"xmlns:" EC_NAMESPACE_PREFIX L"='" ECXML_URI_2_0 L"'");
    MSXML2::IXMLDOMNodePtr xmlNodePtr = pXmlDoc.selectSingleNode (L"/" EC_NAMESPACE_PREFIX L":" EC_SCHEMA_ELEMENT);
    if (NULL == xmlNodePtr)
        {
        Logger::GetLogger()->errorv (L"Invalid ECSchemaXML: Missing a top-level " EC_SCHEMA_ELEMENT L" node in the " ECXML_URI_2_0 L" namespace\n");
        return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml;
        }
    
    MSXML2::IXMLDOMNodePtr schemaNodePtr = xmlNodePtr;       
    MSXML2::IXMLDOMNamedNodeMapPtr nodeAttributesPtr = schemaNodePtr->attributes;
    MSXML2::IXMLDOMNodePtr attributePtr;
    
    // schemaName is a REQUIRED attribute in order to create the schema
    if ((NULL == nodeAttributesPtr) || (NULL == (attributePtr = nodeAttributesPtr->getNamedItem (SCHEMA_NAME_ATTRIBUTE))))
        {
        Logger::GetLogger()->errorv (L"Invalid ECSchemaXML: " EC_SCHEMA_ELEMENT L" element must contain a schemaName attribute\n");
        return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml;
        }

    if (ECOBJECTS_STATUS_Success != CreateSchema (schemaOut, (const wchar_t *)attributePtr->text))
        return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml;

    // OPTIONAL attributes - If these attributes exist they MUST be valid        
    READ_OPTIONAL_XML_ATTRIBUTE (SCHEMA_NAMESPACE_PREFIX_ATTRIBUTE,         schemaOut, NamespacePrefix)
    READ_OPTIONAL_XML_ATTRIBUTE (DESCRIPTION_ATTRIBUTE,                     schemaOut, Description)
    READ_OPTIONAL_XML_ATTRIBUTE (DISPLAY_LABEL_ATTRIBUTE,                   schemaOut, DisplayLabel)

    // OPTIONAL attributes - If these attributes exist they do not need to be valid.  We will ignore any errors setting them and use default values.
    // NEEDSWORK This is due to the current implementation in managed ECObjects.  We should reconsider whether it is the correct behavior.
    ECObjectsStatus setterStatus;
    READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS (SCHEMA_VERSION_ATTRIBUTE,                  schemaOut, VersionFromString)
    if (ECOBJECTS_STATUS_Success != setterStatus)
        Logger::GetLogger()->warningv (L"Invalid version attribute has been ignored while deserializing ECSchema '%s'.  The default version number %d.%d has been applied.\n", 
            schemaOut->Name.c_str(), schemaOut->VersionMajor, schemaOut->VersionMinor);

    if (SCHEMA_DESERIALIZATION_STATUS_Success != (status = schemaOut->ReadSchemaReferencesFromXml(schemaNodePtr, schemaLocators, schemaPaths, schemaContext)))
        return status;

    ClassDeserializationVector classes;
    if (SCHEMA_DESERIALIZATION_STATUS_Success != (status = schemaOut->ReadClassStubsFromXml (schemaNodePtr, classes)))
        return status;

    // NEEDSWORK ECClass inheritance (base classes, properties & relationship endpoints)
    if (SCHEMA_DESERIALIZATION_STATUS_Success != (status = schemaOut->ReadClassContentsFromXml (classes)))
        return status;

    schemaOut->ReadCustomAttributes(schemaNodePtr, schemaOut.get());

    return SCHEMA_DESERIALIZATION_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static bool ClassNameComparer
(
ECClassP class1, 
ECClassP class2
)
    {
    // null values are always less than non-null values
    if (NULL == class1)
        return true;
    if (NULL == class2)
        return false;
    int comparison = wcscmp(class1->Name.c_str(), class2->Name.c_str());
    return (comparison <= 0);
    }
  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaSerializationStatus ECSchema::WriteSchemaReferences
(
MSXML2::IXMLDOMElement &parentNode
)
    {
    SchemaSerializationStatus status = SCHEMA_SERIALIZATION_STATUS_Success;
    ECSchemaReferenceList referencedSchemas = GetReferencedSchemas();
    
    std::set<const bwstring> usedPrefixes;
    std::set<const bwstring>::const_iterator setIterator;
    m_referencedSchemaNamespaceMap.clear();
    ECSchemaReferenceList::const_iterator schemaIterator;
    for (schemaIterator = m_refSchemaList.begin(); schemaIterator != m_refSchemaList.end(); schemaIterator++)
        {
        ECSchemaP refSchema = (*schemaIterator).get();
        bwstring *prefix = new bwstring(refSchema->NamespacePrefix);
        if (prefix->length() == 0)
            prefix = new bwstring(L"s");
            
        setIterator = usedPrefixes.find(*prefix);
        if (setIterator != usedPrefixes.end())
            {
            int subScript;
            for (subScript = 1; subScript < 500; subScript++)
                {
                wchar_t temp[256];
                swprintf(temp, 256, L"%s%d", prefix->c_str(), subScript);
                bwstring tryPrefix(temp);
                setIterator = usedPrefixes.find(tryPrefix);
                if (setIterator == usedPrefixes.end())
                    {
                    prefix = new bwstring(tryPrefix);
                    break;
                    }
                }
            }
        usedPrefixes.insert(prefix->c_str());
        m_referencedSchemaNamespaceMap.insert(std::pair<ECSchemaP, const bwstring *> (refSchema, prefix));
        }

    MSXML2::IXMLDOMTextPtr textPtr = NULL;
    MSXML2::IXMLDOMAttributePtr attributePtr;
    MSXML2::IXMLDOMElementPtr schemaPtr = NULL;
    
    stdext::hash_map<ECSchemaP, const bwstring *>::const_iterator iterator;
    for (iterator = m_referencedSchemaNamespaceMap.begin(); iterator != m_referencedSchemaNamespaceMap.end(); iterator++)
        {
        std::pair<ECSchemaP, const bwstring *> mapPair = *(iterator);
        ECSchemaP refSchema = mapPair.first;
        schemaPtr = parentNode.ownerDocument->createNode(NODE_ELEMENT, EC_SCHEMAREFERENCE_ELEMENT, ECXML_URI_2_0);
        APPEND_CHILD_TO_PARENT(schemaPtr, (&parentNode));
        
        WRITE_XML_ATTRIBUTE(SCHEMAREF_NAME_ATTRIBUTE, refSchema->Name.c_str(), schemaPtr);
        
        wchar_t versionString[8];
        swprintf(versionString, 8, L"%02d.%02d", refSchema->VersionMajor, refSchema->VersionMinor);
        WRITE_XML_ATTRIBUTE(SCHEMAREF_VERSION_ATTRIBUTE, versionString, schemaPtr);
        const bwstring *prefix = mapPair.second;
        WRITE_XML_ATTRIBUTE(SCHEMAREF_PREFIX_ATTRIBUTE, prefix->c_str(), schemaPtr);
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaSerializationStatus ECSchema::WriteCustomAttributeDependencies
(
MSXML2_IXMLDOMElement& parentNode,
IECCustomAttributeContainerCR container
)
    {
    SchemaSerializationStatus status = SCHEMA_SERIALIZATION_STATUS_Success;

    for each (IECInstancePtr instance in container.GetCustomAttributes(false))
        {
        ECClassCR currentClass = instance->GetClass();
        status = WriteClass(parentNode, currentClass);
        if (SCHEMA_SERIALIZATION_STATUS_Success != status)
            return status;
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaSerializationStatus ECSchema::WriteClass
(
MSXML2::IXMLDOMElement &parentNode, 
Bentley::EC::ECClassCR ecClass
)
    {
    SchemaSerializationStatus status = SCHEMA_SERIALIZATION_STATUS_Success;
    // don't serialize any classes that aren't in the schema we're serializing.
    if (&(ecClass.Schema) != this)
        return status;
    
    std::set<const wchar_t *>::const_iterator setIterator;
    setIterator = m_alreadySerializedClasses.find(ecClass.Name.c_str());
    // Make sure we don't serialize any class twice
    if (setIterator != m_alreadySerializedClasses.end())
        return status;
    else
        m_alreadySerializedClasses.insert(ecClass.Name.c_str());
        
    // NEEDSWORK name validation?
    // don't serialize a class with an invalid name
    
    // serialize the base classes first.
    for each (ECClassP baseClass in ecClass.BaseClasses)
        {
        WriteClass(parentNode, *baseClass);
        }
       
    // Serialize relationship constraint dependencies
    ECRelationshipClassP relClass = dynamic_cast<ECRelationshipClassP>((ECClassP) &ecClass);
    if (NULL != relClass)
        {
        for each (ECClassP source in relClass->Source.Classes)
            WriteClass(parentNode, *source);
            
        for each (ECClassP target in relClass->Target.Classes)
            WriteClass(parentNode, *target);
        }
    WritePropertyDependencies(parentNode, ecClass); 
    WriteCustomAttributeDependencies(parentNode, ecClass);
    
    ecClass.WriteXml(parentNode);
    
    return status;
    }  
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaSerializationStatus ECSchema::WritePropertyDependencies
(
MSXML2::IXMLDOMElement &parentNode, 
Bentley::EC::ECClassCR ecClass
)  
    {
    SchemaSerializationStatus status = SCHEMA_SERIALIZATION_STATUS_Success;
    
    for each (ECPropertyP prop in ecClass.GetProperties(false))
        {
        if (prop->IsStruct)
            {
            StructECPropertyP structProperty = prop->GetAsStructProperty();
            WriteClass(parentNode, structProperty->Type);
            }
        else if (prop->IsArray)
            {
            ArrayECPropertyP arrayProperty = prop->GetAsArrayProperty();
            if (arrayProperty->GetKind() == ARRAYKIND_Struct)
                {
                WriteClass(parentNode, *(arrayProperty->GetStructElementType()));
                }
            }
        WriteCustomAttributeDependencies(parentNode, *prop);
        }
    return status;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaSerializationStatus ECSchema::WriteXml
(
MSXML2::IXMLDOMDocument2* pXmlDoc
)
    {
    SchemaSerializationStatus status = SCHEMA_SERIALIZATION_STATUS_Success;

    MSXML2::IXMLDOMProcessingInstructionPtr piPtr = NULL;
    MSXML2::IXMLDOMTextPtr textPtr = NULL;

    piPtr = pXmlDoc->createProcessingInstruction(L"xml", L"version='1.0'  encoding='UTF-8'");
    APPEND_CHILD_TO_PARENT(piPtr, pXmlDoc);
    //CREATE_AND_ADD_TEXT_NODE(L"\n", piPtr);
    
    MSXML2::IXMLDOMElementPtr schemaElementPtr = pXmlDoc->createNode(NODE_ELEMENT, EC_SCHEMA_ELEMENT, ECXML_URI_2_0);
    APPEND_CHILD_TO_PARENT(schemaElementPtr, pXmlDoc);
    
    MSXML2::IXMLDOMAttributePtr attributePtr;
    wchar_t versionString[8];
    swprintf(versionString, 8, L"%02d.%02d", m_versionMajor, m_versionMinor);
    WRITE_XML_ATTRIBUTE(SCHEMA_NAME_ATTRIBUTE, this->Name.c_str(), schemaElementPtr);
    WRITE_OPTIONAL_XML_ATTRIBUTE(SCHEMA_NAMESPACE_PREFIX_ATTRIBUTE, NamespacePrefix, schemaElementPtr);
    WRITE_XML_ATTRIBUTE(SCHEMA_VERSION_ATTRIBUTE, versionString, schemaElementPtr);
    WRITE_OPTIONAL_XML_ATTRIBUTE(DESCRIPTION_ATTRIBUTE, Description, schemaElementPtr);
    if (IsDisplayLabelDefined)
        {
        WRITE_OPTIONAL_XML_ATTRIBUTE(DISPLAY_LABEL_ATTRIBUTE, DisplayLabel, schemaElementPtr);
        }
    WRITE_XML_ATTRIBUTE(L"xmlns:" EC_NAMESPACE_PREFIX, ECXML_URI_2_0, schemaElementPtr);
    
    WriteSchemaReferences(schemaElementPtr);
    
    WriteCustomAttributeDependencies(schemaElementPtr, *this);
    WriteCustomAttributes(schemaElementPtr);
    
    std::list<ECClassP> sortedClasses;
    // sort the classes by name so the order in which they are serialized is predictable.
    for each (ECClassP pClass in Classes)
        sortedClasses.push_back(pClass);
        
    sortedClasses.sort(ClassNameComparer);
    
    for each (ECClassP pClass in sortedClasses)
        {
        WriteClass(schemaElementPtr, *pClass);
        }
        
    m_alreadySerializedClasses.clear();
    ECXml::FormatXml(pXmlDoc);
    return status;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LogXmlLoadError
(
MSXML2::IXMLDOMDocument2& pXmlDoc
)
    {        
    MSXML2::IXMLDOMParseErrorPtr pXMLError = pXmlDoc.parseError;
    if (pXMLError != NULL)
        {
        LONG lErrorCode = pXMLError->errorCode;
        if (lErrorCode != 0)
            {
            long line = pXMLError->Getline();
            long linePos = pXMLError->Getlinepos();
            _bstr_t pBURL = pXMLError->Geturl();
            _bstr_t pBReason = pXMLError->Getreason();

            bwstring file;
            if (NULL != pBURL.GetBSTR())
                file = pBURL;
                
            bwstring reason = pBReason;
                        
            Logger::GetLogger()->errorv (L"line %d, position %d parsing ECSchema file %s. %s\n", line, linePos, file.c_str(), reason.c_str());            
            return ERROR;
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaDeserializationStatus ECSchema::ReadXmlFromFile
(
ECSchemaPtr&          schemaOut, 
const wchar_t *     ecSchemaXmlFile, 
const bvector<IECSchemaLocatorP> * schemaLocators, 
const bvector<const wchar_t *> * schemaPaths,
void * schemaContext
)
    {                  
    SchemaDeserializationStatus status = SCHEMA_DESERIALIZATION_STATUS_Success;

    MSXML2::IXMLDOMDocument2Ptr xmlDocPtr = NULL;        
    VERIFY_HRESULT_OK(xmlDocPtr.CreateInstance(__uuidof(MSXML2::DOMDocument60)), SCHEMA_DESERIALIZATION_STATUS_FailedToInitializeMsmxl);
    xmlDocPtr->put_validateOnParse(VARIANT_TRUE);
    xmlDocPtr->put_async(VARIANT_FALSE);
    
    VARIANT_BOOL returnCode = xmlDocPtr->load(ecSchemaXmlFile);
    if (returnCode != VARIANT_TRUE)
        {
        LogXmlLoadError(xmlDocPtr);
        return SCHEMA_DESERIALIZATION_STATUS_FailedToParseXml;
        }

    status = ReadXml (schemaOut, xmlDocPtr, schemaLocators, schemaPaths, schemaContext);
    if (ECOBJECTS_STATUS_Success != status)
        Logger::GetLogger()->errorv (L"Failed to deserialize XML file: %s\n", ecSchemaXmlFile);
    else
        Logger::GetLogger()->infov (L"Native ECSchema Deserialized from file: fileName='%s', schemaName='%s.%d.%d' classCount='%d' address='0x%x'\n", 
            ecSchemaXmlFile, schemaOut->Name.c_str(), schemaOut->VersionMajor, schemaOut->VersionMinor, schemaOut->m_classMap.size(), schemaOut.get());        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaDeserializationStatus ECSchema::ReadXmlFromString
(
ECSchemaPtr&          schemaOut, 
const wchar_t *     ecSchemaXml,
const bvector<IECSchemaLocatorP> * schemaLocators, 
const bvector<const wchar_t *> * schemaPaths,
void * schemaContext
)
    {                  
    SchemaDeserializationStatus status = SCHEMA_DESERIALIZATION_STATUS_Success;

    MSXML2::IXMLDOMDocument2Ptr xmlDocPtr = NULL;        
    VERIFY_HRESULT_OK(xmlDocPtr.CreateInstance(__uuidof(MSXML2::DOMDocument60)), SCHEMA_DESERIALIZATION_STATUS_FailedToInitializeMsmxl);
    xmlDocPtr->put_validateOnParse(VARIANT_TRUE);
    xmlDocPtr->put_async(VARIANT_FALSE);
    
    VARIANT_BOOL returnCode = xmlDocPtr->loadXML(ecSchemaXml);
    if (returnCode != VARIANT_TRUE)
        {
        LogXmlLoadError(xmlDocPtr);
        return SCHEMA_DESERIALIZATION_STATUS_FailedToParseXml;
        }

    status = ReadXml (schemaOut, xmlDocPtr, schemaLocators, schemaPaths, schemaContext);
    if (ECOBJECTS_STATUS_Success != status)
        Logger::GetLogger()->errorv (L"Failed to deserialize XML from string: %s\n", ecSchemaXml);
    else
        Logger::GetLogger()->infov (L"Native ECSchema Deserialized from string: schemaName='%s.%d.%d' classCount='%d' schemaAddress='0x%x'\n stringAddress='0x%x'", 
            schemaOut->Name.c_str(), schemaOut->VersionMajor, schemaOut->VersionMinor, schemaOut->m_classMap.size(), schemaOut.get(), ecSchemaXml);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchema::SchemasAreEqualByName (ECSchemaCP thisSchema, ECSchemaCP thatSchema)
    {
    return ((thisSchema == thatSchema) ||
            ( (0 == thisSchema->Name.compare(thatSchema->Name)) &&
              (thisSchema->VersionMajor == thatSchema->VersionMajor) &&
              (thisSchema->VersionMinor == thatSchema->VersionMinor)));
    }
    
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaDeserializationStatus ECSchema::ReadXmlFromStream
(
ECSchemaPtr&          schemaOut, 
IStreamP              ecSchemaXmlStream,
const bvector<IECSchemaLocatorP> * schemaLocators, 
const bvector<const wchar_t *> * schemaPaths,
void * schemaContext
)
    {                  
    SchemaDeserializationStatus status = SCHEMA_DESERIALIZATION_STATUS_Success;

    MSXML2::IXMLDOMDocument2Ptr xmlDocPtr = NULL;        
    VERIFY_HRESULT_OK(xmlDocPtr.CreateInstance(__uuidof(MSXML2::DOMDocument60)), SCHEMA_DESERIALIZATION_STATUS_FailedToInitializeMsmxl);
    xmlDocPtr->put_validateOnParse(VARIANT_TRUE);
    xmlDocPtr->put_async(VARIANT_FALSE);
    
    VARIANT_BOOL returnCode = xmlDocPtr->load(ecSchemaXmlStream);
    if (returnCode != VARIANT_TRUE)
        {
        LogXmlLoadError(xmlDocPtr);
        return SCHEMA_DESERIALIZATION_STATUS_FailedToParseXml;
        }

    status = ReadXml (schemaOut, xmlDocPtr, schemaLocators, schemaPaths, schemaContext);
    if (ECOBJECTS_STATUS_Success != status)
        Logger::GetLogger()->errorv (L"Failed to deserialize XML from stream\n");
    return status;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaSerializationStatus ECSchema::WriteXmlToString
(
bwstring  &ecSchemaXml
)
    {
    SchemaSerializationStatus status = SCHEMA_SERIALIZATION_STATUS_Success;

    MSXML2::IXMLDOMDocument2Ptr xmlDocPtr = NULL;        
    VERIFY_HRESULT_OK(xmlDocPtr.CreateInstance(__uuidof(MSXML2::DOMDocument60)), SCHEMA_SERIALIZATION_STATUS_FailedToInitializeMsmxl);
    xmlDocPtr->put_validateOnParse(VARIANT_TRUE);
    xmlDocPtr->put_async(VARIANT_FALSE);
    xmlDocPtr->put_preserveWhiteSpace(VARIANT_TRUE);
    xmlDocPtr->put_resolveExternals(VARIANT_FALSE);
    
    status = WriteXml(xmlDocPtr);
    
    if (status != SCHEMA_SERIALIZATION_STATUS_Success)
        return status;
        
    ecSchemaXml = xmlDocPtr->xml;
    
    return status;
    }
   
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaSerializationStatus ECSchema::WriteXmlToFile
(
const wchar_t * ecSchemaXmlFile
)
    {
    SchemaSerializationStatus status = SCHEMA_SERIALIZATION_STATUS_Success;

    MSXML2::IXMLDOMDocument2Ptr xmlDocPtr = NULL;        
    VERIFY_HRESULT_OK(xmlDocPtr.CreateInstance(__uuidof(MSXML2::DOMDocument60)), SCHEMA_SERIALIZATION_STATUS_FailedToInitializeMsmxl);
    xmlDocPtr->put_validateOnParse(VARIANT_TRUE);
    xmlDocPtr->put_async(VARIANT_FALSE);
    xmlDocPtr->put_preserveWhiteSpace(VARIANT_TRUE);
    xmlDocPtr->put_resolveExternals(VARIANT_FALSE);
    
    status = WriteXml(xmlDocPtr);
    if (status != SCHEMA_SERIALIZATION_STATUS_Success)
        return status;
        
    VERIFY_HRESULT_OK(xmlDocPtr->save(ecSchemaXmlFile), SCHEMA_SERIALIZATION_STATUS_FailedToSaveXml);

    return status;
    }
   
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaSerializationStatus ECSchema::WriteXmlToStream
(
IStreamP ecSchemaXmlStream
)
    {
    SchemaSerializationStatus status = SCHEMA_SERIALIZATION_STATUS_Success;

    MSXML2::IXMLDOMDocument2Ptr xmlDocPtr = NULL;        
    VERIFY_HRESULT_OK(xmlDocPtr.CreateInstance(__uuidof(MSXML2::DOMDocument60)), SCHEMA_SERIALIZATION_STATUS_FailedToInitializeMsmxl);
    xmlDocPtr->put_validateOnParse(VARIANT_TRUE);
    xmlDocPtr->put_async(VARIANT_FALSE);
    xmlDocPtr->put_preserveWhiteSpace(VARIANT_TRUE);
    xmlDocPtr->put_resolveExternals(VARIANT_FALSE);
    
    status = WriteXml(xmlDocPtr);
    if (status != SCHEMA_SERIALIZATION_STATUS_Success)
        return status;
        
    VERIFY_HRESULT_OK(xmlDocPtr->save(ecSchemaXmlStream), SCHEMA_SERIALIZATION_STATUS_FailedToSaveXml);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassContainer::const_iterator  ECClassContainer::begin () const
    {
    return ECClassContainer::const_iterator(m_classMap.begin());        
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassContainer::const_iterator  ECClassContainer::end () const
    {
    return ECClassContainer::const_iterator(m_classMap.end());        
    }   

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassContainer::const_iterator& ECClassContainer::const_iterator::operator++()
    {
    m_state->m_mapIterator++;    
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
bool    ECClassContainer::const_iterator::operator!= (const_iterator const& rhs) const
    {
    return (m_state->m_mapIterator != rhs.m_state->m_mapIterator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassP   ECClassContainer::const_iterator::operator*() const
    {
    std::pair<const wchar_t * , ECClassP> mapPair = *(m_state->m_mapIterator);
    return mapPair.second;
    };

/*---------------------------------------------------------------------------------**//**
* Validates a name and ensures a name contains only alphanumeric and underscore characters and does not start with a digit.
*
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool NameValidator::Validate
(
const bwstring &name
) 
    {
    if (name.empty())
        return false;
    if (isdigit(name[0]))
        return false;
    
    for (bwstring::size_type index = 0; index != name.length(); ++index)
        {
        if (!isalnum(name[index]) && '_' != name[index])
            return false;
        } 
    return true;
    }
        

END_BENTLEY_EC_NAMESPACE
