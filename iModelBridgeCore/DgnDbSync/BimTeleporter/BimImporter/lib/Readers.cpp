/*--------------------------------------------------------------------------------------+
|
|     $Source: BimTeleporter/BimImporter/lib/Readers.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <BimTeleporter/BisJson1Importer.h>

#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <Logging/bentleylogging.h>
#include <DgnPlatform/GenericDomain.h>

#include "SyncInfo.h"
#include "Readers.h"
#include "BisJson1ImporterImpl.h"

#define MODEL_PROP_ModeledElement "ModeledElement"
#define MODEL_PROP_IsPrivate "IsPrivate"
#define MODEL_PROP_Properties "Properties"
#define MODEL_PROP_IsTemplate "IsTemplate"
#define BIS_ELEMENT_PROP_CodeSpecId "CodeSpec"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC

static Utf8CP const JSON_TYPE_KEY = "Type";
static Utf8CP const JSON_OBJECT_KEY = "Object";
static Utf8CP const JSON_TYPE_Model = "Model";
static Utf8CP const JSON_TYPE_CategorySelector = "CategorySelector";
static Utf8CP const JSON_TYPE_ModelSelector = "ModelSelector";
static Utf8CP const JSON_TYPE_DisplayStyle = "DisplayStyle";
static Utf8CP const JSON_TYPE_DictionaryModel = "DictionaryModel";
static Utf8CP const JSON_TYPE_CodeSpec = "CodeSpec";
static Utf8CP const JSON_TYPE_Schema = "Schema";
static Utf8CP const JSON_TYPE_Element = "Element";
static Utf8CP const JSON_TYPE_GeometricElement2d = "GeometricElement2d";
static Utf8CP const JSON_TYPE_GeometricElement3d = "GeometricElement3d";
static Utf8CP const JSON_TYPE_GeometryPart = "GeometryPart";
static Utf8CP const JSON_TYPE_Subject = "Subject";
static Utf8CP const JSON_TYPE_Partition = "Partition";
static Utf8CP const JSON_TYPE_Category = "Category";
static Utf8CP const JSON_TYPE_SubCategory = "SubCategory";
static Utf8CP const JSON_TYPE_ViewDefinition3d = "ViewDefinition3d";
static Utf8CP const JSON_TYPE_ViewDefinition2d = "ViewDefinition2d";
static Utf8CP const JSON_TYPE_ElementRefersToElement = "ElementRefersToElement";
static Utf8CP const JSON_TYPE_ElementGroupsMembers = "ElementGroupsMembers";

static Utf8CP const  BIS_ELEMENT_PROP_CodeSpec = "CodeSpec";
static Utf8CP const  BIS_ELEMENT_PROP_CodeScope = "CodeScope";
static Utf8CP const  BIS_ELEMENT_PROP_CodeValue = "CodeValue";
static Utf8CP const  BIS_ELEMENT_PROP_Model = "Model";
static Utf8CP const  BIS_ELEMENT_PROP_Parent = "Parent";

BEGIN_BIM_TELEPORTER_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct BimImportSchemaLocater : public ECN::IECSchemaLocater
    {
    private:
        ECN::SchemaKey m_key;
        Utf8String m_schemaXml;
        static NativeLogging::ILogger& GetLogger() { return *NativeLogging::LoggingManager::GetLogger("BimTeleporter"); }

        virtual ECN::ECSchemaPtr _LocateSchema(ECN::SchemaKeyR key, ECN::SchemaMatchType matchType, ECN::ECSchemaReadContextR schemaContext) override;

    public:
        BimImportSchemaLocater() {}

        void AddSchemaXmlR(ECN::SchemaKeyCR schemaKey, Utf8StringCR schemaXml);
        ECN::ECSchemaPtr DeserializeSchema(ECN::ECSchemaReadContextR, ECN::SchemaMatchType);
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
void BimImportSchemaLocater::AddSchemaXmlR(ECN::SchemaKeyCR schemaKey, Utf8StringCR schemaXml)
    {
    m_key = schemaKey;
    m_schemaXml = schemaXml;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                 Krischan.Eberle     02/2015
//---------------------------------------------------------------------------------------
ECN::ECSchemaPtr BimImportSchemaLocater::_LocateSchema(ECN::SchemaKeyR key, ECN::SchemaMatchType matchType, ECN::ECSchemaReadContextR schemaContext)
    {
    if (!m_key.Matches(key, matchType))
        return nullptr;
    ECN::ECSchemaPtr schema = nullptr;
    const auto readStat = ECN::ECSchema::ReadFromXmlString(schema, m_schemaXml.c_str(), schemaContext);
    if (readStat != ECN::SchemaReadStatus::Success)
        return nullptr;

    return schema;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
ECN::ECSchemaPtr BimImportSchemaLocater::DeserializeSchema(ECN::ECSchemaReadContextR schemaContext, ECN::SchemaMatchType matchType)
    {
    //Prefer ECDb and standard schemas over once embedded in DGN file.
    schemaContext.SetFinalSchemaLocater(*this);

    auto schema = schemaContext.LocateSchema(m_key, matchType);
    if (schema == nullptr)
        {
        GetLogger().errorv("Failed to deserialize v8 ECSchema '%s'.", m_key.GetFullSchemaName().c_str());
        return nullptr;
        }

    schemaContext.RemoveSchemaLocater(*this);
    return schema;
    }


Reader::Reader(BisJson1ImporterImpl* importer) : m_importer(importer)
    { }

DgnDbP Reader::GetDgnDb()
    {
    return m_importer->GetDgnDb();
    }

SyncInfo* Reader::GetSyncInfo() { return m_importer->m_syncInfo; }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2017
//---------------+---------------+---------------+---------------+---------------+-------
ECN::ECClassCP Reader::GetClassFromKey(Utf8CP classKey)
    {
    bvector<Utf8String> tokens;
    BeStringUtilities::Split(classKey, ".", tokens);
    if (2 != tokens.size())
        {
        GetLogger().errorv("$ECClassKey is malformed: %s", classKey);
        return nullptr;
        }
    bmap<Utf8String, SchemaKey>::iterator key = m_importer->m_schemaNameToKey.find(tokens[0]);
    if (key == m_importer->m_schemaNameToKey.end())
        {
        GetLogger().errorv("Unable to find schema key for $ECClassKey: %s", classKey);
        return nullptr;
        }
    ECSchemaCP schema = m_importer->GetDgnDb()->Schemas().GetSchema(tokens[0].c_str());
    if (nullptr == schema)
        schema = m_importer->m_schemaReadContext->LocateSchema(key->second, SchemaMatchType::Exact).get();
    if (nullptr == schema)
        {
        GetLogger().errorv("Unable to locate schema for $ECClassKey: %s", classKey);
        return nullptr;
        }

    ECClassCP ecClass = schema->GetClassCP(tokens[1].c_str());
    if (nullptr == ecClass)
        {
        GetLogger().errorv("Unable to find %s in schema %s", tokens[1].c_str(), tokens[0].c_str());
        return nullptr;
        }
    return ecClass;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
ECN::IECInstancePtr Reader::_CreateInstance(Json::Value& object)
    {
    ECClassCP ecClass = GetClassFromKey(object["$ECClassKey"].asString().c_str());
    if (nullptr == ecClass)
        return nullptr;

    IECInstancePtr ptr = ecClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
    if (!ptr.IsValid())
        return nullptr;
    if (SUCCESS != ECJsonUtilities::ECInstanceFromJson(*ptr, object))
        return nullptr;
    return ptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus Reader::RemapCodeSpecId(Json::Value& element)
    {
    CodeSpecId codeSpec = GetMappedCodeSpecId(element);
    if (!codeSpec.IsValid())
        return ERROR;

    Utf8PrintfString idStr("%" PRIu64 "%", codeSpec.GetValue());
    element[BIS_ELEMENT_PROP_CodeSpecId]["id"] = idStr;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2017
//---------------+---------------+---------------+---------------+---------------+-------
CodeSpecId Reader::GetMappedCodeSpecId(Json::Value& element)
    {
    if (!element.isMember(BIS_ELEMENT_PROP_CodeSpecId))
        return CodeSpecId();

    Utf8String idString = element[BIS_ELEMENT_PROP_CodeSpecId]["id"].asString();
    uint64_t id;
    if (!Utf8String::IsNullOrEmpty(idString.c_str()))
        {
        if (SUCCESS != BeStringUtilities::ParseUInt64(id, idString.c_str()))
            return CodeSpecId();
        CodeSpecId codeSpec = m_importer->m_syncInfo->LookupCodeSpec(CodeSpecId(id));
        if (!codeSpec.IsValid())
            {
            GetLogger().errorv("Unable to determine CodeSpecId for imported element %s", element.toStyledString().c_str());
            return CodeSpecId();
            }
        return codeSpec;
        }
    return CodeSpecId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ElementReader::RemapParentId(Json::Value& element)
    {
    uint64_t id;
    if (!element.isMember("Parent"))
        return SUCCESS;

    Utf8String parentId = element["Parent"]["id"].asString();
    if (!(Utf8String::IsNullOrEmpty(parentId.c_str())))
        {
        if (SUCCESS != BeStringUtilities::ParseUInt64(id, parentId.c_str()))
            return ERROR;
        DgnElementId mappedId = m_importer->m_syncInfo->LookupElement(DgnElementId(id));
        if (!mappedId.IsValid())
            {
            // What do we do here?
            return ERROR;
            }
        Utf8PrintfString idStr("%" PRIu64 "%", mappedId.GetValue());
        element[BIS_ELEMENT_PROP_Parent]["id"] = idStr.c_str();
        Utf8PrintfString parRelId("%" PRIu64 "%", _GetRelationshipClassId().GetValue());
        element[BIS_ELEMENT_PROP_Parent]["relECClassId"] = parRelId.c_str();
        return SUCCESS;
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ElementReader::RemapCategoryId(Json::Value& element)
    {
    uint64_t id;
    if (!element.isMember("Category"))
        return SUCCESS;

    Utf8String catId = element["Category"]["id"].asString();
    if (!(Utf8String::IsNullOrEmpty(catId.c_str())))
        {
        if (SUCCESS != BeStringUtilities::ParseUInt64(id, catId.c_str()))
            return ERROR;
        DgnElementId mappedId = m_importer->m_syncInfo->LookupElement(DgnElementId(id));
        if (!mappedId.IsValid())
            {
            // What do we do here?
            return ERROR;
            }
        Utf8PrintfString idStr("%" PRIu64 "%", mappedId.GetValue());
        element["Category"]["id"] = idStr;
        return SUCCESS;
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
ECN::IECInstancePtr ElementReader::_CreateInstance(Json::Value& object)
    {
    if (SUCCESS != RemapModelId(object))
        return nullptr;

    if (ERROR == RemapParentId(object))
        return nullptr;

    if (SUCCESS != RemapCodeSpecId(object))
        return nullptr;

    if (SUCCESS != RemapCategoryId(object))
        return nullptr;

    IECInstancePtr ecInstance = T_Super::_CreateInstance(object);
    if (!ecInstance.IsValid())
            return nullptr;


    return ecInstance;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ElementReader::RemapModelId(Json::Value& element)
    {
    DgnModelId mappedId = GetMappedModelId(element);
    if (!mappedId.IsValid())
        return ERROR;

    Utf8PrintfString idStr("%" PRIu64 "%", mappedId.GetValue());
    element[BIS_ELEMENT_PROP_Model]["id"] = idStr;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2017
//---------------+---------------+---------------+---------------+---------------+-------
DgnModelId ElementReader::GetMappedModelId(Json::Value& element)
    {
    // remap modelId
    Utf8String modelId = element["Model"]["id"].asString();
    uint64_t id;
    if (Utf8String::IsNullOrEmpty(modelId.c_str()))
        return DgnModelId();

    if (SUCCESS != BeStringUtilities::ParseUInt64(id, modelId.c_str()))
        {
        GetLogger().errorv("Failed to parse ModelId property of element: %s", element.toStyledString().c_str());
        return DgnModelId();
        }
    DgnModelId mappedId = m_importer->m_syncInfo->LookupModel(DgnModelId(id));
    if (!mappedId.IsValid())
        {
        GetLogger().errorv("Failed to map ModelId property of element: %s", element.toStyledString().c_str());
        return DgnModelId();
        }
    return mappedId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2017
//---------------+---------------+---------------+---------------+---------------+-------
DgnCode ElementReader::CreateCodeFromJson(Json::Value& element)
    {
    CodeSpecId codeSpec = GetMappedCodeSpecId(element);
    if (!codeSpec.IsValid())
        {
        GetLogger().errorv("Failed to map CodeSpecId for element: \n%s", element.toStyledString().c_str());
        return DgnCode();
        }

    if (!element.isMember(BIS_ELEMENT_PROP_CodeScope) || element[BIS_ELEMENT_PROP_CodeScope].isNull())
        {
        GetLogger().errorv("Element is missing code scope value: \n%s", element.toStyledString().c_str());
        return DgnCode();
        }

#if 0
    Utf8String codeScope = element[BIS_ELEMENT_PROP_CodeScope].asString();
#else
    DgnElementId codeScope; // WIP: determine how to map CodeScope
#endif

    if (!element.isMember(BIS_ELEMENT_PROP_CodeValue) || (!element[BIS_ELEMENT_PROP_CodeValue].isNull() && 0 == strlen(element[BIS_ELEMENT_PROP_CodeValue].asString().c_str())))
        {
        GetLogger().errorv("Element has invalid code value: \n%s", element.toStyledString().c_str());
        return DgnCode();
        }

    DgnCode code(codeSpec, codeScope, element[BIS_ELEMENT_PROP_CodeValue].asString());
    return code;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus Reader::RemapPropertyElementId(ECN::IECInstanceR properties, Utf8CP propertyName)
    {
    ECValue val;
    properties.GetValue(val, propertyName);
    uint64_t id;
    id = val.GetNavigationInfo().GetId<BeInt64Id>().GetValue();
    DgnElementId mappedId = GetSyncInfo()->LookupElement(DgnElementId(id));
    if (!mappedId.IsValid())
        {
        GetLogger().errorv("Failed to map %s property of %s class.", propertyName, _GetElementType());
        return ERROR;
        }
    val.SetNavigationInfo(mappedId);
    properties.SetValue(propertyName, val);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2017
//---------------+---------------+---------------+---------------+---------------+-------
DgnElementId Reader::GetMappedElementId(Json::Value& element, Utf8CP propertyName)
    {
    if (!element.isMember(propertyName))
        return DgnElementId();
    Json::Value prop = element[propertyName];
    if (!prop.isMember("id"))
        {
        GetLogger().errorv("Object %s does not have an 'id' property to remap.\n%s", propertyName, element.toStyledString().c_str());
        return DgnElementId();
        }
    Utf8String elementId = element[propertyName]["id"].asString();
    uint64_t id;
    if (SUCCESS != BeStringUtilities::ParseUInt64(id, elementId.c_str()))
        {
        GetLogger().errorv("Failed to parse ElementId property (%s) of element: %s", propertyName, element.toStyledString().c_str());
        return DgnElementId();
        }

    DgnElementId mappedId = GetSyncInfo()->LookupElement(DgnElementId(id));
    if (!mappedId.IsValid())
        {
        GetLogger().errorv("Failed to map %s property of %s class.", propertyName, _GetElementType());
        return DgnElementId();
        }

    return mappedId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus CodeSpecReader::_Read(Json::Value& object)
    {
    uint64_t id = object["Id"].asUInt64();

    CodeSpecId oldId(id);
    if (!oldId.IsValid())
        {
        GetLogger().errorv("Unable to import CodeSpec: %s\n", object.toStyledString().c_str());
        return ERROR;
        }

    CodeSpecId newId = GetDgnDb()->CodeSpecs().QueryCodeSpecId(object["Name"].asCString());
    if (newId.IsValid())
        {
        GetSyncInfo()->InsertCodeSpec(oldId, newId);
        m_importer->RemapCodeSpecId(oldId);
        return SUCCESS;
        }

    CodeSpecPtr codeSpec = CodeSpec::Create(*GetDgnDb(), object["Name"].asCString());
    if (!codeSpec.IsValid())
        {
        GetLogger().errorv("Failed to create CodeSpec: %s\n", object.toStyledString().c_str());
        return ERROR;
        }

    uint8_t specType = (uint8_t) object["CodeSpecType"].asUInt();
    switch (specType)
        {
        case 1:
            codeSpec->SetScope(CodeScopeSpec::CreateRepositoryScope());
            break;
        case 2:
            codeSpec->SetScope(CodeScopeSpec::CreateModelScope());
            break;
        case 3:
            codeSpec->SetScope(CodeScopeSpec::CreateParentElementScope());
            break;
        default:
            GetLogger().warningv("Invalid CodeSpec scope value %s.  Defaulting to RepositoryScope", object.toStyledString().c_str());
            codeSpec->SetScope(CodeScopeSpec::CreateRepositoryScope());
        }
    codeSpec->Insert();
    BeAssert(codeSpec->GetCodeSpecId().IsValid());

    GetSyncInfo()->InsertCodeSpec(oldId, codeSpec->GetCodeSpecId());
    m_importer->RemapCodeSpecId(oldId);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SubjectReader::_Read(Json::Value& subject)
    {
    Utf8String label = subject["Label"].asString();
    uint64_t parentId = subject["ParentId"].asUInt64();
    uint64_t oldInstanceId = subject["$ECInstanceId"].asUInt64();
    SubjectCPtr parentSubject;
    if (-1 == parentId || 0 == parentId)
        parentSubject = GetDgnDb()->Elements().GetRootSubject();
    else
        {
        DgnElementId mappedId = GetSyncInfo()->LookupElement(DgnElementId(parentId));
        if (mappedId.IsValid())
            parentSubject = GetDgnDb()->Elements().Get<Subject>(mappedId);
        else
            {
            GetLogger().warningv("Unable to locate parent subject with exported id %" PRIu64 "%.  Using root subject instead", parentId);
            parentSubject = GetDgnDb()->Elements().GetRootSubject();
            }
        }
    SubjectCPtr subjectElement = Subject::CreateAndInsert(*parentSubject, label.c_str(), subject["Descr"].isNull() ? nullptr : subject["Descr"].asString().c_str());
    if (!subjectElement.IsValid())
        return ERROR;

    SyncInfo::ElementMapping map(DgnElementId(oldInstanceId), subjectElement->GetElementId());
    map.Insert(*GetDgnDb());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus PartitionReader::_Read(Json::Value& partition)
    {
    Utf8String label = partition["Label"].asString();
    uint64_t oldInstanceId = partition["$ECInstanceId"].asUInt64();
    uint64_t subjectId = partition["Subject"].asUInt64();
    SubjectCPtr subject;
    if (-1 == subjectId || 0 == subjectId)
        subject = GetDgnDb()->Elements().GetRootSubject();
    else
        {
        DgnElementId mappedId = GetSyncInfo()->LookupElement(DgnElementId(subjectId));
        if (mappedId.IsValid())
            subject = GetDgnDb()->Elements().Get<Subject>(mappedId);
        else
            {
            GetLogger().warningv("Unable to locate partition's subject with exported id %" PRIu64 "%.  Using root subject instead", subjectId);
            subject = GetDgnDb()->Elements().GetRootSubject();
            }
        }

    Utf8String partitionType = partition["PartitionType"].asString();
    DgnElementId newId;
    if (partitionType.Equals("GroupInformationPartition"))
        {
        GroupInformationPartitionCPtr gip = GroupInformationPartition::CreateAndInsert(*subject, label.c_str(), partition["Descr"].isNull() ? nullptr : partition["Descr"].asString().c_str());
        if (!gip.IsValid())
            {
            GetLogger().errorv("Failed to create a GroupInformationPartition for %" PRIu64 "%", oldInstanceId);
            return ERROR;
            }
        newId = gip->GetElementId();
        }
    else if (partitionType.Equals("PhysicalPartition"))
        {
        PhysicalPartitionCPtr pp = PhysicalPartition::CreateAndInsert(*subject, label.c_str(), partition["Descr"].isNull() ? nullptr : partition["Descr"].asString().c_str());
        if (!pp.IsValid())
            {
            GetLogger().errorv("Failed to create a PhysicalPartition for %" PRIu64 "%", oldInstanceId);
            return ERROR;
            }
        newId = pp->GetElementId();
        }
    else if (partitionType.Equals("DocumentPartition"))
        {
        DocumentPartitionCPtr dp = DocumentPartition::CreateAndInsert(*subject, label.c_str(), partition["Descr"].isNull() ? nullptr : partition["Descr"].asString().c_str());
        if (!dp.IsValid())
            {
            GetLogger().errorv("Failed to create a DocumentPartition for %" PRIu64 "%", oldInstanceId);
            return ERROR;
            }
        newId = dp->GetElementId();
        }
    else if (partitionType.Equals("LinkPartition"))
        {
        LinkPartitionCPtr lp = LinkPartition::CreateAndInsert(*subject, label.c_str(), partition["Descr"].isNull() ? nullptr : partition["Descr"].asString().c_str());
        if (!lp.IsValid())
            {
            GetLogger().errorv("Failed to create a LinkPartition for %" PRIu64 "%", oldInstanceId);
            return ERROR;
            }
        newId = lp->GetElementId();
        }
    else
        {
        GetLogger().errorv("Unknown (or empty) partition type '%s' for element %" PRIu64 "%", partitionType.c_str(), oldInstanceId);
        return ERROR;
        }
    SyncInfo::ElementMapping map(DgnElementId(oldInstanceId), newId);
    map.Insert(*GetDgnDb());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2017
//---------------+---------------+---------------+---------------+---------------+-------
ECN::ECClassId ElementReader::_GetRelationshipClassId()
    {
    return m_importer->GetDgnDb()->Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsChildElements); 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ElementReader::_Read(Json::Value& element)
    {
    IECInstancePtr ecInstance = _CreateInstance(element);
    if (!ecInstance.IsValid())
        return ERROR;

    Utf8String idString = element["$ECInstanceId"].asString();
    if (SUCCESS != BeStringUtilities::ParseUInt64(m_instanceId, idString.c_str()))
        return ERROR;

    if (SUCCESS != _OnInstanceCreated(*ecInstance.get()))
        {

        }
    DgnDbStatus stat;
    DgnElementPtr dgnElement = GetDgnDb()->Elements().CreateElement(*ecInstance, &stat);

    if (!dgnElement.IsValid())
        {
        GetLogger().errorv("Failed to create %s from instance.  Error code: %d\n", _GetElementType(), stat);
        return ERROR;
        }

    _OnElementCreated(*dgnElement.get(), *ecInstance);

    DgnElementCPtr inserted = dgnElement->Insert(&stat);
    if (!inserted.IsValid())
        {
        GetLogger().errorv("Failed to insert newly created %s into db.  Error code %d\n", _GetElementType(), stat);
        return ERROR;
        }

    SyncInfo::ElementMapping map(DgnElementId(m_instanceId), inserted->GetElementId());
    map.Insert(*GetDgnDb());
    return SUCCESS;

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus GeometryReader::_OnInstanceCreated(ECN::IECInstanceR ecInstance)
    {
    ECValue geomValue;
    size_t size = 0;
    ecInstance.GetValue(geomValue, "GeometryStream");
    if (geomValue.IsNull())
        return SUCCESS;
    ByteCP geomBlob = geomValue.GetBinary(size);
    m_geomStream.Append((uint8_t*) geomBlob, (uint32_t) size);

    ECValue empty;
    empty.SetIsNull(true);
    ecInstance.SetValue("GeometryStream", empty);
    
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus GeometricElementReader::_OnElementCreated(DgnElementR element, ECN::IECInstanceR properties)
    {
    GeometryStream dest;
    GeometrySourceP geomSource = element.ToGeometrySourceP();
    DgnDbStatus stat = GeometryStreamIO::Import(dest, m_geomStream, *m_importer);
    if (DgnDbStatus::Success != stat)
        return ERROR;

    GeometryBuilderPtr builder = GeometryBuilder::Create(*geomSource, dest);
    if (!builder.IsValid())
        return ERROR;

    if (SUCCESS != builder->Finish(*geomSource))
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus GeometryPartReader::_OnElementCreated(DgnElementR element, ECN::IECInstanceR properties)
    {
    GeometryStream dest;
    DgnDbStatus stat = GeometryStreamIO::Import(dest, m_geomStream, *m_importer);
    if (DgnDbStatus::Success != stat)
        return ERROR;

    GeometryBuilderPtr builder = GeometryBuilder::CreateGeometryPart(dest, *GetDgnDb());
    if (!builder.IsValid())
        return ERROR;

    DgnGeometryPartP part = const_cast<DgnGeometryPartP>(element.ToGeometryPart());
    if (SUCCESS != builder->Finish(*part))
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewDefinitionReader::_Read(Json::Value& viewDefinition)
    {
    if (!m_is3d)
        return T_Super::_Read(viewDefinition);

    Utf8String idString = viewDefinition["$ECInstanceId"].asString();
    uint64_t instanceId;
    if (SUCCESS != BeStringUtilities::ParseUInt64(instanceId, idString.c_str()))
        return ERROR;

    DgnModelId model = GetMappedModelId(viewDefinition);
    if (!model.IsValid())
        return ERROR;

    DgnCode dgnCode = CreateCodeFromJson(viewDefinition);
    if (!dgnCode.IsValid())
        return ERROR;

    DgnElementId displayStyleId = GetMappedElementId(viewDefinition, "DisplayStyle");
    if (!displayStyleId.IsValid())
        return ERROR;
    DisplayStyle3dPtr displayStyle = GetDgnDb()->Elements().GetForEdit<DisplayStyle3d>(displayStyleId); // We aren't editing it, but the CreateParams wants a ref, not a const ref
    if (!displayStyle.IsValid())
        return ERROR;

    DgnElementId categorySelectorId = GetMappedElementId(viewDefinition, "CategorySelector");
    if (!categorySelectorId.IsValid())
        return ERROR;
    CategorySelectorPtr categorySelector = GetDgnDb()->Elements().GetForEdit<CategorySelector>(categorySelectorId); // We aren't editing it, but the CreateParams wants a ref, not a const ref
    if (!categorySelector.IsValid())
        return ERROR;

    DgnElementId modelSelectorId = GetMappedElementId(viewDefinition, "ModelSelector");
    if (!modelSelectorId.IsValid())
        return ERROR;
    ModelSelectorPtr modelSelector = GetDgnDb()->Elements().GetForEdit<ModelSelector>(modelSelectorId); // We aren't editing it, but the CreateParams wants a ref, not a const ref
    if (!modelSelector.IsValid())
        return ERROR;

    ECClassCP ecClass = GetClassFromKey(viewDefinition["$ECClassKey"].asString().c_str());
    if (nullptr == ecClass)
        return ERROR;

    bool isOrthographic = ecClass->Is(m_importer->m_orthographicViewClass);
    double yaw = viewDefinition["Yaw"].asDouble();
    double pitch = viewDefinition["Pitch"].asDouble();
    double roll = viewDefinition["Roll"].asDouble();
    YawPitchRollAngles ypr = YawPitchRollAngles::FromDegrees(yaw, pitch, roll);

    DPoint3d extents;
    ECJsonUtilities::JsonToPoint3d(extents, viewDefinition["Extents"]);

    DPoint3d origin;
    ECJsonUtilities::JsonToPoint3d(origin, viewDefinition["Origin"]);

    bool isCameraOn = viewDefinition["IsCameraOn"].asBool();
    bool isPrivate = viewDefinition["IsPrivate"].asBool();

    ViewDefinition3d* viewDef;
    ViewDefinition3d::Camera* camera = nullptr;
    if (!isOrthographic)
        {
        DPoint3d eyePoint;
        ECJsonUtilities::JsonToPoint3d(eyePoint, viewDefinition["EyePoint"]);
        double focusDistance = viewDefinition["FocusDistance"].asDouble();
        Angle lensAngle = Angle::FromRadians(viewDefinition["LensAngle"].asDouble());
        
        camera = new ViewDefinition3d::Camera();
        camera->SetFocusDistance(focusDistance);
        camera->SetLensAngle(lensAngle);
        camera->SetEyePoint(eyePoint);

        viewDef = new SpatialViewDefinition(GetDgnDb()->GetDictionaryModel(), dgnCode.GetValue(), *categorySelector, *displayStyle, *modelSelector, camera);
        }
    else
        viewDef = new OrthographicViewDefinition(GetDgnDb()->GetDictionaryModel(), dgnCode.GetValue(), *categorySelector, *displayStyle, *modelSelector);

    viewDef->SetOrigin(origin);
    viewDef->SetExtents(DVec3d::From(extents));
    viewDef->SetRotation(ypr.ToRotMatrix());

    if (!isCameraOn)
        viewDef->TurnCameraOff();
    if (viewDefinition.isMember("IsPrivate") && !viewDefinition["IsPrivate"].isNull())
        viewDef->SetIsPrivate(viewDefinition["IsPrivate"].asBool());
    if (viewDefinition.isMember("Descr") && !viewDefinition["Descr"].isNull())
        viewDef->SetDescription(viewDefinition["Descr"].asString());
    if (viewDefinition.isMember("UserLabel") && !viewDefinition["UserLabel"].isNull())
        viewDef->SetUserLabel(viewDefinition["UserLabel"].asString().c_str());
    
    // WIP: Need to set UserProperties?

    DgnDbStatus stat;
    DgnElementCPtr inserted = viewDef->Insert(&stat);
    if (!inserted.IsValid())
        {
        GetLogger().errorv("Failed to insert newly created %s into db.  Error code %d\n", _GetElementType(), stat);
        return ERROR;
        }
    SyncInfo::ElementMapping map(DgnElementId(instanceId), inserted->GetElementId());
    map.Insert(*GetDgnDb());
    delete viewDef;
    delete camera;
    return SUCCESS;

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewDefinitionReader::_OnInstanceCreated(IECInstanceR properties)
    {
    // remap targets for view definition elements
    if (ERROR == RemapPropertyElementId(properties, "DisplayStyle"))
        return ERROR;

    if (ERROR == RemapPropertyElementId(properties, "CategorySelector"))
        return ERROR;

    ECValue source;
    uint64_t id;
    properties.GetValue(source, "BaseModel");
    DgnModelId mappedId = GetSyncInfo()->LookupModel(DgnModelId((uint64_t) source.GetLong()));
    if (!mappedId.IsValid())
        {
        // What do we do here?
        return ERROR;
        }
    ECValue val(mappedId.GetValue());
    properties.SetValue("BaseModel", val);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus CategoryReader::_Read(Json::Value& category)
    {
    IECInstancePtr ecInstance = _CreateInstance(category);
    if (!ecInstance.IsValid())
        return ERROR;

    Utf8String idString = category["$ECInstanceId"].asString();
    if (SUCCESS != BeStringUtilities::ParseUInt64(m_instanceId, idString.c_str()))
        return ERROR;

    DgnDbStatus stat;
    DgnElementPtr dgnElement = GetDgnDb()->Elements().CreateElement(*ecInstance, &stat);

    if (!dgnElement.IsValid())
        {
        GetLogger().errorv("Failed to create %s from instance.  Error code: %d\n", _GetElementType(), stat);
        return ERROR;
        }

    DgnElementCPtr inserted = dgnElement->Insert(&stat);
    if (!inserted.IsValid())
        {
        GetLogger().errorv("Failed to insert newly created %s into db.  Error code %d\n", _GetElementType(), stat);
        return ERROR;
        }

    GetSyncInfo()->InsertCategory(DgnCategoryId(m_instanceId), DgnCategoryId(inserted->GetElementId().GetValue()));
    m_importer->RemapCategory(DgnCategoryId(m_instanceId));
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SubCategoryReader::_Read(Json::Value& subCategory)
    {
    Utf8String idString = subCategory["$ECInstanceId"].asString();
    uint64_t instanceId;
    uint64_t id;
    if (SUCCESS != BeStringUtilities::ParseUInt64(instanceId, idString.c_str()))
        return ERROR;

    // Trying to set the description during creation of the element causes an assertion.  For now, we work around this by saving the description
    // and then we'll use the API to set it after the element has been created.
    Utf8String description = subCategory["Description"].asString();
    subCategory.removeMember("Description");

    // If this is the default sub category, we don't actually create it.  Just remap it
    if (m_isDefault)
        {
        Utf8String parentId = subCategory["Parent"]["id"].asString();
        if (SUCCESS != BeStringUtilities::ParseUInt64(id, parentId.c_str()))
            return ERROR;
        DgnCategoryId categoryId = GetSyncInfo()->LookupCategory(DgnCategoryId(id));
        if (!categoryId.IsValid())
            {
            GetLogger().errorv("Unable to map CategoryId for default SubCategory");
            GetLogger().infov("%s", subCategory.toStyledString().c_str());
            return ERROR;
            }
        DgnCategoryCPtr cat = GetDgnDb()->Elements().Get<DgnCategory>(categoryId);
        if (!cat.IsValid())
            {
            GetLogger().errorv("Unable to retrieve Category %" PRIu64 "%", categoryId.GetValue());
            return ERROR;
            }
        GetSyncInfo()->InsertSubCategory(categoryId, DgnSubCategoryId(instanceId), cat->GetDefaultSubCategoryId());
        m_importer->RemapSubCategory(categoryId, DgnSubCategoryId(instanceId));
        if (!subCategory["Properties"].isNull())
            {
            DgnSubCategory::Appearance appearance(subCategory["Properties"].asString());
            cat->SetDefaultAppearance(appearance);
            }
        return SUCCESS;
        }

    if (ERROR == RemapParentId(subCategory))
        return ERROR;

    IECInstancePtr ecInstance = Reader::_CreateInstance(subCategory);
    if (!ecInstance.IsValid())
        return ERROR;

    ECValue parentId;
    ecInstance->GetValue(parentId, "Parent");
    DgnCategoryId categoryId((uint64_t) parentId.GetNavigationInfo().GetId<BeInt64Id>().GetValue());
    DgnDbStatus stat;
    DgnSubCategoryPtr dgnSubCategory = GetDgnDb()->Elements().Create<DgnSubCategory>(*ecInstance, &stat);

    if (!dgnSubCategory.IsValid())
        {
        GetLogger().errorv("Failed to create SubCategory from instance.  Error code: %d\n", stat);
        return ERROR;
        }

    dgnSubCategory->SetDescription(description);
    DgnElementCPtr inserted = dgnSubCategory->Insert(&stat);
    if (!inserted.IsValid())
        {
        GetLogger().errorv("Failed to insert newly created SubCategory into db.  Error code %d\n", stat);
        return ERROR;
        }

    GetSyncInfo()->InsertSubCategory(DgnCategoryId(categoryId.GetValue()), DgnSubCategoryId(instanceId), DgnSubCategoryId(inserted->GetElementId().GetValue()));
    m_importer->RemapSubCategory(DgnCategoryId(categoryId.GetValue()), DgnSubCategoryId(instanceId));
    return SUCCESS;

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ModelReader::_Read(Json::Value& model)
    {
    
    // need to change the modeled element id before creating the instance
    if (!m_isDictionary)
        {
        Utf8String idString = model[MODEL_PROP_ModeledElement]["id"].asString();
        uint64_t id;
        if (SUCCESS != BeStringUtilities::ParseUInt64(id, idString.c_str()))
            return ERROR;

        DgnElementId mappedId = GetSyncInfo()->LookupElement(DgnElementId(id));
        if (!mappedId.IsValid())
            {
            // What do we do here? Map to the root subject?
            return ERROR;
            }
        idString.Sprintf("%" PRIu64 "%", mappedId.GetValue());
        model[MODEL_PROP_ModeledElement]["id"] = idString;
        }

    IECInstancePtr ecInstance = _CreateInstance(model);
    if (!ecInstance.IsValid())
        return ERROR;

    Utf8String ecidString = model["$ECInstanceId"].asString();
    uint64_t ecInstanceId;
    if (SUCCESS != BeStringUtilities::ParseUInt64(ecInstanceId, ecidString.c_str()))
        return ERROR;

    SyncInfo::ModelMapping mapping;
    // there is one and only one dictionary model. So if this is a dictionary model, we don't create a model, we just remap
    // its id to the new one
    if (m_isDictionary)
        {
        GetSyncInfo()->InsertModel(mapping, GetDgnDb()->GetDictionaryModel().GetModelId(), DgnModelId(ecInstanceId), nullptr);
        return SUCCESS;
        }

    DgnDbStatus stat;
    DgnModelPtr dgnModel = GetDgnDb()->Models().CreateModel(&stat, *ecInstance);
    if (!dgnModel.IsValid())
        {
        GetLogger().errorv("Failed to create model from instance.  Error code: %d\n", stat);
        return ERROR;
        }
    if (DgnDbStatus::Success != dgnModel->Insert())
        {
        GetLogger().errorv("Failed to insert newly created model into db");
        return ERROR;
        }
    GetSyncInfo()->InsertModel(mapping, dgnModel->GetModelId(), DgnModelId(ecInstanceId), nullptr);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus CategorySelectorReader::_Read(Json::Value& object)
    {
    auto& categories = object["Categories"];
    if (!categories.isArray())
        {
        GetLogger().errorv("Categories value should be an array in the category selector object");
        return ERROR;
        }

    CategorySelector selector(GetDgnDb()->GetDictionaryModel(), object["Name"].asString().c_str());
    uint64_t oldInstanceId = object["Id"].asUInt64();

    for (Json::Value::iterator iter = categories.begin(); iter != categories.end(); iter++)
        {
        Json::Value& member = *iter;
        uint64_t id = member.asUInt64();
        DgnCategoryId lookup = GetSyncInfo()->LookupCategory(DgnCategoryId(id));
        selector.AddCategory(lookup);
        }

    DgnDbStatus stat;
    selector.Insert(&stat);
    if (stat != DgnDbStatus::Success)
        return ERROR;
    SyncInfo::ElementMapping map(DgnElementId(oldInstanceId), selector.GetElementId());
    map.Insert(*GetDgnDb());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ModelSelectorReader::_Read(Json::Value& object)
    {
    auto& models = object["Models"];
    if (!models.isArray())
        {
        GetLogger().errorv("Models value should be an array in the model selector object");
        return ERROR;
        }

    ModelSelector selector(GetDgnDb()->GetDictionaryModel(), object["Name"].asString().c_str());
    uint64_t oldInstanceId = object["Id"].asUInt64();

    for (Json::Value::iterator iter = models.begin(); iter != models.end(); iter++)
        {
        Json::Value& member = *iter;
        uint64_t id = member.asUInt64();
        DgnModelId lookup = GetSyncInfo()->LookupModel(DgnModelId(id));
        selector.AddModel(lookup);
        }

    DgnDbStatus stat;
    selector.Insert(&stat);
    if (stat != DgnDbStatus::Success)
        return ERROR;
    SyncInfo::ElementMapping map(DgnElementId(oldInstanceId), selector.GetElementId());
    map.Insert(*GetDgnDb());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DisplayStyleReader::_Read(Json::Value& object)
    {
    Utf8String displayStyleName = object["Name"].asString();
    if (Utf8String::IsNullOrEmpty(displayStyleName.c_str()))
        {
        GetLogger().error("DisplayStyle name cannot be null");
        return ERROR;
        }
    DisplayStyle* displayStyle;
    if (object["Is3d"].asBool())
        displayStyle = new DisplayStyle3d(GetDgnDb()->GetDictionaryModel(), displayStyleName);
    else
        displayStyle = new DisplayStyle2d(GetDgnDb()->GetDictionaryModel(), displayStyleName);
    if (!object["BackgroundColor"].isNull())
        {
        ColorDef backgroundColor(object["BackgroundColor"].asUInt());
        displayStyle->SetBackgroundColor(backgroundColor);
        }
    if (!object["ViewFlags"].isNull())
        {
        Render::ViewFlags viewFlags;
        viewFlags.FromJson(object["ViewFlags"]);
        displayStyle->SetViewFlags(viewFlags);
        }

    uint64_t oldInstanceId = object["Id"].asUInt64();
    DgnDbStatus stat;
    displayStyle->Insert(&stat);
    if (stat != DgnDbStatus::Success)
        return ERROR;
    SyncInfo::ElementMapping map(DgnElementId(oldInstanceId), displayStyle->GetElementId());
    map.Insert(*GetDgnDb());
    delete displayStyle;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SchemaReader::_Read(Json::Value& jsonValue)
    {
    if (Json::objectValue != jsonValue.type())
        {
        GetLogger().error("Schemas value should be an object.");
        return ERROR;
        }

    for (Json::Value::iterator iter = jsonValue.begin(); iter != jsonValue.end(); iter++)
        {
        Json::Value& schema = *iter;
        if (schema.isNull())
            continue;

        Utf8CP schemaName = iter.memberName();

        BimImportSchemaLocater locater;
        SchemaKey key;
        SchemaKey::ParseSchemaFullName(key, schemaName);
        locater.AddSchemaXmlR(key, schema.asString());
        ECN::ECSchemaPtr ecSchema = locater.DeserializeSchema(*m_importer->m_schemaReadContext, ECN::SchemaMatchType::Exact);
        if (!ecSchema.IsValid())
            {
            GetLogger().fatalv("Failed to deserialize schema %s.  Unable to continue.", schemaName);
            return ERROR;
            }
        // We need to deserialize the known schemas so that they can be used as references, but we don't want to convert or import them.
        bvector<Utf8String> knownSchemas = {"Bentley_Standard_CustomAttributes", "ECDbMap", "ECDbFileInfo", "ECDbSystem", "ECDbMeta", "ECDb_FileInfo", "ECDb_System", "EditorCustomAttributes", "Generic", "MetaSchema", "dgn"};
        if (knownSchemas.end() != std::find(knownSchemas.begin(), knownSchemas.end(), ecSchema->GetName()))
            continue;

        if (!ECSchemaConverter::Convert(*ecSchema))
            {
            GetLogger().fatalv("Failed to convert schema %s to EC3.1.  Unable to continue.", schemaName);
            return ERROR;
            }
        if (SUCCESS != ImportSchema(ecSchema.get()))
            {
            GetLogger().fatalv("Failed to import schema %s.  Unable to continue.", schemaName);
            return ERROR;
            }
        bpair<Utf8String, SchemaKey> pair(ecSchema->GetName(), ecSchema->GetSchemaKey());
        m_importer->m_schemaNameToKey.insert(pair);

        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SchemaReader::ImportSchema(ECN::ECSchemaP schema)
    {
    ECSchemaCache toInsert;

    toInsert.AddSchema(*schema);
    return (SchemaStatus::Success == GetDgnDb()->ImportSchemas(toInsert.GetSchemas())) ? SUCCESS : ERROR;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ElementGroupsMembersReader::_Read(Json::Value& groups)
    {
    if (!groups.isArray())
        {
        GetLogger().error("ElementGroupsMembers value should be an array");
        return ERROR;
        }

    for (Json::Value::iterator iter = groups.begin(); iter != groups.end(); iter++)
        {
        Json::Value& group = *iter;
        DgnElementId mappedGroup = GetSyncInfo()->LookupElement(DgnElementId(group["GroupId"].asUInt64()));
        if (!mappedGroup.IsValid())
            {
            Utf8PrintfString error("Failed to map GroupId element id %" PRIu64 "%.", group["GroupId"].asUInt64());
            GetLogger().warning(error.c_str());
            continue;
            }

        GenericGroupPtr groupElement = GetDgnDb()->Elements().GetForEdit<GenericGroup>(mappedGroup);
        if (!groupElement.IsValid())
            {
            Utf8PrintfString error("Unable to get GroupInformationElement(%" PRIu64 "%).\n%s", mappedGroup.GetValue(), mappedGroup.ToString().c_str());
            GetLogger().warning(error.c_str());
            continue;
            }
        DgnElementId mappedMember = GetSyncInfo()->LookupElement(DgnElementId(group["MemberId"].asUInt64()));
        if (!mappedMember.IsValid())
            {
            Utf8PrintfString error("Failed to map MemberId element id %" PRIu64 "%.", group["MemberId"].asUInt64());
            GetLogger().warning(error.c_str());
            continue;
            }
        DgnElementPtr member = GetDgnDb()->Elements().GetForEdit<DgnElement>(mappedMember);
        if (!member.IsValid())
            {
            Utf8PrintfString error("Unable to get member DgnElement(%" PRIu64 "%).\n%s", mappedGroup.GetValue(), mappedGroup.ToString().c_str());
            GetLogger().warning(error.c_str());
            continue;
            }
        int priority = 0;
        if (group.isMember("MemberPriority"))
            priority = group["MemberPriority"].asInt();
        groupElement->AddMember(*member, priority);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ElementRefersToElementReader::_Read(Json::Value& relationships)
    {
    if (!relationships.isArray())
        {
        GetLogger().error("ElementRefersToElement value should be an array");
        return ERROR;
        }

    Utf8CP ecsql  = "SELECT ECClassId from bis_Element WHERE Id=?";
    CachedStatementPtr classLookup = nullptr;
    auto stat = GetDgnDb()->GetCachedStatement(classLookup, ecsql);
    if (stat != BE_SQLITE_OK)
        {
        BeAssert(false && "Could not retrieve cached statement.");
        return ERROR;
        }

    for (Json::Value::iterator iter = relationships.begin(); iter != relationships.end(); iter++)
        {
        Json::Value& member = *iter;
        Utf8PrintfString relName("%s.%s", member["Schema"].asString().c_str(), member["Class"].asString().c_str());

        DgnElementId mappedSource = GetSyncInfo()->LookupElement(DgnElementId(member["SourceId"].asUInt64()));
        if (!mappedSource.IsValid())
            {
            Utf8PrintfString error("Failed to map source instance id %" PRIu64 "% for ECRelationship %s.", member["SourceId"].asUInt64(), relName.c_str());
            GetLogger().warning(error.c_str());
            continue;
            }

        DgnElementId mappedTarget = GetSyncInfo()->LookupElement(DgnElementId(member["TargetId"].asUInt64()));
        if (!mappedTarget.IsValid())
            {
            Utf8PrintfString error("Failed to map target instance id %" PRIu64 "% for ECRelationship %s.", member["TargetId"].asUInt64(), relName.c_str());
            GetLogger().warning(error.c_str());
            continue;
            }

        ECN::ECRelationshipClassCP relClass = GetDgnDb()->Schemas().GetClass(member["Schema"].asString(), member["Class"].asString())->GetRelationshipClassCP();
        ECInstanceKey relKey;
        GetDgnDb()->InsertLinkTableRelationship(relKey, *relClass, mappedSource, mappedTarget);
        }

    return SUCCESS;
    }

END_BIM_TELEPORTER_NAMESPACE