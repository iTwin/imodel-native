/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECSchema.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"
#include "SchemaXml.h"
#if defined (_WIN32) // WIP_NONPORT - iostreams not support on Android
#include <iomanip>
#endif
#include <Bentley/BeFileName.h>
#include <Bentley/BeFile.h>
#include <Bentley/BeFileListIterator.h>

#include <ECObjects/StronglyConnectedGraph.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

extern ECObjectsStatus GetSchemaFileName (WStringR fullFileName, uint32_t& foundVersionMinor, WCharCP schemaPath, bool useLatestCompatibleMatch);


// If you are developing schemas, particularly when editing them by hand, you want to have this variable set to false so you get the asserts to help you figure out what is going wrong.
// Test programs generally want to get error status back and not assert, so they call ECSchema::AssertOnXmlError (false);
static  bool        s_noAssert = false;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECNameValidation::IsValidName (Utf8CP name)
    {
    return RESULT_Valid == Validate (name);
    }

/*---------------------------------------------------------------------------------**//**
* Currently this is only used by ECValidatedName and ECSchema.
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECNameValidation::ValidationResult ECNameValidation::Validate (Utf8CP name)
    {
    if (NULL == name || 0 == *name)
        return RESULT_NullOrEmpty;
    else if ('0' <= name[0] && '9' >= name[0])
        return RESULT_BeginsWithDigit;
    else
        {
        for (Utf8CP cur = name; *cur; ++cur)
            if (!IsValidAlphaNumericCharacter (*cur))
                return RESULT_IncludesInvalidCharacters;
        }

    return RESULT_Valid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ECNameValidation::AppendEncodedCharacter (WStringR encoded, WChar c)
    {
    WChar buf[5];
    HexFormatOptions opts = (HexFormatOptions)(static_cast<int>(HexFormatOptions::LeadingZeros) | static_cast<int>(HexFormatOptions::Uppercase));
    BeStringUtilities::FormatUInt64 (buf, _countof(buf), (uint64_t)c, opts, 4);
    encoded.append(L"__x");
    encoded.append(buf);
    encoded.append(L"__");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECNameValidation::IsValidAlphaNumericCharacter (WChar c)
    {
    return (((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_'));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECNameValidation::IsValidAlphaNumericCharacter (Utf8Char c)
    {
    return (((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_'));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECNameValidation::DecodeFromValidName (Utf8StringR decoded, Utf8StringCR name)
    {
    // "__x####__"
    //  012345678

    decoded = name;
    if (Utf8String::npos == name.find("__x"))
        return false; // => no decoding was required

    // TFS#298776: The encoding has always been done using UTF-16 - first in .NET, then in Vancouver.
    // Therefore we must convert.
    WString buf(name.c_str(), BentleyCharEncoding::Utf8);
    size_t pos = 0;
    bool wasDecoded = false;
    while (pos + 8 < buf.length() && Utf8String::npos != (pos = buf.find (L"__x", pos)))
        {
        if ('_' == buf[pos+7] && '_' == buf[pos+8])
            {
            uint32_t charCode;
            if (1 == swscanf(buf.c_str() + pos + 3, L"%x", &charCode))
                {
                buf[pos] = (WChar)charCode;
                buf.erase (pos+1, 8);
                wasDecoded = true;
                pos++;
                continue;
                }
            }

        // could not decode this escape code, leave it intact
        pos += 3;
        }

    if (wasDecoded)
        decoded.Assign(buf.c_str());

    return wasDecoded;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECNameValidation::EncodeToValidName (Utf8StringR output, Utf8StringCR input)
    {
    output.clear();
    if (input.empty())
        return false;

    // TFS#298776: The encoding has always been done using UTF-16 - first in .NET, then in Vancouver.
    // Therefore we must convert.
    // Let's avoid doing that unnecessarily, since that's the common case; at the expense of having to repeat our loop in the uncommon case

    // First character cannot be a digit
    bool needToEncode = ('0' <= input[0] && '9' >= input[0]);
    if (!needToEncode)
        {
        for (size_t i = 0; i < input.length(); i++)
            {
            if (!IsValidAlphaNumericCharacter (input[i]))
                {
                needToEncode = true;
                break;
                }
            }
        }

    if (!needToEncode)
        {
        output = input;
        return false; // => no encoding was necessary
        }

    WString name(input.c_str(), BentleyCharEncoding::Utf8);
    WString encoded;
    encoded.reserve (name.length());
    bool wasEncoded = false;

    // First character cannot be a digit
    size_t startIndex = 0;
    if ('0' <= name[0] && '9' >= name[0])
        {
        AppendEncodedCharacter (encoded, name[0]);
        startIndex = 1;
        wasEncoded = true;
        }

    for (size_t i = startIndex; i < name.length(); i++)
        {
        if (!IsValidAlphaNumericCharacter (name[i]))
            {
            AppendEncodedCharacter (encoded, name[i]);
            wasEncoded = true;
            }
        else
            encoded.append (1, name[i]);
        }

    output.Assign(encoded.c_str());
    return wasEncoded;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ECValidatedName::GetDisplayLabel() const
    {
    return m_displayLabel;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ECValidatedName::SetName (Utf8CP name)
    {
    // Note that this method can be called with an un-encoded name (e.g. called by users creating schemas dynamically),
    // or with an encoded name (e.g. when deserializing a schema from xml)
    // Hence we both encode and decode here

    // Encode the name
    m_name.clear();
    ECNameValidation::EncodeToValidName (m_name, name);

    // If the display label has not been explicitly set, use the (decoded) property name
    if (!m_hasExplicitDisplayLabel)
        ECNameValidation::DecodeFromValidName (m_displayLabel, m_name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void ECValidatedName::SetDisplayLabel (Utf8CP label)
    {
    if (NULL == label || '\0' == *label)
        {
        m_hasExplicitDisplayLabel = false;
        m_displayLabel.clear();
        ECNameValidation::DecodeFromValidName (m_displayLabel, m_name);
        }
    else
        {
        m_hasExplicitDisplayLabel = true;
        m_displayLabel = label;
        }
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchema::ECSchema ()
    :m_classContainer(m_classMap), m_isSupplemented(false), m_hasExplicitDisplayLabel(false), m_immutable(false), m_ecSchemaId(0)
    {
    //
    };

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchema::~ECSchema ()
    {
    for (auto entry : m_classMap)
        {
        ECClassP ecClass = entry.second;
        //==========================================================
        //Bug #23511: Publisher crash related to a NULL ECClass name
        //We need to cleanup any derived class link in other schema.
        //If schema fail later during loading it is possible that is
        //had created derived class links in reference ECSchemas. Since
        //This schema would be deleted we need to remove those dead links.
        for(auto baseClass : ecClass->GetBaseClasses())
            {
            if (&baseClass->GetSchema() != this)
                baseClass->RemoveDerivedClass(*ecClass);
            }
        //==========================================================
        }

    ClassMap::iterator  classIterator = m_classMap.begin();
    while (classIterator != m_classMap.end())
        {
        ECClassP ecClass = classIterator->second;
        classIterator = m_classMap.erase(classIterator);
        delete ecClass;
        }
    BeAssert (m_classMap.empty());

    m_refSchemaList.clear();

    memset ((void*)this, 0xececdead, 4);// Replaced sizeof(this) with 4. There is value 
                                        // in trying to make this logic "correct" for a 64-bit build. 
                                        // This memset is clearly just intended to aid in debugging. 
                                        // Attempting to use this object with the high 4 bytes of the vtable 
                                        // pointer set to 0xececdead will crash just as reliably.
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 03/2013
//+---------------+---------------+---------------+---------------+---------------+------
bool ECSchema::IsSystemSchema () const
    {
    return StandardCustomAttributeHelper::IsSystemSchema (*this);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan    02/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchema::IsDynamicSchema () const
    {
    return StandardCustomAttributeHelper::IsDynamicSchema (*this);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan    02/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetIsDynamicSchema (bool isDynamic)
    {
    return StandardCustomAttributeHelper::SetIsDynamicSchema (*this, isDynamic);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECSchema::SetErrorHandling (bool showMessages, bool doAssert)
    {
    s_noAssert = !doAssert;
    ECClass::SetErrorHandling(doAssert);
    SchemaXmlReader::SetErrorHandling(doAssert);
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
Utf8StringCR ECSchema::GetName () const
    {
    return m_key.m_schemaName;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetName (Utf8StringCR name)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    ECNameValidation::EncodeToValidName (m_key.m_schemaName, name);
    if (!m_hasExplicitDisplayLabel)
        ECNameValidation::DecodeFromValidName (m_displayLabel, m_key.m_schemaName);

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ECSchema::GetNamespacePrefix () const
    {
    return m_namespacePrefix;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetNamespacePrefix (Utf8StringCR namespacePrefix)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;
    m_namespacePrefix = namespacePrefix;
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ECSchema::GetDescription () const
    {
    return m_localizedStrings.GetSchemaDescription(this, m_description);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ECSchema::GetInvariantDescription() const
    {
    return m_description;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetDescription (Utf8StringCR description)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    m_description = description;
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ECSchema::GetDisplayLabel () const
    {
    return m_localizedStrings.GetSchemaDisplayLabel(this, m_displayLabel);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ECSchema::GetInvariantDisplayLabel() const
    {
    return m_displayLabel;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetDisplayLabel (Utf8StringCR displayLabel)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    m_displayLabel = displayLabel;
    m_hasExplicitDisplayLabel = true;
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchema::GetIsDisplayLabelDefined () const
    {
    return m_hasExplicitDisplayLabel;
    }

static Utf8CP s_standardSchemaNames[] =
    {
    "Bentley_Standard_CustomAttributes",
    "Bentley_Standard_Classes",
    "Bentley_ECSchemaMap",
    "EditorCustomAttributes",
    "Bentley_Common_Classes",
    "Dimension_Schema",
    "iip_mdb_customAttributes",
    "KindOfQuantity_Schema",
    "rdl_customAttributes",
    "SIUnitSystemDefaults",
    "Unit_Attributes",
    "Units_Schema",
    "USCustomaryUnitSystemDefaults",
    "ECDbMap"
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchema::IsStandardSchema(Utf8StringCR schemaName)
    {
    for (Utf8CP* cur = s_standardSchemaNames, *end = cur + _countof(s_standardSchemaNames); cur < end; ++cur)
        if (schemaName.Equals (*cur))
            return true;

    return false;
    }
/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchema::IsStandardSchema () const
    {
    return IsStandardSchema(m_key.m_schemaName);
    }

static Utf8CP s_originalStandardSchemaFullNames[] =
    {
    "Bentley_Standard_CustomAttributes.01.00",
    "Bentley_Standard_Classes.01.00",
    "EditorCustomAttributes.01.00",
    "Bentley_Common_Classes.01.00",
    "Dimension_Schema.01.00",
    "iip_mdb_customAttributes.01.00",
    "KindOfQuantity_Schema.01.00",
    "rdl_customAttributes.01.00",
    "SIUnitSystemDefaults.01.00",
    "Unit_Attributes.01.00",
    "Units_Schema.01.00",
    "USCustomaryUnitSystemDefaults.01.00"
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchema::IsSamePrimarySchema
(
ECSchemaR primarySchema
) const
    {
    if (0 != BeStringUtilities::Stricmp(this->GetNamespacePrefix().c_str(), primarySchema.GetNamespacePrefix().c_str()))
        return false;

    if (0 != BeStringUtilities::Stricmp(this->GetFullSchemaName().c_str(), primarySchema.GetFullSchemaName().c_str()))
        return false;

    return (GetClassCount() == primarySchema.GetClassCount());
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                07/2015
//+---------------+---------------+---------------+---------------+---------------+------
bool ECSchema::IsSupplementalSchema
(
) const
    {
    SupplementalSchemaMetaDataPtr suppMetaData = nullptr;
    return SupplementalSchemaMetaData::TryGetFromSchema(suppMetaData, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchema::IsSupplemented
(
) const
    {
    return m_isSupplemented;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchema::ShouldNotBeStored () const
    {
    return ShouldNotBeStored (GetSchemaKey());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchema::ShouldNotBeStored (SchemaKeyCR key)
    {
    Utf8String schemaName = key.GetFullSchemaName();
    for (Utf8CP* cur = s_originalStandardSchemaFullNames, *end = cur + _countof(s_originalStandardSchemaFullNames); cur < end; ++cur)
        if (schemaName.Equals (*cur))
            return true;

    // We don't want to import any version of the Units_Schema
    if (BeStringUtilities::Stricmp("Units_Schema", key.m_schemaName.c_str()) == 0)
        return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ECSchema::GetVersionMajor () const
    {
    return m_key.m_versionMajor;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetVersionMajor (const uint32_t versionMajor)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    m_key.m_versionMajor = versionMajor;
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ECSchema::GetVersionMinor
(
) const
    {
    return m_key.m_versionMinor;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetVersionMinor (const uint32_t versionMinor)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    m_key.m_versionMinor = versionMinor;
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ECSchema::GetClassCP (Utf8CP name) const
    {
    return const_cast<ECSchemaP> (this)->GetClassP(name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassP ECSchema::GetClassP (Utf8CP name)
    {
    ClassMap::const_iterator  classIterator;
    classIterator = m_classMap.find (name);

    if ( classIterator != m_classMap.end() )
        return classIterator->second;
    else
        return NULL;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchema::DebugDump()const
    {
    printf("ECSchema: this=0x%" PRIx64 "  %s.%02d.%02d, nClasses=%d\n", (uint64_t)this, m_key.m_schemaName.c_str(), m_key.m_versionMajor, m_key.m_versionMinor, (int) m_classMap.size());
    for (ClassMap::const_iterator it = m_classMap.begin(); it != m_classMap.end(); ++it)
        {
        bpair<Utf8CP, ECClassP>const& entry = *it;
        ECClassCP ecClass = entry.second;
        printf("    ECClass: 0x%" PRIx64 ", %s\n", (uint64_t)ecClass, ecClass->GetName().c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::DeleteClass (ECClassR ecClass)
    {
    ClassMap::iterator iter = m_classMap.find (ecClass.GetName().c_str());
    if (iter == m_classMap.end() || iter->second != &ecClass)
        return ECObjectsStatus::ClassNotFound;

    m_classMap.erase (iter);
    delete &ecClass;
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::RenameClass (ECClassR ecClass, Utf8CP newName)
    {
    ClassMap::iterator iter = m_classMap.find (ecClass.GetName().c_str());
    if (iter == m_classMap.end() || iter->second != &ecClass)
        return ECObjectsStatus::ClassNotFound;

    ECClassP pClass = &ecClass;
    m_classMap.erase (iter);
    ECObjectsStatus renameStatus = ecClass.SetName (newName);
    ECObjectsStatus addStatus = AddClass (pClass, false);
    return ECObjectsStatus::Success != renameStatus ? renameStatus : addStatus;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::AddClass (ECClassP pClass, bool deleteClassIfDuplicate)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    bpair <ClassMap::iterator, bool> resultPair;
    resultPair = m_classMap.insert (bpair<Utf8CP, ECClassP> (pClass->GetName().c_str(), pClass));
    if (resultPair.second == false)
        {
        LOG.warningv ("Cannot create class '%s' because it already exists in the schema", pClass->GetName().c_str());
        if (deleteClassIfDuplicate)
            {
            // preserving weird existing behavior, added option to not do this...
            delete pClass;
            pClass = NULL;
            }

        return ECObjectsStatus::NamedItemAlreadyExists;
        }
    //DebugDump(); wprintf(L"\n");
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::CreateEntityClass (ECEntityClassP& pClass, Utf8StringCR name)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    pClass = new ECEntityClass(*this);
    ECObjectsStatus status = pClass->SetName (name);
    if (ECObjectsStatus::Success != status)
        {
        delete pClass;
        pClass = NULL;
        return status;
        }

    return AddClass (pClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECSchema::CreateStructClass (ECStructClassP& pClass, Utf8StringCR name)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    pClass = new ECStructClass(*this);
    ECObjectsStatus status = pClass->SetName (name);
    if (ECObjectsStatus::Success != status)
        {
        delete pClass;
        pClass = NULL;
        return status;
        }

    return AddClass (pClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECSchema::CreateCustomAttributeClass (ECCustomAttributeClassP& pClass, Utf8StringCR name)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    pClass = new ECCustomAttributeClass(*this);
    ECObjectsStatus status = pClass->SetName (name);
    if (ECObjectsStatus::Success != status)
        {
        delete pClass;
        pClass = NULL;
        return status;
        }

    return AddClass (pClass);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::CopyClass
(
ECClassP& targetClass,
ECClassCR sourceClass
)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    // first make sure the class doesn't already exist in the schema
    if (NULL != this->GetClassCP(sourceClass.GetName().c_str()))
        return ECObjectsStatus::NamedItemAlreadyExists;

    ECObjectsStatus status = ECObjectsStatus::Success;
    ECRelationshipClassCP sourceAsRelationshipClass = sourceClass.GetRelationshipClassCP();
    ECStructClassCP sourceAsStructClass = sourceClass.GetStructClassCP();
    ECCustomAttributeClassCP sourceAsCAClass = sourceClass.GetCustomAttributeClassCP();
    if (NULL != sourceAsRelationshipClass)
        {
        ECRelationshipClassP newRelationshipClass;
        status = this->CreateRelationshipClass(newRelationshipClass, sourceClass.GetName());
        if (ECObjectsStatus::Success != status)
            return status;
        newRelationshipClass->SetStrength(sourceAsRelationshipClass->GetStrength());
        newRelationshipClass->SetStrengthDirection(sourceAsRelationshipClass->GetStrengthDirection());

        sourceAsRelationshipClass->GetSource().CopyTo(newRelationshipClass->GetSource());
        sourceAsRelationshipClass->GetTarget().CopyTo(newRelationshipClass->GetTarget());
        targetClass = newRelationshipClass;
        }
    else if (nullptr != sourceAsStructClass)
        {
        ECStructClassP newStructClass;
        status = this->CreateStructClass(newStructClass, sourceClass.GetName());
        if (ECObjectsStatus::Success != status)
            return status;
        // ECX_WIP: Set struct class properties
        targetClass = newStructClass;
        }
    else if (nullptr != sourceAsCAClass)
        {
        ECCustomAttributeClassP newCAClass;
        status = this->CreateCustomAttributeClass(newCAClass, sourceClass.GetName());
        if (ECObjectsStatus::Success != status)
            return status;
        // ECX_WIP: Set CA class properties
        targetClass = newCAClass;
        }
    else
        {
        ECEntityClassP newEntityClass;
        status = CreateEntityClass(newEntityClass, sourceClass.GetName());
        if (ECObjectsStatus::Success != status)
            return status;
        targetClass = newEntityClass;
        }

    if (sourceClass.GetIsDisplayLabelDefined())
        targetClass->SetDisplayLabel(sourceClass.GetInvariantDisplayLabel());
    targetClass->SetDescription(sourceClass.GetInvariantDescription());
    targetClass->SetClassModifier(sourceClass.GetClassModifier());

    // Set the base classes on the target class from the source class
    // This is inconsistent with the Managed implementation of CopyClass which does not copy base classes
    for (ECClassP baseClass: sourceClass.GetBaseClasses())
        {
        targetClass->AddBaseClass(*baseClass);
        }

    for(ECPropertyCP sourceProperty: sourceClass.GetProperties(false))
        {
        if (sourceProperty->IsForSupplementation())
            continue;
        ECPropertyP destProperty;
        status = targetClass->CopyProperty(destProperty, sourceProperty, true);
        if (ECObjectsStatus::Success != status)
            return status;
        }

    return sourceClass.CopyCustomAttributesTo(*targetClass);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::CreateRelationshipClass (ECRelationshipClassP& pClass, Utf8StringCR name)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    pClass = new ECRelationshipClass(*this);
    ECObjectsStatus status = pClass->SetName (name);
    if (ECObjectsStatus::Success != status)
        {
        delete pClass;
        pClass = NULL;
        return status;
        }

    bpair < ClassMap::iterator, bool > resultPair;
    resultPair = m_classMap.insert (bpair<Utf8CP, ECClassP> (pClass->GetName().c_str(), pClass));
    if (resultPair.second == false)
        {
        delete pClass;
        pClass = NULL;
        LOG.warningv (L"Cannot create relationship class '%ls' because it already exists in the schema", name.c_str());
        return ECObjectsStatus::NamedItemAlreadyExists;
        }

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECSchema::GetFullSchemaName () const
    {
    Utf8Char fullName[1024]; // we decided to use a large buffer instead of calculating the length and using _alloc to boost performance
    BeStringUtilities::Snprintf(fullName, "%s.%02d.%02d", GetName().c_str(), GetVersionMajor(), GetVersionMinor());
    return fullName;
    }

#define     ECSCHEMA_FULLNAME_FORMAT_EXPLANATION " Format must be Name.MM.mm where Name is the schema name, MM is major version and mm is minor version."
#define     ECSCHEMA_FULLNAME_FORMAT_EXPLANATION_W L" Format must be Name.MM.mm where Name is the schema name, MM is major version and mm is minor version."
/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::ParseSchemaFullName (Utf8StringR schemaName, uint32_t& versionMajor, uint32_t& versionMinor, Utf8StringCR  fullName)
    {
    if (fullName.empty())
        return ECObjectsStatus::ParseError;

    Utf8CP fullNameCP = fullName.c_str();
    Utf8CP firstDot = strchr (fullNameCP, '.');
    if (NULL == firstDot)
        {
        LOG.errorv ("Invalid ECSchema FullName String: '%s' does not contain a '.'!" ECSCHEMA_FULLNAME_FORMAT_EXPLANATION, fullName.c_str());
        return ECObjectsStatus::ParseError;
        }

    size_t nameLen = firstDot - fullNameCP;
    if (nameLen < 1)
        {
        LOG.errorv ("Invalid ECSchema FullName String: '%s' does not have any characters before the '.'!" ECSCHEMA_FULLNAME_FORMAT_EXPLANATION, fullName.c_str());
        return ECObjectsStatus::ParseError;
        }

    schemaName.assign (fullNameCP, nameLen);

    return ParseVersionString (versionMajor, versionMinor, firstDot+1);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::ParseSchemaFullName (Utf8StringR schemaName, uint32_t& versionMajor, uint32_t& versionMinor, Utf8CP fullName)
    {
    if (NULL == fullName || '\0' == *fullName)
        return ECObjectsStatus::ParseError;

    Utf8CP firstDot = strchr (fullName, '.');
    if (NULL == firstDot)
        {
        LOG.errorv ("Invalid ECSchema FullName String: '%s' does not contain a '.'!" ECSCHEMA_FULLNAME_FORMAT_EXPLANATION, fullName);
        return ECObjectsStatus::ParseError;
        }

    size_t nameLen = firstDot - fullName;
    if (nameLen < 1)
        {
        LOG.errorv ("Invalid ECSchema FullName String: '%s' does not have any characters before the '.'!" ECSCHEMA_FULLNAME_FORMAT_EXPLANATION, fullName);
        return ECObjectsStatus::ParseError;
        }

    schemaName.assign (fullName, nameLen);

    return ParseVersionString (versionMajor, versionMinor, firstDot+1);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECSchema::FormatSchemaVersion (uint32_t versionMajor, uint32_t versionMinor)
    {
    Utf8Char versionString[80];
    BeStringUtilities::Snprintf (versionString, "%02d.%02d", versionMajor, versionMinor);
    return versionString;
    }

#define     ECSCHEMA_VERSION_FORMAT_EXPLANATION " Format must be MM.mm where MM is major version and mm is minor version."
/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::ParseVersionString (uint32_t& versionMajor, uint32_t& versionMinor, Utf8CP versionString)
    {
    versionMajor = DEFAULT_VERSION_MAJOR;
    versionMinor = DEFAULT_VERSION_MINOR;
    if (NULL == versionString || '\0' == *versionString)
        return ECObjectsStatus::Success;

    Utf8CP theDot = strchr (versionString, L'.');
    if (NULL == theDot)
        {
        LOG.errorv ("Invalid ECSchema Version String: '%s' does not contain a '.'!" ECSCHEMA_VERSION_FORMAT_EXPLANATION, versionString);
        return ECObjectsStatus::ParseError;
        }

    size_t majorLen = theDot - versionString;
    if (majorLen < 1 || majorLen > 3)
        {
        LOG.errorv ("Invalid ECSchema Version String: '%s' does not have 1-3 numbers before the '.'!" ECSCHEMA_VERSION_FORMAT_EXPLANATION, versionString);
        return ECObjectsStatus::ParseError;
        }

    Utf8CP endDot = strchr (theDot+1, L'.');
    size_t minorLen = (NULL != endDot) ? (endDot - theDot) - 1 : strlen (theDot) - 1;
    if (minorLen < 1 || minorLen > 3)
        {
        LOG.errorv ("Invalid ECSchema Version String: '%s' does not have 1-3 numbers after the '.'!" ECSCHEMA_VERSION_FORMAT_EXPLANATION, versionString);
        return ECObjectsStatus::ParseError;
        }

    Utf8P end = NULL;
    uint32_t  localMajor = strtoul (versionString, &end, 10);
    if (versionString == end)
        {
        LOG.errorv ("Invalid ECSchema Version String: '%s' The characters before the '.' must be numeric!" ECSCHEMA_VERSION_FORMAT_EXPLANATION, versionString);
        return ECObjectsStatus::ParseError;
        }
    else
        {
        versionMajor = localMajor;
        }

    uint32_t localMinor = strtoul (&theDot[1], &end, 10);
    if (&theDot[1] == end)
        {
        LOG.errorv ("Invalid ECSchema Version String: '%s' The characters after the '.' must be numeric!" ECSCHEMA_VERSION_FORMAT_EXPLANATION, versionString);
        return ECObjectsStatus::ParseError;
        }
    else
        {
        versionMinor = localMinor;
        }

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetVersionFromString (Utf8CP versionString)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    uint32_t versionMajor;
    uint32_t versionMinor;
    ECObjectsStatus status;
    if ((ECObjectsStatus::Success != (status = ParseVersionString (versionMajor, versionMinor, versionString))) ||
        (ECObjectsStatus::Success != (status = this->SetVersionMajor (versionMajor))) ||
        (ECObjectsStatus::Success != (status = this->SetVersionMinor (versionMinor))))
        return status;
    else
        return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::CreateSchema (ECSchemaPtr& schemaOut, Utf8StringCR schemaName, uint32_t versionMajor, uint32_t versionMinor)
    {
    schemaOut = new ECSchema();

    ECObjectsStatus status;

    if (ECObjectsStatus::Success != (status = schemaOut->SetName (schemaName)) ||
        ECObjectsStatus::Success != (status = schemaOut->SetVersionMajor (versionMajor)) ||
        ECObjectsStatus::Success != (status = schemaOut->SetVersionMinor (versionMinor)))
        {
        schemaOut = NULL;
        return status;
        }

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::CopySchema
(
ECSchemaPtr& schemaOut
) const
    {
    ECObjectsStatus status = ECObjectsStatus::Success;
    status = CreateSchema(schemaOut,  GetName(), GetVersionMajor(), GetVersionMinor());
    if (ECObjectsStatus::Success != status)
        return status;

    schemaOut->SetDescription(m_description);
    if (GetIsDisplayLabelDefined())
        schemaOut->SetDisplayLabel(GetInvariantDisplayLabel());

    ECSchemaReferenceListCR referencedSchemas = GetReferencedSchemas();
    for (ECSchemaReferenceList::const_iterator iter = referencedSchemas.begin(); iter != referencedSchemas.end(); ++iter)
        schemaOut->AddReferencedSchema(*iter->second.get());
        
    for(ECClassP ecClass: m_classContainer)
        {
        ECClassP copyClass;
        status = schemaOut->CopyClass(copyClass, *ecClass);
        if (ECObjectsStatus::Success != status && ECObjectsStatus::NamedItemAlreadyExists != status)
            return status;
        }

    return CopyCustomAttributesTo(*schemaOut);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECSchema::GetSchemaByNamespacePrefixP (Utf8StringCR namespacePrefix) const
    {
    if (namespacePrefix.length() == 0)
        return this;

    // lookup referenced schema by prefix
    bmap<ECSchemaP, Utf8String>::const_iterator schemaIterator;
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
ECObjectsStatus ECSchema::ResolveNamespacePrefix (ECSchemaCR schema, Utf8StringR namespacePrefix) const
    {
    namespacePrefix = EMPTY_STRING;
    if (&schema == this)
        return ECObjectsStatus::Success;

    bmap<ECSchemaP, Utf8String>::const_iterator schemaIterator = m_referencedSchemaNamespaceMap.find((ECSchemaP) &schema);
    if (schemaIterator != m_referencedSchemaNamespaceMap.end())
        {
        namespacePrefix = schemaIterator->second;
        return ECObjectsStatus::Success;
        }

    return ECObjectsStatus::SchemaNotFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassContainerCR ECSchema::GetClasses () const
    {
    return m_classContainer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ECSchema::GetClassCount
(
) const
    {
    return (uint32_t) m_classMap.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaReferenceListCR ECSchema::GetReferencedSchemas () const
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
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::AddReferencedSchema (ECSchemaR refSchema, Utf8StringCR namespacePrefix)
    {
    ECSchemaReadContext context (NULL, false);
    return AddReferencedSchema(refSchema, namespacePrefix, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::AddReferencedSchema (ECSchemaR refSchema, Utf8StringCR namespacePrefix, ECSchemaReadContextR readContext)
    {
    SchemaKeyCR refSchemaKey = refSchema.GetSchemaKey();
    if (m_refSchemaList.end () != m_refSchemaList.find (refSchemaKey))
        return ECObjectsStatus::NamedItemAlreadyExists;

    Utf8String prefix(namespacePrefix);
    if (prefix.length() == 0)
        prefix = "s";

    // Make sure prefix is unique within this schema
    bmap<ECSchemaP, Utf8String>::const_iterator namespaceIterator;
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
            Utf8Char temp[256];
            BeStringUtilities::Snprintf(temp, "%s%d", prefix.c_str(), subScript);
            Utf8String tryPrefix(temp);
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

    m_refSchemaList[refSchemaKey] = &refSchema;
    // Check for recursion
    if (AddingSchemaCausedCycles ())
        {
        m_refSchemaList.erase (refSchemaKey);
        return ECObjectsStatus::SchemaHasReferenceCycle;
        }

    m_referencedSchemaNamespaceMap.insert(bpair<ECSchemaP, const Utf8String> (&refSchema, prefix));
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::RemoveReferencedSchema (ECSchemaR refSchema)
    {
    ECSchemaReferenceList::iterator schemaIterator = m_refSchemaList.find (refSchema.GetSchemaKey());
    if (schemaIterator == m_refSchemaList.end())
        return ECObjectsStatus::SchemaNotFound;

    // Can only remove the reference if nothing actually references it.

    ECSchemaPtr foundSchema = schemaIterator->second;
    for (ECClassP ecClass: GetClasses())
        {
        // First, check each base class to see if the base class uses that schema
        for (ECClassP baseClass: ecClass->GetBaseClasses())
            {
            if ((ECSchemaP) &(baseClass->GetSchema()) == foundSchema.get())
                {
                return ECObjectsStatus::SchemaInUse;
                }
            }

        // If it is a relationship class, check the constraints to make sure the constraints don't use that schema
        ECRelationshipClassP relClass = ecClass->GetRelationshipClassP();
        if (NULL != relClass)
            {
            for (auto target : relClass->GetTarget().GetConstraintClasses())
                {
                if ((ECSchemaP) &(target->GetClass().GetSchema()) == foundSchema.get())
                    {
                    return ECObjectsStatus::SchemaInUse;
                    }
                }
            for (auto source : relClass->GetSource().GetConstraintClasses())
                {
                if ((ECSchemaP) &(source->GetClass().GetSchema()) == foundSchema.get())
                    {
                    return ECObjectsStatus::SchemaInUse;
                    }
                }
            }

        // And make sure that there are no struct types from another schema
        for (ECPropertyP prop: ecClass->GetProperties(false))
            {
            ECClassCP typeClass;
            if (prop->GetIsStruct())
                {
                typeClass = &(prop->GetAsStructProperty()->GetType());
                }
            else if (prop->GetIsStructArray())
                {
                typeClass = prop->GetAsStructArrayProperty()->GetStructElementType();
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
            return ECObjectsStatus::SchemaInUse;
            }
        }

    m_refSchemaList.erase(schemaIterator);
    bmap<ECSchemaP, Utf8String>::iterator iterator = m_referencedSchemaNamespaceMap.find(&refSchema);
    if (iterator != m_referencedSchemaNamespaceMap.end())
        m_referencedSchemaNamespaceMap.erase(iterator);

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchema::SetIsSupplemented
(
bool isSupplemented
)
    {
    m_isSupplemented = isSupplemented;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchema::SetSupplementalSchemaInfo(SupplementalSchemaInfo* info)
    {
    m_supplementalSchemaInfo = info;
    if (NULL == info)
        this->RemoveCustomAttribute(SupplementalSchemaInfo::GetCustomAttributeAccessor());
    else
        {
        IECInstancePtr attribute = info->CreateCustomAttribute();
        if (attribute.IsValid())
            {
            this->SetSupplementedCustomAttribute(*attribute);
            auto& bsca = attribute->GetClass().GetSchema();
            if (!ECSchema::IsSchemaReferenced(*this, bsca ))
                {
                this->AddReferencedSchema(const_cast<ECSchemaR>(bsca));
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SupplementalSchemaInfoPtr const ECSchema::GetSupplementalInfo() const
    {
    return m_supplementalSchemaInfo;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr IECSchemaLocater::LocateSchema(SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR schemaContext)
    {
    return _LocateSchema (key, matchType, schemaContext);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr     ECSchema::LocateSchema (SchemaKeyR key, ECSchemaReadContextR schemaContext)
    {
    return schemaContext.LocateSchema(key, SCHEMAMATCHTYPE_LatestCompatible);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus GetMinorVersionFromSchemaFileName (uint32_t& versionMinor, WCharCP filePath)
    {
    BeFileName  fileName (filePath);

    WString     name;
    fileName.ParseName (NULL, NULL, &name, NULL);

    // after fileName.parse, name contains "SchemaName.XX.XX.eschema".
    WString::size_type firstDot;
    if (WString::npos == (firstDot = name.find ('.')))
        {
        BeAssert (s_noAssert);
        LOG.errorv (L"Invalid ECSchema FileName String: '%ls' does not contain the suffix '.ecschema.xml'!" ECSCHEMA_FULLNAME_FORMAT_EXPLANATION_W, filePath);
        return ECObjectsStatus::ParseError;
        }

    Utf8String     versionString;
    versionString.Assign(name.substr (firstDot+1).c_str());
    uint32_t    versionMajor;
    return ECSchema::ParseVersionString (versionMajor, versionMinor, versionString.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus GetSchemaFileName (WString& fullFileName, uint32_t& foundMinorVersion, WCharCP schemaPath, bool useLatestCompatibleMatch)
    {
    WString     schemaPathWithWildcard = schemaPath;
    schemaPathWithWildcard += L"*";

    BeFileListIterator  fileList (schemaPathWithWildcard.c_str(), false);
    BeFileName          filePath;
    uint32_t currentMinorVersion=0;

    while (SUCCESS == fileList.GetNextFileName (filePath))
        {
        WCharCP     fileName = filePath.GetName();

        if (!useLatestCompatibleMatch)
            {
            fullFileName = fileName;
            return ECObjectsStatus::Success;
            }

        if (fullFileName.empty())
            {
            fullFileName = fileName;
            GetMinorVersionFromSchemaFileName (foundMinorVersion, fileName);
            continue;
            }

        if (ECObjectsStatus::Success != GetMinorVersionFromSchemaFileName (currentMinorVersion, fileName))
            continue;

        if (currentMinorVersion > foundMinorVersion)
            {
            foundMinorVersion = currentMinorVersion;
            fullFileName = fileName;
            }
        }

    if (fullFileName.empty())
        return ECObjectsStatus::Error;

    return ECObjectsStatus::Success;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr       SearchPathSchemaFileLocater::FindMatchingSchema
(
WStringCR                       schemaMatchExpression,
SchemaKeyR                      key,
ECSchemaReadContextR            schemaContext,
SchemaMatchType                 matchType,
bvector<WString>&               searchPaths
)
    {
    WString fullFileName;
    for (WString schemaPathStr: searchPaths)
        {
        BeFileName schemaPath (schemaPathStr.c_str());
        schemaPath.AppendToPath (schemaMatchExpression.c_str());
        LOG.debugv (L"Checking for existence of %ls...", schemaPath.GetName());

        //Finds latest
        SchemaKey foundKey(key);
        if (ECObjectsStatus::Success != GetSchemaFileName (fullFileName, foundKey.m_versionMinor, schemaPath,  matchType == SCHEMAMATCHTYPE_LatestCompatible))
            continue;

        ECSchemaPtr schemaOut = NULL;
        //Check if schema is compatible before reading, as reading it would add the schema to the cache.
        if (!foundKey.Matches(key, matchType))
            {
            if (schemaContext.m_acceptLegacyImperfectLatestCompatibleMatch && matchType == SCHEMAMATCHTYPE_LatestCompatible &&
                0 == foundKey.m_schemaName.CompareTo(key.m_schemaName) && foundKey.m_versionMajor == key.m_versionMajor)
                {
                // See if this imperfect match ECSchema has is already cached (so we can avoid loading it, below)

                //We found a different key;
                if (matchType != SCHEMAMATCHTYPE_Exact)
                    schemaOut = schemaContext.GetFoundSchema(foundKey, SCHEMAMATCHTYPE_Exact);

                if (schemaOut.IsValid())
                    {
                    key.m_versionMinor = foundKey.m_versionMinor;
                    return schemaOut;
                    }
                LOG.warningv (L"Located %ls, which does not meet 'latest compatible' criteria to match %ls, but is being accepted because some legacy schemas are known to require this",
                                                  fullFileName.c_str(), key.GetFullSchemaName().c_str());
                }
            else
                {
                LOG.warningv (L"Located %ls, but it does not meet 'latest compatible' criteria to match %ls.%02d.%02d. Caller can use acceptImperfectLegacyMatch to cause it to be accepted.",
                                                  fullFileName.c_str(), key.m_schemaName.c_str(),   key.m_versionMajor,   key.m_versionMinor);
                continue;
                }
            }

        if (SchemaReadStatus::Success != ECSchema::ReadFromXmlFile (schemaOut, fullFileName.c_str(), schemaContext))
            continue;

        LOG.debugv (L"Located %ls...", fullFileName.c_str());
        // Now check this same path for supplemental schemas
        bvector<ECSchemaP> supplementalSchemas;
        TryLoadingSupplementalSchemas(key.m_schemaName.c_str(), schemaPathStr, schemaContext, supplementalSchemas);
        
        // Check for localization supplementals
        for(WString culture : *(schemaContext.GetCultures()))
            {
            if(culture.Equals(L"en")) // not sure
                continue;
            BeFileName locDir(schemaPathStr.c_str());
            locDir.AppendToPath(culture.c_str());
            TryLoadingSupplementalSchemas(key.m_schemaName.c_str(), locDir, schemaContext, supplementalSchemas);
            }
        if (supplementalSchemas.size() > 0)
            {
            ECN::SupplementedSchemaBuilder builder;
            builder.UpdateSchema(*schemaOut, supplementalSchemas);
            }

        return schemaOut;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool SearchPathSchemaFileLocater::TryLoadingSupplementalSchemas
(
Utf8StringCR schemaName, 
WStringCR schemaFilePath, 
ECSchemaReadContextR schemaContext,
bvector<ECSchemaP>& supplementalSchemas
)
    {
    BeFileName schemaPath (schemaFilePath.c_str());
    WString filter;
    filter.AssignUtf8(schemaName.c_str());
    filter += L"_Supplemental_*.*.*.ecschema.xml";
    schemaPath.AppendToPath(filter.c_str());
    BeFileListIterator fileList(schemaPath.GetName(), false);
    BeFileName filePath;
    while (SUCCESS == fileList.GetNextFileName (filePath))
        {
        WCharCP     fileName = filePath.GetName();
        ECSchemaPtr schemaOut = NULL;

        if (SchemaReadStatus::Success != ECSchema::ReadFromXmlFile (schemaOut, fileName, schemaContext))
            continue;
        supplementalSchemas.push_back(schemaOut.get());
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr     SearchPathSchemaFileLocater::LocateSchemaByPath (SchemaKeyR key, ECSchemaReadContextR schemaContext, SchemaMatchType matchType, bvector<WString>& searchPaths)
    {
    wchar_t versionString[24];

    if (matchType == SCHEMAMATCHTYPE_Latest)
        BeStringUtilities::Snwprintf(versionString, L".*.*.ecschema.xml");
    else if (matchType == SCHEMAMATCHTYPE_LatestCompatible)
        BeStringUtilities::Snwprintf(versionString, L".%02d.*.ecschema.xml", key.m_versionMajor);
    else
        BeStringUtilities::Snwprintf(versionString, L".%02d.%02d.ecschema.xml", key.m_versionMajor, key.m_versionMinor);

    WString schemaMatchExpression;
    schemaMatchExpression.AssignUtf8(key.m_schemaName.c_str());
    schemaMatchExpression += versionString;
    WString fullFileName;

    ECSchemaPtr   schemaOut = FindMatchingSchema (schemaMatchExpression, key, schemaContext, matchType, searchPaths);
    return schemaOut;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SearchPathSchemaFileLocater::SearchPathSchemaFileLocater (bvector<WString> const& searchPaths) : m_searchPaths(searchPaths) {};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SearchPathSchemaFileLocater::~SearchPathSchemaFileLocater () {};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
SearchPathSchemaFileLocaterPtr SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(bvector<WString> const& searchPaths)
    {
    return new SearchPathSchemaFileLocater(searchPaths);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr SearchPathSchemaFileLocater::_LocateSchema(SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR schemaContext)
    {
    return LocateSchemaByPath (key, schemaContext, matchType, m_searchPaths);
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

    LOG.errorv (errorString.c_str());
    LOG.errorv (L"line %d, position %d", line, linePos);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void AddFilePathToSchemaPaths  (ECSchemaReadContextR schemaContext, WCharCP ecSchemaXmlFile)
    {
    BeFileName pathToThisSchema (BeFileName::DevAndDir, ecSchemaXmlFile);
    schemaContext.AddSchemaPath(pathToThisSchema);
    }

/*---------------------------------------------------------------------------------**//**
I initially considered adler-32 for its speed but for small schemas it will not work well.
So we are using crc-32
http://tools.ietf.org/html/rfc3309

The crc 32 function should be moved to use the boost version once the run time check failure #1
has been addressed by boost. This is the version used by the zip library
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static const uint32_t crc_table[256] = {
      0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
      0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
      0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
      0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
      0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
      0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
      0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
      0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
      0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
      0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
      0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
      0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
      0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
      0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
      0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
      0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
      0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
      0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
      0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
      0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
      0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
      0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
      0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
      0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
      0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
      0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
      0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
      0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
      0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
      0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
      0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
      0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
      0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
      0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
      0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
      0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
      0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
      0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
      0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
      0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
      0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
      0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
      0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
      0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
      0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
      0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
      0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
      0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
      0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
      0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
      0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
      0x2d02ef8dL
    };
struct          CheckSumHelper
    {
    #define CRC32(c, b) (crc_table[((int)(c) ^ (b)) & 0xff] ^ ((c) >> 8))
    #define DO1(buf)  crc = CRC32(crc, *buf++)
    #define DO2(buf)  DO1(buf); DO1(buf)
    #define DO4(buf)  DO2(buf); DO2(buf)
    #define DO8(buf)  DO4(buf); DO4(buf)

    static uint32_t crc32(uint32_t crc, const Byte* buf, size_t len)
    { if (buf==NULL) return 0L;
      crc = crc ^ 0xffffffffL;
      while (len >= 8) {DO8(buf); len -= 8;}
      if (len) do {DO1(buf);} while (--len);
      return crc ^ 0xffffffffL;  // (instead of ~c for 64-bit machines)
    }


    static const int BUFFER_SIZE = 1024;
    public:
        static uint32_t ComputeCheckSumForString (Utf8CP string, size_t bufferSize);
        static uint32_t ComputeCheckSumForString (WCharCP string, size_t bufferSize);
        static uint32_t ComputeCheckSumForFile (WCharCP schemaFile);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Krischan.Eberle                  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t CheckSumHelper::ComputeCheckSumForString (Utf8CP string, size_t bufferSize)
    {
    return crc32 (0, (Byte*) string, bufferSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t CheckSumHelper::ComputeCheckSumForString (WCharCP string, size_t bufferSize)
    {
    return crc32 (0, (Byte*) string, bufferSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t        CheckSumHelper::ComputeCheckSumForFile (WCharCP schemaFile)
    {
    uint32_t checkSum = 0;
    BeFile file;
    if (BeFileStatus::Success != file.Open (schemaFile, BeFileAccess::Read))
        {
        BeAssert(false);
        return checkSum;
        }

    Byte buffer [BUFFER_SIZE];
    do
        {
        memset(buffer, 0, BUFFER_SIZE * sizeof(Byte));
        uint32_t bytesRead = 0;
        file.Read(buffer, &bytesRead, BUFFER_SIZE);
        if (bytesRead == 0)
            break;

        checkSum = crc32 (checkSum, buffer, bytesRead);
        } while(true);

    return checkSum;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECSchema::ReadFromXmlFile (ECSchemaPtr& schemaOut, WCharCP ecSchemaXmlFile, ECSchemaReadContextR schemaContext)
    {
    StopWatch timer(L"", true);
    LOG.debugv (L"About to read native ECSchema from file: fileName='%ls'", ecSchemaXmlFile);
    schemaOut = NULL;

    SchemaReadStatus status = SchemaReadStatus::Success;

    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromFile (xmlStatus, ecSchemaXmlFile);
    if ((xmlStatus != BEXML_Success) || !xmlDom.IsValid())
        {
        BeAssert (s_noAssert);
        LogXmlLoadError (xmlDom.get());
        return SchemaReadStatus::FailedToParseXml;
        }

    AddFilePathToSchemaPaths(schemaContext, ecSchemaXmlFile);
    uint32_t checkSum = CheckSumHelper::ComputeCheckSumForFile(ecSchemaXmlFile);

    SchemaXmlReader reader(schemaContext, *xmlDom.get());
    status = reader.Deserialize(schemaOut, checkSum);
    if (SchemaReadStatus::DuplicateSchema == status)
        return status; // already logged

    if (SchemaReadStatus::Success != status)
        LOG.errorv (L"Failed to read XML file: %ls", ecSchemaXmlFile);
    else
        {
        //We have serialized a schema and its valid. Add its checksum
        timer.Stop();
        LOG.infov (L"Read (in %.4f seconds) [%3d ECClasses] %ls", timer.GetElapsedSeconds(), schemaOut->m_classMap.size(), ecSchemaXmlFile);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus     ECSchema::ReadFromXmlString
(
ECSchemaPtr&         schemaOut,
Utf8CP               ecSchemaXml,
ECSchemaReadContextR schemaContext
)
    {
    StopWatch timer(L"", true);
    LOG.debugv (L"About to read native ECSchema read from string."); // mainly included for timing
    schemaOut = NULL;
    SchemaReadStatus status = SchemaReadStatus::Success;

    size_t stringByteCount = strlen (ecSchemaXml) * sizeof(Utf8Char);

    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromString (xmlStatus, ecSchemaXml, stringByteCount);

    if (BEXML_Success != xmlStatus)
        {
        BeAssert (s_noAssert);
        LogXmlLoadError (xmlDom.get());
        return SchemaReadStatus::FailedToParseXml;
        }

    uint32_t checkSum = CheckSumHelper::ComputeCheckSumForString (ecSchemaXml, stringByteCount);
    SchemaXmlReader reader(schemaContext, *xmlDom.get());
    status = reader.Deserialize(schemaOut, checkSum);
    if (SchemaReadStatus::DuplicateSchema == status)
        return status; // already logged

    if (SchemaReadStatus::Success != status)
        {
        Utf8Char first200Bytes[201];

        BeStringUtilities::Strncpy (first200Bytes, ecSchemaXml, 200);
        first200Bytes[200] = '\0';
        LOG.errorv ("Failed to read XML from string (1st 200 characters approx.): %s", first200Bytes);
        }
    else
        {
        timer.Stop();
        LOG.infov (L"Read from string (in %.4f seconds) [%3d ECClasses] %ls", timer.GetElapsedSeconds(),
            schemaOut->m_classMap.size(), schemaOut->GetFullSchemaName().c_str());
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus     ECSchema::ReadFromXmlString
(
ECSchemaPtr&         schemaOut,
WCharCP              ecSchemaXml,
ECSchemaReadContextR schemaContext
)
    {
    StopWatch timer(L"", true);
    LOG.debugv (L"About to read native ECSchema read from string."); // mainly included for timing
    schemaOut = NULL;
    SchemaReadStatus status = SchemaReadStatus::Success;

    BeXmlStatus xmlStatus;
    size_t stringSize = wcslen (ecSchemaXml) * sizeof(WChar);
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromString (xmlStatus, ecSchemaXml, stringSize / sizeof (WChar));

    if (BEXML_Success != xmlStatus)
        {
        BeAssert (s_noAssert);
        LogXmlLoadError (xmlDom.get());
        return SchemaReadStatus::FailedToParseXml;
        }

    uint32_t checkSum = CheckSumHelper::ComputeCheckSumForString(ecSchemaXml, stringSize);
    SchemaXmlReader reader(schemaContext, *xmlDom.get());
    status = reader.Deserialize(schemaOut, checkSum);
    if (SchemaReadStatus::DuplicateSchema == status)
        return status; // already logged

    if (SchemaReadStatus::Success != status)
        {
        WChar first200Characters[201];
        wcsncpy (first200Characters, ecSchemaXml, 200);
        first200Characters[200] = L'\0';
        LOG.errorv (L"Failed to read XML from string (1st 200 characters): %ls", first200Characters);
        }
    else
        {
        timer.Stop();
        LOG.infov (L"Read from string (in %.4f seconds) [%3d ECClasses] %ls", timer.GetElapsedSeconds(),
            schemaOut->m_classMap.size(), schemaOut->GetFullSchemaName().c_str());
        }

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
    ECSchemaReferenceListCR referencedSchemas = thisSchema.GetReferencedSchemas();
    return referencedSchemas.end() != referencedSchemas.find (potentiallyReferencedSchema.GetSchemaKey());
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
    SchemaReadStatus status = SchemaReadStatus::Success;

    MSXML2::IXMLDOMDocument2Ptr xmlDocPtr = NULL;
    VERIFY_HRESULT_OK(xmlDocPtr.CreateInstance(__uuidof(MSXML2::DOMDocument60)), SCHEMA_READ_STATUS_FailedToInitializeMsmxl);
    xmlDocPtr->put_validateOnParse(VARIANT_TRUE);
    xmlDocPtr->put_async(VARIANT_FALSE);

    VARIANT_BOOL returnCode = xmlDocPtr->load(ecSchemaXmlStream);
    if (returnCode != VARIANT_TRUE)
        {
        LogXmlLoadError (xmlDom.get());
        return SchemaReadStatus::FailedToParseXml;
        }

    status = ReadXml (schemaOut, xmlDocPtr, schemaContext);
    if (SchemaReadStatus::DuplicateSchema == status)
        return status; // already logged

    if (ECObjectsStatus::Success != status)
        LOG.errorv (L"Failed to read XML from stream");
    return status;
    return SchemaReadStatus::FailedToParseXml;
    }
#endif //defined (NEEDSWORK_LIBXML)

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECSchema::WriteToXmlString (WStringR ecSchemaXml) const
    {
    ecSchemaXml.clear();

    BeXmlWriterPtr xmlWriter = BeXmlWriter::Create();

    SchemaWriteStatus status;
    SchemaXmlWriter schemaWriter(*xmlWriter.get(), *this);
    if (SchemaWriteStatus::Success != (status = schemaWriter.Serialize()))
        return status;

    xmlWriter->ToString (ecSchemaXml);

    return SchemaWriteStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECSchema::WriteToXmlString (Utf8StringR ecSchemaXml, int ecXmlVersionMajor, int ecXmlVersionMinor) const
    {
    ecSchemaXml.clear();

    BeXmlWriterPtr xmlWriter = BeXmlWriter::Create();
    xmlWriter->SetIndentation(4);

    SchemaWriteStatus status;
    SchemaXmlWriter schemaWriter(*xmlWriter.get(), *this, ecXmlVersionMajor, ecXmlVersionMinor);
    if (SchemaWriteStatus::Success != (status = schemaWriter.Serialize()))
        return status;

    xmlWriter->ToString (ecSchemaXml);

    return SchemaWriteStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECSchema::WriteToXmlFile
(
WCharCP ecSchemaXmlFile,
bool    utf16
) const
    {
    BeXmlWriterPtr xmlWriter = BeXmlWriter::CreateFileWriter(ecSchemaXmlFile);
    xmlWriter->SetIndentation(4);

    SchemaWriteStatus status;
    SchemaXmlWriter schemaWriter(*xmlWriter.get(), *this);
    if (SchemaWriteStatus::Success != (status = schemaWriter.Serialize()))
        return status;

    return SchemaWriteStatus::Success;
    //return (BEXML_Success == xmlDom->ToFile (ecSchemaXmlFile, (BeXmlDom::ToStringOption)(BeXmlDom::TO_STRING_OPTION_Indent | BeXmlDom::TO_STRING_OPTION_Formatted),
    //    utf16 ? BeXmlDom::FILE_ENCODING_Utf16 : BeXmlDom::FILE_ENCODING_Utf8)) ? SchemaWriteStatus::Success : SchemaWriteStatus::FailedToWriteFile;
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
    SchemaWriteStatus status = SchemaWriteStatus::Success;

    MSXML2::IXMLDOMDocument2Ptr xmlDocPtr = NULL;
    VERIFY_HRESULT_OK(xmlDocPtr.CreateInstance(__uuidof(MSXML2::DOMDocument60)), SchemaWriteStatus::FailedToInitializeMsmxl);
    xmlDocPtr->put_validateOnParse(VARIANT_TRUE);
    xmlDocPtr->put_async(VARIANT_FALSE);
    xmlDocPtr->put_preserveWhiteSpace(VARIANT_TRUE);
    xmlDocPtr->put_resolveExternals(VARIANT_FALSE);

    status = WriteXml(xmlDocPtr);
    if (status != SchemaWriteStatus::Success)
        return status;

    VERIFY_HRESULT_OK(xmlDocPtr->save(ecSchemaXmlStream), SchemaWriteStatus::FailedToSaveXml);

    return status;
    }
#endif //defined (NEEDSWORK_LIBXML)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECSchema::FindAllSchemasInGraph (bvector<ECN::ECSchemaP>& allSchemas, bool includeRootSchema)
    {
    FindAllSchemasInGraph ((bvector<ECN::ECSchemaCP>&)allSchemas, includeRootSchema);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECSchema::CollectAllSchemasInGraph (bvector<ECN::ECSchemaCP>& allSchemas, bool includeRootSchema) const
    {
    if (includeRootSchema)
        allSchemas.push_back (this);

    ECSchemaReferenceListCR referencedSchemas = this->GetReferencedSchemas();
    for (ECSchemaReferenceList::const_iterator iter = referencedSchemas.begin(); iter != referencedSchemas.end(); ++iter)
        {
        if (!includeRootSchema)
            {
            if (this == iter->second.get())
                continue;
            }

        bvector<ECN::ECSchemaCP>::iterator it = std::find (allSchemas.begin(), allSchemas.end(), iter->second.get());

        if (it != allSchemas.end())
            {
            allSchemas.erase(it);
            allSchemas.push_back(iter->second.get());
            continue;
            }

        iter->second->CollectAllSchemasInGraph (allSchemas, true);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    07/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECSchema::FindAllSchemasInGraph (bvector<ECN::ECSchemaCP>& allSchemas, bool includeRootSchema) const
    {
    CollectAllSchemasInGraph (allSchemas, includeRootSchema);
    std::reverse(allSchemas.begin(), allSchemas.end());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaId ECSchema::GetId() const
    {
    BeAssert (0 != m_ecSchemaId);
    return m_ecSchemaId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECSchema::FindSchema (SchemaKeyCR schemaKey, SchemaMatchType matchType) const
    {
    if (this->GetSchemaKey().Matches (schemaKey, matchType))
        return this;

    ECSchemaReferenceListCR referencedSchemas = GetReferencedSchemas();
    for (ECSchemaReferenceList::const_iterator iter = referencedSchemas.begin(); iter != referencedSchemas.end(); ++iter)
        {
        ECSchemaCP schema = iter->second->FindSchema (schemaKey, matchType);
        if (NULL != schema)
            return schema;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaP ECSchema::FindSchemaP (SchemaKeyCR schemaKey, SchemaMatchType matchType)
    {
    return const_cast<ECSchemaP> (FindSchema(schemaKey, matchType));
    }

/////////////////////////////////////////////////////////////////////////////////////////
// IStandaloneEnablerLocater
/////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECEnablerPtr  IStandaloneEnablerLocater::LocateStandaloneEnabler (SchemaKeyCR schemaKey, Utf8CP className)
    {
    return  _LocateStandaloneEnabler (schemaKey, className);
    }

DEFINE_KEY_METHOD(IStandaloneEnablerLocater)

/////////////////////////////////////////////////////////////////////////////////////////
// ECSchemaCache
/////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCache::~ECSchemaCache ()
    {
    m_schemas.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
int                             ECSchemaCache::GetCount ()
    {
    return (int)m_schemas.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
IECSchemaLocater& ECSchemaCache::GetSchemaLocater()
    {
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchemaCache::AddSchema   (ECSchemaR ecSchema)
    {
    if (m_schemas.end() != m_schemas.find (ecSchema.GetSchemaKey()))
        return ECObjectsStatus::DuplicateSchema;

    bvector<ECSchemaP> schemas;
    ecSchema.FindAllSchemasInGraph(schemas, true);
    bool inserted = false;

    for (bvector<ECSchemaP>::const_iterator iter = schemas.begin(); iter != schemas.end(); ++iter)
        {
        bpair<SchemaMap::iterator, bool> result = m_schemas.insert(SchemaMap::value_type((*iter)->GetSchemaKey(), *iter));
        inserted |= result.second;
        }

    return inserted ? ECObjectsStatus::Success : ECObjectsStatus::DuplicateSchema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchemaCache::DropSchema  (SchemaKeyCR ecSchemaKey)
    {
    SchemaMap::iterator iter = m_schemas.find (ecSchemaKey);
    if (iter == m_schemas.end())
        return ECObjectsStatus::SchemaNotFound;

    m_schemas.erase(iter);
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchemaCache::DropAllReferencesOfSchema(ECSchemaR schema)
    {
    for (SchemaMap::iterator iter = m_schemas.begin(); iter != m_schemas.end();)
        {
        bool removeSchema = false;
        bvector<ECSchemaP> schemas;
        iter->second->FindAllSchemasInGraph(schemas, true);
        for (bvector<ECSchemaP>::const_iterator refIter = schemas.begin(); refIter != schemas.end(); ++refIter)
            {
            if ((*refIter) == &schema)
                {
                removeSchema = true;
                break;
                }
            }

        if (removeSchema)
            iter = m_schemas.erase(iter);
        else
            ++iter;

        }
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void                             ECSchemaCache::Clear ()
    {
    m_schemas.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaP       ECSchemaCache::GetSchema   (SchemaKeyCR key)
    {
    return GetSchema(key, SCHEMAMATCHTYPE_Identical);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaP       ECSchemaCache::GetSchema(SchemaKeyCR key, SchemaMatchType matchType)
    {
    SchemaMap::iterator iter;
    switch (matchType)
        {
        case SCHEMAMATCHTYPE_Identical:
            {
            iter = m_schemas.find (key);
            break;
            }
        default:
            {
            //Other cases the container is not sorted by the match type.
            iter = std::find_if(m_schemas.begin(), m_schemas.end(), SchemaKeyMatchPredicate(key, matchType));
            break;
            }
        }

    if (iter == m_schemas.end())
        return NULL;

    return iter->second.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr     ECSchemaCache::_LocateSchema (SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR schemaContext)
    {
    return GetSchema(key, matchType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCachePtr    ECSchemaCache::Create ()
    {
    return new ECSchemaCache;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ECSchemaCache::GetSchemas (bvector<ECSchemaP>& schemas) const
    {
    schemas.clear();
    for (SchemaMap::const_iterator it = m_schemas.begin(); it != m_schemas.end(); it++)
        schemas.push_back (it->second.get());
    return schemas.size();
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
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaKeyCR ECSchema::GetSchemaKey ()const
    {
    return m_key;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
IECCustomAttributeContainer& ECSchema::GetCustomAttributeContainer() { return *this; }
IECCustomAttributeContainer const& ECSchema::GetCustomAttributeContainer() const { return *this; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus SchemaKey::ParseSchemaFullName (SchemaKeyR key, Utf8CP schemaFullName)
    {
    return ECSchema::ParseSchemaFullName (key.m_schemaName, key.m_versionMajor, key.m_versionMinor, schemaFullName);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECGetChildFunctor
    {
    typedef bvector<ECSchemaCP> ChildCollection;
    ChildCollection operator () (ECSchemaCP schema) const
        {
        bvector<ECSchemaCP> schemas;
        ECSchemaReferenceListCR referencedSchemas = schema->GetReferencedSchemas();
        for (ECSchemaReferenceList::const_iterator iter = referencedSchemas.begin(); iter != referencedSchemas.end(); ++iter)
            schemas.push_back(iter->second.get());

        return schemas;
        }
    };
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECSchema::AddingSchemaCausedCycles () const
    {
    ECGetChildFunctor fncTor;

    typedef SCCGraph <ECSchemaCP, ECGetChildFunctor> SchemaGraph;
    SchemaGraph graph (fncTor);

    SchemaGraph::SCCContext context;
    graph.StronglyConnect(this, context);

    bool hasCycles = false;
    for (SchemaGraph::SccNodes::const_iterator iter = context.m_components.begin(); iter != context.m_components.end(); ++iter)
        {
        if (1 != iter->size())
            {
            hasCycles = true;
            Utf8String cycleString;
            for (SchemaGraph::NodeVector::const_iterator cycleIter = iter->begin(); cycleIter != iter->end(); ++cycleIter)
                {
                cycleString.append((*cycleIter)->m_node->m_key.GetFullSchemaName());
                cycleString.append("-->");
                }
            cycleString.append( (*iter->begin())->m_node->m_key.GetFullSchemaName());
            LOG.errorv ("ECSchema '%s' contains cycles %s", m_key.GetFullSchemaName().c_str(), cycleString.c_str());

            break;
            }
        }

    return hasCycles;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String         SchemaKey::GetFullSchemaName () const
    {
    Utf8Char schemaName[512] = {0};
    BeStringUtilities::Snprintf(schemaName, "%s.%02d.%02d", m_schemaName.c_str(), m_versionMajor, m_versionMinor);
    return schemaName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECClassFinder
    {
    ECN::SchemaNameClassNamePair const& m_key;
    ECClassP&                          m_class;
    ECClassFinder (ECN::SchemaNameClassNamePair const& key, ECClassP& foundClass)
        :m_key(key), m_class(foundClass)
        {}

    bool operator () (ECSchemaR val)
        {
        if (0 != val.GetName().CompareTo(m_key.m_schemaName))
            return false;

        m_class = val.GetClassP(m_key.m_className.c_str());
        return NULL != m_class;
        }

    bool operator () (ECSchemaPtr const& val)
        {
        if (val.IsNull())
            return false;

        return (*this)(*val);
        }

    bool operator () (bpair<SchemaKey,ECSchemaPtr> const& val)
        {
        return (*this)(val.second);
        }

    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassP        SchemaMapExact::FindClassP (ECN::SchemaNameClassNamePair const& classNamePair) const
    {
    ECClassP classInstance = NULL;
    ECClassFinder classFinder(classNamePair, classInstance);

    SchemaMapExact::const_iterator iter = std::find_if (begin(), end(), classFinder);
    return iter == end() ? NULL : classInstance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t        ECSchema::ComputeSchemaXmlStringCheckSum(Utf8CP str, size_t len)
    {
    return CheckSumHelper::ComputeCheckSumForString (str, len);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECSchema::ReComputeCheckSum ()
    {
    if (m_immutable)
        return;

    WString xmlStr;
    if (SchemaWriteStatus::Success != WriteToXmlString (xmlStr))
        return;

    m_key.m_checkSum = CheckSumHelper::ComputeCheckSumForString (xmlStr.c_str(), sizeof(WChar)* xmlStr.length());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECSchema::SetImmutable()
    {
    BeAssert(!m_immutable);
    ReComputeCheckSum();
    m_immutable = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
int             SchemaKey::CompareByName (Utf8String schemaName) const
    {
    // TFS#223524: This was added to do case-insensitive comparison, but it is being used inappropriately.
    // ECSchema names are case-sensitive. If there are particular contexts in which case should be disregarded,
    // the code that handles those contexts should do so explicitly.
    return strcmp (m_schemaName.c_str(), schemaName.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            SchemaKey::LessThan (SchemaKeyCR rhs, SchemaMatchType matchType) const
    {
    switch (matchType)
        {
        case SCHEMAMATCHTYPE_Identical:
            {
            if (0 != m_checkSum || 0 != rhs.m_checkSum)
                return m_checkSum < rhs.m_checkSum;
            //Fall through
            }
        case SCHEMAMATCHTYPE_Exact:
            {
            int nameCompare = CompareByName (rhs.m_schemaName);

            if (nameCompare != 0)
                return nameCompare < 0;

            if (m_versionMajor != rhs.m_versionMajor)
                return m_versionMajor < rhs.m_versionMajor;

            return m_versionMinor < rhs.m_versionMinor;
            break;
            }
        case SCHEMAMATCHTYPE_LatestCompatible:
            {
            int nameCompare = CompareByName (rhs.m_schemaName);

            if (nameCompare != 0)
                return nameCompare < 0;

            return m_versionMajor < rhs.m_versionMajor;
            }
        case SCHEMAMATCHTYPE_Latest: //Only compare by name
            {
            return CompareByName (rhs.m_schemaName) < 0;
            }
        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            SchemaKey::Matches (SchemaKeyCR rhs, SchemaMatchType matchType) const
    {
    switch (matchType)
        {
        case SCHEMAMATCHTYPE_Identical:
            {
            if (0 != m_checkSum && 0 != rhs.m_checkSum)
                return m_checkSum == rhs.m_checkSum;
            //fall through
            }
        case SCHEMAMATCHTYPE_Exact:
            return 0 == CompareByName (rhs.m_schemaName) && m_versionMajor == rhs.m_versionMajor && m_versionMinor == rhs.m_versionMinor;
        case SCHEMAMATCHTYPE_LatestCompatible:
            return 0 == CompareByName (rhs.m_schemaName) && m_versionMajor == rhs.m_versionMajor && m_versionMinor >= rhs.m_versionMinor;
        case SCHEMAMATCHTYPE_Latest:
            return 0 == CompareByName (rhs.m_schemaName);
        default:
            return false;
        }
    }

static IECTypeAdapterContext::FactoryFn s_typeAdapterContextFactory;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
void IECTypeAdapterContext::RegisterFactory (FactoryFn fn)  { s_typeAdapterContextFactory = fn; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
IECTypeAdapterContextPtr IECTypeAdapterContext::Create (ECPropertyCR prop, IECInstanceCR instance, uint32_t componentIndex)
    {
    return NULL != s_typeAdapterContextFactory ? s_typeAdapterContextFactory (prop, instance, componentIndex) : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String QualifiedECAccessor::ToString() const
    {
    Utf8PrintfString str("%s:%s:%s", m_schemaName.c_str(), m_className.c_str(), m_accessString.c_str());
    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool QualifiedECAccessor::FromString (Utf8CP str)
    {
    bvector<Utf8String> tokens;
    BeStringUtilities::Split (str, ":", tokens);
    if (3 != tokens.size())
        return false;

    m_schemaName = tokens[0];
    m_className = tokens[1];
    m_accessString = tokens[2];

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool QualifiedECAccessor::FromAccessString (ECEnablerCR enabler, Utf8CP accessString)
    {
    ECValueAccessor va;
    if (ECObjectsStatus::Success == ECValueAccessor::PopulateValueAccessor (va, enabler, accessString) && 0 < va.GetDepth() && nullptr != va[0].GetECProperty())
        {
        ECClassCR rootClass = va[0].GetECProperty()->GetClass();
        m_schemaName = rootClass.GetSchema().GetName();
        m_className = rootClass.GetName();
        m_accessString = accessString;
        return true;
        }
    else
        return false;
    }

///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Ramanujam.Raman                 12/2012
//+---------------+---------------+---------------+---------------+---------------+------*/
//void IECClassLocater::RegisterClassLocater (IECClassLocaterR classLocater) 
//    {
//    s_registeredClassLocater = IECClassLocaterPtr (&classLocater);
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Sam.Wilson                      03/2013
//+---------------+---------------+---------------+---------------+---------------+------*/
//void IECClassLocater::UnRegisterClassLocater ()
//    {
//    s_registeredClassLocater = nullptr;
//    }
//
///*---------------------------------------------------------------------------------**//**
//* @bsimethod                                    Ramanujam.Raman                 12/2012
//+---------------+---------------+---------------+---------------+---------------+------*/
//IECClassLocaterP IECClassLocater::GetRegisteredClassLocater() 
//    {
//    return s_registeredClassLocater.get();
//    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool QualifiedECAccessor::Remap (ECSchemaCR pre, ECSchemaCR post, IECSchemaRemapperCR remapper)
    {
    SchemaNameClassNamePair schemaClass (m_schemaName, m_className);
    bool remapped = schemaClass.Remap (pre, post, remapper);
    if (remapped)
        {
        m_schemaName = schemaClass.m_schemaName;
        m_className = schemaClass.m_className;
        }

    ECClassCP newClass = post.GetClassCP (schemaClass.m_className.c_str());
    ECValueAccessor va;
    if (nullptr != newClass && ECObjectsStatus::Success == ECValueAccessor::PopulateAndRemapValueAccessor (va, *newClass->GetDefaultStandaloneEnabler(), m_accessString.c_str(), remapper))
        {
        Utf8String newAccessString = va.GetManagedAccessString();
        if (!newAccessString.Equals (m_accessString))
            {
            m_accessString = newAccessString;
            remapped = true;
            }
        }

    return remapped;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool SchemaNameClassNamePair::Remap (ECSchemaCR pre, ECSchemaCR post, IECSchemaRemapperCR remapper)
    {
    bool remapped = false;
    if (pre.GetName().Equals (m_schemaName))
        {
        if (!pre.GetName().Equals (post.GetName()))
            {
            m_schemaName = post.GetName();
            remapped = true;
            }

        Utf8String newClassName = m_className;
        if (remapper.ResolveClassName (newClassName, post))
            {
            m_className = newClassName;
            remapped = true;
            }
        }

    return remapped;
    }

END_BENTLEY_ECOBJECT_NAMESPACE



