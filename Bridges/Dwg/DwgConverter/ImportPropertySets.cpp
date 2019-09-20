/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "DwgImportInternal.h"

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
    LOG.debugv ("%s : %s", displayName, iter->value.GetString());
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
        if (UtilsLib::ParseAecDbPropertySetDef(json, iter.GetObjectId()) == DwgDbStatus::Success)
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

    // only if both the property name & the default value match will user input, the user target class name will be remapped:
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

#ifdef DWGTOOLKIT_OpenDwg
    AecPsetSchemaFactory factory(aecpsetSchema, *this);
    auto status = factory.ImportFromDwg (this->GetDwgDb());

    for (auto xref : this->GetLoadedXrefs())
        {
        auto& dwg = xref.GetDatabaseR ();
        status = factory.ImportFromDwg (dwg);
        }

    status = factory.CreateUserElementClasses ();
#endif

    return  aecpsetSchema;
    }


/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          08/19
+===============+===============+===============+===============+===============+======*/
struct AecPsetPropertyFactory
{
private:
    IECInstancePtr  m_ecInstance;
    ECClassCR       m_ecClass;
    DwgImporterR    m_importer;

public:
    AecPsetPropertyFactory (ECClassCR c, DwgImporterR i) : m_ecClass(c), m_importer(i) {}

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
ECObjectsStatus ConvertProperties (DwgDbEntityCR entity, DwgDbObjectIdCR aecpsetId)
    {
    ECObjectsStatus status = ECObjectsStatus::NotFound;

    m_ecInstance = m_ecClass.GetDefaultStandaloneEnabler()->CreateInstance ();
    if (!m_ecInstance.IsValid())
        return  status;

    // extract properties from AecDbPropertySet
    rapidjson::Document json(rapidjson::kObjectType);
    if (UtilsLib::ParseAecDbPropertySet(json, entity.GetObjectId(), &aecpsetId) != DwgDbStatus::Success)
        return  status;

    size_t  count = 0;
    for (rapidjson::Value::MemberIterator iter = json.MemberBegin(); iter != json.MemberEnd(); ++iter)
        {
        auto ecName = DwgHelper::ValidateECNameFrom (iter->name.GetString());

#ifdef DUMP_JSON
        LOG.debugv ("%s : %s", iter->name.GetString(), iter->value.GetString());
#endif

        ECValue     value;
        uint32_t    propIndex = 0;
        status = m_ecInstance->GetEnabler().GetPropertyIndex (propIndex, ecName.c_str());

        if (status == ECObjectsStatus::Success && this->GetPropertyValue(value, propIndex, iter->value))
            status = m_ecInstance->SetValue (propIndex, value);
        if (status == ECObjectsStatus::Success)
            count++;
        }

    return  count > 0 ? ECObjectsStatus::Success : ECObjectsStatus::Error;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          08/19
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr  GetEcInstance () const
    {
    return  m_ecInstance;
    }

};  // AecPsetPropertyFactory


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
static DwgDbObjectId    GetPsetsDictionary (DwgDbDictionaryIteratorPtr& iter, DwgDbEntityCR entity)
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
* @bsimethod                                                    Don.Fu          06/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_PostImportEntity (ElementImportResults& results, ElementImportInputs& inputs)
    {
    BentleyStatus   status = BSISUCCESS;
    if (m_aecPropertySetSchema == nullptr)
        return  status;

    auto element = results.GetImportedElement ();
    auto entity = inputs.GetEntityP ();
    if (entity == nullptr || element == nullptr)
        return  status;

    DwgDbDictionaryIteratorPtr psetIter;

    // check & get extension dictionary AecDbPropertySets - most entitie stop here
    auto objectId = GetPsetsDictionary (psetIter, *entity);
    if (!objectId.IsValid() || !psetIter.IsValid())
        return  status;

    DwgDbDictionaryPtr mainDict(m_dwgdb->GetNamedObjectsDictionaryId(), DwgDbOpenMode::ForRead);
    if (mainDict.OpenStatus() != DwgDbStatus::Success || mainDict->GetIdAt(objectId, L"AEC_PROPERTY_SET_DEFS") != DwgDbStatus::Success)
        return  status;

    DwgDbDictionaryPtr psetdefsDict(objectId, DwgDbOpenMode::ForRead);
    if (psetdefsDict.OpenStatus() != DwgDbStatus::Success)
        return  status;

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

        auto ecclassName = DwgHelper::ValidateECNameFrom (psetdefName);
        auto ecClass = m_aecPropertySetSchema->GetClassCP (ecclassName.c_str());
        if (ecClass != nullptr)
            {
            AecPsetPropertyFactory factory(*ecClass, *this);
            if (factory.ConvertProperties(*entity, psetId) != ECObjectsStatus::Success)
                continue;

            auto ecInstance = factory.GetEcInstance ();
            if (ecInstance.IsValid() && DgnElement::GenericMultiAspect::AddAspect(*element, *ecInstance.get()) == DgnDbStatus::Success)
                this->_AddPresentationRuleContent (*element, ecInstance->GetClass());
            }
        }

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          09/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::_PreImportEntity (ElementImportInputs& inputs)
    {
    if (m_elementClassMap.empty() || m_aecPropertySetSchema == nullptr)
        return  BSIERROR;

    // only remap element classes for entities in modelspace
    auto& entity = inputs.GetEntity ();
    auto dwg = entity.GetDatabase ();
    if (dwg.IsValid() && entity.GetOwnerId() != dwg->GetModelspaceId())
        return  BSIERROR;

    auto entityId = entity.GetObjectId ();

    // prepare unique input property names for the DwgDb API:
    T_Utf8StringVector  searchNames1, searchNames2, foundValues1, foundValues2;
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
    
    // search all AEC properties on the entity
    if (UtilsLib::FindAecDbPropertyValues(foundValues1, foundValues2, searchNames1, searchNames2, entityId) == DwgDbStatus::Success)
        {
        // at least 1 property has been found on the entity for element class mapping
        for (size_t i = 0; i < searchNames1.size(); i++)
            {
            // match name & value pair from the user mapping table
            auto found = std::find_if(m_elementClassMap.begin(), m_elementClassMap.end(), [&](ElementClassMap const& map)
                {
                auto mapName = map.GetAecPropertyName();
                auto mapValue = map.GetAecPropertyValue();
                return mapName.Equals(searchNames1[i]) && mapValue.EqualsI(foundValues1[i]);
                });

            if (found != m_elementClassMap.end())
                {
                // first match is found, use it
                auto ecclassName = DwgHelper::ValidateECNameFrom (found->GetTargetClassName());
                auto elementClass = m_aecPropertySetSchema->GetClassCP (ecclassName.c_str());
                if (elementClass != nullptr)
                    {
                    // remap element class
                    inputs.SetClassId (elementClass->GetId());

                    // check if element lable is also set and the value is found:
                    for (size_t j = 0; j < searchNames2.size(); j++)
                        {
                        found = std::find_if(m_elementClassMap.begin(), m_elementClassMap.end(), [&](ElementClassMap const& map)
                            {
                            return map.GetTargetLabelFrom().Equals(searchNames2[j]);
                            });
                        if (found != m_elementClassMap.end() && !foundValues2[i].empty())
                            {
                            // set element display label
                            inputs.SetElementLabel (foundValues2[i]);
                            break;
                            }
                        }
                    
                    return  BSISUCCESS;
                    }
                }
            }
        }

    return  BSIERROR;
    }
