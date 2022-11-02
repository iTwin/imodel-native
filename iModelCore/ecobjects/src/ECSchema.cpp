/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"
#include "SchemaXml.h"
#include "SchemaJson.h"
#include "StronglyConnectedGraph.h"
#if defined (_WIN32) // WIP_NONPORT - iostreams not support on Android
#include <iomanip>
#endif
#include <Bentley/BeFileName.h>
#include <Bentley/BeFile.h>
#include <Bentley/BeFileListIterator.h>
#include <Bentley/SHA1.h>

#include <list>
#include <array>
#include <utility>
#include <cctype>
#include <algorithm>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

// If you are developing schemas, particularly when editing them by hand, you want to have this variable set to false so you get the asserts to help you figure out what is going wrong.
// Test programs generally want to get error status back and not assert, so they call ECSchema::AssertOnXmlError (false);
static bool s_noAssert = false;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECAppData& ECSchema::GetAppData() const {
    if (m_appData.IsNull())
        m_appData = ECAppData::Create();
        
    return *m_appData;
}
/*---------------------------------------------------------------------------------**//**
* Currently this is only used by ECValidatedName and ECSchema.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECNameValidation::ValidationResult ECNameValidation::Validate (Utf8CP name)
    {
    if (nullptr == name || 0 == *name)
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
* @bsimethod
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
* @bsimethod
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
            if (1 == WString::Swscanf_safe(buf.c_str() + pos + 3, L"%x", &charCode))
                {
                buf[pos] = (WChar)charCode;
                buf.erase (pos+1, 8);
                pos++;
                wasDecoded = true;
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECNameValidation::EncodeToValidName (Utf8StringCR name)
    {
    Utf8String encoded;
    EncodeToValidName (encoded, name);
    return encoded;
    }

//---------------------------------------------------------------------------------------//
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------//
bool ECValidatedName::SetValidName(Utf8CP name, bool decodeNameToDisplayLabel)
    {
    if (!ECNameValidation::IsValidName(name))
        return false;

    m_name = name;
    if (decodeNameToDisplayLabel && !m_hasExplicitDisplayLabel)
        m_hasExplicitDisplayLabel = ECNameValidation::DecodeFromValidName(m_displayLabel, m_name);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECValidatedName::SetDisplayLabel(Utf8CP label)
    {
    if (nullptr == label || '\0' == *label)
        {
        m_hasExplicitDisplayLabel = false;
        m_displayLabel.clear();
        }
    else
        {
        m_hasExplicitDisplayLabel = true;
        m_displayLabel = label;
        }
    }

/////////////////////////////////////////////////////////////////////////////////////////
// ECSchemaCache
/////////////////////////////////////////////////////////////////////////////////////////

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

    for (auto entry : m_enumerationMap)
        {
        ECEnumerationP ecEnumeration = entry.second;
        delete ecEnumeration;
        }

    m_enumerationMap.clear();
    BeAssert(m_enumerationMap.empty());

    for (auto entry : m_kindOfQuantityMap)
        {
        auto kindOfQuantity = entry.second;
        delete kindOfQuantity;
        }

    m_kindOfQuantityMap.clear();
    BeAssert(m_kindOfQuantityMap.empty());

    for (auto entry : m_propertyCategoryMap)
        {
        auto propertyCategory = entry.second;
        delete propertyCategory;
        }

    m_propertyCategoryMap.clear();
    BeAssert(m_propertyCategoryMap.empty());

    for (auto entry : m_formatMap)
        {
        auto format = entry.second;
        delete format;
        }
    m_formatMap.clear();
    BeAssert(m_formatMap.empty());

    // The Unit Types are deleted in the destructor of the SchemaUnitContext.
    // delete &m_unitsContext;

    m_refSchemaList.clear();

    memset ((void*)this, 0xececdead, 4);// Replaced sizeof(this) with 4. There is value
                                        // in trying to make this logic "correct" for a 64-bit build.
                                        // This memset is clearly just intended to aid in debugging.
                                        // Attempting to use this object with the high 4 bytes of the vtable
                                        // pointer set to 0xececdead will crash just as reliably.
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchema::ResetId() { 
    BeMutexHolder holder(GetMutex());
    for(auto v : GetClasses())
        v->ResetId();
    
    for (auto v : GetEnumerations())
        v->ResetId();
    
    for (auto v: GetPropertyCategories())
        v->ResetId();
    
    for (auto v : GetKindOfQuantities())
        v->ResetId();
    
    for (auto v : GetUnitSystems())
        v->ResetId();

    for (auto v :  GetUnits())
        v->ResetId();

    for (auto v : GetPhenomena())
        v->ResetId();
    
    for (auto v : GetFormats())
        v->ResetId();
    
    m_ecSchemaId.Invalidate(); 
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchema::SetErrorHandling (bool showMessages, bool doAssert)
    {
    s_noAssert = !doAssert;
    ECClass::SetErrorHandling(doAssert);
    SchemaXmlReader::SetErrorHandling(doAssert);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetName (Utf8StringCR name)
    {
    if (m_immutable)
        return ECObjectsStatus::SchemaIsImmutable;
    else if (!ECNameValidation::IsValidName (name.c_str()))
        return ECObjectsStatus::InvalidName;

    m_key.m_schemaName = name;

    if (OriginalECXmlVersionLessThan(ECVersion::V3_1))
        m_hasExplicitDisplayLabel = ECNameValidation::DecodeFromValidName(m_displayLabel, m_key.m_schemaName);

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetAlias (Utf8StringCR alias)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    else if (Utf8String::IsNullOrEmpty(alias.c_str()))
        return ECObjectsStatus::InvalidName;

    else if (!ECNameValidation::IsValidName(alias.c_str()))
        return ECObjectsStatus::InvalidName;

    ECNameValidation::EncodeToValidName(m_alias, alias);

    return ECObjectsStatus::Success;
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
Utf8StringCR ECSchema::GetDescription() const
    {
    return m_description;
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
Utf8StringCR ECSchema::GetDisplayLabel() const
    {
    return GetInvariantDisplayLabel();
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
    "ECDbMap",
    "CoreCustomAttributes", // New EC3 Standard Schema
    "SchemaLocalizationCustomAttributes", // New EC3 Standard Schema
    "Units", // New EC3 Standard Schema
    "Formats" // New EC3 Standard Schema
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
    "Bentley_Standard_CustomAttributes.01.00.00",
    "Bentley_Standard_Classes.01.00.00",
    "EditorCustomAttributes.01.00.00",
    "Bentley_Common_Classes.01.00.00",
    "Dimension_Schema.01.00.00",
    "iip_mdb_customAttributes.01.00.00",
    "KindOfQuantity_Schema.01.00.00",
    "rdl_customAttributes.01.00.00",
    "SIUnitSystemDefaults.01.00.00",
    "Unit_Attributes.01.00.00",
    "Units_Schema.01.00.00",
    "USCustomaryUnitSystemDefaults.01.00.00"
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchema::IsSamePrimarySchema (ECSchemaR primarySchema) const
    {
    if (0 != BeStringUtilities::StricmpAscii(this->GetAlias().c_str(), primarySchema.GetAlias().c_str()))
        return false;

    if (0 != BeStringUtilities::StricmpAscii(this->GetFullSchemaName().c_str(), primarySchema.GetFullSchemaName().c_str()))
        return false;

    return (GetClassCount() == primarySchema.GetClassCount());
    }

//-------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool ECSchema::IsSupplementalSchema () const
    {
    SupplementalSchemaMetaDataPtr suppMetaData = nullptr;
    return SupplementalSchemaMetaData::TryGetFromSchema(suppMetaData, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchema::ShouldNotBeStored (SchemaKeyCR key)
    {
    Utf8String schemaName = key.GetFullSchemaName();
    for (Utf8CP* cur = s_originalStandardSchemaFullNames, *end = cur + _countof(s_originalStandardSchemaFullNames); cur < end; ++cur)
        if (schemaName.Equals (*cur))
            return true;

    // We don't want to import any version of the Units_Schema
    if (BeStringUtilities::StricmpAscii("Units_Schema", key.m_schemaName.c_str()) == 0)
        return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetVersionRead (const uint32_t versionRead)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    if (versionRead > 999 || versionRead < 0)
        return ECObjectsStatus::InvalidECVersion;
    
    m_key.m_versionRead = versionRead;
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetVersionWrite (const uint32_t value)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    if (value > 999 || value < 0)
        return ECObjectsStatus::InvalidECVersion;

    m_key.m_versionWrite = value;
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetVersionMinor (const uint32_t versionMinor)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    if (versionMinor > 9999999 || versionMinor < 0)
        return ECObjectsStatus::InvalidECVersion;

    m_key.m_versionMinor = versionMinor;
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchema::SetImmutable()
    {
    BeAssert(!m_immutable);
    m_immutable = true;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::CreateECVersion(ECVersion &ecVersion, uint32_t ecMajorVersion, uint32_t ecMinorVersion)
    {
    if (ecMajorVersion == 2 && ecMinorVersion == 0)
        ecVersion = ECVersion::V2_0;
    else if (ecMajorVersion == 3 && ecMinorVersion == 0)
        ecVersion = ECVersion::V3_0;
    else if (ecMajorVersion == 3 && ecMinorVersion == 1)
        ecVersion = ECVersion::V3_1;
    else if (ecMajorVersion == 3 && ecMinorVersion == 2)
        ecVersion = ECVersion::V3_2;
    else
        return ECObjectsStatus::InvalidECVersion;

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECVersion ECSchema::ECVersionToWrite(uint32_t ecMajorVersion, uint32_t ecMinorVersion)
    {
    ECVersion ecVersion;
    auto const ecVersionStatus = ECSchema::CreateECVersion(ecVersion, ecMajorVersion, ecMinorVersion);
    if (ecVersion <= ECVersion::V3_0 || ECObjectsStatus::Success != ecVersionStatus)
        ecVersion = ECVersion::Latest;
    return ecVersion;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::ParseECVersion(uint32_t &ecVersionMajor, uint32_t &ecVersionMinor, ECVersion ecVersion)
    {
    ecVersionMajor = (uint32_t) ((uint32_t) ecVersion >> 16);
    ecVersionMinor = (uint32_t) (0xFFFF & (uint32_t) ecVersion);

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ECSchema::GetECVersionString(ECVersion ecVersion)
    {
    switch (ecVersion)
        {
        case ECVersion::V2_0:
            return "2.0";
        case ECVersion::V3_0:
            return "3.0";
        case ECVersion::V3_1:
            return "3.1";
        case ECVersion::V3_2:
            return "3.2";
        }
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchema::Validate(bool resolveIssues)
    {
    // Need to skip validation for supplemental schemas since they are not required to have fully defined relationships
    if (Utf8String::npos != GetName().find("_Supplemental_"))
        return true;

    bool isValid = true;
    for (ECClassP ecClass : GetClasses())
        {
        ECRelationshipClassCP relClass = ecClass->GetRelationshipClassCP();
        if (relClass != nullptr)
            {
            if (!relClass->Verify(resolveIssues))
                isValid = false;

            continue;
            }

        if (!ecClass->Validate())
            isValid = false;
        }

    for (KindOfQuantityCP koq : GetKindOfQuantities())
        {
        if (!koq->Verify())
            isValid = false;
        }

    // Workaround:  This evaluate makes sure that each UnitSymbol is evaluated immediately after deserialization happens.  Otherwise, it could happen whenever
    // a call to do a conversion happens.  This could lead to a threading issue...
    for (auto const& unit : GetUnits())
        {
        unit->Evaluate();
        }

    for (auto const& phen : GetPhenomena())
        {
        phen->Evaluate();
        }

    if (OriginalECXmlVersionLessThan(ECVersion::V3_1) && resolveIssues && ECClass::SchemaAllowsOverridingArrays(this))
        {
        for (ECClassP ecClass : GetClasses())
            {
            ECObjectsStatus status = ecClass->FixArrayPropertyOverrides();
            if (ECObjectsStatus::Success != status)
                {
                LOG.errorv("Failed to fix array property overrides for properties of class '%s'", ecClass->GetFullName());
                return false;
                }
            }
        }

    if (!isValid)
        {
        // If the validation fails and the schema is read from an ECXML 3.1 or greater, fail to validate.
        if (OriginalECXmlVersionAtLeast(ECVersion::V3_1))
            {
            Utf8String schemaFullName = GetFullSchemaName();
            LOG.errorv("Schema validation failed for schema '%s'.  It is supposed to be EC Version 3.1 but it does not meet the rules for that EC version.  See log for details.",
                       schemaFullName.c_str());
            return false;
            }

        if (!IsECVersion(ECVersion::V3_0))
            {
            LOG.warningv("ECSchemaXML for %s did not pass ECXml 3.1 validation, being downgraded to ECXml 3.0", GetName().c_str());

            // Failed to validate for a 3.1 schema. Downgraded to a 3.0 schema in memory.
            m_ecVersion = ECVersion::V3_0;
            }
        else
            LOG.warningv("ECSchema did not pass EC3.1 validation.");
        }
    else
        m_ecVersion = ECVersion::Latest;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchema::DebugDump()const
    {
    printf("ECSchema: this=0x%" PRIx64 "  %s, nClasses=%d\n", (uint64_t)this, m_key.GetFullSchemaName().c_str(), (int) m_classMap.size());
    for (ClassMap::const_iterator it = m_classMap.begin(); it != m_classMap.end(); ++it)
        {
        bpair<Utf8CP, ECClassP>const& entry = *it;
        ECClassCP ecClass = entry.second;
        printf("    ECClass: 0x%" PRIx64 ", %s\n", (uint64_t)ecClass, ecClass->GetName().c_str());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::RenameClass (ECClassR ecClass, Utf8CP newName)
    {
    ClassMap::iterator iter = m_classMap.find (ecClass.GetName().c_str());
    if (iter == m_classMap.end() || iter->second != &ecClass)
        return ECObjectsStatus::ClassNotFound;

    ECClassP pClass = &ecClass;
    m_classMap.erase (iter);
    ECObjectsStatus renameStatus = ecClass.SetName (newName);
    ECObjectsStatus addStatus = AddClass (pClass);
    return ECObjectsStatus::Success != renameStatus ? renameStatus : addStatus;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
Utf8String ECSchema::FindUniqueSchemaItemName (Utf8CP startingName) const
    {
    //TODO: Rewire this to SchemaConflictHelper::FindUniqueSchemaItemName at some point.
    if (!NamedElementExists(startingName))
        return startingName;
    
    Utf8PrintfString newName ("%s_", startingName);
    while(NamedElementExists(newName.c_str()))
        {
        newName.append("_");
        }
    return newName;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::AddClass(ECClassP pClass, bool resolveConflicts)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    if (NamedElementExists(pClass->GetName().c_str()))
        {
        if (!resolveConflicts)
            {
            LOG.errorv("Cannot create class '%s' because a named element the same identifier already exists in the schema",
                       pClass->GetName().c_str());

            return ECObjectsStatus::NamedItemAlreadyExists;
            }

        Utf8String uniqueName = FindUniqueSchemaItemName(pClass->GetName().c_str());
        pClass->SetName(uniqueName);
        return AddClass(pClass, resolveConflicts);
        }

    if (false == m_classMap.insert(bpair<Utf8CP, ECClassP>(pClass->GetName().c_str(), pClass)).second)
        {
        LOG.errorv("There was a problem adding class '%s' to the schema",
                   pClass->GetName().c_str());

        return ECObjectsStatus::Error;
        }

    if (m_serializationOrder.GetPreserveElementOrder())
        m_serializationOrder.AddElement(pClass->GetName().c_str(), ECSchemaElementType::ECClass);

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
        pClass = nullptr;
        return status;
        }

    if (ECObjectsStatus::Success != (status = AddClass(pClass)))
        {
        delete pClass;
        pClass = nullptr;
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECSchema::CreateMixinClass (ECEntityClassP& pClass, Utf8StringCR name, ECEntityClassCR appliesTo)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    pClass = new ECEntityClass(*this);
    ECObjectsStatus status = pClass->SetName (name);
    if (ECObjectsStatus::Success != status)
        {
        delete pClass;
        pClass = nullptr;
        return status;
        }

    pClass->SetClassModifier(ECClassModifier::Abstract);

    IECInstancePtr mixinInstance = CoreCustomAttributeHelper::CreateCustomAttributeInstance("IsMixin");
    if (!mixinInstance.IsValid())
        {
        delete pClass;
        pClass = nullptr;
        return ECObjectsStatus::UnableToSetMixinCustomAttribute;
        }

    auto& coreCA = mixinInstance->GetClass().GetSchema();
    if (!ECSchema::IsSchemaReferenced(*this, coreCA))
        this->AddReferencedSchema(const_cast<ECSchemaR>(coreCA));

    auto& appliesToClassSchema = appliesTo.GetSchema();
    if ((this != &appliesToClassSchema) && !ECSchema::IsSchemaReferenced(*this, appliesToClassSchema))
        {
        status = this->AddReferencedSchema(const_cast<ECSchemaR>(appliesToClassSchema));
        if (ECObjectsStatus::Success != status)
            {
            delete pClass;
            pClass = nullptr;
            return status;
            }
        }

    ECValue appliesToClass(ECClass::GetQualifiedClassName(*this, appliesTo).c_str());
    status = mixinInstance->SetValue("AppliesToEntityClass", appliesToClass);

    if (ECObjectsStatus::Success != status)
        {
        delete pClass;
        pClass = nullptr;
        return status;
        }

    if (ECObjectsStatus::Success != pClass->SetCustomAttribute(*mixinInstance))
        {
        delete pClass;
        pClass = nullptr;
        return status;
        }

    if (ECObjectsStatus::Success != (status = AddClass(pClass)))
        {
        delete pClass;
        pClass = nullptr;
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECSchema::CreateStructClass (ECStructClassP& pClass, Utf8StringCR name)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    pClass = new ECStructClass(*this);
    ECObjectsStatus status = pClass->SetName (name);
    if (ECObjectsStatus::Success != status)
        {
        delete pClass;
        pClass = nullptr;
        return status;
        }

    if (ECObjectsStatus::Success != (status = AddClass(pClass)))
        {
        delete pClass;
        pClass = nullptr;
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECSchema::CreateCustomAttributeClass (ECCustomAttributeClassP& pClass, Utf8StringCR name)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    pClass = new ECCustomAttributeClass(*this);
    ECObjectsStatus status = pClass->SetName (name);
    if (ECObjectsStatus::Success != status)
        {
        delete pClass;
        pClass = nullptr;
        return status;
        }

    if (ECObjectsStatus::Success != (status = AddClass(pClass)))
        {
        delete pClass;
        pClass = nullptr;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::CreateRelationshipClass (ECRelationshipClassP& pClass, Utf8StringCR name, bool verify)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    pClass = new ECRelationshipClass(*this, verify);
    ECObjectsStatus status = pClass->SetName (name);
    if (ECObjectsStatus::Success != status)
        {
        delete pClass;
        pClass = nullptr;
        return status;
        }

    if (ECObjectsStatus::Success != (status = AddClass(pClass)))
        {
        delete pClass;
        pClass = nullptr;
        return status;
        }

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECSchema::CreateRelationshipClass(ECRelationshipClassP& relationshipClass, Utf8StringCR name, ECEntityClassCR source, Utf8CP sourceRoleLabel, ECEntityClassCR target, Utf8CP targetRoleLabel)
    {
    ECObjectsStatus status = CreateRelationshipClass(relationshipClass, name);
    if (ECObjectsStatus::Success != status)
        return status;

    status = relationshipClass->GetSource().AddClass(source);
    if (ECObjectsStatus::Success != status)
        {
        delete relationshipClass;
        relationshipClass = nullptr;
        return status;
        }

    status = relationshipClass->GetSource().SetRoleLabel(sourceRoleLabel);
    if (ECObjectsStatus::Success != status)
        {
        delete relationshipClass;
        relationshipClass = nullptr;
        return status;
        }

    status = relationshipClass->GetTarget().AddClass(target);
    if (ECObjectsStatus::Success != status)
        {
        delete relationshipClass;
        relationshipClass = nullptr;
        return status;
        }

    status = relationshipClass->GetTarget().SetRoleLabel(targetRoleLabel);
    if (ECObjectsStatus::Success != status)
        {
        delete relationshipClass;
        relationshipClass = nullptr;
        return status;
        }

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::CreateEnumeration(ECEnumerationP & ecEnumeration, Utf8CP name, PrimitiveType type)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    ecEnumeration = new ECEnumeration(*this);
    ecEnumeration->SetName(name);

    auto status = ecEnumeration->SetType(type);
    if (ECObjectsStatus::Success != status)
        {
        delete ecEnumeration;
        ecEnumeration = nullptr;
        return status;
        }

    status = AddSchemaChildToMap<ECEnumeration, EnumerationMap>(ecEnumeration, &m_enumerationMap, ECSchemaElementType::ECEnumeration);
    if (ECObjectsStatus::Success != status)
        {
        delete ecEnumeration;
        ecEnumeration = nullptr;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::CreateKindOfQuantity(KindOfQuantityP& kindOfQuantity, Utf8CP name)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    kindOfQuantity = new KindOfQuantity(*this);
    kindOfQuantity->SetName(name);

    auto status = AddSchemaChildToMap<KindOfQuantity, KindOfQuantityMap>(kindOfQuantity, &m_kindOfQuantityMap, ECSchemaElementType::KindOfQuantity);
    if (ECObjectsStatus::Success != status)
        {
        delete kindOfQuantity;
        kindOfQuantity = nullptr;
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECSchema::CreatePropertyCategory(PropertyCategoryP& propertyCategory, Utf8CP name, bool logError)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    propertyCategory = new PropertyCategory(*this);
    propertyCategory->SetName(name);

    auto status = AddSchemaChildToMap<PropertyCategory, PropertyCategoryMap>(propertyCategory, &m_propertyCategoryMap, ECSchemaElementType::PropertyCategory, logError);
    if (ECObjectsStatus::Success != status)
        {
        delete propertyCategory;
        propertyCategory = nullptr;
        }

    return status;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
ECObjectsStatus ECSchema::CreateUnitSystem(UnitSystemP& system, Utf8CP name, Utf8CP displayLabel, Utf8CP description)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    system = new UnitSystem(*this, name);
    if (nullptr == system)
        return ECObjectsStatus::Error;

    ECObjectsStatus status = ECObjectsStatus::Success;

    if (nullptr != displayLabel)
        status = system->SetDisplayLabel(displayLabel);

    if (ECObjectsStatus::Success != status)
        {
        delete system;
        system = nullptr;
        return status;
        }

    if (nullptr != description)
        status = system->SetDescription(description);

    if (ECObjectsStatus::Success != status)
        {
        delete system;
        system = nullptr;
        return status;
        }

    status = AddUnitType<UnitSystem>(system, ECSchemaElementType::UnitSystem);
    if (ECObjectsStatus::Success != status)
        {
        delete system;
        system = nullptr;
        }

    return status;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
ECObjectsStatus ECSchema::CreatePhenomenon(PhenomenonP& phenomenon, Utf8CP name, Utf8CP definition, Utf8CP displayLabel, Utf8CP description)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    phenomenon = new Phenomenon(*this, name, definition);

    ECObjectsStatus status = ECObjectsStatus::Success;

    if (nullptr != displayLabel)
        status = phenomenon->SetDisplayLabel(displayLabel);

    if (ECObjectsStatus::Success != status)
        {
        delete phenomenon;
        phenomenon = nullptr;
        return status;
        }

    if (nullptr != description)
        status = phenomenon->SetDescription(description);

    if (ECObjectsStatus::Success != status)
        {
        delete phenomenon;
        phenomenon = nullptr;
        return status;
        }

    status = AddUnitType<Phenomenon>(phenomenon, ECSchemaElementType::Phenomenon);
    if (ECObjectsStatus::Success != status)
        {
        delete phenomenon;
        phenomenon = nullptr;
        }

    return status;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
ECObjectsStatus ECSchema::CreateUnit(ECUnitP& unit, Utf8CP name, Utf8CP definition, PhenomenonCR phenom, UnitSystemCR unitSystem, Nullable<double> numerator, Nullable<double> denominator, Nullable<double> offset, Utf8CP label, Utf8CP description)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    const auto* phenomSchema = &phenom.GetSchema();
    if ((this != phenomSchema) && !ECSchema::IsSchemaReferenced(*this, *phenomSchema))
        {
        LOG.errorv("Phenomenon %s with schema %s is not in this or any schema referenced by schema %s", phenom.GetName().c_str(), phenomSchema->GetName().c_str(), this->GetName().c_str());
        return ECObjectsStatus::SchemaNotFound;
        }

    const auto* systemSchema = &unitSystem.GetSchema();
    if ((this != systemSchema) && !ECSchema::IsSchemaReferenced(*this, *systemSchema))
        {
        LOG.errorv("UnitSystem %s with schema %s is not in this or any schema referenced by schema %s", unitSystem.GetName().c_str(), systemSchema->GetName().c_str(), this->GetName().c_str());
        return ECObjectsStatus::SchemaNotFound;
        }

    unit = new ECUnit(*this, name);
    unit->SetSystem(unitSystem);
    unit->SetPhenomenon(phenom);
    unit->SetDefinition(definition);

    if (nullptr == unit)
        return ECObjectsStatus::Error;

    if (numerator.IsValid())
        unit->SetNumerator(numerator.Value());
    if (denominator.IsValid())
        unit->SetDenominator(denominator.Value());
    if (offset.IsValid())
        unit->SetOffset(offset.Value());

    ECObjectsStatus status = ECObjectsStatus::Success;
    const auto cleanupIfNecessary = [&unit, &status]() -> decltype(auto)
        {
        if (ECObjectsStatus::Success != status)
            {
            delete unit;
            unit = nullptr;
            }
        };

    if(nullptr != label)
        {
        unit->SetDisplayLabel(label);
        cleanupIfNecessary();
        }
    if(nullptr != description)
        {
        status = unit->SetDescription(description);
        cleanupIfNecessary();
        if(status != ECObjectsStatus::Success) return status;
        }

    status = AddUnitType<ECUnit>(unit, ECSchemaElementType::Unit);
    cleanupIfNecessary();
    if(status != ECObjectsStatus::Success) return status;

    return status;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
ECObjectsStatus ECSchema::CreateInvertedUnit(ECUnitP& unit, ECUnitCR parent, Utf8CP name, UnitSystemCR unitSystem, Utf8CP label, Utf8CP description)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    ECObjectsStatus status = ECObjectsStatus::Success;
    const auto cleanupIfNecessary = [&unit, &status]() -> decltype(auto)
        {
        if (ECObjectsStatus::Success != status)
            {
            delete unit;
            unit = nullptr;
            }
        };

    const auto* systemSchema = &unitSystem.GetSchema();
    const auto* parentSchema = &parent.GetSchema();

    if ((this != systemSchema) && !ECSchema::IsSchemaReferenced(*this, *systemSchema))
        {
        LOG.errorv("UnitSystem %s with schema %s is not in this or any schema referenced by schema %s", unitSystem.GetName().c_str(), systemSchema->GetName().c_str(), this->GetName().c_str());
        return ECObjectsStatus::SchemaNotFound;
        }
    if ((this != parentSchema) && !ECSchema::IsSchemaReferenced(*this, *parentSchema))
        {
        LOG.errorv("Unit %s with schema %s is not in this or any schema referenced by schema %s", parent.GetName().c_str(), parentSchema->GetName().c_str(), this->GetName().c_str());
        return ECObjectsStatus::SchemaNotFound;
        }

    unit =  new ECUnit(*this, parent, unitSystem, name);

    if (nullptr != label)
        {
        unit->SetDisplayLabel(label);
        cleanupIfNecessary();
        }

    if (nullptr != description)
        {
        status = unit->SetDescription(description);
        cleanupIfNecessary();
        if(status != ECObjectsStatus::Success) return status;
        }

    status = AddUnitType<ECUnit>(unit, ECSchemaElementType::InvertedUnit);
    cleanupIfNecessary();
    if(status != ECObjectsStatus::Success) return status;

    return status;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
ECObjectsStatus ECSchema::CreateConstant(ECUnitP& constant, Utf8CP name, Utf8CP definition, PhenomenonCR phenom, double numerator, Nullable<double> denominator, Utf8CP label, Utf8CP description)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    ECObjectsStatus status = ECObjectsStatus::Success;
    const auto cleanupIfNecessary = [&constant, &status]() -> decltype(auto)
        {
        if (ECObjectsStatus::Success != status)
            {
            delete constant;
            constant = nullptr;
            }
        };

    const auto* phenomSchema = &phenom.GetSchema();
    if ((this != phenomSchema) && !ECSchema::IsSchemaReferenced(*this, *phenomSchema))
        {
        LOG.errorv("Phenomenon %s with schema %s is not in this or any schema referenced by schema %s", phenom.GetName().c_str(), phenomSchema->GetName().c_str(), this->GetName().c_str());
        return ECObjectsStatus::SchemaNotFound;
        }

    constant = new ECUnit(*this, phenom, name, definition, numerator, denominator);

    if (nullptr != label)
        {
        constant->SetDisplayLabel(label);
        cleanupIfNecessary();
        }

    if (nullptr != description)
        {
        status = constant->SetDescription(description);
        cleanupIfNecessary();
        if(status != ECObjectsStatus::Success) return status;
        }
    status = AddUnitType<ECUnit>(constant, ECSchemaElementType::Constant);
    cleanupIfNecessary();
    if(status != ECObjectsStatus::Success) return status;

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECSchema::CreateFormat(ECFormatP& unitFormat, Utf8CP name, Utf8CP displayLabel, Utf8CP description, Formatting::NumericFormatSpecCP nfs, Formatting::CompositeValueSpecCP composite)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    unitFormat = new ECFormat(*this, name);
    if (displayLabel != nullptr)
        {
        ECObjectsStatus status = unitFormat->SetDisplayLabel(displayLabel);
        if (ECObjectsStatus::Success != status)
            {
            delete unitFormat;
            unitFormat = nullptr;
            return status;
            }
        }

    if (description != nullptr)
        {
        ECObjectsStatus status = unitFormat->SetDescription(description);
        if (ECObjectsStatus::Success != status)
            {
            delete unitFormat;
            unitFormat = nullptr;
            return status;
            }
        }

    if (nullptr != nfs)
        unitFormat->SetNumericSpec(*nfs);
    if (nullptr != composite)
        unitFormat->SetCompositeSpec(*composite);

    ECObjectsStatus status = AddSchemaChildToMap<ECFormat, FormatMap>(unitFormat, &m_formatMap, ECSchemaElementType::Format);
    if (ECObjectsStatus::Success != status)
        {
        delete unitFormat;
        unitFormat = nullptr;
        return status;
        }

    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
template<typename T, typename T_MAP>
ECObjectsStatus ECSchema::AddSchemaChildToMap(T* child, T_MAP* map, ECSchemaElementType childType, bool logError)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    if(NamedElementExists(child->GetName().c_str()))
        {
        if (logError)
            LOG.errorv("Cannot create %s '%s' because a named element with the same identifier already exists in the schema", SchemaParseUtils::SchemaElementTypeToString(childType), child->GetName().c_str());
        return ECObjectsStatus::NamedItemAlreadyExists;
        }

    if (false == map->insert(bpair<Utf8CP, T*>(child->GetName().c_str(), child)).second)
        {
        LOG.errorv("There was a problem adding %s '%s' to the schema", SchemaParseUtils::SchemaElementTypeToString(childType), child->GetName().c_str());
        return ECObjectsStatus::Error;
        }

    if (m_serializationOrder.GetPreserveElementOrder())
        m_serializationOrder.AddElement(child->GetName().c_str(), childType);
    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
template<typename T>
ECObjectsStatus ECSchema::AddUnitType(T* child, ECSchemaElementType childType)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    if(NamedElementExists(child->GetName().c_str()))
        {
        LOG.errorv("Cannot create %s '%s' because a named element with the same identifier already exists in the schema", SchemaParseUtils::SchemaElementTypeToString(childType), child->GetName().c_str());
        return ECObjectsStatus::NamedItemAlreadyExists;
        }

    ECObjectsStatus status = m_unitsContext.Add<T>(child, childType);
    if (ECObjectsStatus::Success != status)
        {
        LOG.errorv("There was a problem adding %s '%s' to the schema", SchemaParseUtils::SchemaElementTypeToString(childType), child->GetName().c_str());
        return status;
        }

    if (m_serializationOrder.GetPreserveElementOrder())
        m_serializationOrder.AddElement(child->GetName().c_str(), childType);
    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
template<typename T> ECObjectsStatus ECSchema::AddSchemaChild(T* child, ECSchemaElementType childType) {;}
template<> ECObjectsStatus ECSchema::AddSchemaChild<ECEnumeration>(ECEnumerationP child, ECSchemaElementType childType) {return AddSchemaChildToMap<ECEnumeration, EnumerationMap>(child, &m_enumerationMap, childType);}
template<> ECObjectsStatus ECSchema::AddSchemaChild<PropertyCategory>(PropertyCategoryP child, ECSchemaElementType childType) {return AddSchemaChildToMap<PropertyCategory, PropertyCategoryMap>(child, &m_propertyCategoryMap, childType);}
template<> ECObjectsStatus ECSchema::AddSchemaChild<KindOfQuantity>(KindOfQuantityP child, ECSchemaElementType childType) {return AddSchemaChildToMap<KindOfQuantity, KindOfQuantityMap>(child, &m_kindOfQuantityMap, childType);}
template<> ECObjectsStatus ECSchema::AddSchemaChild<UnitSystem>(UnitSystemP child, ECSchemaElementType childType) {return AddUnitType<UnitSystem>(child, childType);}
template<> ECObjectsStatus ECSchema::AddSchemaChild<Phenomenon>(PhenomenonP child, ECSchemaElementType childType) {return AddUnitType<Phenomenon>(child, childType);}
template<> ECObjectsStatus ECSchema::AddSchemaChild<ECUnit>(ECUnitP child, ECSchemaElementType childType) {return AddUnitType<ECUnit>(child, childType);}
template<> ECObjectsStatus ECSchema::AddSchemaChild<ECFormat>(ECFormatP child, ECSchemaElementType childType) {return AddSchemaChildToMap<ECFormat, FormatMap>(child, &m_formatMap, childType);}
//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
bool ECSchema::NamedElementExists(Utf8CP name) const
    {
    return (m_classMap.find(name) != m_classMap.end()) ||
        (m_enumerationMap.find(name) != m_enumerationMap.end()) ||
        (m_kindOfQuantityMap.find(name) != m_kindOfQuantityMap.end()) ||
        (m_propertyCategoryMap.find(name) != m_propertyCategoryMap.end()) ||
        (m_formatMap.find(name) != m_formatMap.end()) ||
        (nullptr != GetUnitCP(name)) ||
        (nullptr != GetPhenomenonCP(name)) ||
        (nullptr != GetUnitSystemCP(name));
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::SetVersionFromString(Utf8CP versionString)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    uint32_t versionRead;
    uint32_t versionWrite;
    uint32_t versionMinor;
    ECObjectsStatus status;
    if ((ECObjectsStatus::Success != (status = ParseVersionString (versionRead, versionWrite, versionMinor, versionString))) ||
        (ECObjectsStatus::Success != (status = this->SetVersionRead (versionRead))) ||
        (ECObjectsStatus::Success != (status = this->SetVersionWrite(versionWrite))) ||
        (ECObjectsStatus::Success != (status = this->SetVersionMinor (versionMinor))))
        return status;
    else
        return ECObjectsStatus::Success;
    }

//-------------------------------------------------------------------------------------//
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+----//
ECObjectsStatus ECSchema::SetECVersion(ECVersion ecVersion)
    {
    ECObjectsStatus status = ECObjectsStatus::Success;

    switch (ecVersion)
        {
        case ECVersion::V2_0:
        case ECVersion::V3_0:
        case ECVersion::V3_1:
        case ECVersion::V3_2:
            m_ecVersion = ecVersion;
            break;
        default:
            return ECObjectsStatus::InvalidECVersion;
        }

    return status;
    }

//-------------------------------------------------------------------------------------//
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+----//
ECObjectsStatus ECSchema::CreateSchema(ECSchemaPtr& schemaOut, Utf8StringCR schemaName, Utf8StringCR alias, uint32_t versionRead, uint32_t versionWrite, uint32_t versionMinor, ECVersion ecVersion)
    {
    schemaOut = new ECSchema();

    ECObjectsStatus status;
    ECVersion originalXmlVersion = ecVersion;
    if (ECVersion::V3_0 >= ecVersion)
        originalXmlVersion = ECVersion::V3_1;

    if (ECObjectsStatus::Success != (status = schemaOut->ParseECVersion(schemaOut->m_originalECXmlVersionMajor, schemaOut->m_originalECXmlVersionMinor, originalXmlVersion)) ||
        ECObjectsStatus::Success != (status = schemaOut->SetName(schemaName)) ||
        ECObjectsStatus::Success != (status = schemaOut->SetAlias(alias)) ||
        ECObjectsStatus::Success != (status = schemaOut->SetVersionRead (versionRead)) ||
        ECObjectsStatus::Success != (status = schemaOut->SetVersionWrite(versionWrite)) ||
        ECObjectsStatus::Success != (status = schemaOut->SetVersionMinor (versionMinor)) ||
        ECObjectsStatus::Success != (status = schemaOut->SetECVersion (ecVersion)))
        {
        schemaOut = nullptr;
        return status;
        }

    return ECObjectsStatus::Success;
    }

template<typename Item, typename ReferencedItem>
ECObjectsStatus logCopyErrorDueToReferencedItem(Item const* item, ECSchemaCP targetSchema, ReferencedItem const* refItem, Utf8CP refItemTypeName, ECSchemaCP searchedRefSchema, bool copyReferences)
    {
    LOG.errorv("Could not copy '%s:%s' into '%s' because the referenced %s '%s' was not found in the target copy of the schema '%s'.  Copy References was set to '%s'.",
        item->GetSchema().GetFullSchemaName().c_str(), item->GetName().c_str(), targetSchema->GetFullSchemaName().c_str(), refItemTypeName, refItem->GetName().c_str(),
        searchedRefSchema->GetFullSchemaName().c_str(), copyReferences ? "true" : "false");
    return ECObjectsStatus::NotFound;
    }

// Gets the existing reference matching possibleReference or adds possible reference.  Always returns a reference to the actual schema reference relative to the 'schema' object
ECObjectsStatus getOrAddReferencedSchema(ECSchemaP schema, ECSchemaCR possibleReference, ECSchemaP& actualReference, bool willCopyRef)
    {
    if (willCopyRef || schema->GetName().Equals(possibleReference.GetName().c_str()))
        {
        actualReference = schema;
        return ECObjectsStatus::Success;
        }

    auto referencedSchemas = schema->GetReferencedSchemas();
    auto schemaRef = referencedSchemas.Find(possibleReference.GetSchemaKey(), SchemaMatchType::Latest);
    bool referenceNeeded = schemaRef == referencedSchemas.end();
    if (referenceNeeded)
        {
        ECObjectsStatus status = schema->AddReferencedSchema(const_cast<ECSchemaR>(possibleReference));
        actualReference = const_cast<ECSchemaP> (&possibleReference);
        return status;
        }

    actualReference = schemaRef->second.get();
    return ECObjectsStatus::Success;
    }

template<typename NamedItemA, typename NamedItemB>
bool areFromSameSchema (NamedItemA const* itemA, NamedItemB const* itemB)
    {
    if (nullptr == itemA || nullptr == itemB) return false;
    return itemA->GetSchema().GetName().Equals(itemB->GetSchema().GetName().c_str());
    }

template<typename Item, typename RefItem>
ECObjectsStatus ECSchema::GetOrCopyReferencedItemForCopy(const Item & itemWithRef, RefItem *& refForCopy, RefItem const* startingRef, bool copyReferences, ECSchemaElementType refItemType, RefItem *(ECSchema::*getItemP)(Utf8CP), ECObjectsStatus(ECSchema::*copyItem)(RefItem *&, const RefItem &, bool, Utf8CP))
    {
    bool willCopyRef = copyReferences && areFromSameSchema(&itemWithRef, startingRef);
    ECSchemaP refSchema;
    ECObjectsStatus status = getOrAddReferencedSchema(this, startingRef->GetSchema(), refSchema, willCopyRef);
    if (ECObjectsStatus::Success != status)
        return status;

    refForCopy = (refSchema->*getItemP)(startingRef->GetName().c_str());
    if (nullptr == refForCopy)
        {
        if (willCopyRef)
            status = (this->*copyItem)(refForCopy, *startingRef, copyReferences, nullptr);
        else
            status = logCopyErrorDueToReferencedItem(&itemWithRef, this, startingRef, SchemaParseUtils::SchemaElementTypeToString(refItemType), refSchema, copyReferences);
        }
    return status;
    }

ECObjectsStatus ECSchema::GetOrCopyReferencedClassForCopy(ECClassCR classWithRef, ECClassP& refForCopy, ECClassCP startingRef, bool copyReferences)
    {
    return GetOrCopyReferencedItemForCopy(classWithRef, refForCopy, startingRef, copyReferences, ECSchemaElementType::ECClass, &ECSchema::GetClassP, &ECSchema::CopyClass);
    }
ECObjectsStatus ECSchema::GetOrCopyReferencedEnumerationForCopy(ECClassCR classWithRef, ECEnumerationP& refForCopy, ECEnumerationCP startingRef, bool copyReferences)
    {
    return GetOrCopyReferencedItemForCopy(classWithRef, refForCopy, startingRef, copyReferences, ECSchemaElementType::ECEnumeration, &ECSchema::GetEnumerationP, &ECSchema::CopyEnumeration);
    }
ECObjectsStatus ECSchema::GetOrCopyReferencedKindOfQuantityForCopy(ECClassCR classWithRef, KindOfQuantityP& refForCopy, KindOfQuantityCP startingRef, bool copyReferences)
    {
    return GetOrCopyReferencedItemForCopy(classWithRef, refForCopy, startingRef, copyReferences, ECSchemaElementType::KindOfQuantity, &ECSchema::GetKindOfQuantityP, &ECSchema::CopyKindOfQuantity);
    }
ECObjectsStatus ECSchema::GetOrCopyReferencedPropertyCategoryForCopy(ECClassCR classWithRef, PropertyCategoryP& refForCopy, PropertyCategoryCP startingRef, bool copyReferences)
    {
    return GetOrCopyReferencedItemForCopy(classWithRef, refForCopy, startingRef, copyReferences, ECSchemaElementType::PropertyCategory, &ECSchema::GetPropertyCategoryP, &ECSchema::CopyPropertyCategory);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECSchema::CopyClass(ECClassP& targetClass, ECClassCR sourceClass, bool copyReferences, Utf8CP newName)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;
    Utf8String targetClassName((newName == nullptr ? sourceClass.GetName().c_str() : newName));
    // Make sure the name is not already used
    if (NamedElementExists(targetClassName.c_str()))
        return ECObjectsStatus::NamedItemAlreadyExists;

    ECObjectsStatus status = ECObjectsStatus::Success;
    switch (sourceClass.GetClassType())
        {
        case ECClassType::Relationship:
            { // Brackets are needed to get around compiler error, C2360, where the initializer of an identifier cannot be skipped by any case
            // since the variable lives through the entire switch/case statement. Need to put in a scope block to make it explicit.
            ECRelationshipClassCP sourceAsRelationshipClass = sourceClass.GetRelationshipClassCP();
            ECRelationshipClassP newRelationshipClass;
            status = this->CreateRelationshipClass(newRelationshipClass, targetClassName);
            if (ECObjectsStatus::Success == status)
                {
                newRelationshipClass->SetStrength(sourceAsRelationshipClass->GetStrength());
                newRelationshipClass->SetStrengthDirection(sourceAsRelationshipClass->GetStrengthDirection());

                status = sourceAsRelationshipClass->GetSource().CopyTo(newRelationshipClass->GetSource(), copyReferences);
                if (ECObjectsStatus::Success == status)
                    status = sourceAsRelationshipClass->GetTarget().CopyTo(newRelationshipClass->GetTarget(), copyReferences);

                if (ECObjectsStatus::Success != status)
                    {
                    DeleteClass(*newRelationshipClass);
                    newRelationshipClass = nullptr;
                    }
                }
            targetClass = newRelationshipClass;
            break;
            }
        case ECClassType::Struct:
            ECStructClassP newStructClass;
            status = CreateStructClass(newStructClass, targetClassName);
            targetClass = newStructClass;
            break;
        case ECClassType::CustomAttribute:
            ECCustomAttributeClassP newCAClass;
            status = CreateCustomAttributeClass(newCAClass, targetClassName);
            if (ECObjectsStatus::Success == status)
                newCAClass->SetContainerType(sourceClass.GetCustomAttributeClassCP()->GetContainerType());
            targetClass = newCAClass;
            break;
        case ECClassType::Entity:
            ECEntityClassP newEntityClass;
            status = CreateEntityClass(newEntityClass, targetClassName);
            targetClass = newEntityClass;
            break;
        default:
            LOG.errorv("Unable to copy class '%s' into '%s' because the class type is not handled", sourceClass.GetFullName(), GetFullSchemaName().c_str());
            status = ECObjectsStatus::ECClassNotSupported;
            break;
        }

    if (ECObjectsStatus::Success != status)
        return status;

    if (sourceClass.GetIsDisplayLabelDefined())
        targetClass->SetDisplayLabel(sourceClass.GetInvariantDisplayLabel().c_str());
    targetClass->SetDescription(sourceClass.GetInvariantDescription().c_str());
    targetClass->SetClassModifier(sourceClass.GetClassModifier());

    // Set the base classes on the target class from the source class
    // This is inconsistent with the Managed implementation of CopyClass which does not copy base classes
    for (ECClassP baseClass: sourceClass.GetBaseClasses())
        {
        ECClassP targetBaseClass = nullptr;
        if (ECObjectsStatus::Success == (status = GetOrCopyReferencedClassForCopy(sourceClass, targetBaseClass, baseClass, copyReferences)))
            {
            // Not validating the class to be added since it should already be valid schema. Also this avoids some of the inheritance rule checking
            // for Mixins and Relationships
            status = targetClass->_AddBaseClass(*targetBaseClass, false, false, false);
            }
        if (ECObjectsStatus::Success != status)
            {
            DeleteClass(*targetClass);
            targetClass = nullptr;
            return status;
            }
        }

    for(ECPropertyCP sourceProperty: sourceClass.GetProperties(false))
        {
        ECPropertyP destProperty;
        status = targetClass->CopyProperty(destProperty, sourceProperty, sourceProperty->GetName().c_str(), true, true, copyReferences);
        if (ECObjectsStatus::Success != status)
            {
            DeleteClass(*targetClass);
            targetClass = nullptr;
            return status;
            }
        }

    return sourceClass.CopyCustomAttributesTo(*targetClass, copyReferences);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::CopyEnumeration(ECEnumerationP& targetEnumeration, ECEnumerationCR sourceEnumeration, bool copyReferences, Utf8CP newName)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    ECObjectsStatus status;
    status = CreateEnumeration(targetEnumeration, (newName == nullptr ? sourceEnumeration.GetName().c_str() : newName), sourceEnumeration.GetType());
    if (ECObjectsStatus::Success != status)
        return status;

    if (sourceEnumeration.GetIsDisplayLabelDefined())
        targetEnumeration->SetDisplayLabel(sourceEnumeration.GetInvariantDisplayLabel().c_str());

    targetEnumeration->SetDescription(sourceEnumeration.GetInvariantDescription().c_str());
    targetEnumeration->SetIsStrict(sourceEnumeration.GetIsStrict());

    for (auto sourceEnumerator : sourceEnumeration.GetEnumerators())
        {
        ECEnumeratorP targetEnumerator;
        if (PrimitiveType::PRIMITIVETYPE_Integer == targetEnumeration->GetType())
            targetEnumeration->CreateEnumerator(targetEnumerator, sourceEnumerator->GetName().c_str(), sourceEnumerator->GetInteger());
        else
            targetEnumeration->CreateEnumerator(targetEnumerator, sourceEnumerator->GetName().c_str(), sourceEnumerator->GetString().c_str());

        if (sourceEnumerator->GetIsDisplayLabelDefined())
            targetEnumerator->SetDisplayLabel(sourceEnumerator->GetInvariantDisplayLabel().c_str());

        targetEnumerator->SetDescription(sourceEnumerator->GetInvariantDescription().c_str());
        }

    return ECObjectsStatus::Success;
    }

ECObjectsStatus ECSchema::TryFillKindOfQuantity(KindOfQuantityP targetKOQ, KindOfQuantityCR sourceKOQ, bool copyReferences)
    {
    ECObjectsStatus status;
    if (sourceKOQ.GetIsDisplayLabelDefined())
        targetKOQ->SetDisplayLabel(sourceKOQ.GetInvariantDisplayLabel().c_str());

    targetKOQ->SetDescription(sourceKOQ.GetInvariantDescription().c_str());
    targetKOQ->SetRelativeError(sourceKOQ.GetRelativeError());

    ECUnitCP persistUnit = sourceKOQ.GetPersistenceUnit();
    if (nullptr != persistUnit)
        {
        ECUnitP targetPersistUnit = nullptr;
        status = GetOrCopyReferencedItemForCopy(sourceKOQ, targetPersistUnit, persistUnit, copyReferences, ECSchemaElementType::Unit, &ECSchema::GetUnitP, &ECSchema::CopyUnit);
        if (ECObjectsStatus::Success != status)
            return status;

        targetKOQ->SetPersistenceUnit(*targetPersistUnit);
        }

    if (sourceKOQ.HasPresentationFormats())
        {
        for (NamedFormatCR format : sourceKOQ.GetPresentationFormats())
            {
            ECFormatCP parentFormat = format.GetParentFormat();
            ECFormatP targetParentFormat = nullptr;
            status = GetOrCopyReferencedItemForCopy(sourceKOQ, targetParentFormat, parentFormat, copyReferences, ECSchemaElementType::Format, &ECSchema::GetFormatP, &ECSchema::CopyFormat);
            if (ECObjectsStatus::Success != status)
                return status;

            NamedFormat newFormat = format;
            newFormat.SetParentFormat(targetParentFormat);
            targetKOQ->AddPresentationFormatInternal(newFormat);
            }
        }
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECSchema::CopyKindOfQuantity(KindOfQuantityP& targetKOQ, KindOfQuantityCR sourceKOQ, bool copyReferences, Utf8CP newName)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    ECObjectsStatus status;
    status = CreateKindOfQuantity(targetKOQ, (newName == nullptr ? sourceKOQ.GetName().c_str() : newName));
    if (ECObjectsStatus::Success != status)
        return status;

    status = TryFillKindOfQuantity(targetKOQ, sourceKOQ, copyReferences);
    if (ECObjectsStatus::Success != status)
        {
        DeleteKindOfQuantity(*targetKOQ);
        return status;
        }

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECSchema::CopyPropertyCategory(PropertyCategoryP& targetPropCategory, PropertyCategoryCR sourcePropCategory, bool copyReferences, Utf8CP newName)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    ECObjectsStatus status;
    status = CreatePropertyCategory(targetPropCategory, (newName == nullptr ?  sourcePropCategory.GetName().c_str() : newName));
    if (ECObjectsStatus::Success != status)
        return status;

    if (sourcePropCategory.GetIsDisplayLabelDefined())
        targetPropCategory->SetDisplayLabel(sourcePropCategory.GetInvariantDisplayLabel().c_str());

    targetPropCategory->SetDescription(sourcePropCategory.GetInvariantDescription().c_str());
    targetPropCategory->SetPriority(sourcePropCategory.GetPriority());

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECSchema::CopyUnitSystem(UnitSystemP& targetUnitSystem, UnitSystemCR sourceUnitSystem, bool copyReferences, Utf8CP newName)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    ECObjectsStatus status;
    status = CreateUnitSystem(targetUnitSystem, (newName == nullptr ? sourceUnitSystem.GetName().c_str() : newName));

    if (ECObjectsStatus::Success != status)
        return status;

    if (sourceUnitSystem.GetIsDisplayLabelDefined())
        targetUnitSystem->SetDisplayLabel(sourceUnitSystem.GetInvariantDisplayLabel().c_str());

    if (sourceUnitSystem.GetIsDescriptionDefined())
        targetUnitSystem->SetDescription(sourceUnitSystem.GetDescription());

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// NOTE: Does not handle implicit references added by the definition string.
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECSchema::CopyPhenomenon(PhenomenonP& targetPhenom, PhenomenonCR sourcePhenom, bool copyReferences, Utf8CP newName)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    ECObjectsStatus status;
    status = CreatePhenomenon(targetPhenom, (newName == nullptr ? sourcePhenom.GetName().c_str() : newName), sourcePhenom.GetDefinition().c_str());

    if (ECObjectsStatus::Success != status)
        return status;

    if (sourcePhenom.GetIsDisplayLabelDefined())
        targetPhenom->SetDisplayLabel(sourcePhenom.GetInvariantDisplayLabel().c_str());

    if (sourcePhenom.GetIsDescriptionDefined())
        targetPhenom->SetDescription(sourcePhenom.GetInvariantDescription().c_str());

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// NOTE: This method does not attempt to copy or add references for the units referenced by the sourceUnits definition
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECSchema::CopyUnit(ECUnitP& targetUnit, ECUnitCR sourceUnit, bool copyReferences, Utf8CP newName)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    targetUnit = nullptr;
    Utf8CP name = (newName == nullptr ? sourceUnit.GetName().c_str() : newName);
    if (NamedElementExists(name)) return ECObjectsStatus::NamedItemAlreadyExists;

    ECObjectsStatus status;

    PhenomenonCP phenom = sourceUnit.GetPhenomenon();
    PhenomenonP targetPhenom = nullptr;
    status = GetOrCopyReferencedItemForCopy(sourceUnit, targetPhenom, phenom, copyReferences, ECSchemaElementType::Phenomenon, &ECSchema::GetPhenomenonP, &ECSchema::CopyPhenomenon);
    if (ECObjectsStatus::Success != status)
        return status;

    if (sourceUnit.IsConstant())
        {
        status = CreateConstant(targetUnit, name, sourceUnit.GetDefinition().c_str(), *targetPhenom, sourceUnit.GetNumerator());
        }
    else
        {
        if (!sourceUnit.HasUnitSystem())
            {
            // Checks on construction of the source unit should keep this from ever happening.
            LOG.errorv ("Cannot copy unit '%s' because it does not have a unit system defined and all units besides constants must define a unit system", sourceUnit.GetName().c_str());
            return ECObjectsStatus::InvalidUnit;
            }

        UnitSystemCP system = nullptr;
        UnitSystemP targetSystem = nullptr;
        system = sourceUnit.GetUnitSystem();
        status = GetOrCopyReferencedItemForCopy(sourceUnit, targetSystem, system, copyReferences, ECSchemaElementType::UnitSystem, &ECSchema::GetUnitSystemP, &ECSchema::CopyUnitSystem);
        if (ECObjectsStatus::Success != status)
            return status;

        if (sourceUnit.IsInvertedUnit())
            {
            ECUnitCP parent = sourceUnit.GetInvertingUnit();
            ECUnitP targetParent = nullptr;
            status = GetOrCopyReferencedItemForCopy(sourceUnit, targetParent, parent, copyReferences, ECSchemaElementType::Unit, &ECSchema::GetUnitP, &ECSchema::CopyUnit);
            if (ECObjectsStatus::Success != status)
                return status;

            status = CreateInvertedUnit(targetUnit, *targetParent, name, *targetSystem);
            }
        else // Normal unit
            {
            status = CreateUnit(targetUnit, name, sourceUnit.GetDefinition().c_str(), *targetPhenom, *targetSystem);
            }
        }

    if (ECObjectsStatus::Success != status)
        return status;

    if (sourceUnit.HasNumerator())
        targetUnit->SetNumerator(sourceUnit.GetNumerator());

    if (sourceUnit.HasDenominator())
        targetUnit->SetDenominator(sourceUnit.GetDenominator());

    if (sourceUnit.HasOffset())
        targetUnit->SetOffset(sourceUnit.GetOffset());

    if (sourceUnit.GetIsDisplayLabelDefined())
        targetUnit->SetDisplayLabel(sourceUnit.GetInvariantDisplayLabel().c_str());

    if (sourceUnit.GetIsDescriptionDefined())
        targetUnit->SetDescription(sourceUnit.GetInvariantDescription().c_str());

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECSchema::CopyFormat(ECFormatP& targetFormat, ECFormatCR sourceFormat, bool copyReferences, Utf8CP newName)
    {
    if (m_immutable) return ECObjectsStatus::SchemaIsImmutable;

    ECObjectsStatus status = CreateFormat(targetFormat, (newName == nullptr ? sourceFormat.GetName().c_str() : newName));
    if (ECObjectsStatus::Success != status)
        return status;
    if (sourceFormat.GetIsDisplayLabelDefined())
        targetFormat->SetDisplayLabel(sourceFormat.GetInvariantDisplayLabel().c_str());
    if (sourceFormat.GetIsDescriptionDefined())
        targetFormat->SetDescription(sourceFormat.GetInvariantDescription().c_str());
    if (sourceFormat.HasNumeric())
        targetFormat->SetNumericSpec(*sourceFormat.GetNumericSpec());
    if (sourceFormat.HasComposite())
        {
        auto sourceComp = sourceFormat.GetCompositeSpec();
        const auto getTargetUnit = [&](Units::UnitCP source) -> Units::UnitCP
            {
            if (nullptr == source)
                return nullptr;
            ECUnitCP unit = (ECUnitCP)source;
            ECUnitP targetUnit = nullptr;
            status = GetOrCopyReferencedItemForCopy(sourceFormat, targetUnit, unit, copyReferences, ECSchemaElementType::Unit, &ECSchema::GetUnitP, &ECSchema::CopyUnit);
            if (ECObjectsStatus::Success != status)
                return nullptr;

            return targetUnit;
            };
        auto comp = Formatting::CompositeValueSpec(*getTargetUnit(sourceComp->GetMajorUnit()),
                                                    *getTargetUnit(sourceComp->GetMiddleUnit()),
                                                    *getTargetUnit(sourceComp->GetMinorUnit()),
                                                    *getTargetUnit(sourceComp->GetSubUnit()));
        if (ECObjectsStatus::Success != status)
            return status;

        comp.SetSpacer(sourceComp->GetSpacer().c_str());
        comp.SetIncludeZero(sourceComp->IsIncludeZero());
        if (sourceComp->HasMajorLabel())
            comp.SetMajorLabel(sourceComp->GetMajorLabel());
        if (sourceComp->HasMiddleLabel())
            comp.SetMiddleLabel(sourceComp->GetMiddleLabel());
        if (sourceComp->HasMinorLabel())
            comp.SetMinorLabel(sourceComp->GetMinorLabel());
        if (sourceComp->HasSubLabel())
            comp.SetSubLabel(sourceComp->GetSubLabel());

        targetFormat->SetCompositeSpec(comp);
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::CopySchema(ECSchemaPtr& schemaOut, IECSchemaLocaterP schemaLocater) const
    {
    ECObjectsStatus status = ECObjectsStatus::Success;
    status = CreateSchema(schemaOut,  GetName().c_str(), GetAlias().c_str(), GetVersionRead(), GetVersionWrite(), GetVersionMinor(), m_ecVersion);
    if (ECObjectsStatus::Success != status)
        return status;

    // We intentionally do not copy original ECXML version because ... (maybe write something here or just don't, not sure)
    // schemaOut->SetOriginalECXmlVersion(GetOriginalECXmlVersionMajor(), GetOriginalECXmlVersionMinor());

    schemaOut->SetDescription(GetInvariantDescription().c_str());
    if (GetIsDisplayLabelDefined())
        schemaOut->SetDisplayLabel(GetInvariantDisplayLabel().c_str());

    ECSchemaReferenceListCR referencedSchemas = GetReferencedSchemas();
    if(schemaLocater == nullptr)
        {
        for (ECSchemaReferenceList::const_iterator iter = referencedSchemas.begin(); iter != referencedSchemas.end(); ++iter)
            schemaOut->AddReferencedSchema(*iter->second.get());
        }
    else
        {
        ECSchemaReadContextPtr schemaContext = ECSchemaReadContext::CreateContext(false, true);
        for (auto& ref : GetReferencedSchemas())
            {
            SchemaKey desiredKey(ref.first);
            ECSchemaPtr referencedSchema = schemaLocater->LocateSchema(desiredKey, SchemaMatchType::Latest, *schemaContext);
            if(referencedSchema == nullptr)
                return ECObjectsStatus::SchemaNotFound;

            schemaOut->AddReferencedSchema(*referencedSchema);
            }
        }

    for (auto ecEnumeration : m_enumerationContainer)
        {
        ECEnumerationP copyEnumeration;
        status = schemaOut->CopyEnumeration(copyEnumeration, *ecEnumeration);
        if (ECObjectsStatus::Success != status && ECObjectsStatus::NamedItemAlreadyExists != status)
            return status;
        }

    for (auto propertyCategory : m_propertyCategoryContainer)
        {
        PropertyCategoryP copyPropertyCategory;
        status = schemaOut->CopyPropertyCategory(copyPropertyCategory, *propertyCategory);
        if (ECObjectsStatus::Success != status && ECObjectsStatus::NamedItemAlreadyExists != status)
            return status;
        }

    for (auto system : m_unitsContext.m_unitSystemContainer)
        {
        UnitSystemP copySystem;
        status = schemaOut->CopyUnitSystem(copySystem, *system);
        if (ECObjectsStatus::Success != status && ECObjectsStatus::NamedItemAlreadyExists != status)
            return status;
        }

    for (auto phenom : m_unitsContext.m_phenomenonContainer)
        {
        PhenomenonP copyPhenom;
        status = schemaOut->CopyPhenomenon(copyPhenom, *phenom, true);
        if (ECObjectsStatus::Success != status && ECObjectsStatus::NamedItemAlreadyExists != status)
            return status;
        }

    for (auto unit : m_unitsContext.m_unitContainer)
        {
        ECUnitP copyUnit;
        status = schemaOut->CopyUnit(copyUnit, *unit, true);
        if (ECObjectsStatus::Success != status && ECObjectsStatus::NamedItemAlreadyExists != status)
            return status;
        }

    for (auto format : m_formatContainer)
        {
        ECFormatP copyFormat;
        status = schemaOut->CopyFormat(copyFormat, *format, true);
        if (ECObjectsStatus::Success != status && ECObjectsStatus::NamedItemAlreadyExists != status)
            return status;
        }

    for (auto koq : m_kindOfQuantityContainer)
        {
        KindOfQuantityP copyKOQ;
        status = schemaOut->CopyKindOfQuantity(copyKOQ, *koq, true);
        if (ECObjectsStatus::Success != status && ECObjectsStatus::NamedItemAlreadyExists != status)
            return status;
        }

    for (ECClassP ecClass : m_classContainer)
        {
        ECClassP copyClass;
        status = schemaOut->CopyClass(copyClass, *ecClass, true);
        if (ECObjectsStatus::Success != status && ECObjectsStatus::NamedItemAlreadyExists != status)
            return status;
        }

    return CopyCustomAttributesTo(*schemaOut, true);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
ECSchemaP ECSchema::GetSchemaPByAlias(Utf8StringCR alias)
    {
    if (alias.empty())
        return this;

    // lookup referenced schema by alias
    bmap<ECSchemaP, Utf8String>::const_iterator schemaIterator;
    for (schemaIterator = m_referencedSchemaAliasMap.begin(); schemaIterator != m_referencedSchemaAliasMap.end(); schemaIterator++)
        {
        if (alias.EqualsIAscii(schemaIterator->second))
            return schemaIterator->first;
        }

    return nullptr;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
ECSchemaCP ECSchema::GetSchemaByAliasP(Utf8StringCR alias) const
    {
    if (alias.empty())
        return this;

    // lookup referenced schema by alias
    bmap<ECSchemaP, Utf8String>::const_iterator schemaIterator;
    for (schemaIterator = m_referencedSchemaAliasMap.begin(); schemaIterator != m_referencedSchemaAliasMap.end(); schemaIterator++)
        {
        if (alias.EqualsIAscii(schemaIterator->second))
            return schemaIterator->first;
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::ResolveAlias(ECSchemaCR schema, Utf8StringR alias) const
    {
    alias = EMPTY_STRING;
    if (&schema == this)
        return ECObjectsStatus::Success;

    if (schema.GetSchemaKey() == GetSchemaKey())
        return ECObjectsStatus::Success;

    bmap<ECSchemaP, Utf8String>::const_iterator schemaIterator = m_referencedSchemaAliasMap.find(const_cast<ECSchemaP>(&schema));
    if (schemaIterator != m_referencedSchemaAliasMap.end())
        {
        alias = schemaIterator->second;
        return ECObjectsStatus::Success;
        }

    bmap<ECSchemaP, Utf8String>::const_iterator aliasIterator;
    for (aliasIterator = m_referencedSchemaAliasMap.begin(); aliasIterator != m_referencedSchemaAliasMap.end(); aliasIterator++)
        {
        if (aliasIterator->first->GetSchemaKey() == schema.GetSchemaKey())
            {
            alias = aliasIterator->second;
            return ECObjectsStatus::Success;
            }
        }
    return ECObjectsStatus::SchemaNotFound;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
ECClassCP ECSchema::LookupClass(Utf8CP name, bool useFullName) const
    {
    Utf8String aliasOrSchemaName;
    Utf8String unqualifiedName;
    if (ECObjectsStatus::Success != ECClass::ParseClassName(aliasOrSchemaName, unqualifiedName, name))
        return nullptr;
    // Have to check !useFullName for checking alias because of the possibility of someone naming their schema the same as this schema's alias
    // Without this the lookup would occur on this schema instead of the referenced schema it was supposed to.
    if (aliasOrSchemaName.empty() || (!useFullName && aliasOrSchemaName.EqualsIAscii(GetAlias())) || (useFullName && aliasOrSchemaName.EqualsIAscii(GetName())))
        return GetClassCP(unqualifiedName.c_str());

    if (useFullName)
        {
        for (const auto& s : GetReferencedSchemas())
            {
            if (aliasOrSchemaName.EqualsIAscii(s.second->GetName()))
                return s.second->GetClassCP(unqualifiedName.c_str());
            }
        return nullptr;
        }

    ECSchemaCP resolvedUnitSchema = GetSchemaByAliasP(aliasOrSchemaName);
    return (nullptr != resolvedUnitSchema ? resolvedUnitSchema->GetClassCP(unqualifiedName.c_str()) : nullptr);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
KindOfQuantityCP ECSchema::LookupKindOfQuantity(Utf8CP name, bool useFullName) const
    {
    Utf8String aliasOrSchemaName;
    Utf8String unqualifiedName;
    if (ECObjectsStatus::Success != ECClass::ParseClassName(aliasOrSchemaName, unqualifiedName, name))
        return nullptr;
    // Have to check !useFullName for checking alias because of the possibility of someone naming their schema the same as this schema's alias
    // Without this the lookup would occur on this schema instead of the referenced schema it was supposed to.
    if (aliasOrSchemaName.empty() || (!useFullName && aliasOrSchemaName.EqualsIAscii(GetAlias())) || (useFullName && aliasOrSchemaName.EqualsIAscii(GetName())))
        return GetKindOfQuantityCP(unqualifiedName.c_str());

    if (useFullName)
        {
        for (const auto& s : GetReferencedSchemas())
            {
            if (aliasOrSchemaName.EqualsIAscii(s.second->GetName()))
                return s.second->GetKindOfQuantityCP(unqualifiedName.c_str());
            }
        return nullptr;
        }

    ECSchemaCP resolvedUnitSchema = GetSchemaByAliasP(aliasOrSchemaName);
    return (nullptr != resolvedUnitSchema ? resolvedUnitSchema->GetKindOfQuantityCP(unqualifiedName.c_str()) : nullptr);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
ECEnumerationCP ECSchema::LookupEnumeration(Utf8CP name, bool useFullName) const
    {
    Utf8String aliasOrSchemaName;
    Utf8String unqualifiedName;
    if (ECObjectsStatus::Success != ECClass::ParseClassName(aliasOrSchemaName, unqualifiedName, name))
        return nullptr;
    // Have to check !useFullName for checking alias because of the possibility of someone naming their schema the same as this schema's alias
    // Without this the lookup would occur on this schema instead of the referenced schema it was supposed to.
    if (aliasOrSchemaName.empty() || (!useFullName && aliasOrSchemaName.EqualsIAscii(GetAlias())) || (useFullName && aliasOrSchemaName.EqualsIAscii(GetName())))
        return GetEnumerationCP(unqualifiedName.c_str());

    if (useFullName)
        {
        for (const auto& s : GetReferencedSchemas())
            {
            if (aliasOrSchemaName.EqualsIAscii(s.second->GetName()))
                return s.second->GetEnumerationCP(unqualifiedName.c_str());
            }
        return nullptr;
        }

    ECSchemaCP resolvedUnitSchema = GetSchemaByAliasP(aliasOrSchemaName);
    return (nullptr != resolvedUnitSchema ? resolvedUnitSchema->GetEnumerationCP(unqualifiedName.c_str()) : nullptr);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
PropertyCategoryCP ECSchema::LookupPropertyCategory(Utf8CP name, bool useFullName) const
    {
    Utf8String aliasOrSchemaName;
    Utf8String unqualifiedName;
    if (ECObjectsStatus::Success != ECClass::ParseClassName(aliasOrSchemaName, unqualifiedName, name))
        return nullptr;
    // Have to check !useFullName for checking alias because of the possibility of someone naming their schema the same as this schema's alias
    // Without this the lookup would occur on this schema instead of the referenced schema it was supposed to.
    if (aliasOrSchemaName.empty() || (!useFullName && aliasOrSchemaName.EqualsIAscii(GetAlias())) || (useFullName && aliasOrSchemaName.EqualsIAscii(GetName())))
        return GetPropertyCategoryCP(unqualifiedName.c_str());

    if (useFullName)
        {
        for (const auto& s : GetReferencedSchemas())
            {
            if (aliasOrSchemaName.EqualsIAscii(s.second->GetName()))
                return s.second->GetPropertyCategoryCP(unqualifiedName.c_str());
            }
        return nullptr;
        }

    ECSchemaCP resolvedUnitSchema = GetSchemaByAliasP(aliasOrSchemaName);
    return (nullptr != resolvedUnitSchema ? resolvedUnitSchema->GetPropertyCategoryCP(unqualifiedName.c_str()) : nullptr);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
template<typename T>
ECObjectsStatus ECSchema::DeleteUnitType(T& child)
    {
    // NOTE: This is probably a bad idea. Need to make a copy of the name, because it will be destructed in the following call to
    // SchemaUnitContext. However, it will still be needed after to make sure it is removed from serialization order.
    Utf8String name = child.GetName().c_str();

    ECObjectsStatus status = m_unitsContext.Delete<T>(child);
    if (ECObjectsStatus::Success != status)
        return status;

    m_serializationOrder.RemoveElement(name.c_str());
    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
UnitSystemP ECSchema::GetUnitSystemP(Utf8CP name)
    {
    return m_unitsContext.Get<UnitSystem>(name);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
ECObjectsStatus ECSchema::DeleteUnitSystem(UnitSystemR unitSystem)
    {
    return DeleteUnitType<UnitSystem>(unitSystem);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
PhenomenonP ECSchema::GetPhenomenonP(Utf8CP name)
    {
    return m_unitsContext.Get<Phenomenon>(name);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
ECObjectsStatus ECSchema::DeletePhenomenon(PhenomenonR phenomenon)
    {
    return DeleteUnitType<Phenomenon>(phenomenon);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
ECUnitP ECSchema::GetUnitP(Utf8CP name)
    {
    return m_unitsContext.Get<ECUnit>(name);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
ECFormatCP ECSchema::LookupFormat(Utf8CP name, bool useFullName) const
    {
    Utf8String aliasOrSchemaName;
    Utf8String unqualifiedName;
    if (ECObjectsStatus::Success != ECClass::ParseClassName(aliasOrSchemaName, unqualifiedName, name))
        return nullptr;
    // Have to check !useFullName for checking alias because of the possibility of someone naming their schema the same as this schema's alias
    // Without this the lookup would occur on this schema instead of the referenced schema it was supposed to.
    if (aliasOrSchemaName.empty() || (!useFullName && aliasOrSchemaName.EqualsIAscii(GetAlias())) || (useFullName && aliasOrSchemaName.EqualsIAscii(GetName())))
        return GetFormatCP(unqualifiedName.c_str());

    if (useFullName)
        {
        for (const auto& s : GetReferencedSchemas())
            {
            if (aliasOrSchemaName.EqualsIAscii(s.second->GetName()))
                return s.second->GetFormatCP(unqualifiedName.c_str());
            }
        return nullptr;
        }

    ECSchemaCP resolvedUnitSchema = GetSchemaByAliasP(aliasOrSchemaName);
    return (nullptr != resolvedUnitSchema ? resolvedUnitSchema->GetFormatCP(unqualifiedName.c_str()) : nullptr);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
ECObjectsStatus ECSchema::DeleteUnit(ECUnitR unit)
    {
    return DeleteUnitType<ECUnit>(unit);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECUnitP ECSchema::GetInvertedUnitP(Utf8CP name)
    {
    auto unit = GetUnitP(name);
    if((nullptr != unit) && (unit->IsInvertedUnit()))
        return unit;
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECUnitP ECSchema::GetConstantP(Utf8CP name)
    {
    auto unit = GetUnitP(name);
    if((nullptr != unit) && (unit->IsConstant()))
        return unit;
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::AddReferencedSchema(ECSchemaR refSchema, Utf8StringCR alias)
    {
    ECSchemaReadContext context(nullptr, false, false);
    return AddReferencedSchema(refSchema, alias, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::AddReferencedSchema(ECSchemaR refSchema, Utf8StringCR alias, ECSchemaReadContextR readContext)
    {
    // disallow adding a supplemental schema as a referenced schema
    if (refSchema.IsSupplementalSchema())
        {
        LOG.warningv("%s is trying to add %s as a referenced schema.  Supplemental schemas are not allowed to be referenced.  Ignoring this reference.", this->GetFullSchemaName().c_str(), refSchema.GetFullSchemaName().c_str());
        return ECObjectsStatus::Success; // returning success even though we didn't add it because this should not cause the entire serialization to fail
        }

    SchemaKeyCR refSchemaKey = refSchema.GetSchemaKey();

    if (GetSchemaKey() == refSchemaKey)
        return ECObjectsStatus::SchemaHasReferenceCycle;

    if (m_refSchemaList.end () != m_refSchemaList.find (refSchemaKey))
        return ECObjectsStatus::NamedItemAlreadyExists;

    Utf8String tmpAlias(alias);
    if (tmpAlias.length() == 0)
        tmpAlias = "s";

    // Make sure the alias is unique within this schema
    bmap<ECSchemaP, Utf8String>::const_iterator aliasIterator;
    for (aliasIterator = m_referencedSchemaAliasMap.begin(); aliasIterator != m_referencedSchemaAliasMap.end(); aliasIterator++)
        {
        if (0 == tmpAlias.compare (aliasIterator->second))
            {
            break;
            }
        }

    // We found a matching alias already being referenced
    if (aliasIterator != m_referencedSchemaAliasMap.end())
        {
        int subScript;
        for (subScript = 1; subScript < 500; subScript++)
            {
            Utf8Char temp[256];
            BeStringUtilities::Snprintf(temp, "%s%d", tmpAlias.c_str(), subScript);
            Utf8String tryAlias(temp);
            for (aliasIterator = m_referencedSchemaAliasMap.begin(); aliasIterator != m_referencedSchemaAliasMap.end(); aliasIterator++)
                {
                if (0 == tryAlias.compare (aliasIterator->second))
                    {
                    break;
                    }
                }
            // we didn't find the alias in the map
            if (aliasIterator == m_referencedSchemaAliasMap.end())
                {
                tmpAlias = tryAlias;
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

    m_referencedSchemaAliasMap.insert(bpair<ECSchemaP, const Utf8String> (&refSchema, tmpAlias));
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECSchema::RemoveReferencedSchema(SchemaKeyCR schemaKey, SchemaMatchType matchType, bool dropReferences)
    {
    ECSchemaReferenceListCR schemas = GetReferencedSchemas();
    auto schemaIt = schemas.Find(schemaKey, matchType);
    return schemaIt != schemas.end() ? RemoveReferencedSchema(*schemaIt->second) : ECObjectsStatus::SchemaNotFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchema::RemoveReferencedSchema(ECSchemaR refSchema, bool dropReferences)
    {
    ECSchemaReferenceList::iterator schemaIterator = m_refSchemaList.find (refSchema.GetSchemaKey());
    if (schemaIterator == m_refSchemaList.end())
        return ECObjectsStatus::SchemaNotFound;

    ECSchemaPtr refSchemaPtr = schemaIterator->second;

    const auto HandleCustomAttribute = [&](IECCustomAttributeContainerP container, IECInstancePtr ca) -> ECObjectsStatus
        {
        if (ca->GetClass().GetSchema().GetSchemaKey() == refSchemaPtr->GetSchemaKey())
            {
            if (dropReferences)
                {
                container->RemoveCustomAttribute(ca->GetClass());
                }
            else
                return ECObjectsStatus::SchemaInUse;
            }
        return ECObjectsStatus::Success;
        };

    // Check for referenced schema in custom attribute
    for (auto ca : GetCustomAttributes(false))
        if (ca->GetClass().GetSchema().GetSchemaKey() == refSchemaPtr->GetSchemaKey())
            {
            auto handleAttrStatus = HandleCustomAttribute(this, ca);
            if (ECObjectsStatus::Success != handleAttrStatus)
                return handleAttrStatus;
            }

    const auto unitUsesSchema = [&](ECUnitCP unit) -> bool
        {
        if (nullptr == unit)
            return false;
        if (unit->GetSchema().GetSchemaKey() == refSchemaPtr->GetSchemaKey())
            return true;
        if (((nullptr != unit->GetPhenomenon()) && (unit->GetPhenomenon()->GetSchema().GetSchemaKey() == refSchemaPtr->GetSchemaKey()))
            || ((nullptr != unit->GetUnitSystem()) && (unit->GetUnitSystem()->GetSchema().GetSchemaKey() == refSchemaPtr->GetSchemaKey())))
            return true;
        return false;
        };

    const auto formatUsesSchema = [&](NamedFormatCP format) -> bool
        {
        if (nullptr == format)
            return false;
        if ((nullptr != format->GetParentFormat()) && format->GetParentFormat()->GetSchema().GetSchemaKey() == refSchemaPtr->GetSchemaKey())
            return true;
        if (!format->HasComposite())
            return false;
        if (unitUsesSchema((ECUnitCP)format->GetCompositeMajorUnit())
            || unitUsesSchema((ECUnitCP)format->GetCompositeMiddleUnit())
            || unitUsesSchema((ECUnitCP)format->GetCompositeMinorUnit())
            || unitUsesSchema((ECUnitCP)format->GetCompositeSubUnit()))
            return true;
        return false;
        };
    
    const auto kindOfQuantityUsesSchema = [&](KindOfQuantityCP koq) -> bool
        {
        if (nullptr == koq)
            return false;
        if (koq->GetSchema().GetSchemaKey() == refSchemaPtr->GetSchemaKey())
            return true;
        if ((nullptr != koq->GetPersistenceUnit()) && unitUsesSchema(koq->GetPersistenceUnit()))
            return true;
        if (!koq->GetPresentationFormats().empty())
            {
            for (auto format : koq->GetPresentationFormats())
                {
                if (formatUsesSchema(&format))
                    return true;
                }
            }
        return false;
        };

    // check for classes referencing it
    for (ECClassP ecClass: GetClasses())
        {
        std::vector<ECClassP> baseClassesToRemove;
        // First, check each base class to see if the base class uses that schema
        for (ECClassP baseClass: ecClass->GetBaseClasses())
            {
            if (baseClass->GetSchema().GetSchemaKey() == refSchemaPtr->GetSchemaKey())
                {
                if (dropReferences)
                    baseClassesToRemove.push_back(baseClass);
                else
                    return ECObjectsStatus::SchemaInUse;
                }
            }
        if (dropReferences)
            {
            for (ECClassP baseClass : baseClassesToRemove)
                {
                ECObjectsStatus status = ecClass->RemoveBaseClass(*baseClass);
                if (ECObjectsStatus::Success != status)
                    return status;
                }
            }

        if (ecClass->IsEntityClass())
            {
            ECEntityClassCP appliesToClass = ecClass->GetEntityClassCP()->GetAppliesToClass();
            if (nullptr != appliesToClass && appliesToClass->GetSchema().GetSchemaKey() == refSchemaPtr->GetSchemaKey())
                {
                if (dropReferences)
                    {
                    auto setAppliesToStatus = ecClass->GetEntityClassP()->SetAppliesToClass(nullptr);
                    if (ECObjectsStatus::Success != setAppliesToStatus)
                        return setAppliesToStatus;
                    }
                else
                    return ECObjectsStatus::SchemaInUse;
                }
            }

        for (auto ca : ecClass->GetCustomAttributes(false))
            if (ca->GetClass().GetSchema().GetSchemaKey() == refSchemaPtr->GetSchemaKey())
                {
                auto handleAttrStatus = HandleCustomAttribute(ecClass, ca);
                if (ECObjectsStatus::Success != handleAttrStatus)
                    return handleAttrStatus;
                }

        // If it is a relationship class, check the constraints to make sure the constraints don't use that schema
        ECRelationshipClassP relClass = ecClass->GetRelationshipClassP();
        if (nullptr != relClass)
            {
            for (auto* constraint : std::array<ECRelationshipConstraintP, 2>({&relClass->GetTarget(), &relClass->GetSource()}))
                {
                for (auto ca : constraint->GetCustomAttributes(false))
                    if (ca->GetClass().GetSchema().GetSchemaKey() == refSchemaPtr->GetSchemaKey())
                        {
                        auto handleAttrStatus = HandleCustomAttribute(constraint, ca);
                        if (ECObjectsStatus::Success != handleAttrStatus)
                            return handleAttrStatus;
                        }

                if (constraint->IsAbstractConstraintDefined())
                    {
                    if (constraint->GetAbstractConstraint()->GetSchema().GetSchemaKey() == refSchemaPtr->GetSchemaKey())
                        {
                        if (dropReferences)
                            constraint->RemoveAbstractConstraint();
                        else
                            return ECObjectsStatus::SchemaInUse;
                        }
                    }
                std::vector<ECClassCP> constraintsToRemove;
                for (auto constraintClass : constraint->GetConstraintClasses())
                    {
                    if (constraintClass->GetSchema().GetSchemaKey() == refSchemaPtr->GetSchemaKey())
                        {
                        if (dropReferences)
                            constraintsToRemove.push_back(constraintClass);
                        else
                            return ECObjectsStatus::SchemaInUse;
                        }
                    }
                if (dropReferences)
                    {
                    for (ECClassCP constraintClass : constraintsToRemove)
                        {
                        ECObjectsStatus status = constraint->RemoveClass(*constraintClass);
                        if (ECObjectsStatus::Success != status)
                            return status;
                        }
                    }
                }
            }

        std::vector<ECPropertyP> propertiesToDelete;
        // And make sure that there are no struct types from another schema
        for (ECPropertyP prop: ecClass->GetProperties(false))
            {
            // Check Custom Attributes before checking property type
            for (auto ca : prop->GetCustomAttributes(false))
                if (ca->GetClass().GetSchema().GetSchemaKey() == refSchemaPtr->GetSchemaKey())
                    {
                    auto handleAttrStatus = HandleCustomAttribute(prop, ca);
                    if (ECObjectsStatus::Success != handleAttrStatus)
                        return handleAttrStatus;
                    }

            if (prop->IsCategoryDefinedLocally())
                {
                if (prop->GetCategory()->GetSchema().GetSchemaKey() == refSchemaPtr->GetSchemaKey())
                    {
                    if (dropReferences)
                        {
                        auto setCategoryStatus = prop->SetCategory(nullptr);
                        if (ECObjectsStatus::Success != setCategoryStatus)
                            return setCategoryStatus;
                        }
                    else
                        return ECObjectsStatus::SchemaInUse;
                    }
                }
            if (prop->IsKindOfQuantityDefinedLocally())
                {
                if ((nullptr != prop->GetKindOfQuantity()) && kindOfQuantityUsesSchema(prop->GetKindOfQuantity()))
                    {
                    if (dropReferences)
                        {
                        auto removeKoqStatus = prop->SetKindOfQuantity(nullptr);
                        if (ECObjectsStatus::Success != removeKoqStatus)
                            return removeKoqStatus;
                        }
                    else
                        return ECObjectsStatus::SchemaInUse;
                    }
                }

            ECEnumerationCP enumeration;
            if (prop->GetIsPrimitive())
                {
                PrimitiveECPropertyCP primProp = prop->GetAsPrimitiveProperty();
                enumeration = primProp->GetEnumeration();
                }
            else if (prop->GetIsPrimitiveArray())
                {
                PrimitiveArrayECPropertyCP primArrayProp = prop->GetAsPrimitiveArrayProperty();
                enumeration = primArrayProp->GetEnumeration();
                }
            else
                {
                enumeration = nullptr;
                }
            
            if (nullptr != enumeration && enumeration->GetSchema().GetSchemaKey() == refSchemaPtr->GetSchemaKey())
                {
                if (dropReferences)
                    propertiesToDelete.push_back(prop);
                else
                    return ECObjectsStatus::SchemaInUse;
                }

            ECClassCP typeClass = nullptr;
            if (prop->GetIsStruct())
                {
                typeClass = &(prop->GetAsStructProperty()->GetType());
                }
            else if (prop->GetIsStructArray())
                {
                typeClass = &(prop->GetAsStructArrayProperty()->GetStructElementType());
                }
            else if (prop->GetIsNavigation())
                {
                typeClass = prop->GetAsNavigationProperty()->GetRelationshipClass();
                }
            if (nullptr == typeClass)
                continue;

            if (typeClass->GetSchema().GetSchemaKey() == refSchemaPtr->GetSchemaKey())
                {
                if (dropReferences)
                    propertiesToDelete.push_back(prop);
                else
                    return ECObjectsStatus::SchemaInUse;
                }
            }

            if (dropReferences)
                {
                for (auto propertyToDelete : propertiesToDelete)
                    {
                    if (nullptr != propertyToDelete)
                        {
                        auto removeEnumerationStatus = ecClass->DeleteProperty(*propertyToDelete);
                        if (ECObjectsStatus::Success != removeEnumerationStatus)
                            return removeEnumerationStatus;
                        }
                    }
                }
        }
    
    std::vector<KindOfQuantityP> koqToDelete;
    for (auto koq : GetKindOfQuantities())
        {
        ECUnitCP persUnit = koq->GetPersistenceUnit();
        if (nullptr != persUnit && persUnit->GetSchema().GetSchemaKey() == refSchemaPtr->GetSchemaKey())
            {
            if (dropReferences)
                koqToDelete.push_back(koq);
            else
                return ECObjectsStatus::SchemaInUse;
            }

        for (auto f : koq->GetPresentationFormats())
            {
            if (formatUsesSchema(&f))
                {
                if (dropReferences)
                    koq->RemovePresentationFormat(f);
                else
                    return ECObjectsStatus::SchemaInUse;
                }
            }
        }
    
    if (dropReferences)
        {
        for (auto koq : koqToDelete)
            {
            auto deleteKoqStatus = DeleteKindOfQuantity(*koq);
            if (ECObjectsStatus::Success != deleteKoqStatus)
                return deleteKoqStatus;
            }
        }

    std::vector<ECUnitP> unitsToDelete;
    for (auto unit : GetUnits())
        {
        if (unitUsesSchema(unit))
            {
            if (dropReferences)
                unitsToDelete.push_back(unit);
            else
                return ECObjectsStatus::SchemaInUse;
            }
        }

    if (dropReferences)
        {
        for (auto unit : unitsToDelete)
            {
            auto deleteUnitStatus = DeleteUnit(*unit);
            if (ECObjectsStatus::Success != deleteUnitStatus)
                return deleteUnitStatus;
            }
        }

    std::vector<ECFormatP> formatsToDelete;
    for (auto format : GetFormats())
        {
        if (formatUsesSchema(format))
            {
            if (dropReferences)
                formatsToDelete.push_back(format);
            else
                return ECObjectsStatus::SchemaInUse;
            }
        }

    if (dropReferences)
        {
        for (auto format : formatsToDelete)
            {
            auto deleteFormatStatus = DeleteFormat(*format);
            if (ECObjectsStatus::Success != deleteFormatStatus)
                return deleteFormatStatus;
            }
        }
    
    m_refSchemaList.erase(schemaIterator);
    bmap<ECSchemaP, Utf8String>::iterator iterator = m_referencedSchemaAliasMap.find(&refSchema);
    if (iterator != m_referencedSchemaAliasMap.end())
        m_referencedSchemaAliasMap.erase(iterator);

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int ECSchema::RemoveUnusedSchemaReferences()
    {
    bvector<SchemaKey> refSchemaKeys;
    for (auto refSchema : GetReferencedSchemas())
        refSchemaKeys.push_back(refSchema.first);

    int numRemovedSchema = 0;
    for (auto key : refSchemaKeys)
        {
        if (ECObjectsStatus::Success == RemoveReferencedSchema(key))
            ++numRemovedSchema;
        }
    return numRemovedSchema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchema::SetSupplementalSchemaInfo(SupplementalSchemaInfo* info)
    {
    m_supplementalSchemaInfo = info;
    if (nullptr == info)
        this->RemoveCustomAttribute(SupplementalSchemaInfo::GetCustomAttributeSchemaName(),
                                    SupplementalSchemaInfo::GetCustomAttributeAccessor());
    else
        {
        IECInstancePtr attribute = info->CreateCustomAttribute();
        if (attribute.IsValid())
            {
            auto& coreCA = attribute->GetClass().GetSchema();
            if (!ECSchema::IsSchemaReferenced(*this, coreCA))
                this->AddReferencedSchema(const_cast<ECSchemaR>(coreCA));
            this->SetCustomAttribute(*attribute);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr ECSchema::LocateSchema(SchemaKeyR key, ECSchemaReadContextR schemaContext)
    {
    return schemaContext.LocateSchema(key, SchemaMatchType::LatestWriteCompatible);
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
static void AddFilePathToSchemaPaths(ECSchemaReadContextR schemaContext, WCharCP ecSchemaXmlFile)
    {
    BeFileName pathToThisSchema (BeFileName::DevAndDir, ecSchemaXmlFile);
    schemaContext.AddSchemaPath(pathToThisSchema);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
static ECSchemaPtr ParseHeaderAndLocateSchema(WCharCP schemaXmlFile, ECSchemaReadContextR schemaContext, const SchemaMatchType matchType, SchemaReadStatus* outStatus)
    {
    pugi::xml_document xmlDoc;
    pugi::xml_parse_result result = xmlDoc.load_file(schemaXmlFile);
    if(!result)
    {
        BeAssert(s_noAssert);
        LOG.errorv ("Error loading XML file %ls: %s (error at char %d)", schemaXmlFile, result.description(), result.offset);
        return nullptr;
    }

    SchemaKey searchKey;
    uint32_t ecXmlMajorVersion, ecXmlMinorVersion;
    pugi::xml_node schemaNode;
    if (SchemaReadStatus::Success != SchemaXmlReader::ReadSchemaStub(searchKey, ecXmlMajorVersion, ecXmlMinorVersion, schemaNode, xmlDoc))
        return nullptr;

    ECSchemaPtr schema = schemaContext.LocateSchema(searchKey, matchType);
    if (!schema.IsValid())
        {
        const auto status = ECSchema::ReadFromXmlFile(schema, schemaXmlFile, schemaContext);
        if (outStatus != nullptr)
            *outStatus = status;
        }
    else {
        if (outStatus != nullptr)
            *outStatus = SchemaReadStatus::Success;
        }
    return schema;
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECSchemaPtr ECSchema::LocateSchema(WCharCP schemaXmlFile, ECSchemaReadContextR schemaContext, const SchemaMatchType matchType, SchemaReadStatus* readStatus)
    {
    return ParseHeaderAndLocateSchema(schemaXmlFile, schemaContext, matchType, readStatus);
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECSchemaPtr ECSchema::LocateSchema(Utf8CP schemaXmlFile, ECSchemaReadContextR schemaContext, const SchemaMatchType matchType, SchemaReadStatus* readStatus)
    {
    return ParseHeaderAndLocateSchema(WString(schemaXmlFile, BentleyCharEncoding::Utf8).c_str(), schemaContext, matchType, readStatus);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SearchPathSchemaFileLocater::TryLoadingSupplementalSchemas(Utf8StringCR schemaName, WStringCR schemaFilePath, ECSchemaReadContextR schemaContext, bvector<ECSchemaP>& supplementalSchemas)
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
        ECSchemaPtr schemaOut = nullptr;

        if (SchemaReadStatus::Success != ECSchema::ReadFromXmlFile (schemaOut, fileName, schemaContext))
            continue;
        supplementalSchemas.push_back(schemaOut.get());
        }

    //The first file filter already finds files with 3 digits, this one would return them a second time
    /*BeFileName schemaPath2(schemaFilePath.c_str());
    filter.AssignUtf8(schemaName.c_str());
    filter += L"_Supplemental_*.*.*.*.ecschema.xml";
    schemaPath2.AppendToPath(filter.c_str());
    BeFileListIterator fileList2(schemaPath2.GetName(), false);
    BeFileName filePath2;
    while (SUCCESS == fileList2.GetNextFileName(filePath2))
        {
        WCharCP     fileName = filePath2.GetName();
        ECSchemaPtr schemaOut = NULL;

        if (SchemaReadStatus::Success != ECSchema::ReadFromXmlFile(schemaOut, fileName, schemaContext))
            continue;
        supplementalSchemas.push_back(schemaOut.get());
        }*/

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SearchPathSchemaFileLocater::SearchPathSchemaFileLocater(bvector<WString> const& searchPaths, bool includeFilesWithNoVerExt) : m_searchPaths(searchPaths), m_includeFilesWithNoVersionExt(includeFilesWithNoVerExt) {};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SearchPathSchemaFileLocater::~SearchPathSchemaFileLocater () {};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SearchPathSchemaFileLocaterPtr SearchPathSchemaFileLocater::CreateSearchPathSchemaFileLocater(bvector<WString> const& searchPaths, bool includeFilesWithNoVerExt)
    {
    return new SearchPathSchemaFileLocater(searchPaths, includeFilesWithNoVerExt);
    }

void SearchPathSchemaFileLocater::AddCandidateSchemas(bvector<CandidateSchema>& foundFiles, WStringCR schemaPath, WStringCR fileFilter, SchemaKeyR desiredSchemaKey, SchemaMatchType matchType, ECSchemaReadContextCR schemaContext)
    {
    BeFileName fileExpression(schemaPath.c_str());
    fileExpression.AppendToPath(fileFilter.c_str());

    BeFileListIterator  fileList(fileExpression.c_str(), false);
    BeFileName          filePath;

    while (SUCCESS == fileList.GetNextFileName(filePath))
        {
        Utf8String fileName(filePath.GetFileNameWithoutExtension());

        SchemaKey key;
        if (SchemaKey::ParseSchemaFullName(key, fileName.c_str()) != ECObjectsStatus::Success)
            {
            LOG.warningv(L"Failed to parse schema file name %s. Skipping that file.", fileName.c_str());
            continue;
            }

        SchemaKey ciKey(Utf8String(key.GetName().c_str()).ToLower().c_str(), key.GetVersionRead(), key.GetVersionWrite(), key.GetVersionMinor());
        SchemaKey ciDesiredSchemaKey(Utf8String(desiredSchemaKey.GetName().c_str()).ToLower().c_str(), desiredSchemaKey.GetVersionRead(), desiredSchemaKey.GetVersionWrite(), desiredSchemaKey.GetVersionMinor());
        //If key matches, OR the legacy compatible match evaluates true
        if (ciKey.Matches(ciDesiredSchemaKey, matchType) ||
            (schemaContext.m_acceptLegacyImperfectLatestCompatibleMatch && (matchType == SchemaMatchType::LatestWriteCompatible || matchType == SchemaMatchType::LatestReadCompatible) &&
             0 == ciKey.m_schemaName.CompareTo(ciDesiredSchemaKey.m_schemaName) && key.m_versionRead == desiredSchemaKey.m_versionRead))
            {
            foundFiles.push_back(CandidateSchema());
            auto& candidate = foundFiles.back();
            candidate.FileName = filePath;
            candidate.Key = key;
            candidate.SearchPath = schemaPath;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SearchPathSchemaFileLocater::AddCandidateNoExtensionSchema(bvector<CandidateSchema>& foundFiles, WStringCR schemaPath, Utf8CP schemaName, SchemaKeyR desiredSchemaKey, SchemaMatchType matchType, ECSchemaReadContextCR schemaContext)
    {
    BeAssert(m_includeFilesWithNoVersionExt && "Should be called only when no-extension schemas are to be examined");

    BeFileName schemaPathname(schemaPath.c_str());
    schemaPathname.AppendSeparator();
    schemaPathname.AppendUtf8(schemaName);
    schemaPathname.AppendUtf8(".ecschema.xml");

    if (!schemaPathname.DoesPathExist())
        return;

    pugi::xml_document xmlDoc;
    pugi::xml_parse_result result = xmlDoc.load_file(schemaPathname.GetNameUtf8().c_str());
    if(!result)
    {
        BeAssert(s_noAssert);
        LOG.warningv(L"Failed to read schema from %ls: %s", schemaPathname.c_str(), result.description());
        return;
    }

    SchemaKey key;
    uint32_t ecXmlMajorVersion, ecXmlMinorVersion;
    pugi::xml_node schemaNode;
    if (SchemaReadStatus::Success != SchemaXmlReader::ReadSchemaStub(key, ecXmlMajorVersion, ecXmlMinorVersion, schemaNode, xmlDoc))
        {
        LOG.warningv(L"Failed to read schema version from %ls", schemaPathname.c_str());
        return;
        }

    SchemaKey ciKey(Utf8String(key.GetName().c_str()).ToLower().c_str(), key.GetVersionRead(), key.GetVersionWrite(), key.GetVersionMinor());
    SchemaKey ciDesiredSchemaKey(Utf8String(desiredSchemaKey.GetName().c_str()).ToLower().c_str(), desiredSchemaKey.GetVersionRead(), desiredSchemaKey.GetVersionWrite(), desiredSchemaKey.GetVersionMinor());
    //If key matches, OR the legacy compatible match evaluates true
    if (ciKey.Matches(ciDesiredSchemaKey, matchType) ||
        (schemaContext.m_acceptLegacyImperfectLatestCompatibleMatch && matchType == SchemaMatchType::LatestWriteCompatible &&
        0 == ciKey.m_schemaName.CompareTo(ciDesiredSchemaKey.m_schemaName) && key.m_versionRead == desiredSchemaKey.m_versionRead))
        {
        foundFiles.push_back(CandidateSchema());
        auto& candidate = foundFiles.back();
        candidate.FileName = schemaPathname;
        candidate.Key = key;
        candidate.SearchPath = schemaPath;
        }
    }

void SearchPathSchemaFileLocater::FindEligibleSchemaFiles(bvector<CandidateSchema>& foundFiles, SchemaKeyR desiredSchemaKey, SchemaMatchType matchType, ECSchemaReadContextCR schemaContext)
    {
    Utf8CP schemaName = desiredSchemaKey.m_schemaName.c_str();
    WString twoVersionExpression;
    WString threeVersionExpression;
    twoVersionExpression.AssignUtf8(schemaName);
    threeVersionExpression.AssignUtf8(schemaName);

    Utf8String twoVersionSuffix;
    Utf8String threeVersionSuffix;

    if (matchType == SchemaMatchType::Latest)
        {
        twoVersionSuffix = ".*.*.ecschema.xml";
        threeVersionSuffix = ".*.*.*.ecschema.xml";
        }
    else if (matchType == SchemaMatchType::LatestWriteCompatible)
        {
        twoVersionSuffix.Sprintf(".%02" PRIu32 ".*.ecschema.xml", desiredSchemaKey.m_versionRead);
        threeVersionSuffix.Sprintf(".%02" PRIu32 ".%02" PRIu32 ".*.ecschema.xml", desiredSchemaKey.m_versionRead, desiredSchemaKey.m_versionWrite);
        }
    else if (matchType == SchemaMatchType::LatestReadCompatible)
        {
        twoVersionSuffix.Sprintf(".%02" PRIu32 ".*.ecschema.xml", desiredSchemaKey.m_versionRead);
        threeVersionSuffix.Sprintf(".%02" PRIu32 ".*.*.ecschema.xml", desiredSchemaKey.m_versionRead);
        }
    else //MatchType_Exact
        {
        twoVersionSuffix.Sprintf(".%02" PRIu32 ".%02" PRIu32 ".ecschema.xml", desiredSchemaKey.m_versionRead, desiredSchemaKey.m_versionMinor);
        threeVersionSuffix.Sprintf(".%02" PRIu32 ".%02" PRIu32 ".%02" PRIu32 ".ecschema.xml",
                                   desiredSchemaKey.m_versionRead, desiredSchemaKey.m_versionWrite, desiredSchemaKey.m_versionMinor);
        }

    twoVersionExpression.AppendUtf8(twoVersionSuffix.c_str());
    threeVersionExpression.AppendUtf8(threeVersionSuffix.c_str());

    for (WString schemaPathStr : m_searchPaths)
        {
        //LOG.debugv("(SearchPathSchemaFileLocater) Attempting to locate schema %s in path %ls", schemaName, schemaPathStr.c_str());

        if (m_includeFilesWithNoVersionExt)
            AddCandidateNoExtensionSchema(foundFiles, schemaPathStr, schemaName, desiredSchemaKey, matchType, schemaContext);
        AddCandidateSchemas(foundFiles, schemaPathStr, twoVersionExpression, desiredSchemaKey, matchType, schemaContext);
        AddCandidateSchemas(foundFiles, schemaPathStr, threeVersionExpression, desiredSchemaKey, matchType, schemaContext);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr SearchPathSchemaFileLocater::_LocateSchema(SchemaKeyR key, SchemaMatchType matchType, ECSchemaReadContextR schemaContext)
    {
    bpair<SchemaKey, SchemaMatchType> lookup = make_bpair<SchemaKey, SchemaMatchType>(key, matchType);
    bmap<bpair<SchemaKey, SchemaMatchType>, ECSchemaPtr>::iterator iter = m_knownSchemas.find(lookup);
    if (iter != m_knownSchemas.end())
        return iter->second;

    bvector<CandidateSchema> eligibleSchemaFiles;
    FindEligibleSchemaFiles(eligibleSchemaFiles, key, matchType, schemaContext);

    size_t resultCount = eligibleSchemaFiles.size();
    if (resultCount == 0)
        {
        m_knownSchemas.Insert(lookup, nullptr);
        return nullptr;
        }

    auto& schemaToLoad = *std::max_element(eligibleSchemaFiles.begin(), eligibleSchemaFiles.end(), SchemyKeyIsLessByVersion);
    LOG.debugv(L"SearchPathSchemaFileLocater::_LocateSchema: Attempting to load schema %ls...", schemaToLoad.FileName.GetName());

    //Get cached version of the schema
    ECSchemaPtr schemaOut = schemaContext.GetFoundSchema(schemaToLoad.Key, SchemaMatchType::Exact);;
    if (schemaOut.IsValid())
        {
        m_knownSchemas.Insert(make_bpair<SchemaKey, SchemaMatchType>(key, SchemaMatchType::Exact), schemaOut);
        return schemaOut;
        }

    if (SchemaReadStatus::Success != ECSchema::ReadFromXmlFile(schemaOut, schemaToLoad.FileName.c_str(), schemaContext, false))
        {
        m_knownSchemas.Insert(lookup, nullptr);
        return nullptr;
        }

    LOG.debugv(L"Located %ls...", schemaToLoad.FileName.c_str());

    // Now check this same path for supplemental schemas
    bvector<ECSchemaP> supplementalSchemas;
    TryLoadingSupplementalSchemas(schemaToLoad.Key.m_schemaName.c_str(), schemaToLoad.SearchPath, schemaContext, supplementalSchemas);

    if (supplementalSchemas.size() > 0)
        {
        ECN::SupplementedSchemaBuilder builder;
        builder.UpdateSchema(*schemaOut, supplementalSchemas);
        }

    m_knownSchemas.Insert(lookup, schemaOut);
    return schemaOut;
    }

struct ChecksumHelper
{
    static Utf8String ComputeCheckSumForString(Utf8CP string, size_t len)
        {
        SHA1 sha1;
        return sha1((Byte const*)string, sizeof(Utf8Char) * len);
        }
    static Utf8String ComputeCheckSumForString(WCharCP string, size_t len)
        {
        SHA1 sha1;
        return sha1((Byte const*)string, sizeof(WChar) * len);
        }
    static Utf8String ComputeCheckSumForFile(WCharCP schemaFile)
        {
        Utf8String checkSum;
        BeFile file;
        if (BeFileStatus::Success != file.Open (schemaFile, BeFileAccess::Read))
            {
            BeAssert(false);
            return checkSum;
            }

        ByteStream fileContents;
        if (BeFileStatus::Success != file.ReadEntireFile(fileContents))
            {
            BeAssert(false);
            return checkSum;
            }

        SHA1 sha1;
        return sha1((Byte const*)fileContents.GetData(), fileContents.GetSize());
        }
};

/*---------------------------------------------------------------------------------**//**
 * @bsimethod
 +---------------+---------------+---------------+---------------+---------------+------*/
// static
Utf8String ECSchema::ComputeSchemaXmlStringCheckSum(Utf8CP str, size_t len)
    {
    return ChecksumHelper::ComputeCheckSumForString(str, len);
    }

 /*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECSchema::ComputeCheckSum()
    {
    Utf8String xmlStr;
    ECVersion xmlVersion;

    if (ECObjectsStatus::Success != CreateECVersion(xmlVersion, m_originalECXmlVersionMajor, m_originalECXmlVersionMinor))
        xmlVersion = ECVersion::V3_1;
    if (SchemaWriteStatus::Success != WriteToXmlString (xmlStr, xmlVersion))
        return "";

    SHA1 sha1;
    m_key.m_checksum = sha1((Byte const*)xmlStr.c_str(), sizeof(Utf8Char) * xmlStr.length());
    return m_key.m_checksum;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECSchema::ReadFromXmlFile(ECSchemaPtr& schemaOut, WCharCP ecSchemaXmlFile, ECSchemaReadContextR schemaContext, bool addFilePathAsSearchPath)
    {
    StopWatch timer(true);
    LOG.debugv (L"About to read native ECSchema from file: fileName='%ls'", ecSchemaXmlFile);
    schemaOut = nullptr;

    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(ecSchemaXmlFile);
    if(!result)
    {
        LOG.errorv ("Error loading XML file %ls: %s (error at char %d)", ecSchemaXmlFile, result.description(), result.offset);
        return SchemaReadStatus::FailedToParseXml;
    }

    if (addFilePathAsSearchPath)
        AddFilePathToSchemaPaths(schemaContext, ecSchemaXmlFile);

    SchemaXmlReader reader(schemaContext, doc);
    auto status = reader.Deserialize(schemaOut, schemaContext.GetCalculateChecksum() ? ChecksumHelper::ComputeCheckSumForFile(ecSchemaXmlFile).c_str() : nullptr);

    if (SchemaReadStatus::Success != status)
        {
        if (SchemaReadStatus::DuplicateSchema == status)
            LOG.errorv(L"Failed to read XML file: %ls.  \nSchema already loaded.  Use ECSchemaReadContext::LocateSchema to load schema", ecSchemaXmlFile);
        else
            LOG.errorv(L"Failed to read XML file: %ls", ecSchemaXmlFile);

        schemaContext.RemoveSchema(*schemaOut);
        schemaOut = nullptr;
        }
    else
        {
        //We have serialized a schema and its valid.
        timer.Stop();
        LOG.infov (L"Read (in %.4f seconds) [%3" PRIu64 " ECClasses] %ls", timer.GetElapsedSeconds(), (uint64_t) schemaOut->m_classMap.size(), ecSchemaXmlFile);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECSchema::ReadFromXmlString(ECSchemaPtr& schemaOut, Utf8CP ecSchemaXml, ECSchemaReadContextR schemaContext)
    {
    StopWatch timer(true);
    schemaOut = nullptr;
    SchemaReadStatus status = SchemaReadStatus::Success;

    size_t stringByteCount = strlen (ecSchemaXml) * sizeof(Utf8Char);

    pugi::xml_document xmldoc;
    pugi::xml_parse_result result = xmldoc.load_string(ecSchemaXml);

    if (!result)
        {
        BeAssert(false);
        LOG.errorv("Failed to read ECSchema from XML %s", result.description());
        return SchemaReadStatus::FailedToParseXml;
        }

    SchemaXmlReader reader(schemaContext, xmldoc);
    status = reader.Deserialize(schemaOut, schemaContext.GetCalculateChecksum() ? ChecksumHelper::ComputeCheckSumForString(ecSchemaXml, stringByteCount).c_str() : nullptr);

    if (SchemaReadStatus::Success != status)
        {
        Utf8Char first200Bytes[201];
        BeStringUtilities::Strncpy(first200Bytes, ecSchemaXml, 200);
        first200Bytes[200] = '\0';
        if (SchemaReadStatus::DuplicateSchema == status)
            schemaContext.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::InvalidInputData, "Failed to read XML from string(1st 200 characters approx.): %s.  \nSchema already loaded.  Use ECSchemaReadContext::LocateSchema to load schema", first200Bytes);
        else
            schemaContext.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::InvalidInputData, "Failed to read XML from string (1st 200 characters approx.): %s", first200Bytes);

        schemaContext.RemoveSchema(*schemaOut);
        schemaOut = nullptr;
        }
    else
        {
        timer.Stop();
        LOG.infov (L"Read from string (in %.4f seconds) [%3" PRIx64 " ECClasses] %s", timer.GetElapsedSeconds(),
            (uint64_t) schemaOut->m_classMap.size(), WString(schemaOut->GetFullSchemaName().c_str(), true).c_str());
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECSchema::ReadFromXmlString(ECSchemaPtr& schemaOut, WCharCP ecSchemaXml, ECSchemaReadContextR schemaContext)
    {
    StopWatch timer(true);
    schemaOut = nullptr;
    SchemaReadStatus status = SchemaReadStatus::Success;

    size_t stringSize = wcslen (ecSchemaXml) * sizeof(WChar);

    Utf8String xmlUtf8;
    if(BeStringUtilities::WCharToUtf8(xmlUtf8, ecSchemaXml) != BentleyStatus::SUCCESS)
        {
        BeAssert(false);
        LOG.error("Failed to convert schema xml from wchar to char.");
        return SchemaReadStatus::FailedToParseXml;
        }

    pugi::xml_document xmldoc;
    pugi::xml_parse_result result = xmldoc.load_string(xmlUtf8.c_str());

    if (!result)
        {
        BeAssert(false);
        LOG.errorv("Failed to read ECSchema from XML %s", result.description());
        return SchemaReadStatus::FailedToParseXml;
        }

    SchemaXmlReader reader(schemaContext, xmldoc);
    status = reader.Deserialize(schemaOut, schemaContext.GetCalculateChecksum() ? ChecksumHelper::ComputeCheckSumForString(ecSchemaXml, stringSize / sizeof(WChar)).c_str() : nullptr);

    if (SchemaReadStatus::Success != status)
        {
        WChar first200Characters[201];
PUSH_DISABLE_DEPRECATION_WARNINGS
        wcsncpy(first200Characters, ecSchemaXml, 200);
POP_DISABLE_DEPRECATION_WARNINGS
        first200Characters[200] = L'\0';
        if (SchemaReadStatus::DuplicateSchema == status)
            LOG.errorv(L"Failed to read XML from string(1st 200 characters approx.): %s.  \nSchema already loaded.  Use ECSchemaReadContext::LocateSchema to load schema", first200Characters);
        else
            {
            LOG.errorv(L"Failed to read XML from string (1st 200 characters): %ls", first200Characters);
            }
        schemaContext.RemoveSchema(*schemaOut);
        schemaOut = nullptr;
        }
    else
        {
        timer.Stop();
        LOG.infov (L"Read from string (in %.4f seconds) [%3" PRIx64 " ECClasses] %s", timer.GetElapsedSeconds(),
            (uint64_t) schemaOut->m_classMap.size(), schemaOut->GetFullSchemaName().c_str());
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchema::IsSchemaReferenced(ECSchemaCR thisSchema, ECSchemaCR potentiallyReferencedSchema)
    {
    return ECSchema::IsSchemaReferenced(thisSchema, potentiallyReferencedSchema, ECN::SchemaMatchType::Exact);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchema::IsSchemaReferenced(ECSchemaCR thisSchema, ECSchemaCR potentiallyReferencedSchema, ECN::SchemaMatchType matchType)
    {
    ECSchemaReferenceListCR referencedSchemas = thisSchema.GetReferencedSchemas();
    return referencedSchemas.end() != referencedSchemas.Find(potentiallyReferencedSchema.GetSchemaKey(), matchType);
    }

/*---------------------------------------------------------------------------------**//**
* Returns true if thisSchema directly references possiblyReferencedSchema
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool DirectlyReferences(ECSchemaCP thisSchema, ECSchemaCP possiblyReferencedSchema)
    {
    ECSchemaReferenceListCR referencedSchemas = thisSchema->GetReferencedSchemas();
    for (ECSchemaReferenceList::const_iterator it = referencedSchemas.begin(); it != referencedSchemas.end(); ++it)
        {
        if (it->second.get() == possiblyReferencedSchema)
            return true;
        }
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
static bool DependsOn(ECSchemaCP thisSchema, ECSchemaCP possibleDependency)
    {
    if (DirectlyReferences(thisSchema, possibleDependency))
        return true;

    SupplementalSchemaMetaDataPtr metaData;
    if (SupplementalSchemaMetaData::TryGetFromSchema(metaData, *possibleDependency)
        && metaData.IsValid()
        && metaData->IsForPrimarySchema(thisSchema->GetName(), 0, 0, SchemaMatchType::Latest))
        {
        return true; // possibleDependency supplements thisSchema. possibleDependency must be imported before thisSchema
        }

    // Maybe possibleDependency supplements one of my references?
    ECSchemaReferenceListCR referencedSchemas = thisSchema->GetReferencedSchemas();
    for (ECSchemaReferenceList::const_iterator it = referencedSchemas.begin(); it != referencedSchemas.end(); ++it)
        {
        if (DependsOn(it->second.get(), possibleDependency))
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void InsertSchemaInDependencyOrderedList(bvector<ECSchemaCP>& schemas, ECSchemaCP insertSchema)
    {
    if (std::find(schemas.begin(), schemas.end(), insertSchema) != schemas.end())
        return; // This (and its referenced ECSchemas) are already in the list

    for (auto rit = schemas.rbegin(); rit < schemas.rend(); ++rit)
        {
        if (DependsOn(*rit, insertSchema))
            {
            schemas.insert(rit.base(), insertSchema); // insert right after the referenced schema in the list
            return;
            }
        }

    schemas.insert(schemas.begin(), insertSchema); // insert at the beginning
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static void BuildDependencyOrderedSchemaList(bvector<ECSchemaCP>& schemas, ECSchemaCP insertSchema)
    {
    InsertSchemaInDependencyOrderedList(schemas, insertSchema);
    ECSchemaReferenceListCR referencedSchemas = insertSchema->GetReferencedSchemas();
    for (const auto& refedSchema : referencedSchemas)
        BuildDependencyOrderedSchemaList(schemas, refedSchema.second.get());
    }

/*----------------------------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+-------------------------*/
void ECSchema::SortSchemasInDependencyOrder(bvector<ECSchemaCP>& schemas)
    {
    bvector<ECSchemaCP> temp;
    for (const auto& schema : schemas)
        BuildDependencyOrderedSchemaList(temp, schema);
    std::reverse(temp.begin(), temp.end());
    schemas = temp;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECSchema::WriteToXmlString(WStringR ecSchemaXml, ECVersion ecXmlVersion) const
    {
    ecSchemaXml.clear();

    BeXmlWriterPtr xmlWriter = BeXmlWriter::Create();

    SchemaWriteStatus status;
    SchemaXmlWriter schemaWriter(*xmlWriter.get(), *this, ecXmlVersion);
    if (SchemaWriteStatus::Success != (status = schemaWriter.Serialize()))
        return status;

    xmlWriter->ToString (ecSchemaXml);

    return SchemaWriteStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECSchema::WriteToXmlString(Utf8StringR ecSchemaXml, ECVersion ecXmlVersion) const
    {
    ecSchemaXml.clear();

    BeXmlWriterPtr xmlWriter = BeXmlWriter::Create();
    xmlWriter->SetIndentation(4);

    SchemaWriteStatus status;
    SchemaXmlWriter schemaWriter(*xmlWriter.get(), *this, ecXmlVersion);
    if (SchemaWriteStatus::Success != (status = schemaWriter.Serialize()))
        return status;

    xmlWriter->ToString (ecSchemaXml);

    return SchemaWriteStatus::Success;
    }

//---------------------------------------------------------------------------------------//
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------//
// static
SchemaWriteStatus ECSchema::WriteToEC2XmlString(Utf8StringR ec2SchemaXml, ECSchemaP schemaToSerialize)
    {
    if (nullptr == schemaToSerialize)
        return SchemaWriteStatus::FailedToCreateXml;

    ECSchemaPtr schemaCopy;
    schemaToSerialize->CopySchema(schemaCopy);
    ECSchemaDownConverter::Convert(*schemaCopy);
    return schemaCopy->WriteToXmlString(ec2SchemaXml, ECVersion::V2_0);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECSchema::WriteToXmlFile(WCharCP ecSchemaXmlFile, ECVersion ecXmlVersion, bool utf16) const
    {
    BeXmlWriterPtr xmlWriter = BeXmlWriter::CreateFileWriter(ecSchemaXmlFile);
    xmlWriter->SetIndentation(4);

    SchemaWriteStatus status;
    SchemaXmlWriter schemaWriter(*xmlWriter.get(), *this, ecXmlVersion);
    if (SchemaWriteStatus::Success != (status = schemaWriter.Serialize(utf16)))
        return status;

    return SchemaWriteStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool ECSchema::WriteToJsonValue(BeJsValue ecSchemaJsonValue) const
    {
    ecSchemaJsonValue.SetNull();
    SchemaJsonWriter schemaWriter(ecSchemaJsonValue, *this);

    if (!schemaWriter.Serialize())
        return false;

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool ECSchema::WriteToJsonString(Utf8StringR ecSchemaJsonString, bool minify) const
    {
    Json::Value jsonSchema;

    if (!WriteToJsonValue(jsonSchema))
        return false;

    ecSchemaJsonString = minify ? jsonSchema.ToString() : jsonSchema.toStyledString();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchema::CollectAllSchemasInGraph(bvector<ECN::ECSchemaCP>& allSchemas, bool includeRootSchema) const
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
            iter->second->CollectAllSchemasInGraph(allSchemas, false);
            }
        else
            iter->second->CollectAllSchemasInGraph (allSchemas, true);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchema::FindAllSchemasInGraph(bvector<ECN::ECSchemaCP>& allSchemas, bool includeRootSchema) const
    {
    CollectAllSchemasInGraph (allSchemas, includeRootSchema);
    std::reverse(allSchemas.begin(), allSchemas.end());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECSchema::FindSchema(SchemaKeyCR schemaKey, SchemaMatchType matchType) const
    {
    if (this->GetSchemaKey().Matches (schemaKey, matchType))
        return this;

    ECSchemaReferenceListCR referencedSchemas = GetReferencedSchemas();
    for (ECSchemaReferenceList::const_iterator iter = referencedSchemas.begin(); iter != referencedSchemas.end(); ++iter)
        {
        ECSchemaCP schema = iter->second->FindSchema (schemaKey, matchType);
        if (nullptr != schema)
            return schema;
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECSchema::AddingSchemaCausedCycles () const
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchema::RemoveInvalidDisplayCharacters(ECSchemaR schema)
    {
    Utf8String temp;
    if (ECNameValidation::RemoveControlCharacters(temp, schema.GetDisplayLabel()))
            {
            schema.SetDisplayLabel(temp);
            }

    if(ECNameValidation::RemoveControlCharacters(temp, schema.GetDescription()))
        {
        schema.SetDescription(temp);
        }

    for (ECClassP ecClass : schema.GetClasses())
        {
        if (ECNameValidation::RemoveControlCharacters(temp, ecClass->GetDisplayLabel()))
            {
            ecClass->SetDisplayLabel(temp);
            }

        if(ECNameValidation::RemoveControlCharacters(temp, ecClass->GetDescription()))
            {
            ecClass->SetDescription(temp);
            }

        ECRelationshipClassCP relClass = ecClass->GetRelationshipClassCP();
        if (relClass != nullptr)
            {
            if (ECNameValidation::RemoveControlCharacters(temp, relClass->GetSource().GetRoleLabel()))
                {
                relClass->GetSource().SetRoleLabel(temp);
                }

            if (ECNameValidation::RemoveControlCharacters(temp, relClass->GetTarget().GetRoleLabel()))
                {
                relClass->GetTarget().SetRoleLabel(temp);
                }
            }

        for (ECPropertyP prop : ecClass->GetProperties())
            {
            if (ECNameValidation::RemoveControlCharacters(temp, prop->GetDisplayLabel()))
                {
                prop->SetDisplayLabel(temp);
                }

            if (ECNameValidation::RemoveControlCharacters(temp, prop->GetDescription()))
                {
                prop->SetDescription(temp);
                }
            }
        }
    }

/////////////////////////////////////////////////////////////////////////////////////////
// IStandaloneEnablerLocater
/////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECEnablerPtr IStandaloneEnablerLocater::LocateStandaloneEnabler(SchemaKeyCR schemaKey, Utf8CP className)
    {
    return _LocateStandaloneEnabler(schemaKey, className);
    }

DEFINE_KEY_METHOD(IStandaloneEnablerLocater)

/////////////////////////////////////////////////////////////////////////////////////////
// ECSchemaCache
/////////////////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchemaCache::AddSchema(ECSchemaR ecSchema)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECSchemaCache::DropSchema(SchemaKeyCR ecSchemaKey)
    {
    SchemaMap::iterator iter = m_schemas.find (ecSchemaKey);
    if (iter == m_schemas.end())
        return ECObjectsStatus::SchemaNotFound;

    m_schemas.erase(iter);
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaP ECSchemaCache::GetSchema(SchemaKeyCR key, SchemaMatchType matchType) const
    {
    SchemaMap::const_iterator iter;
    switch (matchType)
        {
        case SchemaMatchType::Identical:
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
        return nullptr;

    return iter->second.get();
    }

//---------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bvector<ECSchemaCP> ECSchemaCache::GetSchemas() const
    {
    bvector<ECSchemaCP> schemas;
    for (auto const& kvPair : m_schemas)
        schemas.push_back(kvPair.second.get());

    return schemas;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ECSchemaCache::GetSchemas(bvector<ECSchemaP>& schemas) const
    {
    schemas.clear();

    for (auto const& kvPair : m_schemas)
        schemas.push_back(kvPair.second.get());

    return schemas.size();
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
void ECSchemaCache::GetSupplementalSchemasFor(Utf8CP schemaName, bvector<ECSchemaP>& supplementalSchemas) const
    {
    supplementalSchemas.clear();

    Utf8String supplementalName(schemaName);
    supplementalName.append("_Supplemental");
    for (auto const& schema : m_schemas)
        {
        if (schema.first.GetName().StartsWithI(supplementalName.c_str()))
            supplementalSchemas.push_back(schema.second.get());
        }
    }

/////////////////////////////////////////////////////////////////////////////////////////
// ECSchemaElementsOrder
/////////////////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSchemaElementsOrder::CreateAlphabeticalOrder(ECSchemaCR ecSchema)
    {
    m_elementVector.clear();
    AddElements<ECEnumeration, ECEnumerationContainer>(ecSchema.GetEnumerations(), ECSchemaElementType::ECEnumeration);
    AddElements<ECClass, ECClassContainer>(ecSchema.GetClasses(), ECSchemaElementType::ECClass);
    AddElements<KindOfQuantity, KindOfQuantityContainer>(ecSchema.GetKindOfQuantities(), ECSchemaElementType::KindOfQuantity);
    AddElements<PropertyCategory, PropertyCategoryContainer>(ecSchema.GetPropertyCategories(), ECSchemaElementType::PropertyCategory);
    AddElements<UnitSystem, UnitSystemContainer>(ecSchema.GetUnitSystems(), ECSchemaElementType::UnitSystem);
    AddElements<Phenomenon, PhenomenonContainer>(ecSchema.GetPhenomena(), ECSchemaElementType::Phenomenon);
    AddElements<ECFormat, FormatContainer>(ecSchema.GetFormats(), ECSchemaElementType::Format);

    // Units are a special case because all 3 types are in the same container.
    for (const auto u: ecSchema.GetUnits())
        {
        if (nullptr == u)
            {
            BeAssert(false);
            continue;
            }
        if (u->IsConstant())
            AddElement(u->GetName().c_str(), ECSchemaElementType::Constant);
        else if (u->IsInvertedUnit())
            AddElement(u->GetName().c_str(), ECSchemaElementType::InvertedUnit);
        else
            AddElement(u->GetName().c_str(), ECSchemaElementType::Unit);
        }
    }

/////////////////////////////////////////////////////////////////////////////////////////
// SchemaKey
/////////////////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SchemaKey::FormatFullSchemaName(Utf8CP schemaName, uint32_t versionRead, uint32_t versionWrite, uint32_t versionMinor)
    {
    Utf8String versionString = FormatSchemaVersion(versionRead, versionWrite, versionMinor);
    Utf8PrintfString formattedString("%s.%s", schemaName, versionString.c_str());
    return formattedString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SchemaKey::FormatSchemaVersion (uint32_t versionRead, uint32_t versionWrite, uint32_t versionMinor)
    {
    Utf8String versionString;
    versionString.Sprintf("%02" PRIu32 ".%02" PRIu32 ".%02" PRIu32, versionRead, versionWrite, versionMinor);
    return versionString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SchemaKey::FormatLegacySchemaVersion (uint32_t versionRead, uint32_t versionMinor)
    {
    Utf8String versionString;
    versionString.Sprintf("%02" PRIu32 ".%02" PRIu32, versionRead, versionMinor);
    return versionString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String SchemaKey::FormatLegacyFullSchemaName(Utf8CP schemaName, uint32_t versionRead, uint32_t versionMinor)
    {
    Utf8PrintfString formattedString("%s.%s", schemaName, FormatLegacySchemaVersion(versionRead, versionMinor).c_str());
    return formattedString;
    }

#define ECSCHEMA_VERSION_FORMAT_EXPLANATION " Format must be either RR.mm or RR.ww.mm where RR is read version, ww is the write compatibility version and mm is minor version."
#define ECSCHEMA_VERSION_FORMAT_EXPLANATION_EC32 " Format must be RR.ww.mm where RR is read version, ww is the write compatibility version and mm is minor version."
#define ECSCHEMA_FULLNAME_FORMAT_EXPLANATION " Format must be either Name.RR.mm or Name.RR.ww.mm where RR is read version, ww is the write compatibility version and mm is minor version."

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus SchemaKey::ParseSchemaFullName (Utf8StringR schemaName, uint32_t& versionRead, uint32_t& versionWrite, uint32_t& versionMinor, Utf8CP fullName)
    {
    if (Utf8String::IsNullOrEmpty(fullName))
        return ECObjectsStatus::ParseError;

    Utf8CP firstDot = strchr (fullName, '.');
    if (nullptr == firstDot)
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

    ECObjectsStatus stat = ParseVersionString(versionRead, versionWrite, versionMinor, firstDot + 1);
    if (ECObjectsStatus::Success != stat)
        return stat;

    // There are some schemas out there that reference the non-existent Unit_Attributes.1.1 schema.  We need to deliver 1.0, which does not match our criteria
    // for LatestCompatible.
    if (0 == schemaName.CompareTo("Unit_Attributes") && 1 == versionRead && 1 == versionMinor)
        versionMinor = 0;

    return stat;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus SchemaKey::ParseVersionString (uint32_t& versionRead, uint32_t& versionWrite, uint32_t& versionMinor, Utf8CP versionString)
    {
    if(Utf8String::IsNullOrEmpty(versionString))
        return ECObjectsStatus::Success;

    bvector<Utf8String> tokens;
    BeStringUtilities::Split(versionString, ".", tokens);
    size_t digits = tokens.size();

    if (digits < 2)
        {
        LOG.errorv("Invalid ECSchema Version String: '%s' at least 2 version numbers are required!" ECSCHEMA_VERSION_FORMAT_EXPLANATION, versionString);
        return ECObjectsStatus::ParseError;
        }

    Utf8P end = nullptr;
    Utf8CP chars = tokens[0].c_str();
    versionRead = strtoul(chars, &end, 10);
    if (end == chars)
        {
        versionRead = DEFAULT_VERSION_READ;
        LOG.errorv("Invalid ECSchema Version String: '%s' The characters '%s' must be numeric!" ECSCHEMA_VERSION_FORMAT_EXPLANATION, versionString, chars);
        return ECObjectsStatus::ParseError;
        }

    chars = tokens[1].c_str();
    uint32_t second = strtoul(chars, &end, 10);
    if (end == chars)
        {
        versionWrite = DEFAULT_VERSION_WRITE;
        versionMinor = DEFAULT_VERSION_MINOR;
        LOG.errorv("Invalid ECSchema Version String: '%s' The characters '%s' must be numeric!" ECSCHEMA_VERSION_FORMAT_EXPLANATION, versionString, chars);
        return ECObjectsStatus::ParseError;
        }

    if (digits == 2)
        {
        versionWrite = DEFAULT_VERSION_WRITE;
        versionMinor = second;
        return ECObjectsStatus::Success;
        }


    chars = tokens[2].c_str();
    uint32_t third = strtoul(chars, &end, 10);
    //We have to support this case because some callers pass stuff like "1.5.ecschema.xml" to this method, which used to work
    //before 3 number versions were introduced.
    if (end == chars)
        {
        versionWrite = DEFAULT_VERSION_WRITE;
        versionMinor = second;
        }
    else
        {
        versionWrite = second;
        versionMinor = third;
        }

    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus SchemaKey::ParseVersionStringStrict(uint32_t& versionRead, uint32_t& versionWrite, uint32_t& versionMinor, Utf8CP versionString)
    {
    if (Utf8String::IsNullOrEmpty(versionString))
        return ECObjectsStatus::InvalidECVersion;
    int count = 0;
    const int maxVersionSize = 10; //If we somehow got a non terminated string the max a version can be is xx.xx.xx\0
    int iter = 0;
    while (versionString[iter] != '\0' && iter < maxVersionSize)
        {
        if (versionString[iter] == '.')
            ++count;
        ++iter;
        }

    if (count < 2)
        {
        LOG.errorv("Invalid ECSchema Version String: '%s' at least 3 version numbers are required!" ECSCHEMA_VERSION_FORMAT_EXPLANATION_EC32, versionString);
        return ECObjectsStatus::InvalidECVersion;
        }

    return ParseVersionString(versionRead, versionWrite, versionMinor, versionString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int SchemaKey::CompareByName (Utf8StringCR schemaName) const
    {
    // TFS#223524: This was added to do case-insensitive comparison, but it is being used inappropriately.
    // ECSchema names are case-sensitive. If there are particular contexts in which case should be disregarded,
    // the code that handles those contexts should do so explicitly.
    return strcmp (m_schemaName.c_str(), schemaName.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int SchemaKey::CompareByVersion(SchemaKeyCR rhs) const
    {
    SchemaKeyCR lhs = *this;

    if (lhs.m_versionRead != rhs.m_versionRead)
        return lhs.m_versionRead - rhs.m_versionRead;

    if (lhs.m_versionWrite != rhs.m_versionWrite)
        return lhs.m_versionWrite - rhs.m_versionWrite;

    return lhs.m_versionMinor - rhs.m_versionMinor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SchemaKey::LessThan (SchemaKeyCR rhs, SchemaMatchType matchType) const
    {
    switch (matchType)
        {
        case SchemaMatchType::Identical:
            {
            if (!m_checksum.empty() || !rhs.m_checksum.empty())
                return 0 > strcmp(m_checksum.c_str(), rhs.m_checksum.c_str());
            //Fall through
            }
        case SchemaMatchType::Exact:
            {
            int nameCompare = CompareByName (rhs.m_schemaName);

            if (nameCompare != 0)
                return nameCompare < 0;

            if (m_versionRead != rhs.m_versionRead)
                return m_versionRead < rhs.m_versionRead;

            if (m_versionWrite != rhs.m_versionWrite)
                return m_versionWrite < rhs.m_versionWrite;

            return m_versionMinor < rhs.m_versionMinor;
            break;
            }
        case SchemaMatchType::LatestWriteCompatible:
            {
            int nameCompare = CompareByName (rhs.m_schemaName);

            if (nameCompare != 0)
                return nameCompare < 0;

            if (m_versionRead < rhs.m_versionRead)
                return true;

            if (m_versionRead == rhs.m_versionRead)
                return m_versionWrite < rhs.m_versionWrite;

            return false;
            }
        case SchemaMatchType::LatestReadCompatible:
            {
            int nameCompare = CompareByName(rhs.m_schemaName);

            if (nameCompare != 0)
                return nameCompare < 0;

            return m_versionRead < rhs.m_versionRead;
            }
        case SchemaMatchType::Latest: //Only compare by name
            {
            return CompareByName (rhs.m_schemaName) < 0;
            }
        default:
            return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SchemaKey::Matches (SchemaKeyCR rhs, SchemaMatchType matchType) const
    {
    switch (matchType)
        {
        case SchemaMatchType::Identical:
            {
            if (!m_checksum.empty() && !rhs.m_checksum.empty())
                return 0 == strcmp(m_checksum.c_str(), rhs.m_checksum.c_str());
            //fall through
            }
        case SchemaMatchType::Exact:
            return 0 == CompareByName (rhs.m_schemaName) && m_versionRead == rhs.m_versionRead &&
                m_versionWrite == rhs.m_versionWrite && m_versionMinor == rhs.m_versionMinor;
        case SchemaMatchType::LatestReadCompatible:
            if (CompareByName(rhs.m_schemaName) != 0)
                return false;

            if (rhs.m_versionRead != m_versionRead)
                return false;

            if (m_versionWrite == rhs.m_versionWrite)
                return m_versionMinor >= rhs.m_versionMinor;

            return m_versionWrite > rhs.m_versionWrite;
        case SchemaMatchType::LatestWriteCompatible:
            return 0 == CompareByName (rhs.m_schemaName) && m_versionRead == rhs.m_versionRead &&
                m_versionWrite == rhs.m_versionWrite && m_versionMinor >= rhs.m_versionMinor;
        case SchemaMatchType::Latest:
            return 0 == CompareByName (rhs.m_schemaName);
        default:
            return false;
        }
    }

/////////////////////////////////////////////////////////////////////////////////////////
// SchemaMapExact
/////////////////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
        return nullptr != m_class;
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassP SchemaMapExact::FindClassP (ECN::SchemaNameClassNamePair const& classNamePair) const
    {
    ECClassP classInstance = nullptr;
    ECClassFinder classFinder(classNamePair, classInstance);

    SchemaMapExact::const_iterator iter = std::find_if (begin(), end(), classFinder);
    return iter == end() ? nullptr : classInstance;
    }

/////////////////////////////////////////////////////////////////////////////////////////
// QualifiedECAccessor
/////////////////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String QualifiedECAccessor::ToString() const
    {
    Utf8PrintfString str("%s:%s:%s", m_schemaName.c_str(), m_className.c_str(), m_accessString.c_str());
    return str;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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

/////////////////////////////////////////////////////////////////////////////////////////
// SchemaNameClassNamePair
/////////////////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/

bool ECNameValidation::RemoveControlCharacters(Utf8StringR cntrlCharsRemoved, Utf8StringCR cntrlCharsPresent)
    {
    cntrlCharsRemoved = cntrlCharsPresent;
    auto iter = std::remove_if(cntrlCharsRemoved.begin(), cntrlCharsRemoved.end(), [](Utf8Char c) {return iscntrl(c); });
    if (iter != cntrlCharsRemoved.end())
        {
        cntrlCharsRemoved.erase(iter, cntrlCharsRemoved.end());
        return true;
        }
    return false;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
