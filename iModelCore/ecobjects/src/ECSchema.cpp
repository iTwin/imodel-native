/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECSchema.cpp $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"
#if defined (_WIN32) // WIP_NONPORT - iostreams not support on Android
#include <iomanip>
#endif
#include <list>
#include <Bentley/BeFileName.h>

BEGIN_BENTLEY_EC_NAMESPACE

extern ECObjectsStatus GetSchemaFileName (WStringR fullFileName, UInt32& foundVersionMinor, WCharCP schemaPath, bool useLatestCompatibleMatch);

//#define DEBUG_SCHEMA_LEAKS
#ifdef DEBUG_SCHEMA_LEAKS
static LeakDetector<ECSchema> g_leakDetector (L"ECSchema", L"ECSchemas", true);
#else
static LeakDetector<ECSchema> g_leakDetector (L"ECSchema", L"ECSchemas", false);
#endif


// If you are developing schemas, particularly when editing them by hand, you want to have this variable set to false so you get the asserts to help you figure out what is going wrong.
// Test programs generally want to get error status back and not assert, so they call ECSchema::AssertOnXmlError (false);
static  bool        s_noAssert = false;


/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                 
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchema::ECSchema (bool hideFromLeakDetection)
    :
    m_versionMajor (DEFAULT_VERSION_MAJOR), m_versionMinor (DEFAULT_VERSION_MINOR), m_classContainer(m_classMap),
    m_hideFromLeakDetection(hideFromLeakDetection)
    {
    if ( ! m_hideFromLeakDetection)
        g_leakDetector.ObjectCreated(*this);
    };

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                 
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchema::~ECSchema ()
    {
    // NEEDSWORK make sure everything is destroyed
    ECObjectsLogger::Log()->debugv (L"~~~~ Destroying ECSchema: %s", GetName().c_str());
    ClassMap::iterator          classIterator = m_classMap.begin();
    ClassMap::const_iterator    classEnd = m_classMap.end();        
    ECObjectsLogger::Log()->debugv(L"     Freeing memory for %d classes", m_classMap.size());
    while (classIterator != classEnd)
        {
        ECClassP ecClass = classIterator->second;
        ECRelationshipClassP relClass = dynamic_cast<ECRelationshipClassP>(ecClass);
        classIterator = m_classMap.erase(classIterator);
        if (NULL != relClass)
            delete relClass;
        else
            delete ecClass;
        }

    assert (m_classMap.empty());

    /*
    for (ECSchemaReferenceVector::iterator sit = m_referencedSchemas.begin(); sit != m_referencedSchemas.end(); sit++)
        {
        CECSchemaReference & schemaRef = *sit;
        if (NULL != schemaRef.m_pECSchema)
            delete schemaRef.m_pECSchema; //needswork: are we sure that something else isn't holding it... we need a DgnECManager
        }
    m_referencedSchemas.clear();*/

    if ( ! m_hideFromLeakDetection)
        g_leakDetector.ObjectDestroyed(*this);

    memset (this, 0xececdead, sizeof(this));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/10
+---------------+---------------+---------------+---------------+---------------+------*/
ILeakDetector&  ECSchema::Debug_GetLeakDetector() { return g_leakDetector; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECSchema::SetErrorHandling (bool showMessages, bool doAssert) 
    { 
    s_noAssert = !doAssert; 
    BeXmlDom::SetErrorHandling (showMessages, doAssert);
    ECClass::SetErrorHandling(doAssert);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECSchema::_GetContainerSchema() const
    {
    return this;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR ECSchema::GetName () const
    {
    return m_name;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetName (WStringCR name)
    {        
    if (!NameValidator::Validate(name))
        return ECOBJECTS_STATUS_InvalidName;

    m_name = name;        
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR ECSchema::GetNamespacePrefix () const
    {        
    return m_namespacePrefix;    
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetNamespacePrefix (WStringCR namespacePrefix)
    {        
    m_namespacePrefix = namespacePrefix;  
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR ECSchema::GetDescription () const
    {
    return m_description;        
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetDescription (WStringCR description)
    {        
    m_description = description;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR ECSchema::GetDisplayLabel () const
    {
    return (m_displayLabel.empty()) ? GetName() : m_displayLabel;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetDisplayLabel (WStringCR displayLabel)
    {        
    m_displayLabel = displayLabel;
    return ECOBJECTS_STATUS_Success;
    }
    
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchema::GetIsDisplayLabelDefined () const
    {
    return (!m_displayLabel.empty());        
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<WString> s_standardSchemaNames;
void initStandardSchemaNames()
    {
    if (!s_standardSchemaNames.empty())
        return;
    s_standardSchemaNames.push_back(L"Bentley_Standard_CustomAttributes");
    s_standardSchemaNames.push_back(L"Bentley_Standard_Classes");
    s_standardSchemaNames.push_back(L"Bentley_ECSchemaMap");
    s_standardSchemaNames.push_back(L"EditorCustomAttributes");
    s_standardSchemaNames.push_back(L"Bentley_Common_Classes");
    s_standardSchemaNames.push_back(L"Dimension_Schema");
    s_standardSchemaNames.push_back(L"iip_mdb_customAttributes");
    s_standardSchemaNames.push_back(L"KindOfQuantity_Schema");
    s_standardSchemaNames.push_back(L"rdl_customAttributes");
    s_standardSchemaNames.push_back(L"SIUnitSystemDefaults");
    s_standardSchemaNames.push_back(L"Unit_Attributes");
    s_standardSchemaNames.push_back(L"Units_Schema");
    s_standardSchemaNames.push_back(L"USCustomaryUnitSystemDefaults");

    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchema::IsStandardSchema () const
    {
    initStandardSchemaNames();
    bvector<WString>::iterator iter = std::find(s_standardSchemaNames.begin(), s_standardSchemaNames.end(), m_name);
    return (iter != s_standardSchemaNames.end());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static bvector<WString> s_originalStandardSchemaFullNames;

void initOriginalStandardSchemaNames()
    {
    if (!s_originalStandardSchemaFullNames.empty())
        return;
    s_originalStandardSchemaFullNames.push_back(L"Bentley_Standard_CustomAttributes.01.00");
    s_originalStandardSchemaFullNames.push_back(L"Bentley_Standard_Classes.01.00");
    s_originalStandardSchemaFullNames.push_back(L"EditorCustomAttributes.01.00");
    s_originalStandardSchemaFullNames.push_back(L"Bentley_Common_Classes.01.00");
    s_originalStandardSchemaFullNames.push_back(L"Dimension_Schema.01.00");
    s_originalStandardSchemaFullNames.push_back(L"iip_mdb_customAttributes.01.00");
    s_originalStandardSchemaFullNames.push_back(L"KindOfQuantity_Schema.01.00");
    s_originalStandardSchemaFullNames.push_back(L"rdl_customAttributes.01.00");
    s_originalStandardSchemaFullNames.push_back(L"SIUnitSystemDefaults.01.00");
    s_originalStandardSchemaFullNames.push_back(L"Unit_Attributes.01.00");
    s_originalStandardSchemaFullNames.push_back(L"Units_Schema.01.00");
    s_originalStandardSchemaFullNames.push_back(L"USCustomaryUnitSystemDefaults.01.00");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchema::ShouldNotBeStored () const
    {
    initOriginalStandardSchemaNames();
    bvector<WString>::iterator iter = std::find(s_originalStandardSchemaFullNames.begin(), s_originalStandardSchemaFullNames.end(), GetFullSchemaName());
    if (iter != s_originalStandardSchemaFullNames.end())
        return true;

    // We don't want to import any version of the Units_Schema
    if (BeStringUtilities::Wcsicmp(L"Units_Schema", m_name.c_str()) == 0)
        return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 ECSchema::GetVersionMajor () const
    {
    return m_versionMajor;        
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetVersionMajor (const UInt32 versionMajor)
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
ECObjectsStatus ECSchema::SetVersionMinor (const UInt32 versionMinor)
    {        
    m_versionMinor = versionMinor;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassP ECSchema::GetClassP (WCharCP name) const
    {
    ClassMap::const_iterator  classIterator;
    classIterator = m_classMap.find (name);
    
    if ( classIterator != m_classMap.end() )
        return classIterator->second;
    else
        return NULL;
    }

void ECSchema::DebugDump()const
    {
    wprintf(L"ECSchema: this=0x%x  %s.%02d.%02d, nClasses=%d\n", this, m_name.c_str(), m_versionMajor, m_versionMinor, m_classMap.size());
    for (ClassMap::const_iterator it = m_classMap.begin(); it != m_classMap.end(); ++it)
        {
        bpair<WCharCP, ECClassP>const& entry = *it;
        ECClassCP ecClass = entry.second;
        wprintf(L"    ECClass: 0x%x, %s\n", ecClass, ecClass->GetName().c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::AddClass (ECClassP& pClass)
    {
    bpair <ClassMap::iterator, bool> resultPair;
    resultPair = m_classMap.insert (bpair<WCharCP, ECClassP> (pClass->GetName().c_str(), pClass));
    if (resultPair.second == false)
        {
        ECObjectsLogger::Log()->warningv (L"Can not create class '%s' because it already exists in the schema", pClass->GetName().c_str());
        delete pClass;
        pClass = NULL;        
        return ECOBJECTS_STATUS_NamedItemAlreadyExists;
        }
    //DebugDump(); wprintf(L"\n");
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::CreateClass (ECClassP& pClass, WStringCR name)
    {
    pClass = new ECClass(*this, m_hideFromLeakDetection);
    ECObjectsStatus status = pClass->SetName (name);
    if (ECOBJECTS_STATUS_Success != status)
        {
        delete pClass;
        pClass = NULL;
        return status;
        }

    return AddClass (pClass);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::CreateRelationshipClass (ECRelationshipClassP& pClass, WStringCR name)
    {
    pClass = new ECRelationshipClass(*this);
    ECObjectsStatus status = pClass->SetName (name);
    if (ECOBJECTS_STATUS_Success != status)
        {
        delete pClass;
        pClass = NULL;
        return status;
        }

    bpair < ClassMap::iterator, bool > resultPair;
    resultPair = m_classMap.insert (bpair<WCharCP, ECClassP> (pClass->GetName().c_str(), pClass));
    if (resultPair.second == false)
        {
        delete pClass;
        pClass = NULL;
        ECObjectsLogger::Log()->warningv (L"Can not create relationship class '%s' because it already exists in the schema", name.c_str());
        return ECOBJECTS_STATUS_NamedItemAlreadyExists;
        }

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECOBJECTS_EXPORT WString ECSchema::GetFullSchemaName () const
    {
    wchar_t fullName[1024]; // we decided to use a large buffer instead of caculating the length and using _alloc to boost performance 

    BeStringUtilities::Snwprintf (fullName, L"%s.%02d.%02d", GetName().c_str(), GetVersionMajor(), GetVersionMinor());
    return fullName;
    }

#define     ECSCHEMA_FULLNAME_FORMAT_EXPLANATION L" Format must be Name.MM.mm where Name is the schema name, MM is major version and mm is minor version."
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::ParseSchemaFullName (WStringR schemaName, UInt32& versionMajor, UInt32& versionMinor, WStringCR  fullName)
    {
    if (fullName.empty())
        return ECOBJECTS_STATUS_ParseError;

    WCharCP fullNameCP = fullName.c_str();
    WCharCP firstDot = wcschr (fullNameCP, L'.');
    if (NULL == firstDot)
        {
        ECObjectsLogger::Log()->errorv (L"Invalid ECSchema FullName String: '%s' does not contain a '.'!" ECSCHEMA_FULLNAME_FORMAT_EXPLANATION, fullName.c_str());
        return ECOBJECTS_STATUS_ParseError;
        }

    size_t nameLen = firstDot - fullNameCP;
    if (nameLen < 1)
        {
        ECObjectsLogger::Log()->errorv (L"Invalid ECSchema FullName String: '%s' does not have any characters before the '.'!" ECSCHEMA_FULLNAME_FORMAT_EXPLANATION, fullName.c_str());
        return ECOBJECTS_STATUS_ParseError;
        }

    schemaName.assign (fullNameCP, nameLen);

    return ParseVersionString (versionMajor, versionMinor, firstDot+1);
    }

#define     ECSCHEMA_FULLNAME_FORMAT_EXPLANATION L" Format must be Name.MM.mm where Name is the schema name, MM is major version and mm is minor version."
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::ParseSchemaFullName (WStringR schemaName, UInt32& versionMajor, UInt32& versionMinor, WCharCP fullName)
    {
    if (NULL == fullName || '\0' == *fullName)
        return ECOBJECTS_STATUS_ParseError;

    WCharCP firstDot = wcschr (fullName, L'.');
    if (NULL == firstDot)
        {
        ECObjectsLogger::Log()->errorv (L"Invalid ECSchema FullName String: '%s' does not contain a '.'!" ECSCHEMA_FULLNAME_FORMAT_EXPLANATION, fullName);
        return ECOBJECTS_STATUS_ParseError;
        }

    size_t nameLen = firstDot - fullName;
    if (nameLen < 1)
        {
        ECObjectsLogger::Log()->errorv (L"Invalid ECSchema FullName String: '%s' does not have any characters before the '.'!" ECSCHEMA_FULLNAME_FORMAT_EXPLANATION, fullName);
        return ECOBJECTS_STATUS_ParseError;
        }

    schemaName.assign (fullName, nameLen);

    return ParseVersionString (versionMajor, versionMinor, firstDot+1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECSchema::FormatSchemaVersion (UInt32& versionMajor, UInt32& versionMinor)
    {
    wchar_t versionString[80];
    BeStringUtilities::Snwprintf (versionString, _countof(versionString), L"%02d.%02d", versionMajor, versionMinor);
    return WString (versionString);
    }

#define     ECSCHEMA_VERSION_FORMAT_EXPLANATION L" Format must be MM.mm where MM is major version and mm is minor version."
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::ParseVersionString (UInt32& versionMajor, UInt32& versionMinor, WCharCP versionString)
    {
    versionMajor = DEFAULT_VERSION_MAJOR;
    versionMinor = DEFAULT_VERSION_MINOR;
    if (NULL == versionString || '\0' == *versionString)
        return ECOBJECTS_STATUS_Success;

    WCharCP theDot = wcschr (versionString, L'.');
    if (NULL == theDot)
        {
        ECObjectsLogger::Log()->errorv (L"Invalid ECSchema Version String: '%s' does not contain a '.'!" ECSCHEMA_VERSION_FORMAT_EXPLANATION, versionString);
        return ECOBJECTS_STATUS_ParseError;
        }

    size_t majorLen = theDot - versionString;
    if (majorLen < 1 || majorLen > 3)
        {
        ECObjectsLogger::Log()->errorv (L"Invalid ECSchema Version String: '%s' does not have 1-3 numbers before the '.'!" ECSCHEMA_VERSION_FORMAT_EXPLANATION, versionString);
        return ECOBJECTS_STATUS_ParseError;
        }

    WCharCP endDot = wcschr (theDot+1, L'.');
    size_t minorLen = (NULL != endDot) ? (endDot - theDot) - 1 : wcslen (theDot) - 1;
    if (minorLen < 1 || minorLen > 3)
        {
        ECObjectsLogger::Log()->errorv (L"Invalid ECSchema Version String: '%s' does not have 1-3 numbers after the '.'!" ECSCHEMA_VERSION_FORMAT_EXPLANATION, versionString);
        return ECOBJECTS_STATUS_ParseError;
        }

    WCharP end = NULL;    
    UInt32    localMajor = wcstoul (versionString, &end, 10);
    if (versionString == end)
        {
        ECObjectsLogger::Log()->errorv (L"Invalid ECSchema Version String: '%s' The characters before the '.' must be numeric!" ECSCHEMA_VERSION_FORMAT_EXPLANATION, versionString);
        return ECOBJECTS_STATUS_ParseError;
        }
    else
        {
        versionMajor = localMajor;
        }

    UInt32 localMinor = wcstoul (&theDot[1], &end, 10);
    if (&theDot[1] == end)
        {
        ECObjectsLogger::Log()->errorv (L"Invalid ECSchema Version String: '%s' The characters after the '.' must be numeric!" ECSCHEMA_VERSION_FORMAT_EXPLANATION, versionString);
        return ECOBJECTS_STATUS_ParseError;
        }
    else
        {
        versionMinor = localMinor;
        }

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetVersionFromString (WCharCP versionString)
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
WCharCP         soughtName,
UInt32          soughtMajor,
UInt32          soughtMinor,
WCharCP         candidateName,
UInt32          candidateMajor,
UInt32          candidateMinor
)
    {
    if (BeStringUtilities::Wcsicmp (candidateName, soughtName))
        return false;
    
    return ((matchType == SCHEMAMATCHTYPE_Latest) ||
            (matchType == SCHEMAMATCHTYPE_LatestCompatible && candidateMajor == soughtMajor && candidateMinor >= soughtMinor) ||
            (matchType == SCHEMAMATCHTYPE_Exact            && candidateMajor == soughtMajor && candidateMinor == soughtMinor));
    }
    
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::CreateSchema (ECSchemaP& schemaOut, WStringCR schemaName, UInt32 versionMajor, UInt32 versionMinor, IECSchemaOwnerR schemaOwner, bool hideFromLeakDetection)
    {    
    if (!NameValidator::Validate(schemaName))
        return ECOBJECTS_STATUS_InvalidName;

    ECSchemaP   schema = new ECSchema(hideFromLeakDetection);

    ECObjectsStatus status;
    
    if (SUCCESS != (status = schema->SetName (schemaName)) ||
        SUCCESS != (status = schema->SetVersionMajor (versionMajor)) ||
        SUCCESS != (status = schema->SetVersionMinor (versionMinor)))
        {
        delete schema;
        return status;
        }

    if (SUCCESS != (status = schemaOwner.AddSchema (*schema)))
        {
        delete schema;
        return status;
        }

    schemaOut = schema;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::CreateSchema (ECSchemaP& schemaOut, WStringCR schemaName, UInt32 versionMajor, UInt32 versionMinor, IECSchemaOwnerR schemaOwner)
    {
    return CreateSchema (schemaOut, schemaName, versionMajor, versionMinor, schemaOwner, false);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
void    ECSchema::DestroySchema (ECSchemaP& schema)
    {
    delete schema;
    schema = NULL;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaP ECSchema::GetSchemaByNamespacePrefixP (WStringCR namespacePrefix) const
    {
    if (namespacePrefix.length() == 0)
        return (ECSchemaP)this;

    // lookup referenced schema by prefix
    bmap<ECSchemaP, WString const>::const_iterator schemaIterator;
    for (schemaIterator = m_referencedSchemaNamespaceMap.begin(); schemaIterator != m_referencedSchemaNamespaceMap.end(); schemaIterator++)
        {
        if (0 == namespacePrefix.compare (schemaIterator->second))
            return schemaIterator->first;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::ResolveNamespacePrefix (ECSchemaCR schema, WStringR namespacePrefix) const
    {
    namespacePrefix = EMPTY_STRING;
    if (&schema == this)
        return ECOBJECTS_STATUS_Success;

    bmap<ECSchemaP, WString const>::const_iterator schemaIterator = m_referencedSchemaNamespaceMap.find((ECSchemaP) &schema);
    if (schemaIterator != m_referencedSchemaNamespaceMap.end())
        {
        namespacePrefix = schemaIterator->second;
        return ECOBJECTS_STATUS_Success;
        }

    return ECOBJECTS_STATUS_SchemaNotFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassContainerCR ECSchema::GetClasses () const
    {
    return m_classContainer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
const ECSchemaReferenceList& ECSchema::GetReferencedSchemas () const
    {
    return m_refSchemaList;
    }
  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::AddReferencedSchema (ECSchemaR refSchema)
    {
    return AddReferencedSchema (refSchema, refSchema.GetNamespacePrefix());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::AddReferencedSchema (ECSchemaR refSchema, WStringCR namespacePrefix)
    {
    ECSchemaReferenceList::const_iterator schemaIterator;
    for (schemaIterator = m_refSchemaList.begin(); schemaIterator != m_refSchemaList.end(); schemaIterator++)
        {
        if (*schemaIterator == &refSchema)
            return ECOBJECTS_STATUS_NamedItemAlreadyExists;
        }
    
    WString prefix(namespacePrefix);
    if (prefix.length() == 0)
        prefix = L"s";

    // Make sure prefix is unique within this schema
    bmap<ECSchemaP, WString const>::const_iterator namespaceIterator;
    for (namespaceIterator = m_referencedSchemaNamespaceMap.begin(); namespaceIterator != m_referencedSchemaNamespaceMap.end(); namespaceIterator++)
        {
        if (0 == prefix.compare (namespaceIterator->second))
            {
            break;
            }
        }

    // We found a matching prefix already being referenced
    if (namespaceIterator != m_referencedSchemaNamespaceMap.end())
        {
        int subScript;
        for (subScript = 1; subScript < 500; subScript++)
            {
            wchar_t temp[256];
            swprintf(temp, 256, L"%s%d", prefix.c_str(), subScript);
            WString tryPrefix(temp);
            for (namespaceIterator = m_referencedSchemaNamespaceMap.begin(); namespaceIterator != m_referencedSchemaNamespaceMap.end(); namespaceIterator++)
                {
                if (0 == tryPrefix.compare (namespaceIterator->second))
                    {
                    break;
                    }
                }
            // we didn't find the prefix in the map
            if (namespaceIterator == m_referencedSchemaNamespaceMap.end())
                {
                prefix = tryPrefix;
                break;
                }
            }
        }

    // Check for recursion
    m_refSchemaList.push_back(&refSchema);
    m_referencedSchemaNamespaceMap.insert(bpair<ECSchemaP, const WString> (&refSchema, prefix));
    return ECOBJECTS_STATUS_Success;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::RemoveReferencedSchema (ECSchemaR refSchema)
    {
    ECSchemaReferenceList::iterator schemaIterator;
    for (schemaIterator = m_refSchemaList.begin(); schemaIterator != m_refSchemaList.end(); schemaIterator++)
        {
        if (*schemaIterator == &refSchema)
            {
            // We need to verify that nothing references the schema before we can remove it.
            break;
            }
        }
    if (m_refSchemaList.end() == schemaIterator)
        return ECOBJECTS_STATUS_SchemaNotFound;
        
    // Can only remove the reference if nothing actually references it.
    
    ECSchemaP foundSchemaP = *schemaIterator;
    FOR_EACH (ECClassP ecClass, GetClasses())
        {
        // First, check each base class to see if the base class uses that schema
        FOR_EACH (ECClassP baseClass, ecClass->GetBaseClasses())
            {
            if ((ECSchemaP) &(baseClass->GetSchema()) == foundSchemaP)
                {
                return ECOBJECTS_STATUS_SchemaInUse;
                }
            }
            
        // If it is a relationship class, check the constraints to make sure the constraints don't use that schema
        ECRelationshipClassP relClass = dynamic_cast<ECRelationshipClassP>(ecClass);
        if (NULL != relClass)
            {
            FOR_EACH (ECClassP target, relClass->GetTarget().GetClasses())
                {
                if ((ECSchemaP) &(target->GetSchema()) == foundSchemaP)
                    {
                    return ECOBJECTS_STATUS_SchemaInUse;
                    }
                }
            FOR_EACH (ECClassP source, relClass->GetSource().GetClasses())
                {
                if ((ECSchemaP) &(source->GetSchema()) == foundSchemaP)
                    {
                    return ECOBJECTS_STATUS_SchemaInUse;
                    }
                }
            }
            
        // And make sure that there are no struct types from another schema
        FOR_EACH (ECPropertyP prop, ecClass->GetProperties(false))
            {
            ECClassCP typeClass;
            if (prop->GetIsStruct())
                {
                typeClass = &(prop->GetAsStructProperty()->GetType());
                }
            else if (prop->GetIsArray())
                {
                typeClass = prop->GetAsArrayProperty()->GetStructElementType();
                }
            else
                {
                typeClass = NULL;
                }
            if (NULL == typeClass)
                continue;
            if (this->GetName().compare(typeClass->GetSchema().GetName()) == 0 && this->GetVersionMajor() == typeClass->GetSchema().GetVersionMajor() &&
                this->GetVersionMinor() == typeClass->GetSchema().GetVersionMinor())
                continue;
            return ECOBJECTS_STATUS_SchemaInUse;
            }
        }

    m_refSchemaList.erase(schemaIterator); 
    bmap<ECSchemaP, const WString>::iterator iterator = m_referencedSchemaNamespaceMap.find((ECSchemaP) &refSchema);
    if (iterator != m_referencedSchemaNamespaceMap.end())
        {
        m_referencedSchemaNamespaceMap.erase(iterator);
        }

    return ECOBJECTS_STATUS_Success;
    }
    
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECSchema::ReadClassStubsFromXml (BeXmlNodeR schemaNode, ClassDeserializationVector& classes, ECSchemaReadContextR schemaContext)
    {
    SchemaReadStatus status = SCHEMA_READ_STATUS_Success;

    // Create ECClass Stubs (no attributes or properties)
    BeXmlDom::IterableNodeSet classNodes;
    schemaNode.SelectChildNodes (classNodes, EC_NAMESPACE_PREFIX ":" EC_CLASS_ELEMENT " | " EC_NAMESPACE_PREFIX ":" EC_RELATIONSHIP_CLASS_ELEMENT);
    FOR_EACH (BeXmlNodeP& classNode, classNodes)
        {
        ECClassP                ecClass;
        ECRelationshipClassP    ecRelationshipClass;
        
        if (0 == strcmp (classNode->GetName(), EC_CLASS_ELEMENT))
            {            
            ecClass = new ECClass (*this, m_hideFromLeakDetection);
            ecRelationshipClass = NULL;
            }
        else
            {            
            ecRelationshipClass = new ECRelationshipClass (*this);            
            ecClass = ecRelationshipClass;
            }

        if (SCHEMA_READ_STATUS_Success != (status = ecClass->_ReadXmlAttributes (*classNode)))
            {
            delete ecClass;
            return status;           
            }

        ECClassP existingClass = this->GetClassP (ecClass->GetName().c_str());

        if (NULL != existingClass)
            {
            existingClass->_ReadXmlAttributes (*classNode);
            delete ecClass;
            ecClass = existingClass;
            }
        else if (ECOBJECTS_STATUS_Success != this->AddClass (ecClass))
            return SCHEMA_READ_STATUS_InvalidECSchemaXml;

        if (NULL == ecRelationshipClass)
            ECObjectsLogger::Log()->tracev (L"    Created ECClass Stub: %s", ecClass->GetName().c_str());
        else
            ECObjectsLogger::Log()->tracev (L"    Created Relationship ECClass Stub: %s", ecClass->GetName().c_str());

        classes.push_back (make_bpair (ecClass, classNode));
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
 - Expects class stubs have already been read and created.  They are stored in the vector passed into this method.
 - Expects referenced schemas have been resolved and read so that base classes & structs in other schemas can be located.
 - Reads the contents of each XML node cached in the classes vector and populates the in-memory EC:ECClass with
   base classes, properties & relationship endpoints.
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECSchema::ReadClassContentsFromXml (ClassDeserializationVector& classes, ECSchemaReadContextR schemaContext)
    {
    SchemaReadStatus status = SCHEMA_READ_STATUS_Success;

    ClassDeserializationVector::const_iterator  classesStart, classesEnd, classesIterator;
    ECClassP    ecClass;
    BeXmlNodeP  classNode;
    for (classesStart = classes.begin(), classesEnd = classes.end(), classesIterator = classesStart; classesIterator != classesEnd; classesIterator++)
        {
        ecClass     = classesIterator->first;
        classNode   = classesIterator->second;
        status = ecClass->_ReadXmlContents (*classNode, schemaContext.GetStandaloneEnablerLocater());
        if (SCHEMA_READ_STATUS_Success != status)
            return status;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECSchema::ReadSchemaReferencesFromXml (BeXmlNodeR schemaNode, ECSchemaReadContextR schemaContext)
    {
    SchemaReadStatus status = SCHEMA_READ_STATUS_Success;
        
    m_referencedSchemaNamespaceMap.clear();

    BeXmlDom::IterableNodeSet schemaReferenceNodes;
    schemaNode.SelectChildNodes (schemaReferenceNodes, EC_NAMESPACE_PREFIX ":" EC_SCHEMAREFERENCE_ELEMENT);
    FOR_EACH (BeXmlNodeP& schemaReferenceNode, schemaReferenceNodes)
        {
        WString schemaName;
        if (BEXML_Success != schemaReferenceNode->GetAttributeStringValue (schemaName, SCHEMAREF_NAME_ATTRIBUTE))
            {
            ECObjectsLogger::Log()->errorv (L"Invalid ECSchemaXML: %hs element must contain a %hs attribute", schemaReferenceNode->GetName(), SCHEMAREF_NAME_ATTRIBUTE);
            return SCHEMA_READ_STATUS_InvalidECSchemaXml;
            }

        WString prefix;
        if (BEXML_Success != schemaReferenceNode->GetAttributeStringValue (prefix, SCHEMAREF_PREFIX_ATTRIBUTE))
            {
            ECObjectsLogger::Log()->errorv (L"Invalid ECSchemaXML: %hs element must contain a %hs attribute", schemaReferenceNode->GetName(), SCHEMAREF_PREFIX_ATTRIBUTE);
            return SCHEMA_READ_STATUS_InvalidECSchemaXml;
            }

        WString versionString;
        if (BEXML_Success != schemaReferenceNode->GetAttributeStringValue (versionString, SCHEMAREF_VERSION_ATTRIBUTE))
            {
            ECObjectsLogger::Log()->errorv (L"Invalid ECSchemaXML: %hs element must contain a %hs attribute", schemaReferenceNode->GetName(), SCHEMAREF_VERSION_ATTRIBUTE);
            return SCHEMA_READ_STATUS_InvalidECSchemaXml;
            }

        UInt32 versionMajor;
        UInt32 versionMinor;
        if (ECOBJECTS_STATUS_Success != ParseVersionString (versionMajor, versionMinor, versionString.c_str()))
            {
            ECObjectsLogger::Log()->errorv (L"Invalid ECSchemaXML: unable to parse version string for referenced schema %s.", schemaName.c_str());
            return SCHEMA_READ_STATUS_InvalidECSchemaXml;
            }
            
        // If the schema (uselessly) references itself, just skip it
        if (m_name.compare(schemaName) == 0)
            continue;

        ECObjectsLogger::Log()->debugv (L"About to locate referenced ECSchema %s.%02d.%02d", schemaName.c_str(), versionMajor, versionMinor);
        ECSchemaP referencedSchema = LocateSchema (schemaName, versionMajor, versionMinor, schemaContext);

        if (NULL != referencedSchema)
            {
            AddReferencedSchema (*referencedSchema, prefix);
            }
        else
            {
            ECObjectsLogger::Log()->errorv(L"Unable to locate referenced schema %s.%02d.%02d", schemaName.c_str(), versionMajor, versionMinor);
            return SCHEMA_READ_STATUS_ReferencedSchemaNotFound;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                 
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaP IECSchemaLocater::LocateSchema(WCharCP name, UInt32& versionMajor, UInt32& versionMinor, SchemaMatchType matchType, ECSchemaReadContextR schemaContext)
    {
    return _LocateSchema (name, versionMajor, versionMinor, matchType, schemaContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaP       ECSchema::LocateSchema (WStringCR name, UInt32& versionMajor, UInt32& versionMinor, ECSchemaReadContextR schemaContext)
    {
    // Step 1: First check if the owner already owns a copy of the schema.
    //         This will catch schema referencing cycles since the schemas are
    //         added to the owner as they are constructed. 

    // try exact match first
    ECSchemaP schema = schemaContext.GetSchemaOwner().GetSchema(name.c_str(), versionMajor, versionMinor);
    if (NULL != schema)
        return schema;

    // allow latest compatible
    schema = schemaContext.GetSchemaOwner().LocateSchema(name.c_str(), versionMajor, versionMinor, SCHEMAMATCHTYPE_LatestCompatible);
    if (NULL != schema)
        return schema;
  
    // Step 2: ask the schemaLocaters
    FOR_EACH (IECSchemaLocaterP schemaLocater, schemaContext.GetSchemaLocaters())
        {
        if ( ! EXPECTED_CONDITION (NULL != schemaLocater))
            continue;

        schema = schemaLocater->LocateSchema(name.c_str(), versionMajor, versionMinor, SCHEMAMATCHTYPE_LatestCompatible, schemaContext);
        if (NULL != schema)
            return schema;
        }

    // Step 3: look in the paths provided by the context
    schema = LocateSchemaByPath (name, versionMajor, versionMinor, schemaContext, true, NULL);
    if (NULL != schema)
        return schema;

    // Step 4: look in a set of standard paths
    schema = LocateSchemaByStandardPaths (name, versionMajor, versionMinor, schemaContext);
    if (NULL != schema)
        return schema;

    // Step 5: the final locator... this should be a managed locator from the interop layer, if it is present
    IECSchemaLocaterP schemaLocater = schemaContext.GetFinalSchemaLocater();
    if (NULL == schemaLocater)
        return NULL;

    schema = schemaLocater->LocateSchema(name.c_str(), versionMajor, versionMinor, SCHEMAMATCHTYPE_LatestCompatible, schemaContext);
    return schema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus GetMinorVersionFromSchemaFileName (UInt32& versionMinor, WCharCP filePath)
    {
    BeFileName  fileName (filePath);
    
    WString     name;
    fileName.ParseName (NULL, NULL, &name, NULL);

    // after fileName.parse, name contains "SchemaName.XX.XX.eschema". 
    WString::size_type firstDot;
    if (WString::npos == (firstDot = name.find ('.')))
        {
        assert (s_noAssert);
        ECObjectsLogger::Log()->errorv (L"Invalid ECSchema FileName String: '%s' does not contain the suffix '.ecschema.xml'!" ECSCHEMA_FULLNAME_FORMAT_EXPLANATION, filePath);
        return ECOBJECTS_STATUS_ParseError;
        }

    WString     versionString = name.substr (firstDot+1);
    UInt32      versionMajor;
    return ECSchema::ParseVersionString (versionMajor, versionMinor, versionString.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaP       ECSchema::FindMatchingSchema
(
bool&                           foundImperfectLegacyMatch,
WStringCR                       schemaMatchExpression,
WStringCR                       name,
UInt32&                         versionMajor,
UInt32&                         versionMinor,
ECSchemaReadContextR            schemaContext,
bool                            useLatestCompatibleMatch,
bool                            acceptImperfectLegacyMatch,
bvector<WString>*               searchPaths
)
    {
    foundImperfectLegacyMatch = false;
    if (!useLatestCompatibleMatch)
        acceptImperfectLegacyMatch = false;

    ECSchemaP   schemaOut = NULL;
    WString fullFileName;

    if (NULL == searchPaths)
        searchPaths = &schemaContext.GetSchemaPaths();
        
    FOR_EACH (WString schemaPath, *searchPaths)
        {
        if (schemaPath[schemaPath.length() - 1] != '\\')
            schemaPath += '\\';
        schemaPath += schemaMatchExpression;
        ECObjectsLogger::Log()->debugv (L"Checking for existence of %s...", schemaPath.c_str());

        //Finds latest
        UInt32 foundVersionMinor;
        if (SUCCESS != GetSchemaFileName (fullFileName, foundVersionMinor, schemaPath.c_str(), useLatestCompatibleMatch))
            continue;

        //Check if schema is compatible before reading, as reading it would add the schema to the cache.
        if (!SchemasMatch (useLatestCompatibleMatch ? SCHEMAMATCHTYPE_LatestCompatible : SCHEMAMATCHTYPE_Exact, 
                            name.c_str(),   versionMajor,   versionMinor,
                            name.c_str(),   versionMajor,   foundVersionMinor))
            {
            foundImperfectLegacyMatch = true;
            if (acceptImperfectLegacyMatch)
                {
                ECObjectsLogger::Log()->warningv (L"Located %s, which does not meet 'latest compatible' criteria to match %s.%02d.%02d, but is being accepted because some legacy schemas are known to require this", 
                                                  fullFileName.c_str(), name.c_str(), versionMajor, versionMinor);
                // See if this imperfect match ECSchema has is already cached (so we can avoid loading it, below)
                schemaOut = schemaContext.GetSchemaOwner().LocateSchema (name.c_str(), versionMajor, foundVersionMinor, SCHEMAMATCHTYPE_Exact);
                if (NULL != schemaOut)
                    return schemaOut;
                }
            else
                {
                ECObjectsLogger::Log()->warningv (L"Located %s, but it does not meet 'latest compatible' criteria to match %s.%02d.%02d. Caller can use acceptImperfectLegacyMatch to cause it to be accepted.", 
                                                  fullFileName.c_str(), name.c_str(), versionMajor, versionMinor);
                continue;
                }
            }

        if (SCHEMA_READ_STATUS_Success != ECSchema::ReadFromXmlFile (schemaOut, fullFileName.c_str(), schemaContext))
            continue;

        ECObjectsLogger::Log()->debugv (L"Located %s...", fullFileName.c_str());

        return schemaOut;
        }

    return schemaOut;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaP       ECSchema::LocateSchemaByPath (WStringCR name, UInt32& versionMajor, UInt32& versionMinor, ECSchemaReadContextR schemaContext, bool useLatestCompatibleMatch, bvector<WString>* searchPaths)
    {
    wchar_t versionString[24];
    if (useLatestCompatibleMatch)
        swprintf(versionString, 24, L".%02d.*.ecschema.xml", versionMajor);
    else
        swprintf(versionString, 24, L".%02d.%02d.ecschema.xml", versionMajor, versionMinor);

    WString schemaMatchExpression = name;
    schemaMatchExpression += versionString;
    WString fullFileName;

    bool foundImperfectLegacyMatch;
    ECSchemaP   schemaOut = FindMatchingSchema (foundImperfectLegacyMatch, schemaMatchExpression, name, versionMajor, versionMinor, schemaContext, useLatestCompatibleMatch, false, searchPaths);
    if (schemaContext.m_acceptLegacyImperfectLatestCompatibleMatch && NULL == schemaOut && foundImperfectLegacyMatch && useLatestCompatibleMatch)
        schemaOut = FindMatchingSchema (foundImperfectLegacyMatch, schemaMatchExpression, name, versionMajor, versionMinor, schemaContext, useLatestCompatibleMatch, true, searchPaths);

    return schemaOut;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SearchPathSchemaFileLocater::SearchPathSchemaFileLocater (bvector<WString>& searchPaths) : m_searchPaths(searchPaths) {};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SearchPathSchemaFileLocater::~SearchPathSchemaFileLocater () {};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SearchPathSchemaFileLocaterPtr SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(bvector<WString>& searchPaths)
    {
    return new SearchPathSchemaFileLocater(searchPaths);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaP SearchPathSchemaFileLocater::_LocateSchema(WCharCP name, UInt32& versionMajor, UInt32& versionMinor, SchemaMatchType matchType, ECSchemaReadContextR schemaContext)
    {
    WString schemaName(name);
    bool useLatestCompatibleMatch = matchType != SCHEMAMATCHTYPE_Exact;
    return ECSchema::LocateSchemaByPath (schemaName, versionMajor, versionMinor, schemaContext, useLatestCompatibleMatch, &m_searchPaths);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaP       ECSchema::LocateSchemaByStandardPaths (WStringCR name, UInt32& versionMajor, UInt32& versionMinor, ECSchemaReadContextR schemaContext)
    {
    WString dllPath = ECFileUtilities::GetDllPath();
    if (0 == dllPath.length())
        return NULL;
    
    bvector<WString> searchPaths;
    searchPaths.push_back (dllPath);    
    
    wchar_t schemaPath[MAX_PATH];
    wchar_t generalPath[MAX_PATH];
    wchar_t libraryPath[MAX_PATH];
    
    swprintf(schemaPath, MAX_PATH, L"%sECSchemas\\Standard", dllPath.c_str());
    swprintf(generalPath, MAX_PATH, L"%sECSchemas\\Standard\\General", dllPath.c_str());
    swprintf(libraryPath, MAX_PATH, L"%sECSchemas\\Standard\\LibraryUnits", dllPath.c_str());
    searchPaths.push_back (schemaPath);
    searchPaths.push_back (generalPath);
    searchPaths.push_back (libraryPath);
    
    return LocateSchemaByPath (name, versionMajor, versionMinor, schemaContext, true, &searchPaths);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECSchema::ReadXml (ECSchemaP& schemaOut, BeXmlDomR xmlDom, ECSchemaReadContextR schemaContext)
    {            
    SchemaReadStatus status = SCHEMA_READ_STATUS_Success;
    
    xmlDom.RegisterNamespace (EC_NAMESPACE_PREFIX, ECXML_URI_2_0);

    BeXmlNodeP      schemaNode;
    if ( (BEXML_Success != xmlDom.SelectNode (schemaNode, "/" EC_NAMESPACE_PREFIX ":" EC_SCHEMA_ELEMENT, NULL, BeXmlDom::NODE_BIAS_First)) || (NULL == schemaNode) )
        {
        assert (s_noAssert);
        ECObjectsLogger::Log()->errorv (L"Invalid ECSchemaXML: Missing a top-level %hs node in the %hs namespace", EC_SCHEMA_ELEMENT, ECXML_URI_2_0);
        return SCHEMA_READ_STATUS_InvalidECSchemaXml;
        }
    
    // schemaName is a REQUIRED attribute in order to create the schema
    WString schemaName;
    if (BEXML_Success != schemaNode->GetAttributeStringValue (schemaName, SCHEMA_NAME_ATTRIBUTE))
        {
        assert (s_noAssert);
        ECObjectsLogger::Log()->errorv (L"Invalid ECSchemaXML: %hs element must contain a schemaName attribute", EC_SCHEMA_ELEMENT);
        return SCHEMA_READ_STATUS_InvalidECSchemaXml;
        }

    UInt32  versionMajor = DEFAULT_VERSION_MAJOR;
    UInt32  versionMinor = DEFAULT_VERSION_MINOR;

    // OPTIONAL attributes - If these attributes exist they do not need to be valid.  We will ignore any errors setting them and use default values.
    // NEEDSWORK This is due to the current implementation in managed ECObjects.  We should reconsider whether it is the correct behavior.
    WString     versionString;
    if ( (BEXML_Success != schemaNode->GetAttributeStringValue (versionString, SCHEMA_VERSION_ATTRIBUTE)) || 
         (SUCCESS != ParseVersionString (versionMajor, versionMinor, versionString.c_str())) )
        {
        ECObjectsLogger::Log()->warningv (L"Invalid version attribute has been ignored while reading ECSchema '%s'.  The default version number %02d.%02d has been applied.", 
            schemaName.c_str(), versionMajor, versionMinor);
        }

    ECObjectsLogger::Log()->debugv (L"Reading ECSchema %s.%02d.%02d", (WCharCP)schemaName.c_str(), versionMajor, versionMinor);

    IECSchemaOwnerR schemaOwner = schemaContext.GetSchemaOwner();
    bool            hideFromLeakDetection = schemaContext.GetHideSchemasFromLeakDetection();
    
    ECObjectsStatus createStatus = CreateSchema (schemaOut, schemaName, versionMajor, versionMinor, schemaOwner, hideFromLeakDetection);
    if (ECOBJECTS_STATUS_DuplicateSchema == createStatus)
        return SCHEMA_READ_STATUS_DuplicateSchema;
    
    if (ECOBJECTS_STATUS_Success != createStatus)
        return SCHEMA_READ_STATUS_InvalidECSchemaXml;

    // OPTIONAL attributes - If these attributes exist they MUST be valid        
    WString value;  // used by macro.
    READ_OPTIONAL_XML_ATTRIBUTE ((*schemaNode), SCHEMA_NAMESPACE_PREFIX_ATTRIBUTE,         schemaOut, NamespacePrefix)
    READ_OPTIONAL_XML_ATTRIBUTE ((*schemaNode), DESCRIPTION_ATTRIBUTE,                     schemaOut, Description)
    READ_OPTIONAL_XML_ATTRIBUTE ((*schemaNode), DISPLAY_LABEL_ATTRIBUTE,                   schemaOut, DisplayLabel)

    if (SCHEMA_READ_STATUS_Success != (status = schemaOut->ReadSchemaReferencesFromXml (*schemaNode, schemaContext)))
        {
        schemaOwner.DropSchema (*schemaOut);
        schemaOut = NULL;
        return status;
        }

    ClassDeserializationVector classes;
    if (SCHEMA_READ_STATUS_Success != (status = schemaOut->ReadClassStubsFromXml (*schemaNode, classes, schemaContext)))
        {
        schemaOwner.DropSchema (*schemaOut);
        schemaOut = NULL;
        return status;
        }

    // NEEDSWORK ECClass inheritance (base classes, properties & relationship endpoints)
    if (SCHEMA_READ_STATUS_Success != (status = schemaOut->ReadClassContentsFromXml (classes, schemaContext)))
        {
        schemaOwner.DropSchema (*schemaOut);
        schemaOut = NULL;
        return status;
        }

    schemaOut->ReadCustomAttributes(*schemaNode, *schemaOut, schemaContext.GetStandaloneEnablerLocater());

    return SCHEMA_READ_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static bool ClassNameComparer (ECClassP class1, ECClassP class2)
    {
    // null values are always less than non-null values
    if (NULL == class1)
        return true;
    if (NULL == class2)
        return false;
    int comparison = wcscmp(class1->GetName().c_str(), class2->GetName().c_str());
    return (comparison <= 0);
    }
  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECSchema::WriteSchemaReferences (BeXmlNodeR parentNode) const
    {
    SchemaWriteStatus status = SCHEMA_WRITE_STATUS_Success;
    bmap<ECSchemaP, const WString>::const_iterator iterator;
    for (iterator = m_referencedSchemaNamespaceMap.begin(); iterator != m_referencedSchemaNamespaceMap.end(); iterator++)
        {
        bpair<ECSchemaP, const WString> mapPair = *(iterator);
        ECSchemaP   refSchema           = mapPair.first;
        BeXmlNodeP  schemaReferenceNode = parentNode.AddEmptyElement (EC_SCHEMAREFERENCE_ELEMENT);
        
        schemaReferenceNode->AddAttributeStringValue (SCHEMAREF_NAME_ATTRIBUTE, refSchema->GetName().c_str());
        
        wchar_t versionString[8];
        swprintf(versionString, 8, L"%02d.%02d", refSchema->GetVersionMajor(), refSchema->GetVersionMinor());
        schemaReferenceNode->AddAttributeStringValue (SCHEMAREF_VERSION_ATTRIBUTE, versionString);

        const WString prefix = mapPair.second;
        schemaReferenceNode->AddAttributeStringValue (SCHEMAREF_PREFIX_ATTRIBUTE, prefix.c_str());
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECSchema::WriteCustomAttributeDependencies (BeXmlNodeR parentNode, IECCustomAttributeContainerCR container, ECSchemaWriteContext& context) const
    {
    SchemaWriteStatus status = SCHEMA_WRITE_STATUS_Success;

    FOR_EACH (IECInstancePtr instance, container.GetCustomAttributes(false))
        {
        ECClassCR currentClass = instance->GetClass();
        status = WriteClass (parentNode, currentClass, context);
        if (SCHEMA_WRITE_STATUS_Success != status)
            return status;
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECSchema::WriteClass (BeXmlNodeR parentNode, ECClassCR ecClass, ECSchemaWriteContext& context) const
    {
    SchemaWriteStatus status = SCHEMA_WRITE_STATUS_Success;
    // don't write any classes that aren't in the schema we're writing.
    if (&(ecClass.GetSchema()) != this)
        return status;
    
    bset<WCharCP>::const_iterator setIterator;
    setIterator = context.m_alreadyWrittenClasses.find(ecClass.GetName().c_str());
    // Make sure we don't write any class twice
    if (setIterator != context.m_alreadyWrittenClasses.end())
        return status;
    else
        context.m_alreadyWrittenClasses.insert(ecClass.GetName().c_str());
        
    // write the base classes first.
    FOR_EACH (ECClassP baseClass, ecClass.GetBaseClasses())
        {
        WriteClass(parentNode, *baseClass, context);
        }
       
    // Serialize relationship constraint dependencies
    ECRelationshipClassP relClass = dynamic_cast<ECRelationshipClassP>((ECClassP) &ecClass);
    if (NULL != relClass)
        {
        FOR_EACH (ECClassP source, relClass->GetSource().GetClasses())
            WriteClass(parentNode, *source, context);
            
        FOR_EACH (ECClassP target, relClass->GetTarget().GetClasses())
            WriteClass(parentNode, *target, context);
        }
    WritePropertyDependencies(parentNode, ecClass, context); 
    WriteCustomAttributeDependencies(parentNode, ecClass, context);
    
    BeXmlNodeP  classNode;
    ecClass._WriteXml (classNode, parentNode);
    
    return status;
    }  
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECSchema::WritePropertyDependencies (BeXmlNodeR parentNode, ECClassCR ecClass, ECSchemaWriteContext& context) const
    {
    SchemaWriteStatus status = SCHEMA_WRITE_STATUS_Success;
    
    FOR_EACH (ECPropertyP prop, ecClass.GetProperties(false))
        {
        if (prop->GetIsStruct())
            {
            StructECPropertyP structProperty = prop->GetAsStructProperty();
            WriteClass(parentNode, structProperty->GetType(), context);
            }
        else if (prop->GetIsArray())
            {
            ArrayECPropertyP arrayProperty = prop->GetAsArrayProperty();
            if (arrayProperty->GetKind() == ARRAYKIND_Struct)
                {
                WriteClass(parentNode, *(arrayProperty->GetStructElementType()), context);
                }
            }
        WriteCustomAttributeDependencies(parentNode, *prop, context);
        }
    return status;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECSchema::WriteXml (BeXmlDomR xmlDom) const
    {
    BeXmlNodeP  schemaNode = xmlDom.AddNewElement (EC_SCHEMA_ELEMENT, NULL, NULL);

    wchar_t versionString[8];
    BeStringUtilities::Snwprintf (versionString, _countof(versionString), L"%02d.%02d", m_versionMajor, m_versionMinor);

    schemaNode->AddAttributeStringValue (SCHEMA_NAME_ATTRIBUTE, this->GetName().c_str());
    schemaNode->AddAttributeStringValue (SCHEMA_NAMESPACE_PREFIX_ATTRIBUTE, this->GetNamespacePrefix().c_str());
    schemaNode->AddAttributeStringValue (SCHEMA_VERSION_ATTRIBUTE, versionString);
    schemaNode->AddAttributeStringValue (DESCRIPTION_ATTRIBUTE, this->GetDescription().c_str());
    if (GetIsDisplayLabelDefined())
        schemaNode->AddAttributeStringValue (DISPLAY_LABEL_ATTRIBUTE, this->GetDisplayLabel().c_str());

    // make sure the schema node has a namespace, and if it doesn't set its default namespace.
    if (NULL == schemaNode->GetNamespace())
        schemaNode->SetNamespace (NULL, ECXML_URI_2_0);

    WString namespaceSpec (ECXML_URI_2_0);
    schemaNode->AddAttributeStringValue ("xmlns:" EC_NAMESPACE_PREFIX, namespaceSpec.c_str());
    
    WriteSchemaReferences (*schemaNode);
    
    ECSchemaWriteContext context;
    WriteCustomAttributeDependencies (*schemaNode, *this, context);
    WriteCustomAttributes (*schemaNode);
    
    std::list<ECClassP> sortedClasses;
    // sort the classes by name so the order in which they are written is predictable.
    FOR_EACH (ECClassP pClass, GetClasses())
        sortedClasses.push_back(pClass);
        
    sortedClasses.sort (ClassNameComparer);
    
    FOR_EACH (ECClassP pClass, sortedClasses)
        {
        WriteClass (*schemaNode, *pClass, context);
        }
        
    ECXml::FormatXml (xmlDom);
    return SCHEMA_WRITE_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus LogXmlLoadError (BeXmlDomP xmlDom)
    {        
    WString     errorString;
    int         line = 0, linePos = 0;
    if (NULL == xmlDom)
        {
        BeXmlDom::GetLastErrorString (errorString);
        }
    else
        {
        xmlDom->GetErrorMessage (errorString);
        xmlDom->GetErrorLocation (line, linePos);
        }

    ECObjectsLogger::Log()->errorv (errorString.c_str());
    ECObjectsLogger::Log()->errorv (L"line %d, position %d", line, linePos);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
void AddFilePathToSchemaPaths  (ECSchemaReadContextR schemaContext, T_WStringVectorR schemaPaths, WCharCP ecSchemaXmlFile)
    {
    WString dev, dir;
    BeFileName::ParseName (&dev, &dir, NULL, NULL, ecSchemaXmlFile);
    WString pathToThisSchema = dev + WCSDIR_DEV_SEPARATOR_CHAR + dir;
    FOR_EACH(WStringCR schemaPath, schemaPaths)
        {
        BeFileName::ParseName (&dev, &dir, NULL, NULL, schemaPath.c_str());
        WString normalizedPath = dev + WCSDIR_DEV_SEPARATOR_CHAR + dir;
        if (0 == normalizedPath.CompareToI(pathToThisSchema))
            return; // it's already there
        }
    schemaContext.AddSchemaPath(pathToThisSchema.c_str());
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECSchema::ReadFromXmlFile (ECSchemaP& schemaOut, WCharCP ecSchemaXmlFile, ECSchemaReadContextR schemaContext)
    {
    ECObjectsLogger::Log()->debugv (L"About to read native ECSchema read from file: fileName='%s'", ecSchemaXmlFile);
	schemaOut = NULL;
	
    SchemaReadStatus status = SCHEMA_READ_STATUS_Success;

    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromFile (xmlStatus, ecSchemaXmlFile);
    if ((xmlStatus != BEXML_Success) || !xmlDom.IsValid())
        {
        assert (s_noAssert);
        LogXmlLoadError (xmlDom.get());
        return SCHEMA_READ_STATUS_FailedToParseXml;
        }

    AddFilePathToSchemaPaths(schemaContext, schemaContext.GetSchemaPaths(), ecSchemaXmlFile);

    status = ReadXml (schemaOut, *xmlDom.get(), schemaContext);
    if (SCHEMA_READ_STATUS_DuplicateSchema == status)
        return status; // already logged

    if (ECOBJECTS_STATUS_Success != status)
        ECObjectsLogger::Log()->errorv (L"Failed to read XML file: %s", ecSchemaXmlFile);
    else
        ECObjectsLogger::Log()->infov (L"Native ECSchema read from file: fileName='%s', schemaName='%s.%02d.%02d' classCount='%d' address='0x%x'", 
            ecSchemaXmlFile, schemaOut->GetName().c_str(), schemaOut->GetVersionMajor(), schemaOut->GetVersionMinor(), schemaOut->m_classMap.size(), schemaOut);        

    return status;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus     ECSchema::ReadFromXmlString
(
ECSchemaP&                      schemaOut, 
WCharCP                         ecSchemaXml,
ECSchemaReadContextR schemaContext
)
    {                  
    ECObjectsLogger::Log()->debugv (L"About to read native ECSchema read from string."); // mainly included for timing
	schemaOut = NULL;
    SchemaReadStatus status = SCHEMA_READ_STATUS_Success;

    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromString (xmlStatus, ecSchemaXml, wcslen (ecSchemaXml) * sizeof(WChar));
    
    if (BEXML_Success != xmlStatus)
        {
        assert (s_noAssert);
        LogXmlLoadError (xmlDom.get());
        return SCHEMA_READ_STATUS_FailedToParseXml;
        }

    status = ReadXml (schemaOut, *xmlDom.get(), schemaContext);
    if (SCHEMA_READ_STATUS_DuplicateSchema == status)
        return status; // already logged

    if (ECOBJECTS_STATUS_Success != status)
        {
        WChar first200Characters[201];
        wcsncpy (first200Characters, ecSchemaXml, 200);
        first200Characters[200] = L'\0';
        ECObjectsLogger::Log()->errorv (L"Failed to read XML from string (1st 200 characters): %s", first200Characters);
        }
    else
        ECObjectsLogger::Log()->infov (L"Native ECSchema read from string: schemaName='%s.%02d.%02d' classCount='%d' schemaAddress='0x%x' stringAddress='0x%x'", 
            schemaOut->GetName().c_str(), schemaOut->GetVersionMajor(), schemaOut->GetVersionMinor(), schemaOut->m_classMap.size(), schemaOut, ecSchemaXml);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchema::IsSchemaReferenced
(
ECSchemaCR thisSchema, 
ECSchemaCR potentiallyReferencedSchema
)
    {
    ECSchemaReferenceList referencedSchemas = thisSchema.GetReferencedSchemas();
    ECSchemaReferenceList::const_iterator schemaIterator;
    for (schemaIterator = referencedSchemas.begin(); schemaIterator != referencedSchemas.end(); schemaIterator++)
        {
        ECSchemaP refSchema = *schemaIterator;
        if (ECSchema::SchemasAreEqualByName (refSchema, &(potentiallyReferencedSchema)))
            {
            return true;
            }
        }
    return false;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchema::SchemasAreEqualByName (ECSchemaCP thisSchema, ECSchemaCP thatSchema)
    {
    return ((thisSchema == thatSchema) ||
            ( (0 == thisSchema->GetName().compare(thatSchema->GetName())) &&
              (thisSchema->GetVersionMajor() == thatSchema->GetVersionMajor()) &&
              (thisSchema->GetVersionMinor() == thatSchema->GetVersionMinor())));
    }
    
#if defined (NEEDSWORK_LIBXML)
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus     ECSchema::ReadFromXmlStream
(
ECSchemaP&                      schemaOut, 
IStreamP                        ecSchemaXmlStream,
ECSchemaReadContextR schemaContext
)
    {                  
    SchemaReadStatus status = SCHEMA_READ_STATUS_Success;

    MSXML2::IXMLDOMDocument2Ptr xmlDocPtr = NULL;        
    VERIFY_HRESULT_OK(xmlDocPtr.CreateInstance(__uuidof(MSXML2::DOMDocument60)), SCHEMA_READ_STATUS_FailedToInitializeMsmxl);
    xmlDocPtr->put_validateOnParse(VARIANT_TRUE);
    xmlDocPtr->put_async(VARIANT_FALSE);
    
    VARIANT_BOOL returnCode = xmlDocPtr->load(ecSchemaXmlStream);
    if (returnCode != VARIANT_TRUE)
        {
        LogXmlLoadError (xmlDom.get());
        return SCHEMA_READ_STATUS_FailedToParseXml;
        }

    status = ReadXml (schemaOut, xmlDocPtr, schemaContext);
    if (SCHEMA_READ_STATUS_DuplicateSchema == status)
        return status; // already logged
    
    if (ECOBJECTS_STATUS_Success != status)
        ECObjectsLogger::Log()->errorv (L"Failed to read XML from stream");
    return status;
    return SCHEMA_READ_STATUS_FailedToParseXml;
    }
#endif //defined (NEEDSWORK_LIBXML)

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECSchema::WriteToXmlString (WStringR ecSchemaXml) const
    {
    ecSchemaXml.clear();

    BeXmlDomPtr xmlDom = BeXmlDom::CreateEmpty();        

    SchemaWriteStatus status;
    if (SCHEMA_WRITE_STATUS_Success != (status = WriteXml (*xmlDom.get())))
        return status;

    xmlDom->ToString (ecSchemaXml, BeXmlDom::TO_STRING_OPTION_OmitByteOrderMark);

    return SCHEMA_WRITE_STATUS_Success;
    }
   
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECSchema::WriteToXmlFile
(
WCharCP ecSchemaXmlFile,
bool    utf16
)
    {
    BeXmlDomPtr xmlDom = BeXmlDom::CreateEmpty();        

    SchemaWriteStatus status;
    if (SCHEMA_WRITE_STATUS_Success != (status = WriteXml (*xmlDom.get())))
        return status;

    return (BEXML_Success == xmlDom->ToFile (ecSchemaXmlFile, BeXmlDom::TO_STRING_OPTION_Indent, utf16 ? BeXmlDom::FILE_ENCODING_Utf16 : BeXmlDom::FILE_ENCODING_Utf8)) 
        ? SCHEMA_WRITE_STATUS_Success : SCHEMA_WRITE_STATUS_FailedToWriteFile;
    }
   
#if defined (NEEDSWORK_LIBXML)
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECSchema::WriteToXmlStream
(
IStreamP ecSchemaXmlStream,
bool     utf16
)
    {
    SchemaWriteStatus status = SCHEMA_WRITE_STATUS_Success;

    MSXML2::IXMLDOMDocument2Ptr xmlDocPtr = NULL;        
    VERIFY_HRESULT_OK(xmlDocPtr.CreateInstance(__uuidof(MSXML2::DOMDocument60)), SCHEMA_WRITE_STATUS_FailedToInitializeMsmxl);
    xmlDocPtr->put_validateOnParse(VARIANT_TRUE);
    xmlDocPtr->put_async(VARIANT_FALSE);
    xmlDocPtr->put_preserveWhiteSpace(VARIANT_TRUE);
    xmlDocPtr->put_resolveExternals(VARIANT_FALSE);
    
    status = WriteXml(xmlDocPtr);
    if (status != SCHEMA_WRITE_STATUS_Success)
        return status;
        
    VERIFY_HRESULT_OK(xmlDocPtr->save(ecSchemaXmlStream), SCHEMA_WRITE_STATUS_FailedToSaveXml);

    return status;
    }
#endif //defined (NEEDSWORK_LIBXML)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECSchema::FindAllSchemasInGraph (bvector<EC::ECSchemaCP>& allSchemas, ECSchemaCR rootSchema, bool includeRootSchema)
    {
    if (includeRootSchema)
        allSchemas.push_back (&rootSchema);

    FOR_EACH(ECSchemaP refSchema , rootSchema.GetReferencedSchemas())
        {
        if (!includeRootSchema)
            {
            if (&rootSchema == refSchema)
                continue;
            }

        bvector<EC::ECSchemaCP>::iterator it = std::find (allSchemas.begin(), allSchemas.end(), refSchema);

        if (it != allSchemas.end())
            continue;

        FindAllSchemasInGraph (allSchemas, *refSchema);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod 
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECSchema::FindSchema (WCharCP schemaName) const
    {
    if (NULL == schemaName)
        return NULL;
    
    if (m_name.EqualsI(schemaName))
        return this;
    
    FOR_EACH (ECSchemaP referencedSchema, GetReferencedSchemas())
        {
        ECSchemaCP schema = referencedSchema->FindSchema (schemaName);
        if (NULL != schema)
            return schema;
        }
    
    return NULL;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaReadContext::ECSchemaReadContext(IECSchemaOwnerR owner, IStandaloneEnablerLocaterP enablerLocater, bool acceptLegacyImperfectLatestCompatibleMatch)
    :
    m_schemaOwner (owner), m_standaloneEnablerLocater(enablerLocater), m_hideSchemasFromLeakDetection (false), 
    m_acceptLegacyImperfectLatestCompatibleMatch(acceptLegacyImperfectLatestCompatibleMatch),
    m_finalSchemaLocater (NULL)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    06/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaReadContextPtr  ECSchemaReadContext::CreateContext (IECSchemaOwnerR owner, IStandaloneEnablerLocaterP enablerLocater, bool acceptLegacyImperfectLatestCompatibleMatch)   
                                                                                        { return new ECSchemaReadContext(owner, enablerLocater, acceptLegacyImperfectLatestCompatibleMatch); }
ECSchemaReadContextPtr  ECSchemaReadContext::CreateContext (IECSchemaOwnerR owner, bool acceptLegacyImperfectLatestCompatibleMatch) 
    { 
    return CreateContext (owner, NULL, acceptLegacyImperfectLatestCompatibleMatch); 
    }
void  ECSchemaReadContext::AddSchemaLocaters (bvector<EC::IECSchemaLocaterP>& locators) { m_locators.insert (m_locators.begin(), locators.begin(), locators.end());  }
void  ECSchemaReadContext::AddSchemaLocater (IECSchemaLocaterR locater)      { m_locators.push_back (&locater);  }
void  ECSchemaReadContext::AddSchemaPath (WCharCP path)               { m_searchPaths.push_back (WString(path));   }
void  ECSchemaReadContext::HideSchemasFromLeakDetection ()                   { m_hideSchemasFromLeakDetection = true; }
bvector<IECSchemaLocaterP>& ECSchemaReadContext::GetSchemaLocaters ()                { return m_locators;    }
void                        ECSchemaReadContext::SetFinalSchemaLocater (IECSchemaLocaterR locater) { m_finalSchemaLocater = &locater;  }
IECSchemaLocaterP           ECSchemaReadContext::GetFinalSchemaLocater ()                { return m_finalSchemaLocater; }
T_WStringVectorR            ECSchemaReadContext::GetSchemaPaths ()                   { return m_searchPaths; }
void                        ECSchemaReadContext::ClearSchemaPaths ()                 { m_searchPaths.clear();    }
IECSchemaOwnerR             ECSchemaReadContext::GetSchemaOwner()                    { return m_schemaOwner;  }
IStandaloneEnablerLocaterP  ECSchemaReadContext::GetStandaloneEnablerLocater()       { return m_standaloneEnablerLocater;  }
bool                        ECSchemaReadContext::GetHideSchemasFromLeakDetection()   { return m_hideSchemasFromLeakDetection;  }


/////////////////////////////////////////////////////////////////////////////////////////
// IECSchemaOwner
/////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus IECSchemaOwner::AddSchema  (ECSchemaR schema) { return _AddSchema (schema); }
ECObjectsStatus IECSchemaOwner::DropSchema (ECSchemaR schema) { return _DropSchema (schema); }
ECSchemaP       IECSchemaOwner::GetSchema  (WCharCP name, UInt32 major, UInt32 minor) { return _GetSchema (name, major, minor); }
ECSchemaP       IECSchemaOwner::LocateSchema (WCharCP name, UInt32 major, UInt32 minor, SchemaMatchType matchType) { return _LocateSchema (name, major, minor, matchType); }

/////////////////////////////////////////////////////////////////////////////////////////
// IStandaloneEnablerLocater
/////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECEnablerPtr  IStandaloneEnablerLocater::LocateStandaloneEnabler (WCharCP schemaName, WCharCP className)
    {
    return  _LocateStandaloneEnabler (schemaName, className);
    }

/////////////////////////////////////////////////////////////////////////////////////////
// ECSchemaCache
/////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCache::~ECSchemaCache ()
    {
    if (m_ecEnablerMap.size())
        m_ecEnablerMap.clear ();

    if (0 == m_schemas.size())
        return;

    FOR_EACH (ECSchemaP ecSchema, m_schemas)
        ECSchema::DestroySchema (ecSchema);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
int                             ECSchemaCache::GetCount ()
    {
    return (int)m_schemas.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchemaCache::_AddSchema   (ECSchemaR ecSchema)
    {
    m_schemas.push_back (&ecSchema);

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchemaCache::_DropSchema  (ECSchemaR ecSchema)
    {
    bvector<ECSchemaP>::iterator iter = std::find(m_schemas.begin(), m_schemas.end(), &ecSchema);
    if (iter == m_schemas.end())
        return ECOBJECTS_STATUS_SchemaNotFound;

    ECSchema::DestroySchema (*iter);

    m_schemas.erase(iter);

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void                             ECSchemaCache::Clear ()
    {
    for (bvector<ECSchemaP>::iterator it = m_schemas.begin(); it != m_schemas.end(); ++it)
        ECSchema::DestroySchema (*it);
    m_schemas.clear();
    m_ecEnablerMap.clear();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaP       ECSchemaCache::_GetSchema   (WCharCP schemaName, UInt32 versionMajor, UInt32 versionMinor)
    {
    return _LocateSchema (schemaName, versionMajor, versionMinor, SCHEMAMATCHTYPE_Exact);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaP       ECSchemaCache::_LocateSchema (WCharCP schemaName, UInt32 versionMajor, UInt32 versionMinor, SchemaMatchType matchType)
    {
    FOR_EACH (ECSchemaP ecSchema, m_schemas)
        {
        if (ECSchema::SchemasMatch (matchType, schemaName, versionMajor, versionMinor, ecSchema->GetName().c_str(), ecSchema->GetVersionMajor(), ecSchema->GetVersionMinor()))
            return ecSchema;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCachePtr    ECSchemaCache::Create ()
    {
    return new ECSchemaCache;
    }

/////////////////////////////////////////////////////////////////////////////////////////
// ECClassContainer
/////////////////////////////////////////////////////////////////////////////////////////
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
bool    ECClassContainer::const_iterator::operator== (const_iterator const& rhs) const
    {
    return (m_state->m_mapIterator == rhs.m_state->m_mapIterator);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassP const& ECClassContainer::const_iterator::operator*() const
    {
    // Get rid of ECClassContainer or make it return a pointer directly
#ifdef CREATES_A_TEMP
    bpair<WCharCP , ECClassP> const& mapPair = *(m_state->m_mapIterator);
    return mapPair.second;
#else
    return m_state->m_mapIterator->second;
#endif
    };

/*---------------------------------------------------------------------------------**//**
* Validates a name and ensures a name contains only alphanumeric and underscore characters and does not start with a digit.
*
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool NameValidator::Validate (WStringCR name) 
    {
    if (name.empty())
        return false;
    if (   L'0' <= name[0]
        && L'9' >= name[0])
        return false; 
    for (WString::size_type index = 0; index != name.length(); ++index)
        {
        if(    (L'a' > name[index] || L'z' < name[index]) 
            && (L'A' > name[index] || L'Z' < name[index])
            && (L'0' > name[index] || L'9' < name[index])
            && '_'  != name[index])
            return false;
        } 
    return true;
    }

END_BENTLEY_EC_NAMESPACE


