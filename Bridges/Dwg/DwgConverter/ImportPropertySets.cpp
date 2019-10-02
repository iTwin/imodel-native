/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "DwgImportInternal.h"
// workaround rapidjson::GenericValue::GetObject resolved to GetObjectW for RealDWG (OpenDWG has Windows.h included before UNICODE is defined)!
#if defined(DWGTOOLKIT_RealDwg) && defined(GetObject)
    #undef GetObject
#endif

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/19
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus AecPsetSchemaFactory::CreateTargetSchema ()
    {
    if (m_targetSchema.IsValid())
        return  ECObjectsStatus::Success;

    auto status = ECSchema::CreateSchema (m_targetSchema, SCHEMAName_AecPropertySets, SCHEMAAlias_AecPropertySets, 1, 0, 0);
    if (ECObjectsStatus::Success != status)
        return  status;

    status = m_targetSchema->SetDisplayLabel (SCHEMALabel_AecPropertySets);
    status = m_targetSchema->SetDescription (SCHEMADescription_AecPSets);

    // Reference schema Generic
    ECSchemaCP  refSchema = m_dgndb.Schemas().GetSchema (GENERIC_DOMAIN_NAME);
    if (nullptr != refSchema)
        status = m_targetSchema->AddReferencedSchema (const_cast<ECSchemaR>(*refSchema));
    else
        BeAssert (false && "Failed finding schema: Generic!");

    // Reference schema Dgn
    if (nullptr != (refSchema = m_dgndb.Schemas().GetSchema(BIS_ECSCHEMA_NAME)))
        status = m_targetSchema->AddReferencedSchema (const_cast<ECSchemaR>(*refSchema));
    else
        BeAssert (false && "Failed finding schema: Dgn!");

    // Reference schema BisCore
    status = ECObjectsStatus::SchemaNotFound;
    if (nullptr != (refSchema = CoreCustomAttributeHelper::GetSchema().get()))
        status = m_targetSchema->AddReferencedSchema (const_cast<ECSchemaR>(*refSchema));
    else
        BeAssert (false && "Failed finding schema: BisCore!");

    if (status != ECObjectsStatus::Success)
        return  status;

    // set dynamic schema
    auto ecInstance = CoreCustomAttributeHelper::CreateCustomAttributeInstance ("DynamicSchema");
    if (ecInstance.IsValid())
        status = m_targetSchema->SetCustomAttribute (*ecInstance);
    else
        status = ECObjectsStatus::DynamicSchemaCustomAttributeWasNotFound;

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/19
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyP AecPsetSchemaFactory::CreateECProperty (ECEntityClassP aecpsetClass, rapidjson::Value::ConstMemberIterator const& iter)
    {
    ECObjectsStatus status = ECObjectsStatus::PropertyNotFound;
    ECPropertyP     ecProp = nullptr;

    auto displayName = iter->name.GetString();
    auto ecName = DwgHelper::ValidateECNameFrom (displayName);

#ifdef DUMP_JSON
    auto strValue = BeRapidJsonUtilities::ToString(iter->value).DropQuotes ();
    LOG.debugv ("%s : %s", displayName, strValue);
#endif

    switch (iter->value.GetType())
        {
        case rapidjson::kStringType:
            {
            PrimitiveECPropertyP primEc = nullptr;
            status = aecpsetClass->CreatePrimitiveProperty (primEc, ecName, PRIMITIVETYPE_String);
            if (status == ECObjectsStatus::Success)
                ecProp = primEc;
            break;
            }
        case rapidjson::kTrueType:
        case rapidjson::kFalseType:
            {
            PrimitiveECPropertyP primEc = nullptr;
            status = aecpsetClass->CreatePrimitiveProperty (primEc, ecName, PRIMITIVETYPE_Boolean);
            if (status == ECObjectsStatus::Success)
                ecProp = primEc;
            break;
            }
        case rapidjson::kNumberType:
            {
            PrimitiveType   ecType = PrimitiveType::PRIMITIVETYPE_Integer;
            if (iter->value.IsInt64() || iter->value.IsUint64())
                ecType = PrimitiveType::PRIMITIVETYPE_Long;
            else if (iter->value.IsDouble())
                ecType = PrimitiveType::PRIMITIVETYPE_Double;

            PrimitiveECPropertyP primEc = nullptr;
            status = aecpsetClass->CreatePrimitiveProperty (primEc, ecName, ecType);
            if (status == ECObjectsStatus::Success)
                ecProp = primEc;
            break;
            }
        case rapidjson::kArrayType:
            {
            if (iter->value.GetArray().Empty())
                return  nullptr;

            PrimitiveType   ecType = PRIMITIVETYPE_Integer;
            switch (iter->value.GetArray()[0].GetType())
                {
                case rapidjson::kStringType:
                    ecType = PRIMITIVETYPE_String;
                    break;
                case rapidjson::kTrueType:
                case rapidjson::kFalseType:
                    ecType = PRIMITIVETYPE_Boolean;
                    break;
                case rapidjson::kNumberType:
                    if (iter->value.IsInt64() || iter->value.IsUint64())
                        ecType = PrimitiveType::PRIMITIVETYPE_Long;
                    else if (iter->value.IsDouble())
                        ecType = PrimitiveType::PRIMITIVETYPE_Double;
                    break;
                default:
                    return  nullptr;
                }
            PrimitiveArrayECPropertyP ecArray = nullptr;
            status = aecpsetClass->CreatePrimitiveArrayProperty (ecArray, ecName, ecType);
            if (status == ECObjectsStatus::Success)
                ecProp = ecArray;
            break;
            }
        default:
            {
            // WIP - need test case
            BeAssert (false && "Unsupported property type!");
            break;
            }
        }

    if (status == ECObjectsStatus::Success && ecProp != nullptr)
        ecProp->SetDisplayLabel (displayName);

    this->TrackPropertiesForElementMapping (displayName, iter);

    return  ecProp;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AecPsetSchemaFactory::ImportFromDictionaries (DwgDbDictionaryIteratorR iter)
    {
    if (this->CreateTargetSchema() != ECObjectsStatus::Success)
        return  BSIERROR;
    
    auto multiAspect = m_dgndb.Schemas().GetClass (BIS_ECSCHEMA_NAME, BIS_CLASS_ElementMultiAspect);
    if (multiAspect == nullptr)
        return  BSIERROR;

    for (; !iter.Done(); iter.Next())
        {
        auto dictionaryName = iter.GetName();
        auto ecclassName = DwgHelper::ValidateECNameFrom (dictionaryName);

        ECEntityClassP  aecpsetClass = nullptr;
        auto status = m_targetSchema->CreateEntityClass (aecpsetClass, ecclassName.c_str());
        if (ECObjectsStatus::Success != status)
            continue;

        aecpsetClass->AddBaseClass (*multiAspect);
        aecpsetClass->SetDisplayLabel (Utf8String(dictionaryName.c_str()));

#ifdef DUMP_JSON
        LOG.debugv ("Dumpping AecDbPropertySet %ls", dictionaryName.c_str());
#endif

        rapidjson::Document json(rapidjson::kObjectType);
        if (UtilsLib::ParseAecDbPropertySetDef(json, json.GetAllocator(), iter.GetObjectId()) == DwgDbStatus::Success)
            {
            for (rapidjson::Value::ConstMemberIterator i = json.MemberBegin(); i != json.MemberEnd(); ++i)
                this->CreateECProperty (aecpsetClass, i);
            }
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AecPsetSchemaFactory::ImportFromDwg (DwgDbDatabaseR dwg)
    {
    DwgDbDictionaryPtr  mainDictionary(dwg.GetNamedObjectsDictionaryId(), DwgDbOpenMode::ForRead);
    if (mainDictionary.OpenStatus() != DwgDbStatus::Success)
        return  BSISUCCESS;

    DwgDbObjectId   objectId;
    if (mainDictionary->GetIdAt(objectId, L"AEC_PROPERTY_SET_DEFS") != DwgDbStatus::Success)
        return  BSISUCCESS;

    DwgDbDictionaryPtr  propsetdefs(objectId, DwgDbOpenMode::ForRead);
    if (mainDictionary.OpenStatus() != DwgDbStatus::Success)
        return  BSISUCCESS;

    auto iter = propsetdefs->GetIterator ();
    if (!iter.IsValid() || !iter->IsValid())
        return  BSISUCCESS;

    this->PrepareForElementMapping ();

    return this->ImportFromDictionaries (*iter.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/19
+---------------+---------------+---------------+---------------+---------------+------*/
void AecPsetSchemaFactory::TrackPropertiesForElementMapping (Utf8StringCR propName, rapidjson::Value::ConstMemberIterator const& iter)
    {
    // check if the property name is in the user property name list - this test alone shall filter out most properties
    if (m_userPropertyNames.find(propName) == m_userPropertyNames.end())
        return;

    // only if both the property name & the default value match the user input, the user target class name will be remapped:
    auto defaultValue = BeRapidJsonUtilities::ToString(iter->value).DropQuotes ();
    if (defaultValue.empty())
        return;

    auto &userElementMap = m_importer.GetElementClassMapR ();
    auto found = std::find_if(userElementMap.begin(), userElementMap.end(), [&](DwgImporter::ElementClassMap& map)
        {
        return defaultValue.EqualsI(map.GetAecPropertyValue());
        });
    if (found != userElementMap.end())
        m_foundUserClassNames.insert (found->GetTargetClassName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/19
+---------------+---------------+---------------+---------------+---------------+------*/
void AecPsetSchemaFactory::PrepareForElementMapping ()
    {
    // for performance reason, build a property name set from the user property name list used for element class mapping
    m_userPropertyNames.clear ();
    m_foundUserClassNames.clear ();

    // raw user mapping table read from the config file:
    auto& elementMapList = m_importer.GetElementClassMapR ();

    // create a unique property name list:
    for (auto map : elementMapList)
        m_userPropertyNames.insert (map.GetAecPropertyName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   AecPsetSchemaFactory::CreateUserElementClasses ()
    {
    if (!m_targetSchema.IsValid() || m_foundUserClassNames.empty())
        return  BSISUCCESS;

    ECClassCP   physicalBase = nullptr;

    /*-----------------------------------------------------------------------------------
    The default importer maps modelspace entities to sealed GenericPhysicalObject or GraphicDrawing
    element classes.  Remap them to user element classes, which will be based on an abstract class, 
    PhysicalElement or GraphicElement2d.  If an extended importer maps to a different element class
    change it PhysicalElement based user class.
    -----------------------------------------------------------------------------------*/
    DwgDbBlockTableRecordPtr modelspace(m_importer.GetDwgDb().GetModelspaceId(), DwgDbOpenMode::ForRead);
    if (modelspace.OpenStatus() == DwgDbStatus::Success)
        {
        physicalBase = m_dgndb.Schemas().GetClass (m_importer._GetElementType (*modelspace));
        if (physicalBase != nullptr)
            {
            auto name = physicalBase->GetName ();
            if (name.Equals(BIS_CLASS_DrawingGraphic))
                physicalBase = m_dgndb.Schemas().GetClass (BIS_ECSCHEMA_NAME, BIS_CLASS_GraphicalElement2d);
            else
                physicalBase = m_dgndb.Schemas().GetClass (BIS_ECSCHEMA_NAME, BIS_CLASS_PhysicalElement);
            }
        }
    if (physicalBase == nullptr)
        return  BSIERROR;

    // create user element classes
    for (auto userClassName : m_foundUserClassNames)
        {
        ECEntityClassP  elementClass = nullptr;
        auto ecclassName = DwgHelper::ValidateECNameFrom (userClassName);

        auto status = m_targetSchema->CreateEntityClass (elementClass, ecclassName);
        if (ECObjectsStatus::Success == status)
            {
            elementClass->AddBaseClass (*physicalBase);
            elementClass->SetDisplayLabel (userClassName.c_str());
            }
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/19
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaPtr DwgImporter::_CreateAecPropertySetSchema ()
    {
    ECSchemaPtr aecpsetSchema;

    AecPsetSchemaFactory factory(aecpsetSchema, *this);
    auto status = factory.ImportFromDwg (this->GetDwgDb());

    for (auto xref : this->GetLoadedXrefs())
        {
        auto& dwg = xref.GetDatabaseR ();
        status = factory.ImportFromDwg (dwg);
        }

    status = factory.CreateUserElementClasses ();

    return  aecpsetSchema;
    }


/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          08/19
+===============+===============+===============+===============+===============+======*/
struct AecPsetPropertyFactory
{
private:
    IECInstancePtr  m_ecInstance;
    DwgImporterR    m_importer;
    ExtendedImportInputs& m_extendedInputs;

public:
    AecPsetPropertyFactory (DwgImporterR i, ExtendedImportInputs& e) : m_importer(i), m_extendedInputs(e) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/19
+---------------+---------------+---------------+---------------+---------------+------*/
void GetEcValueFromNumber (ECValueR ecValue, rapidjson::Value const& jsValue)
    {
    if (jsValue.IsInt64())
        ecValue.SetLong (jsValue.GetInt64());
    else if (jsValue.IsUint64())
        ecValue.SetLong (jsValue.GetUint64());
    else if (jsValue.IsInt())
        ecValue.SetInteger (jsValue.GetInt());
    else if (jsValue.IsUint())
        ecValue.SetInteger (jsValue.GetUint());
    else if (jsValue.IsDouble())
        ecValue.SetDouble (jsValue.GetDouble());
    else
        ecValue.SetIsNull (true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/19
+---------------+---------------+---------------+---------------+---------------+------*/
void SetEcInstanceFromArray (uint32_t propIndex, rapidjson::Value& jsValue)
    {
    //ecValue.SetPrimitiveArrayInfo (PrimitiveType primitiveElementtype, uint32_t count, bool isFixedSize);
    auto jsArray = jsValue.GetArray ();
    auto size = jsArray.Size ();
    if (size < 1)
        return;

    uint32_t    arrayIndex = 0;
    ECValue     ecValue;
    for (auto& entry : jsArray)
        {
        if (this->GetPropertyValue(ecValue, propIndex, entry))
            m_ecInstance->SetValue (propIndex, ecValue, arrayIndex++);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/19
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ConvertTypeAndSetValue (ECValueR value, uint32_t propIndex)
    {
    ECValue defValue;
    auto status = m_ecInstance->GetValue (defValue, propIndex);
    if (status == ECObjectsStatus::Success)
        {
        auto defType = defValue.GetPrimitiveType();
        if (value.ConvertToPrimitiveType(defType))
            status = m_ecInstance->SetValue (propIndex, value);
        else
            status = ECObjectsStatus::DataTypeMismatch;
        }
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/19
+---------------+---------------+---------------+---------------+---------------+------*/
bool GetPropertyValue (ECValueR value, uint32_t propIndex, rapidjson::Value& jsValue)
    {
    switch (jsValue.GetType())
        {
        case rapidjson::kStringType:
            value.SetUtf8CP (Utf8String(jsValue.GetString()).c_str());
            break;
        case rapidjson::kTrueType:
            value.SetBoolean (true);
            break;
        case rapidjson::kFalseType:
            value.SetBoolean (false);
            break;
        case rapidjson::kNumberType:
            this->GetEcValueFromNumber (value, jsValue);
            break;
        case rapidjson::kArrayType:
            this->SetEcInstanceFromArray (propIndex, jsValue);
            value.SetIsNull (true);
            break;
        default:
            value.SetIsNull (true);
            break;
        }
    return  !value.IsNull();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/19
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus SetVolumeFromEntity (uint32_t propIndex)
    {
    DwgDb3dSolid::MassProperties    mass;
    auto solid3d = DwgDb3dSolid::Cast(&m_extendedInputs.GetEntity());
    if (solid3d != nullptr && solid3d->GetMassProperties(mass) == DwgDbStatus::Success)
        {
        ECValue value(mass.GetVolume());
        return  this->ConvertTypeAndSetValue(value, propIndex);
        }
    return  ECObjectsStatus::PropertyValueNull;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/19
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus SetLengthFromEntity (uint32_t propIndex)
    {
    auto line = DwgDbLine::Cast(&m_extendedInputs.GetEntity());
    if (line != nullptr)
        {
        double length = line->GetStartPoint().Distance (line->GetEndPoint());
        ECValue value(length);
        return  this->ConvertTypeAndSetValue(value, propIndex);
        }
    return  ECObjectsStatus::PropertyValueNull;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/19
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus SetValueForAutomaticProperty (uint32_t propIndex, Utf8CP name)
    {
    /*-----------------------------------------------------------------------------------
    This is needed for RealDWG based importer: the API's provided by Autodesk do not have "automatic"
    properties on the entity - we must obtain these from the definition.  Based on their property names
    we try calculating the value from the input entity. Types of supported automatic properties may
    grow as needed.

    On the other hand, OpenDWG based importer would not hit here - its architectural extension extracts
    automatic properties for the entity as string properties.
    -----------------------------------------------------------------------------------*/
    if (name == nullptr || name[0] == 0)
        return  ECObjectsStatus::OperationNotSupported;
    else if (BeStringUtilities::Stricmp(name, "Volume") == 0)
        return this->SetVolumeFromEntity (propIndex);
    else if (BeStringUtilities::Stricmp(name, "Length") == 0)
        return this->SetLengthFromEntity (propIndex);

    return  ECObjectsStatus::PropertyNotSupported;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/19
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus CreateEcInstance (ECClassCR ecClass, rapidjson::Value::MemberIterator& objectIter)
    {
    ECObjectsStatus status = ECObjectsStatus::NotFound;
    if (!objectIter->value.IsObject())
        return  status;

    m_ecInstance = ecClass.GetDefaultStandaloneEnabler()->CreateInstance ();
    if (!m_ecInstance.IsValid())
        return  status;

    auto json = objectIter->value.GetObject ();
    auto& enabler = m_ecInstance->GetEnabler ();

    // set these up to track "automatic" properties, which are only available in the defs using RealDWG toolkit
    size_t  countFromInput = json.MemberCount ();
    size_t  countFromDef = ecClass.GetPropertyCount (false);

    bool    hasAutomaticProps = countFromInput < countFromDef;
    bvector<uint32_t>   missingIndices;
    if (hasAutomaticProps && enabler.GetPropertyIndices(missingIndices, 0) != ECObjectsStatus::Success)
        hasAutomaticProps = false;

    // the first prop is always "Element" which we do not want in our tracking list
    if (hasAutomaticProps)
        missingIndices.erase (missingIndices.begin());

    size_t  countCreated = 0;
    for (rapidjson::Value::MemberIterator iter = json.MemberBegin(); iter != json.MemberEnd(); ++iter)
        {
        auto ecName = DwgHelper::ValidateECNameFrom (iter->name.GetString());

#ifdef DUMP_JSON
        auto strValue = BeRapidJsonUtilities::ToString(iter->value).DropQuotes ();
        LOG.debugv ("%s : %s", iter->name.GetString(), strValue);
#endif

        ECValue     value;
        uint32_t    propIndex = 0;
        status = enabler.GetPropertyIndex (propIndex, ecName.c_str());

        if (status == ECObjectsStatus::Success)
            {
            // track "automatic" properties
            if (hasAutomaticProps)
                {
                auto found = std::find(missingIndices.begin(), missingIndices.end(), propIndex);
                if (found != missingIndices.end())
                    missingIndices.erase (found);
                }

            if (this->GetPropertyValue(value, propIndex, iter->value))
                {
                status = m_ecInstance->SetValue (propIndex, value);
                /*-----------------------------------------------------------------------
                OpenDWG+Arch Extension returns us "automatic" properties, but the value types may differ
                from that in the definition - convert these to match the definition.
                -----------------------------------------------------------------------*/
                if (status == ECObjectsStatus::DataTypeMismatch)
                    status = this->ConvertTypeAndSetValue (value, propIndex);
                }
            }
        if (status == ECObjectsStatus::Success)
            countCreated++;
        }

    // set "automatic" properties
    if (hasAutomaticProps)
        {
        for (auto propIndex : missingIndices)
            {
            Utf8CP  name = nullptr;
            status = enabler.GetAccessString (name, propIndex);
            if (status == ECObjectsStatus::Success)
                status = this->SetValueForAutomaticProperty (propIndex, name);

            if (status == ECObjectsStatus::Success)
                countCreated++;
            }
        }

    return  countCreated > 0 ? ECObjectsStatus::Success : ECObjectsStatus::Error;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
DwgDbObjectId   GetPsetsDictionary (DwgDbDictionaryIteratorPtr& iter, DwgDbEntityCR entity)
    {
    // get AecDbPropertySets dictionary ID from entity's extension dictionary
    DwgDbObjectId   dictionaryId;
    DwgDbDictionaryPtr extDict(entity.GetExtensionDictionary(), DwgDbOpenMode::ForRead);
    if (extDict.OpenStatus() == DwgDbStatus::Success)
        {
        if (extDict->GetIdAt(dictionaryId, L"AEC_PROPERTY_SETS") == DwgDbStatus::Success)
            {
            DwgDbDictionaryPtr psetsDict(dictionaryId, DwgDbOpenMode::ForRead);
            if (psetsDict.OpenStatus() == DwgDbStatus::Success)
                {
                iter = psetsDict->GetIterator ();
                if (iter.IsNull() || !iter->IsValid())
                    dictionaryId.SetNull();
                }
            else
                {
                dictionaryId.SetNull();
                }
            }
        }
    return  dictionaryId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   CreateEcInstances (DgnElementR hostElement)
    {
    auto entity = m_extendedInputs.GetEntityP ();
    if (entity == nullptr)
        return  BSIERROR;

    auto aecPropertySetSchema = m_importer.GetAecPropertySetSchema ();
    if (aecPropertySetSchema == nullptr)
        return  BSIERROR;

    auto& json = m_extendedInputs.GetAecPropertySetsR ();
    for (rapidjson::Value::MemberIterator iter = json.MemberBegin(); iter != json.MemberEnd(); ++iter)
        {
        if (!iter->value.IsObject())
            {
            BeAssert (false && "Top level of the json must be a json value of kObjectType");
            continue;
            }

        auto ecclassName = DwgHelper::ValidateECNameFrom (iter->name.GetString());
        auto ecClass = aecPropertySetSchema->GetClassCP (ecclassName.c_str());
        if (ecClass != nullptr)
            {
            if (this->CreateEcInstance(*ecClass, iter) != ECObjectsStatus::Success)
                continue;

            if (m_ecInstance.IsValid() && DgnElement::GenericMultiAspect::AddAspect(hostElement, *m_ecInstance.get()) == DgnDbStatus::Success)
                m_importer._AddPresentationRuleContent (hostElement, m_ecInstance->GetClass());
            }
        }

    return  BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ExtractFromEntity ()
    {
    auto entity = m_extendedInputs.GetEntityP ();
    if (entity == nullptr)
        return  BSIERROR;
    auto entityId = entity->GetObjectId ();
    if (!entityId.IsValid())
        return  BSIERROR;

    auto& aecPropertySets = m_extendedInputs.GetAecPropertySetsR ();
    auto& allocator = aecPropertySets.GetAllocator ();
    BeAssert (aecPropertySets.IsObject());

    DwgDbDictionaryIteratorPtr psetIter;

    // check & get extension dictionary AecDbPropertySets - most entitie stop here
    auto objectId = this->GetPsetsDictionary (psetIter, *entity);
    if (!objectId.IsValid() || !psetIter.IsValid())
        return  BSIERROR;

    DwgDbDictionaryPtr mainDict(m_importer.GetDwgDb().GetNamedObjectsDictionaryId(), DwgDbOpenMode::ForRead);
    if (mainDict.OpenStatus() != DwgDbStatus::Success || mainDict->GetIdAt(objectId, L"AEC_PROPERTY_SET_DEFS") != DwgDbStatus::Success)
        return  BSIERROR;

    DwgDbDictionaryPtr psetdefsDict(objectId, DwgDbOpenMode::ForRead);
    if (psetdefsDict.OpenStatus() != DwgDbStatus::Success)
        return  BSIERROR;

    // check all AecDbPropertySet entries under AecDbPropertySets
    for (; !psetIter->Done(); psetIter->Next())
        {
        // find an AcDbAecPropertySetDef entry under AecDbPropertySetDefs of the main dictionary
        auto psetId = psetIter->GetObjectId ();
        auto psetdefId = UtilsLib::GetAecDbPropertySetDef (psetId);
        if (!psetdefId.IsValid())
            continue;

        // get its own name from its parent dictionary!
        DwgString   psetdefName;
        if (psetdefsDict->GetNameAt(psetdefName, psetdefId) != DwgDbStatus::Success)
            continue;

        rapidjson::Document json(rapidjson::kObjectType);
        if (UtilsLib::ParseAecDbPropertySet(json, allocator, entityId, &psetId) == DwgDbStatus::Success)
            {
            Utf8String  utf8(psetdefName.c_str());
            rapidjson::Value jsKey(rapidjson::StringRef(utf8.c_str()), (rapidjson::SizeType)utf8.size(), allocator);
            aecPropertySets.AddMember (jsKey, json.GetObject(), allocator);
            }
        }

    return  aecPropertySets.IsNull() || aecPropertySets.MemberCount() == 0 ? BSIERROR : BSISUCCESS;
    }

};  // AecPsetPropertyFactory


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_PostImportEntity (ElementImportResults& results, ElementImportInputs& inputs)
    {
    BentleyStatus   status = BSISUCCESS;
    if (m_aecPropertySetSchema == nullptr || inputs.GetEntityP() == nullptr)
        return  status;

    ExtendedImportInputs* extendedInputs = static_cast<ExtendedImportInputs*> (&inputs);
    if (extendedInputs == nullptr)
        return  BSIERROR;

    auto element = results.GetImportedElement ();

    AecPsetPropertyFactory factory(*this, *extendedInputs);
    return factory.CreateEcInstances (*element);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_PreImportEntity (ElementImportInputs& inputs)
    {
    if (m_elementClassMap.empty() || m_aecPropertySetSchema == nullptr)
        return  BSIERROR;

    ExtendedImportInputs* extendedInputs = static_cast<ExtendedImportInputs*> (&inputs);
    if (extendedInputs == nullptr)
        return  BSIERROR;

    // only remap element classes for entities in modelspace
    auto& entity = inputs.GetEntity ();
    auto dwg = entity.GetDatabase ();
    if (dwg.IsValid() && entity.GetOwnerId() != dwg->GetModelspaceId())
        return  BSIERROR;

    auto entityId = entity.GetObjectId ();

    // extract AEC property sets from the entity and convert them to the json collection in the extended inputs:
    AecPsetPropertyFactory factory(*this, *extendedInputs);
    auto status = factory.ExtractFromEntity ();
    if (status != BSISUCCESS)
        return  status;

    // get back our collected json for element class remapping
    auto& aecPropertySets = extendedInputs->GetAecPropertySetsR ();
    if (aecPropertySets.MemberCount() < 1)
        return  status;

    // prepare unique input property names
    T_Utf8StringVector  searchNames1, searchNames2;
    for (auto map : m_elementClassMap)
        {
        auto name = map.GetAecPropertyName ();
        auto found = std::find_if(searchNames1.begin(), searchNames1.end(), [&](Utf8StringCR s)
            {
            return !s.empty() && name.Equals(s);
            });
        if (found == searchNames1.end())
            searchNames1.push_back (name);

        name = map.GetTargetLabelFrom ();
        found = std::find_if(searchNames2.begin(), searchNames2.end(), [&](Utf8StringCR s)
            {
            return !s.empty() && name.Equals(s);
            });
        if (found == searchNames2.end())
            searchNames2.push_back (name);
        }
    if (searchNames1.empty())
        return  BSISUCCESS;
    
    // search all AEC properties on the entity
    for (auto pset = aecPropertySets.MemberBegin(); pset != aecPropertySets.MemberEnd(); ++pset)
        {
        if (!pset->value.IsObject())
            continue;

        auto json = pset->value.GetObject ();
        if (json.ObjectEmpty())
            continue;

        for (auto iter = json.MemberBegin(); iter != json.MemberEnd(); ++iter)
            {
            Utf8String  name(iter->name.GetString());
            auto foundName = std::find_if(searchNames1.begin(), searchNames1.end(), [&](Utf8StringCR n){return name.Equals(n);});
            if (foundName != searchNames1.end())
                {
                // use string value
                auto propertyValue = BeRapidJsonUtilities::ToString(iter->value).DropQuotes ();
                
                // match name & value pair from the user mapping table
                auto foundMap = std::find_if(m_elementClassMap.begin(), m_elementClassMap.end(), [&](ElementClassMap const& map)
                    {
                    auto mapName = map.GetAecPropertyName();
                    auto mapValue = map.GetAecPropertyValue();
                    return mapName.Equals(name) && mapValue.EqualsI(propertyValue);
                    });

                if (foundMap != m_elementClassMap.end())
                    {
                    // first match is found, use it
                    auto ecclassName = DwgHelper::ValidateECNameFrom (foundMap->GetTargetClassName());
                    auto elementClass = m_aecPropertySetSchema->GetClassCP (ecclassName.c_str());
                    if (elementClass != nullptr)
                        {
                        // remap element class
                        inputs.SetClassId (elementClass->GetId());

                        // check if element label is also set and the value is found:
                        for (size_t j = 0; j < searchNames2.size(); j++)
                            {
                            auto foundValue = std::find_if(m_elementClassMap.begin(), m_elementClassMap.end(), [&](ElementClassMap const& map)
                                {
                                return map.GetTargetLabelFrom().Equals(searchNames2[j]);
                                });
                            if (foundValue != m_elementClassMap.end() && !foundValue->GetTargetLabelFrom().empty())
                                {
                                // set element display label
                                inputs.SetElementLabel (propertyValue);
                                break;
                                }
                            }

                        return  BSISUCCESS;
                        }
                    }
                // do not want to search for more than one name match!
                return  BSIERROR;
                }
            }
        }

    return  BSIERROR;
    }
