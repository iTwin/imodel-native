/*--------------------------------------------------------------------------------------+
|
|     $Source: BimTeleporter/BimImporter/lib/Readers.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <BimTeleporter/BisJson1Importer.h>

#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <Logging/bentleylogging.h>
#include <DgnPlatform/GenericDomain.h>
#include <DgnPlatform/Render.h>
#include <Bentley/Base64Utilities.h>
#include <Planning/PlanningApi.h>
#include "SyncInfo.h"
#include "Readers.h"
#include "BisJson1ImporterImpl.h"
#include "SchemaFlattener.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_PLANNING

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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
Reader::Reader(BisJson1ImporterImpl* importer) : m_importer(importer)
    { }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnDbP Reader::GetDgnDb()
    {
    return m_importer->GetDgnDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
SyncInfo* Reader::GetSyncInfo() { return m_importer->m_syncInfo; }


//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2017
//---------------+---------------+---------------+---------------+---------------+-------
ECN::ECClassCP Reader::_GetClassFromName(Utf8CP className, Json::Value& element)
    {
    bvector<Utf8String> tokens;
    BeStringUtilities::Split(className, ".", tokens);
    if (2 != tokens.size())
        {
        GetLogger().errorv("className is malformed: %s", className);
        return nullptr;
        }
    bmap<Utf8String, SchemaKey>::iterator key = m_importer->m_schemaNameToKey.find(tokens[0]);
    if (key == m_importer->m_schemaNameToKey.end())
        {
        GetLogger().errorv("Unable to find schema key for className: %s", className);
        return nullptr;
        }
    ECSchemaCP schema = m_importer->GetDgnDb()->Schemas().GetSchema(tokens[0].c_str());
    if (nullptr == schema)
        schema = m_importer->m_schemaReadContext->LocateSchema(key->second, SchemaMatchType::Exact).get();
    if (nullptr == schema)
        {
        GetLogger().errorv("Unable to locate schema for className: %s", className);
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
    ECClassCP ecClass = _GetClassFromName(object[ECJsonUtilities::json_className()].asCString(), object);
    if (nullptr == ecClass)
        {
        GetLogger().errorv("Failed to get ECClass from key %s\n", object[ECJsonUtilities::json_className()].asCString());
        return nullptr;
        }

    IECInstancePtr ptr = ecClass->GetDefaultStandaloneEnabler()->CreateInstance(0);
    if (!ptr.IsValid())
        {
        GetLogger().errorv("Failed to create IECInstancePtr from %s's standalone enabler\n", object[ECJsonUtilities::json_className()].asCString());
        return nullptr;
        }

    if (SUCCESS != JsonECInstanceConverter::JsonToECInstance(*ptr, object, m_importer->GetDgnDb()->GetClassLocater(), true, &m_importer->GetRemapper()))
        {
        GetLogger().errorv("Failed to create ECInstanceFromJson\n");
        return nullptr;
        }
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

    ECJsonUtilities::IdToJson(element[BIS_ELEMENT_PROP_CodeSpecId][ECJsonUtilities::json_navId()], codeSpec);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2017
//---------------+---------------+---------------+---------------+---------------+-------
CodeSpecId Reader::GetMappedCodeSpecId(Json::Value& element)
    {
    if (!element.isMember(BIS_ELEMENT_PROP_CodeSpecId))
        {
        GetLogger().errorv("No CodeSpec specified for %s element", _GetElementType());
        return CodeSpecId();
        }

    Utf8String idString = element[BIS_ELEMENT_PROP_CodeSpecId][ECJsonUtilities::json_navId()].asString();
    if (!Utf8String::IsNullOrEmpty(idString.c_str()))
        {
        CodeSpecId id = ECJsonUtilities::JsonToId<CodeSpecId>(element[BIS_ELEMENT_PROP_CodeSpecId]["id"]);
        if (!id.IsValid())
            {
            GetLogger().errorv("Failed to parse CodeSpec.id property value '%s' into a CodeSpecId.", idString.c_str());
            return CodeSpecId();
            }
        CodeSpecId codeSpec = m_importer->m_syncInfo->LookupCodeSpec(id);
        if (!codeSpec.IsValid())
            {
            GetLogger().errorv("Unable to remap CodeSpecId '%s'", idString.c_str());
            return CodeSpecId();
            }
        return codeSpec;
        }
    GetLogger().errorv("Missing or empty CodeSpecId field");
    return CodeSpecId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ElementReader::RemapParentId(Json::Value& element)
    {
    if (!element.isMember(BIS_ELEMENT_PROP_Parent))
        return SUCCESS;

    Utf8String parentId = element[BIS_ELEMENT_PROP_Parent][ECJsonUtilities::json_navId()].asString();
    if (!(Utf8String::IsNullOrEmpty(parentId.c_str())))
        {
        DgnElementId id = ECJsonUtilities::JsonToId<DgnElementId>(element[BIS_ELEMENT_PROP_Parent][ECJsonUtilities::json_navId()]);
        if (!id.IsValid())
            {
            GetLogger().errorv("Failed to parse Parent.id property '%s' into a DgnElementId.\n", parentId.c_str());
            return ERROR;
            }
        DgnElementId mappedId = m_importer->m_syncInfo->LookupElement(id);
        if (!mappedId.IsValid())
            {
            GetLogger().errorv("Failed to remap Parent.id '%s'\n", parentId.c_str());
            return ERROR;
            }

        ECJsonUtilities::IdToJson(element[BIS_ELEMENT_PROP_Parent][ECJsonUtilities::json_navId()], mappedId);
        element[BIS_ELEMENT_PROP_Parent][ECJsonUtilities::json_navRelClassName()] = _GetRelationshipClassName().c_str();
        return SUCCESS;
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ElementReader::RemapCategoryId(Json::Value& element)
    {
    if (!element.isMember("Category"))
        return SUCCESS;

    Utf8String catId = element["Category"][ECJsonUtilities::json_navId()].asString();
    if (!(Utf8String::IsNullOrEmpty(catId.c_str())))
        {
        DgnElementId id = ECJsonUtilities::JsonToId<DgnElementId>(element["Category"][ECJsonUtilities::json_navId()]);
        if (!id.IsValid())
            {
            GetLogger().errorv("Failed to parse Category.id property '%s' into a DgnElementId.\n", catId.c_str());
            return ERROR;
            }
        DgnElementId mappedId = m_importer->m_syncInfo->LookupElement(id);
        if (!mappedId.IsValid())
            {
            GetLogger().errorv("Failed to remap Category.id '%s'\n", catId.c_str());
            return ERROR;
            }
        ECJsonUtilities::IdToJson(element["Category"][ECJsonUtilities::json_navId()], mappedId);
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

    ECJsonUtilities::IdToJson(element[BIS_ELEMENT_PROP_Model][ECJsonUtilities::json_navId()], mappedId);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2017
//---------------+---------------+---------------+---------------+---------------+-------
DgnModelId ElementReader::GetMappedModelId(Json::Value& element, Utf8CP propertyName)
    {
    // remap modelId
    Utf8String modelId = element[propertyName]["id"].asString();
    if (Utf8String::IsNullOrEmpty(modelId.c_str()))
        {
        GetLogger().errorv("Model roperty '%s.id' is empty.\n", propertyName);
        return DgnModelId();
        }

    DgnModelId id = ECJsonUtilities::JsonToId<DgnModelId>(element[propertyName][ECJsonUtilities::json_navId()]);
    if (!id.IsValid())
        {
        GetLogger().errorv("Failed to parse %s property value '%s' into a DgnModelId.", propertyName, modelId.c_str());
        return DgnModelId();
        }
    DgnModelId mappedId = m_importer->m_syncInfo->LookupModel(DgnModelId(id));
    if (!mappedId.IsValid())
        {
        GetLogger().errorv("Failed to map '%s.id' property value '%s'", propertyName, modelId.c_str());
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

    DgnElementId codeScope = GetMappedElementId(element, BIS_ELEMENT_PROP_CodeScope);

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
    if (val.IsNull() || val.IsUninitialized())
        return SUCCESS;
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
    DgnElementId id = ECJsonUtilities::JsonToId<DgnElementId>(element[propertyName][ECJsonUtilities::json_navId()]);
    Utf8String elementId = element[propertyName]["id"].asString();
    if (!id.IsValid())
        {
        GetLogger().errorv("Failed to parse ElementId property (%s) of element: %s", propertyName, element.toStyledString().c_str());
        return DgnElementId();
        }

    DgnElementId mappedId = GetSyncInfo()->LookupElement(id);
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
    CodeSpecId oldId = ECJsonUtilities::JsonToId<CodeSpecId>(object[ECJsonUtilities::json_navId()]);
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
// @bsimethod                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus LsComponentReader::_Read(Json::Value& object)
    {
    LsComponentId v10Id;
    
    Json::Value jsonValue;
    Utf8String name = object["Name"].asString();
    LsComponentType componentType = LsComponentType::LineCode;
    if (name.EqualsIAscii("CompoundV1"))
        componentType = LsComponentType::Compound;
    else if (name.EqualsIAscii("LineCodeV1"))
        componentType = LsComponentType::LineCode;
    else if (name.EqualsIAscii("LinePointV1"))
        componentType = LsComponentType::LinePoint;
    else if (name.EqualsIAscii("PointSymV1"))
        componentType = LsComponentType::PointSymbol;
    else if (name.EqualsIAscii("RasterImageV1"))
        componentType = LsComponentType::RasterImage;

    uint32_t id = object["Id"].asUInt();
    Json::Reader::Parse(object["StrData"].asString().c_str(), jsonValue);
    if (jsonValue.isMember("geomPartId"))
        {
        uint64_t oldId = jsonValue["geomPartId"].asUInt64();
        DgnElementId mappedId = GetSyncInfo()->LookupElement(DgnElementId(oldId));
        if (!mappedId.IsValid())
            {
            GetLogger().errorv("Failed to remap geometry part Id '%" PRIu64 " LsComponent", oldId);
            return ERROR;
            }
        jsonValue["geomPartId"] = mappedId.GetValue();
        }
    LineStyleStatus lstat = LsComponent::AddComponentAsJsonProperty(v10Id, *GetDgnDb(), componentType, jsonValue);

    if (LINESTYLE_STATUS_Success != lstat)
        {
        GetLogger().warningv("Unable to add linestyle property.  Error code: %d\n", lstat);
        return ERROR;
        }

    LsComponentId oldId(componentType, id);
    GetSyncInfo()->InsertLsComponent(oldId, v10Id);

    return SUCCESS;
    }

#define EMBEDDED_FACE_DATA_PROP_NS "dgn_Font"
#define EMBEDDED_FACE_DATA_PROP_NAME "EmbeddedFaceData"
static const PropertySpec EMBEDDED_FACE_DATA_PROPERTY_SPEC(EMBEDDED_FACE_DATA_PROP_NAME, EMBEDDED_FACE_DATA_PROP_NS);

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus FontFaceReader::_Read(Json::Value& object)
    {
    Statement idQuery;
    idQuery.Prepare(*GetDgnDb(), "SELECT MAX(Id) FROM " BEDB_TABLE_Property " WHERE Namespace='" EMBEDDED_FACE_DATA_PROP_NS "' AND Name='" EMBEDDED_FACE_DATA_PROP_NAME "'");
    if (BE_SQLITE_ROW != idQuery.Step())
        {
        BeAssert(false);
        return ERROR;
        }

    uint64_t id = (idQuery.GetValueInt64(0) + 1);

    bvector<Byte> data;
    int size = object["RawSize"].asInt();
    data.resize((size_t) size);
    data.clear();
    Utf8String dataAsStr(object["Base64EncodedData"].asString().c_str());
    int encodedSize = object["Base64EncodedSize"].asInt();
    Base64Utilities::Decode(data, dataAsStr.c_str(), encodedSize);

    // Db does not expose a SaveProperty that lets us save a string and blob at the same time.
    // Use the API for the blob so we get compression; manually insert the table of contents string.

    if (BE_SQLITE_OK != GetDgnDb()->SaveProperty(EMBEDDED_FACE_DATA_PROPERTY_SPEC, data.data(), (uint32_t) size, id, 0))
        return ERROR;

    Statement update;
    update.Prepare(*GetDgnDb(), "UPDATE " BEDB_TABLE_Property " SET StrData=? WHERE Namespace='" EMBEDDED_FACE_DATA_PROP_NS "' AND Name='" EMBEDDED_FACE_DATA_PROP_NAME "' AND Id=?");
    update.BindText(1, object["StrData"].asString().c_str(), Statement::MakeCopy::No);
    update.BindInt64(2, id);

    if (BE_SQLITE_DONE != update.Step())
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus FontReader::_Read(Json::Value& font)
    {
    DgnFontId id;
    DbResult rc = GetDgnDb()->GetServerIssuedId(id, "dgn_Font", "Id");

    if (BE_SQLITE_OK != rc)
        {
        BeAssert(false);
        return ERROR;
        }

    Statement insert;
    insert.Prepare(*GetDgnDb(), SqlPrintfString("INSERT INTO %s (Id,Type,Name,Metadata) VALUES (?,?,?,?)", "dgn_Font"));
    insert.BindInt(1, static_cast<int>(id.GetValue()));
    insert.BindInt(2, static_cast<int>(font["Type"].asInt()));
    insert.BindText(3, font["Name"].asString().c_str(), Statement::MakeCopy::No);

    bvector<Byte> metadata;
    size_t size = font["Metadata"].asString().SizeInBytes();
    metadata.resize(size);
    Base64Utilities::Decode(metadata, font["Metadata"].asString().c_str(), size);

    insert.BindBlob(4, metadata.data(), (int) metadata.size(), Statement::MakeCopy::No);

    if (BE_SQLITE_DONE != insert.Step())
        {
        BeAssert(false);
        return ERROR;
        }

    DgnFontId oldId = ECJsonUtilities::JsonToId<DgnFontId>(font["Id"]);
    GetSyncInfo()->InsertFont(oldId, id);
    m_importer->RemapFont(oldId);


    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SubjectReader::_Read(Json::Value& subject)
    {
    Utf8String label = subject["Label"].asString();
    DgnElementId parentId = ECJsonUtilities::JsonToId<DgnElementId>(subject["ParentId"]);
    DgnElementId oldInstanceId = ECJsonUtilities::JsonToId<DgnElementId>(subject[ECJsonSystemNames::Id()]);
    SubjectCPtr parentSubject;
    if (!parentId.IsValid())
        parentSubject = GetDgnDb()->Elements().GetRootSubject();
    else
        {
        DgnElementId mappedId = GetSyncInfo()->LookupElement(parentId);
        if (mappedId.IsValid())
            parentSubject = GetDgnDb()->Elements().Get<Subject>(mappedId);
        else
            {
            GetLogger().warningv("Unable to locate parent subject with exported id %s.  Using root subject instead", parentId.ToString().c_str());
            parentSubject = GetDgnDb()->Elements().GetRootSubject();
            }
        }
    SubjectCPtr subjectElement = Subject::CreateAndInsert(*parentSubject, label.c_str(), subject["Descr"].isNull() ? nullptr : subject["Descr"].asString().c_str());
    if (!subjectElement.IsValid())
        return ERROR;

    if (!GetDgnDb()->TableExists(SYNCINFO_ATTACH(SYNC_TABLE_File)))
        GetSyncInfo()->CreateTables();

    SyncInfo::ElementMapping map(oldInstanceId, subjectElement->GetElementId());
    map.Insert(*GetDgnDb());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus PartitionReader::_Read(Json::Value& partition)
    {
    Utf8String label = partition["Label"].asString();
    DgnElementId oldInstanceId = ECJsonUtilities::JsonToId<DgnElementId>(partition[ECJsonSystemNames::Id()]);
    DgnElementId subjectId = ECJsonUtilities::JsonToId<DgnElementId>(partition["Subject"]);
    SubjectCPtr subject;
    if (!subjectId.IsValid())
        subject = GetDgnDb()->Elements().GetRootSubject();
    else
        {
        DgnElementId mappedId = GetSyncInfo()->LookupElement(subjectId);
        if (mappedId.IsValid())
            subject = GetDgnDb()->Elements().Get<Subject>(mappedId);
        else
            {
            GetLogger().warningv("Unable to locate partition's subject with exported id %s.  Using root subject instead", subjectId.ToString().c_str());
            subject = GetDgnDb()->Elements().GetRootSubject();
            }
        }

    Utf8String partitionType = partition["PartitionType"].asString();
    DgnElementId newId;
    if (partitionType.Equals("GroupInformationPartition"))
        {
        GroupInformationPartitionCPtr gip = GroupInformationPartition::CreateAndInsert(*subject, label.c_str(), partition["Descr"].isNull() ? nullptr : partition["Descr"].asCString());
        if (!gip.IsValid())
            {
            GetLogger().errorv("Failed to create a GroupInformationPartition for %" PRIu64 "", oldInstanceId);
            return ERROR;
            }
        newId = gip->GetElementId();
        }
    else if (partitionType.Equals("PhysicalPartition"))
        {
        PhysicalPartitionCPtr pp = PhysicalPartition::CreateAndInsert(*subject, label.c_str(), partition["Descr"].isNull() ? nullptr : partition["Descr"].asCString());
        if (!pp.IsValid())
            {
            GetLogger().errorv("Failed to create a PhysicalPartition for %s", oldInstanceId.ToString().c_str());
            return ERROR;
            }
        newId = pp->GetElementId();
        }
    else if (partitionType.Equals("DocumentPartition"))
        {
        DocumentPartitionCPtr dp = DocumentPartition::CreateAndInsert(*subject, label.c_str(), partition["Descr"].isNull() ? nullptr : partition["Descr"].asCString());
        if (!dp.IsValid())
            {
            GetLogger().errorv("Failed to create a DocumentPartition for %s", oldInstanceId.ToString().c_str());
            return ERROR;
            }
        newId = dp->GetElementId();
        }
    else if (partitionType.Equals("LinkPartition"))
        {
        LinkPartitionCPtr lp = LinkPartition::CreateAndInsert(*subject, label.c_str(), partition["Descr"].isNull() ? nullptr : partition["Descr"].asCString());
        if (!lp.IsValid())
            {
            GetLogger().errorv("Failed to create a LinkPartition for %s", oldInstanceId.ToString().c_str());
            return ERROR;
            }
        newId = lp->GetElementId();
        }
    else if (partitionType.Equals("DefinitionPartition"))
        {
        DefinitionPartitionCPtr dp = DefinitionPartition::CreateAndInsert(*subject, label.c_str(), partition["Descr"].isNull() ? nullptr : partition["Descr"].asCString());
        if (!dp.IsValid())
            {
            GetLogger().errorv("Failed to create DefinitionPartition for %s", oldInstanceId.ToString().c_str());
            return ERROR;
            }
        newId = dp->GetElementId();
        }
    else if (partitionType.Equals("PlanningPartition"))
        {
        PlanningPartitionCPtr pp = PlanningPartition::CreateAndInsert(*subject, label.c_str(), partition["Descr"].isNull() ? nullptr : partition["Descr"].asCString());
        if (!pp.IsValid())
            {
            GetLogger().errorv("Failed to create PlanningPartition for %s", oldInstanceId.ToString().c_str());
            return ERROR;
            }
        newId = pp->GetElementId();
        }
    else
        {
        GetLogger().errorv("Unknown (or empty) partition type '%s' for element %s", partitionType.c_str(), oldInstanceId.ToString().c_str());
        return ERROR;
        }
    SyncInfo::ElementMapping map(oldInstanceId, newId);
    map.Insert(*GetDgnDb());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2017
//---------------+---------------+---------------+---------------+---------------+-------
Utf8String ElementReader::_GetRelationshipClassName()
    {
    return BIS_SCHEMA(BIS_REL_ElementOwnsChildElements); 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2017
//---------------+---------------+---------------+---------------+---------------+-------
ECN::ECClassCP ElementReader::_GetClassFromName(Utf8CP classKey, Json::Value& element)
    {
    ECN::ECClassCP ecClass = T_Super::_GetClassFromName(classKey, element);
    if (nullptr != ecClass)
        return ecClass;

    GetLogger().warningv("Unable to find ECClass for %s.  This is possibly due to a failed schema import.  Defaulting to generic BisCore element.", classKey);
    DgnModelId mappedId = GetMappedModelId(element);
    DgnModelPtr model = GetDgnDb()->Models().GetModel(mappedId);
    if (model.IsValid() && model->Is2dModel())
        return GetDgnDb()->Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_CLASS_DrawingGraphic);
    else
        return GetDgnDb()->Schemas().GetClass(GENERIC_DOMAIN_NAME, GENERIC_CLASS_PhysicalObject);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ElementReader::_Read(Json::Value& element)
    {
    m_instanceId = ECJsonUtilities::JsonToId<DgnElementId>(element[ECJsonSystemNames::Id()]);

    if (!m_instanceId.IsValid())
        return ERROR;

    IECInstancePtr ecInstance = _CreateInstance(element);
    if (!ecInstance.IsValid())
        {
        GetLogger().errorv("Failed to create IECInstance for element %s\n", _GetElementType());
        return ERROR;
        }

    if (SUCCESS != RemapPropertyElementId(*ecInstance, BIS_ELEMENT_PROP_CodeScope))
        {
        GetLogger().errorv("Failed to map CodeScopeId for %s instance.", _GetElementType());
        return ERROR;
        }

    if (SUCCESS != _OnInstanceCreated(*ecInstance.get()))
        {
        GetLogger().errorv("Failed to post-process IECInstance for %s\n", _GetElementType());
        return ERROR;
        }

    DgnDbStatus stat;
    DgnElementPtr dgnElement = GetDgnDb()->Elements().CreateElement(*ecInstance, true, &stat);

    if (!dgnElement.IsValid())
        {
        GetLogger().errorv("Failed to create %s from instance.  Error code: %d\n", _GetElementType(), stat);
        return ERROR;
        }

    if (SUCCESS != _OnElementCreated(*dgnElement.get(), *ecInstance))
        {
        GetLogger().errorv("Failed to post-process element for %s\n", _GetElementType());
        return ERROR;
        }

    DgnElementCPtr inserted = dgnElement->Insert(&stat);
    if (DgnDbStatus::DuplicateCode == stat)
        {
        DgnDbStatus stat2 = dgnElement->SetCode(DgnCode::CreateEmpty()); // just leave the code null
        BeAssert(DgnDbStatus::Success == stat2);
        inserted = dgnElement->Insert(&stat);
        }

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
    if (!m_geomStream.HasData())
        return SUCCESS;
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
// @bsimethod                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus AnnotationTextStyleReader::_Read(Json::Value& object)
    {
    Json::Value properties(object["ExplicitProperties"]);
    object.removeMember("ExplicitProperties");

    m_instanceId = ECJsonUtilities::JsonToId<DgnElementId>(object[ECJsonSystemNames::Id()]);
    if (!m_instanceId.IsValid())
        return ERROR;

    IECInstancePtr ecInstance = _CreateInstance(object);
    if (!ecInstance.IsValid())
        {
        GetLogger().errorv("Failed to create IECInstance for element %s\n", _GetElementType());
        return ERROR;
        }

    if (SUCCESS != _OnInstanceCreated(*ecInstance.get()))
        {
        GetLogger().errorv("Failed to post-process IECInstance for %s\n", _GetElementType());
        return ERROR;
        }

    DgnDbStatus stat;
    AnnotationTextStylePtr ats = GetDgnDb()->Elements().Create<AnnotationTextStyle>(*ecInstance, &stat);
    if (!ats.IsValid())
        {
        GetLogger().errorv("Failed to create AnnotationTextStyle from instance.  Error code: %d\n", stat);
        return ERROR;
        }

    ats->SetColorType((AnnotationColorType) properties["AnnotationColorType"].asInt());
    ats->SetColorValue(ColorDef(properties["ColorValue"].asInt()));
    ats->SetFontId(ECJsonUtilities::JsonToId<DgnFontId>(properties["FontId"]));
    ats->SetHeight(properties["Height"].asDouble());
    ats->SetLineSpacingFactor(properties["LineSpacingFactor"].asDouble());
    ats->SetIsBold(properties["SetIsBold"].asBool());
    ats->SetIsItalic(properties["SetIsItalic"].asBool());
    ats->SetIsUnderlined(properties["SetIsUnderlined"].asBool());
    ats->SetStackedFractionScale(properties["StackedFractionScale"].asDouble());
    ats->SetStackedFractionType((AnnotationStackedFractionType) properties["StackedFractionType"].asInt());
    ats->SetSubScriptOffsetFactor(properties["SubScriptOffsetFactor"].asDouble());
    ats->SetSubScriptScale(properties["SubScriptScale"].asDouble());
    ats->SetWidthFactor(properties["WidthFactor"].asDouble());

    DgnElementCPtr inserted = ats->Insert();
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
// @bsimethod                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus LineStyleReader::_OnElementCreated(DgnElementR element, ECN::IECInstanceR properties)
    {
    ECValue v;
    properties.GetValue(v, "Data");
    Json::Value json;
    Json::Reader::Parse(Utf8String(v.GetUtf8CP()), json);
    
    uint32_t id = json["compId"].asUInt();
    int32_t type = json["compType"].asInt();
    LsComponentId oldId((LsComponentType) type, id);
    LsComponentId newId = GetSyncInfo()->LookupLsComponent(oldId);
    json["compId"] = newId.GetValue();

    Utf8String data = Json::FastWriter::ToString(json);
    v.SetUtf8CP(data.c_str());
    properties.SetValue("Data", v);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ViewDefinitionReader::_Read(Json::Value& viewDefinition)
    {
    DgnElementId instanceId = ECJsonUtilities::JsonToId<DgnElementId>(viewDefinition[ECJsonSystemNames::Id()]);
    if (!instanceId.IsValid())
        return ERROR;

    DgnModelId modelId = GetMappedModelId(viewDefinition);
    if (!modelId.IsValid())
        return ERROR;

    DgnCode dgnCode = CreateCodeFromJson(viewDefinition);
    if (!dgnCode.IsValid())
        return ERROR;

    DgnElementId displayStyleId = GetMappedElementId(viewDefinition, "DisplayStyle");
    if (!displayStyleId.IsValid())
        return ERROR;

    DgnElementId categorySelectorId = GetMappedElementId(viewDefinition, "CategorySelector");
    if (!categorySelectorId.IsValid())
        return ERROR;
    CategorySelectorPtr categorySelector = GetDgnDb()->Elements().GetForEdit<CategorySelector>(categorySelectorId); // We aren't editing it, but the CreateParams wants a ref, not a const ref
    if (!categorySelector.IsValid())
        return ERROR;

    ECClassCP ecClass = _GetClassFromName(viewDefinition[ECJsonUtilities::json_className()].asString().c_str(), viewDefinition);
    if (nullptr == ecClass)
        return ERROR;

    DefinitionModelPtr model = GetDgnDb()->Models().Get<DefinitionModel>(DgnModelId(modelId.GetValue()));
    if (!model.IsValid())
        return ERROR;

    if (m_is3d)
        {
        DisplayStyle3dPtr displayStyle = GetDgnDb()->Elements().GetForEdit<DisplayStyle3d>(displayStyleId); // We aren't editing it, but the CreateParams wants a ref, not a const ref
        if (!displayStyle.IsValid())
            return ERROR;

        if (nullptr == m_importer->m_orthographicViewClass)
            m_importer->m_orthographicViewClass = GetDgnDb()->Schemas().GetClass(BIS_ECSCHEMA_NAME, "OrthographicViewDefinition");

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

        DgnElementId modelSelectorId = GetMappedElementId(viewDefinition, "ModelSelector");
        if (!modelSelectorId.IsValid())
            return ERROR;
        ModelSelectorPtr modelSelector = GetDgnDb()->Elements().GetForEdit<ModelSelector>(modelSelectorId); // We aren't editing it, but the CreateParams wants a ref, not a const ref
        if (!modelSelector.IsValid())
            return ERROR;

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

            viewDef = new SpatialViewDefinition(*model, dgnCode.GetValueUtf8(), *categorySelector, *displayStyle, *modelSelector, camera);
            }
        else
            viewDef = new OrthographicViewDefinition(*model, dgnCode.GetValueUtf8(), *categorySelector, *displayStyle, *modelSelector);

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
        SyncInfo::ElementMapping map(instanceId, inserted->GetElementId());
        map.Insert(*GetDgnDb());
        delete viewDef;
        delete camera;
        }
    else
        {
        DisplayStyle2dPtr displayStyle = GetDgnDb()->Elements().GetForEdit<DisplayStyle2d>(displayStyleId); // We aren't editing it, but the CreateParams wants a ref, not a const ref
        if (!displayStyle.IsValid())
            return ERROR;

        DgnModelId baseModelId = GetMappedModelId(viewDefinition, "BaseModel");
        if (!baseModelId.IsValid())
            return ERROR;

        DPoint2d extents;
        ECJsonUtilities::JsonToPoint2d(extents, viewDefinition["Extents"]);

        DPoint2d origin;
        ECJsonUtilities::JsonToPoint2d(origin, viewDefinition["Origin"]);

        ViewDefinition2d* viewDef;
        if (nullptr == m_importer->m_sheetViewClass)
            m_importer->m_sheetViewClass = GetDgnDb()->Schemas().GetClass(BIS_ECSCHEMA_NAME, "SheetViewDefinition");

        if (ecClass->Is(m_importer->m_sheetViewClass))
            viewDef = new SheetViewDefinition(*model, dgnCode.GetValueUtf8(), baseModelId, *categorySelector, *displayStyle);
        else
            viewDef = new DrawingViewDefinition(*model, dgnCode.GetValueUtf8(), baseModelId, *categorySelector, *displayStyle);

        DgnDbStatus stat;
        DgnElementCPtr inserted = viewDef->Insert(&stat);
        if (!inserted.IsValid())
            {
            GetLogger().errorv("Failed to insert newly created %s into db.  Error code %d\n", _GetElementType(), stat);
            return ERROR;
            }
        SyncInfo::ElementMapping map(instanceId, inserted->GetElementId());
        map.Insert(*GetDgnDb());
        delete viewDef;
        }

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

    return T_Super::_OnInstanceCreated(properties);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus CategoryReader::_Read(Json::Value& category)
    {
    IECInstancePtr ecInstance = _CreateInstance(category);
    if (!ecInstance.IsValid())
        return ERROR;

    m_instanceId = ECJsonUtilities::JsonToId<DgnCategoryId>(category[ECJsonSystemNames::Id()]);
    if (!m_instanceId.IsValid())
        return ERROR;

    DgnDbStatus stat;
    DgnElementPtr dgnElement = GetDgnDb()->Elements().CreateElement(*ecInstance, false, &stat);

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

    GetSyncInfo()->InsertCategory(DgnCategoryId(m_instanceId.GetValue()), DgnCategoryId(inserted->GetElementId().GetValue()));
    m_importer->RemapCategory(DgnCategoryId(m_instanceId.GetValue()));
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SubCategoryReader::_Read(Json::Value& subCategory)
    {
    DgnSubCategoryId instanceId = ECJsonUtilities::JsonToId<DgnSubCategoryId>(subCategory[ECJsonSystemNames::Id()]);

    // Trying to set the description during creation of the element causes an assertion.  For now, we work around this by saving the description
    // and then we'll use the API to set it after the element has been created.
    Utf8String description = subCategory["Description"].asString();
    subCategory.removeMember("Description");

    // If this is the default sub category, we don't actually create it.  Just remap it
    if (m_isDefault)
        {
        DgnCategoryId oldParentId = ECJsonUtilities::JsonToId<DgnCategoryId>(subCategory["Parent"][ECJsonSystemNames::Id()]);
        if (!oldParentId.IsValid())
            return ERROR;
        DgnCategoryId categoryId = GetSyncInfo()->LookupCategory(oldParentId);
        if (!categoryId.IsValid())
            {
            GetLogger().errorv("Unable to map CategoryId for default SubCategory");
            GetLogger().infov("%s", subCategory.toStyledString().c_str());
            return ERROR;
            }
        DgnCategoryCPtr cat = GetDgnDb()->Elements().Get<DgnCategory>(categoryId);
        if (!cat.IsValid())
            {
            GetLogger().errorv("Unable to retrieve Category %" PRIu64 "", categoryId.GetValue());
            return ERROR;
            }
        GetSyncInfo()->InsertSubCategory(categoryId, instanceId, cat->GetDefaultSubCategoryId());
        m_importer->RemapSubCategory(categoryId, instanceId);
        if (!subCategory["Properties"].isNull())
            {
            Json::Value val = Json::Value::From(subCategory["Properties"].asString());
            DgnSubCategory::Appearance appearance(val);
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

    GetSyncInfo()->InsertSubCategory(categoryId, instanceId, DgnSubCategoryId(inserted->GetElementId().GetValue()));
    m_importer->RemapSubCategory(categoryId, instanceId);
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
        DgnElementId id = ECJsonUtilities::JsonToId<DgnElementId>(model[MODEL_PROP_ModeledElement][ECJsonSystemNames::Id()]);
        DgnElementId mappedId = GetSyncInfo()->LookupElement(id);
        if (!mappedId.IsValid())
            {
            // What do we do here? Map to the root subject?
            return ERROR;
            }
        ECJsonUtilities::IdToJson(model[MODEL_PROP_ModeledElement][ECJsonSystemNames::Id()], mappedId);
        }

    DgnModelId ecInstanceId = ECJsonUtilities::JsonToId<DgnModelId>(model[ECJsonSystemNames::Id()]);
    if (!ecInstanceId.IsValid())
        return ERROR;

    IECInstancePtr ecInstance = _CreateInstance(model);
    if (!ecInstance.IsValid())
        return ERROR;

    SyncInfo::ModelMapping mapping;
    // there is one and only one dictionary model. So if this is a dictionary model, we don't create a model, we just remap
    // its id to the new one.  Also create an element mapping as this is needed for CodeScope remappings
    if (m_isDictionary)
        {
        GetSyncInfo()->InsertModel(mapping, GetDgnDb()->GetDictionaryModel().GetModelId(), DgnModelId(ecInstanceId), nullptr);
        SyncInfo::ElementMapping map(DgnElementId(ecInstanceId.GetValue()), GetDgnDb()->GetDictionaryModel().GetModeledElementId());
        map.Insert(*GetDgnDb());

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
    GetSyncInfo()->InsertModel(mapping, dgnModel->GetModelId(), ecInstanceId, nullptr);

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

    DgnElementId definitionModelId = GetMappedElementId(object, "DefinitionModel");
    if (!definitionModelId.IsValid())
        {
        GetLogger().errorv("Could not map DefinitionModel for CategorySelector");
        return ERROR;
        }

    DefinitionModelPtr model = GetDgnDb()->Models().Get<DefinitionModel>(DgnModelId(definitionModelId.GetValue()));
    if (!model.IsValid())
        return ERROR;

    CategorySelector selector(*model, object["Name"].asString().c_str());
    DgnElementId oldInstanceId = ECJsonUtilities::JsonToId<DgnElementId>(object[ECJsonSystemNames::Id()]);

    for (Json::Value::iterator iter = categories.begin(); iter != categories.end(); iter++)
        {
        Json::Value& member = *iter;
        DgnCategoryId id = ECJsonUtilities::JsonToId<DgnCategoryId>(member);
        DgnCategoryId lookup = GetSyncInfo()->LookupCategory(id);
        selector.AddCategory(lookup);
        }

    DgnDbStatus stat;
    selector.Insert(&stat);
    if (stat != DgnDbStatus::Success)
        return ERROR;
    SyncInfo::ElementMapping map(oldInstanceId, selector.GetElementId());
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

    DgnElementId definitionModelId = GetMappedElementId(object, "DefinitionModel");
    if (!definitionModelId.IsValid())
        {
        GetLogger().errorv("Could not map DefinitionModel for CategorySelector");
        return ERROR;
        }

    DefinitionModelPtr model = GetDgnDb()->Models().Get<DefinitionModel>(DgnModelId(definitionModelId.GetValue()));
    if (!model.IsValid())
        return ERROR;

    ModelSelector selector(*model, object["Name"].asString().c_str());
    DgnElementId oldInstanceId = ECJsonUtilities::JsonToId<DgnElementId>(object[ECJsonSystemNames::Id()]);

    for (Json::Value::iterator iter = models.begin(); iter != models.end(); iter++)
        {
        Json::Value& member = *iter;
        DgnModelId id = ECJsonUtilities::JsonToId<DgnModelId>(member);
        DgnModelId lookup = GetSyncInfo()->LookupModel(DgnModelId(id));
        selector.AddModel(lookup);
        }

    DgnDbStatus stat;
    selector.Insert(&stat);
    if (stat != DgnDbStatus::Success)
        return ERROR;
    SyncInfo::ElementMapping map(oldInstanceId, selector.GetElementId());
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
    DgnModelId modelId = GetMappedModelId(object);
    if (!modelId.IsValid())
        return ERROR;

    DefinitionModelPtr model = GetDgnDb()->Models().Get<DefinitionModel>(modelId);
    if (!model.IsValid())
        return ERROR;

    DisplayStyle* displayStyle;
    if (object["Is3d"].asBool())
        displayStyle = new DisplayStyle3d(*model, displayStyleName);
    else
        displayStyle = new DisplayStyle2d(*model, displayStyleName);
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

    DgnElementId oldInstanceId = ECJsonUtilities::JsonToId<DgnElementId>(object[ECJsonSystemNames::Id()]);
    DgnDbStatus stat;
    displayStyle->Insert(&stat);
    if (stat != DgnDbStatus::Success)
        return ERROR;
    SyncInfo::ElementMapping map(oldInstanceId, displayStyle->GetElementId());
    map.Insert(*GetDgnDb());
    delete displayStyle;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus TextureReader::_Read(Json::Value& texture)
    {
    m_instanceId = ECJsonUtilities::JsonToId<DgnElementId>(texture[ECJsonSystemNames::Id()]);
    if (!m_instanceId.IsValid())
        return ERROR;

    DgnModelId modelId = GetMappedModelId(texture);
    if (!modelId.IsValid())
        return ERROR;

    DefinitionModelPtr model = GetDgnDb()->Models().Get<DefinitionModel>(modelId);
    if (!model.IsValid())
        return ERROR;

    uint32_t height = texture["Height"].asUInt();
    uint32_t width = texture["Width"].asUInt();
    Utf8String description = texture["Description"].asString();
    Render::ImageSource::Format format = (Render::ImageSource::Format) (texture["Format"].asUInt());

    bvector<Byte> data;
    size_t size = texture["Data"].asString().SizeInBytes();
    data.resize(size);
    Base64Utilities::Decode(data, texture["Data"].asString().c_str(), size);
    Render::ImageSource image(format, ByteStream(data.data(), size));
    Utf8String name = texture["UserLabel"].asString();
    if (Utf8String::IsNullOrEmpty(name.c_str()))
        name = texture["CodeValue"].asString();

    DgnTexture dgnTexture(DgnTexture::CreateParams(*model, texture["UserLabel"].asString(), image, width, height, description));
    DgnDbStatus stat;
    dgnTexture.Insert(&stat);
    if (stat != DgnDbStatus::Success)
        return ERROR;
    SyncInfo::ElementMapping map(m_instanceId, dgnTexture.GetElementId());
    map.Insert(*GetDgnDb());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2018
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus PlanReader::_OnInstanceCreated(ECN::IECInstanceR instance)
    {
    // remap the Plan id
    if (ERROR == RemapPropertyElementId(instance, "Plan"))
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2018
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus SchemaReader::ValidateBaseClasses(ECN::ECSchemaP schema)
    {
    ECN::ECClassP definitionElement = const_cast<ECN::ECClassP>(m_importer->GetDgnDb()->Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_CLASS_DefinitionElement));
    ECN::ECClassCP elementClass = m_importer->GetDgnDb()->Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_CLASS_Element);
    ECN::ECClassCP multiAspectClass = m_importer->GetDgnDb()->Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_CLASS_ElementMultiAspect);
    ECN::ECClassCP uniqueAspectClass = m_importer->GetDgnDb()->Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_CLASS_ElementUniqueAspect);
    ECN::ECRelationshipClassCP elementToMulti = m_importer->GetDgnDb()->Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsMultiAspects)->GetRelationshipClassCP();
    ECN::ECRelationshipClassCP elementToUnique = m_importer->GetDgnDb()->Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_REL_ElementOwnsUniqueAspect)->GetRelationshipClassCP();
    ECN::ECRelationshipClassCP elementToElement = m_importer->GetDgnDb()->Schemas().GetClass(BIS_ECSCHEMA_NAME, BIS_REL_ElementRefersToElements)->GetRelationshipClassCP();
    // need to confirm that all relationship classes have a base class.  In 0601 we were able to get away with not having a base class.  That doesn't work here
    for (ECN::ECClassCP ecClass : schema->GetClasses())
        {
        if (ecClass->GetBaseClasses().size() != 0)
            continue;

        if (ecClass->IsEntityClass())
            {
            ECN::ECClassP nonConstClass = const_cast<ECN::ECClassP>(ecClass);
            if (ECN::ECObjectsStatus::Success != nonConstClass->AddBaseClass(*definitionElement))
                {
                schema->AddReferencedSchema(definitionElement->GetSchemaR(), "bis");
                nonConstClass->AddBaseClass(*definitionElement);
                }
            continue;
            }
        ECN::ECRelationshipClassP relClass = const_cast<ECN::ECRelationshipClassP>(ecClass->GetRelationshipClassCP());
        if (nullptr == relClass)
            continue;

        bool hasNav = false;
        // If there is a class that has a NavigationProperty to this class, then it doesn't need a base class
        for (ECN::ECClassCP ecClass2 : schema->GetClasses())
            {
            if (!ecClass2->IsEntityClass())
                continue;
            for (ECN::ECPropertyCP prop : ecClass2->GetProperties(false))
                {
                ECN::NavigationECPropertyCP navProp = prop->GetAsNavigationProperty();
                if (nullptr == navProp)
                    continue;
                if (navProp->GetRelationshipClass() == relClass)
                    {
                    hasNav = true;
                    break;
                    }
                }
            if (hasNav)
                break;
            }
        if (hasNav)
            continue;
        ECN::ECClassCP source = relClass->GetSource().GetConstraintClasses()[0];
        ECN::ECClassCP target = relClass->GetTarget().GetConstraintClasses()[0];
        ECN::ECRelationshipClassCP base = nullptr;
        if (source->Is(elementClass))
            {
            if (target->Is(uniqueAspectClass))
                base = elementToUnique;
            else if (target->Is(multiAspectClass))
                base = elementToMulti;
            else if (target->Is(elementClass))
                base = elementToElement;
            }
        if (nullptr == base)
            {
            return ERROR;
            }
        relClass->GetSource().SetMultiplicity(base->GetSource().GetMultiplicity());
        relClass->SetStrength(base->GetStrength());
        relClass->SetStrengthDirection(base->GetStrengthDirection());
        relClass->AddBaseClass(*base);
        }
    return BSISUCCESS;
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
        BeFileName bimFileName = GetDgnDb()->GetFileName();
//#define EXPORT_0601JSON 1
#ifdef EXPORT_0601JSON
        BeFileName jsonFolder = bimFileName.GetDirectoryName().AppendToPath(bimFileName.GetFileNameWithoutExtension().AppendUtf8("_json").c_str());
        if (!jsonFolder.DoesPathExist())
            BeFileName::CreateNewDirectory(jsonFolder.GetName());

        WString jsonFile;
        jsonFile.AssignUtf8(schemaName);
        jsonFile.append(L".json");

        BeFileName jsonPath(jsonFolder);
        jsonPath.AppendToPath(jsonFile.c_str());

        if (jsonPath.DoesPathExist())
            jsonPath.BeDeleteFile();

        BeFile file;
        if (file.Create(jsonPath, true) == BeFileStatus::Success)
            {
            file.Write(nullptr, schema.asString().c_str(), (uint32_t) strlen(schema.asString().c_str()));
            file.Close();
            }
#endif

        BimImportSchemaLocater locater;
        SchemaKey key;
        SchemaKey::ParseSchemaFullName(key, schemaName);
        if (0 == strcmp("Planning", key.GetName().c_str()))
            continue;

        locater.AddSchemaXmlR(key, schema.asString());
        ECN::ECSchemaPtr ecSchema = locater.DeserializeSchema(*m_importer->m_schemaReadContext, ECN::SchemaMatchType::Exact);
        if (!ecSchema.IsValid())
            {
            GetLogger().fatalv("Failed to deserialize schema %s.  Unable to continue.", schemaName);
            return ERROR;
            }

//#define EXPORT_0601ECSCHEMAS 1
#ifdef EXPORT_0601ECSCHEMAS
        BeFileName outFolder = bimFileName.GetDirectoryName().AppendToPath(bimFileName.GetFileNameWithoutExtension().AppendUtf8("_0601").c_str());
        if (!outFolder.DoesPathExist())
            BeFileName::CreateNewDirectory(outFolder.GetName());

        WString fileName;
        fileName.AssignUtf8(ecSchema->GetFullSchemaName().c_str());
        fileName.append(L".ecschema.xml");

        BeFileName outPath(outFolder);
        outPath.AppendToPath(fileName.c_str());

        if (outPath.DoesPathExist())
            outPath.BeDeleteFile();

        ecSchema->WriteToXmlFile(outPath.GetName());
#endif

        // We need to deserialize the known schemas so that they can be used as references, but we don't want to convert or import them.
        bvector<Utf8String> knownSchemas = {"Bentley_Standard_CustomAttributes", "ECDbMap", "ECDbFileInfo", "ECDbSystem", "ECDbMeta", "ECDb_FileInfo", "ECDb_System", "EditorCustomAttributes", "Generic", "MetaSchema", "dgn", "Unit_Attributes"};
        if (knownSchemas.end() != std::find(knownSchemas.begin(), knownSchemas.end(), ecSchema->GetName()))
            continue;

        if (ecSchema->OriginalECXmlVersionLessThan(ECVersion::V3_1) && !ECSchemaConverter::Convert(*ecSchema))
            {
            GetLogger().fatalv("Failed to convert schema %s to EC3.1.  Unable to continue.", schemaName);
            return ERROR;
            }
        
        bvector<Utf8CP> schemasWithMultiInheritance = {"OpenPlant_3D", "BuildingDataGroup", "StructuralModelingComponents", "OpenPlant", "jclass", "pds", "group",
            "ams", "bmf", "pid", "schematics", "OpenPlant_PID", "OpenPlant3D_PID", "speedikon", "autoplant_PIW", "ECXA_autoplant_PIW", "Bentley_Plant", "globals", "Electrical_RCM", "pid_ansi"};
        auto found = std::find_if(schemasWithMultiInheritance.begin(), schemasWithMultiInheritance.end(), [key] (Utf8CP reserved) ->bool { return BeStringUtilities::StricmpAscii(key.GetName().c_str(), reserved) == 0; }) != schemasWithMultiInheritance.end();

        if (found || ecSchema->GetName().StartsWithI("ECXA_"))
            {
            SchemaFlattener flattener(m_importer->m_schemaReadContext);
            flattener.FlattenSchemas(ecSchema.get());
            }
        else if (ecSchema->GetName().StartsWithIAscii("SP3D"))
            {
            ECClassCP baseInterface = ecSchema->GetClassCP("BaseInterface");
            ECClassCP baseObject = ecSchema->GetClassCP("BaseObject");
            if (nullptr != baseInterface && nullptr != baseObject)
                {
                SchemaFlattener flattener(m_importer->m_schemaReadContext);
                flattener.ProcessSP3DSchema(ecSchema.get(), baseInterface, baseObject);
                }
            }
        else
            {
            ValidateBaseClasses(ecSchema.get());
            }

        ECSchemaP toImport = m_importer->m_schemaReadContext->GetCache().GetSchema(ecSchema->GetSchemaKey(), SchemaMatchType::Latest);
//#define EXPORT_FLATTENEDECSCHEMAS 1
#ifdef EXPORT_FLATTENEDECSCHEMAS
        BeFileName flatFolder = bimFileName.GetDirectoryName().AppendToPath(bimFileName.GetFileNameWithoutExtension().AppendUtf8("_flat").c_str());
        if (!flatFolder.DoesPathExist())
            BeFileName::CreateNewDirectory(flatFolder.GetName());

        WString flatFileName;
        flatFileName.AssignUtf8(toImport->GetFullSchemaName().c_str());
        flatFileName.append(L".ecschema.xml");

        BeFileName flatPath(flatFolder);
        flatPath.AppendToPath(flatFileName.c_str());

        if (flatPath.DoesPathExist())
            flatPath.BeDeleteFile();

        toImport->WriteToXmlFile(flatPath.GetName());
#endif

        if (SUCCESS != ImportSchema(toImport))
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
    return (SchemaStatus::Success == GetDgnDb()->ImportV8LegacySchemas(toInsert.GetSchemas())) ? SUCCESS : ERROR;
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
        DgnElementId groupId = ECJsonUtilities::JsonToId<DgnElementId>(group["GroupId"]);
        DgnElementId mappedGroup = GetSyncInfo()->LookupElement(groupId);
        if (!mappedGroup.IsValid())
            {
            Utf8PrintfString error("Failed to map GroupId element id %s.", groupId.ToString().c_str());
            GetLogger().warning(error.c_str());
            continue;
            }

        GenericGroupPtr groupElement = GetDgnDb()->Elements().GetForEdit<GenericGroup>(mappedGroup);
        if (!groupElement.IsValid())
            {
            Utf8PrintfString error("Unable to get GroupInformationElement(%s).", mappedGroup.ToString().c_str());
            GetLogger().warning(error.c_str());
            continue;
            }
        DgnElementId memberId = ECJsonUtilities::JsonToId<DgnElementId>(group["MemberId"]);
        DgnElementId mappedMember = GetSyncInfo()->LookupElement(memberId);
        if (!mappedMember.IsValid())
            {
            Utf8PrintfString error("Failed to map MemberId element id %s.", memberId.ToString().c_str());
            GetLogger().warning(error.c_str());
            continue;
            }
        DgnElementPtr member = GetDgnDb()->Elements().GetForEdit<DgnElement>(mappedMember);
        if (!member.IsValid())
            {
            Utf8PrintfString error("Unable to get member DgnElement(%s).", mappedMember.ToString().c_str());
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
BentleyStatus LinkTableReader::_Read(Json::Value& relationships)
    {
    if (!relationships.isArray())
        {
        GetLogger().error("ElementRefersToElement value should be an array.\n");
        return ERROR;
        }

    for (Json::Value::iterator iter = relationships.begin(); iter != relationships.end(); iter++)
        {
        Json::Value& member = *iter;
        Utf8PrintfString relName("%s.%s", member["Schema"].asString().c_str(), member["Class"].asString().c_str());

        DgnElementId sourceId = ECJsonUtilities::JsonToId<DgnElementId>(member["SourceId"]);
        DgnElementId mappedSource = GetSyncInfo()->LookupElement(sourceId);
        if (!mappedSource.IsValid())
            {
            Utf8PrintfString error("Failed to map source instance id %s for ECRelationship '%s'.", sourceId.ToString().c_str(), relName.c_str());
            GetLogger().warning(error.c_str());
            continue;
            }

        DgnElementId targetId = ECJsonUtilities::JsonToId<DgnElementId>(member["TargetId"]);
        DgnElementId mappedTarget = GetSyncInfo()->LookupElement(targetId);
        if (!mappedTarget.IsValid())
            {
            Utf8PrintfString error("Failed to map target instance id %s for ECRelationship %s.\n", targetId.ToString().c_str(), relName.c_str());
            GetLogger().warning(error.c_str());
            continue;
            }

        ECN::ECRelationshipClassCP relClass = GetDgnDb()->Schemas().GetClass(member["Schema"].asString(), member["Class"].asString())->GetRelationshipClassCP();
        ECInstanceKey relKey;
        GetDgnDb()->InsertLinkTableRelationship(relKey, *relClass, mappedSource, mappedTarget);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus ElementHasLinksReader::_Read(Json::Value& hasLinks)
    {
    if (!hasLinks.isArray())
        {
        GetLogger().error("ElementHasLinks value should be an array.\n");
        return ERROR;
        }

    for (Json::Value::iterator iter = hasLinks.begin(); iter != hasLinks.end(); iter++)
        {
        Json::Value& link = *iter;
        DgnElementId elementId = ECJsonUtilities::JsonToId<DgnElementId>(link["ElementId"]);
        DgnElementId mappedSource = GetSyncInfo()->LookupElement(elementId);
        if (!mappedSource.IsValid())
            {
            Utf8PrintfString error("Failed to map ElementId for ElementHasLinks relationship %s.", elementId.ToString().c_str());
            GetLogger().warning(error.c_str());
            continue;
            }

        DgnElementId linkId = ECJsonUtilities::JsonToId<DgnElementId>(link["LinkId"]);
        DgnElementId mappedLink = GetSyncInfo()->LookupElement(linkId);
        if (!mappedLink.IsValid())
            {
            Utf8PrintfString error("Failed to map LinkId for ElementHasLinks relationship%s.", linkId.ToString().c_str());
            GetLogger().warning(error.c_str());
            continue;
            }
        LinkElementPtr linkElement = GetDgnDb()->Elements().GetForEdit<LinkElement>(mappedLink);
        if (!linkElement.IsValid())
            {
            Utf8PrintfString error("Unable to get LinkElement DgnElement(%s).\n", linkElement->GetElementId().ToString().c_str());
            GetLogger().warning(error.c_str());
            continue;
            }
        linkElement->AddToSource(mappedSource);
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2018
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus BaselineReader::_Read(Json::Value& baseline)
    {
    PlanId id = ECJsonUtilities::JsonToId<PlanId>(baseline["Plan"]["id"]);
    DgnElementId mappedPlan = GetSyncInfo()->LookupElement(DgnElementId(id.GetValue()));
    if (!mappedPlan.IsValid())
        {
        Utf8PrintfString error("Failed to map PlanId for Baseline aspect %s.", id.ToString().c_str());
        GetLogger().warning(error.c_str());
        return ERROR;
        }
    
    PlanPtr plan = Plan::GetForEdit(*GetDgnDb(), mappedPlan);
    if (!plan.IsValid())
        {
        Utf8PrintfString error("Failed to get Plan for mapped id %s.", mappedPlan.ToString().c_str());
        GetLogger().warning(error.c_str());
        return ERROR;
        }
    if (nullptr == plan->CreateBaseline(baseline["Label"].asCString()))
        {
        Utf8PrintfString error("Failed to set baseline label '%s' on Plan %s.", baseline["Label"].asCString(), mappedPlan.ToString().c_str());
        GetLogger().warning(error.c_str());
        return ERROR;
        }
    plan->Update();
    GetDgnDb()->SaveChanges();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2018
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus PropertyDataReader::_Read(Json::Value& propData)
    {
    if (propData.isMember("DefaultView"))
        {
        DgnElementId viewId = ECJsonUtilities::JsonToId<DgnElementId>(propData["DefaultView"]["id"]);
        DgnElementId mappedView = GetSyncInfo()->LookupElement(viewId);
        if (!mappedView.IsValid())
            {
            Utf8PrintfString error("Failed to map DefaultViewId %s.", viewId.ToString().c_str());
            GetLogger().warning(error.c_str());
            return ERROR;
            }
        PropertySpec prop = DgnViewProperty::DefaultView();
        GetDgnDb()->SaveProperty(prop, &mappedView, sizeof(mappedView));
        }
    return SUCCESS;
    }
END_BIM_TELEPORTER_NAMESPACE
