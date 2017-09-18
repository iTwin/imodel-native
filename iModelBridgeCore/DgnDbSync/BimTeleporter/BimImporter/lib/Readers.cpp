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
#include <Bentley/Base64Utilities.h>
#include "SyncInfo.h"
#include "Readers.h"
#include "BisJson1ImporterImpl.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC

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
    ECClassCP ecClass = GetClassFromKey(object[ECJsonUtilities::json_className()].asCString());
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

    if (SUCCESS != ECJsonUtilities::ECInstanceFromJson(*ptr, object, m_importer->GetDgnDb()->GetClassLocater(), &m_importer->GetRemapper()))
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

    Utf8PrintfString idStr("%" PRIu64 "", codeSpec.GetValue());
    element[BIS_ELEMENT_PROP_CodeSpecId][ECJsonUtilities::json_navId()] = idStr;
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

    Utf8String idString = element[BIS_ELEMENT_PROP_CodeSpecId]["id"].asString();
    uint64_t id;
    if (!Utf8String::IsNullOrEmpty(idString.c_str()))
        {
        if (SUCCESS != BeStringUtilities::ParseUInt64(id, idString.c_str()))
            {
            GetLogger().errorv("Failed to parse CodeSpec.id property value '%s' into a UInt64.", idString.c_str());
            return CodeSpecId();
            }
        CodeSpecId codeSpec = m_importer->m_syncInfo->LookupCodeSpec(CodeSpecId(id));
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
    uint64_t id;
    if (!element.isMember("Parent"))
        return SUCCESS;

    Utf8String parentId = element["Parent"]["id"].asString();
    if (!(Utf8String::IsNullOrEmpty(parentId.c_str())))
        {
        if (SUCCESS != BeStringUtilities::ParseUInt64(id, parentId.c_str()))
            {
            GetLogger().errorv("Failed to parse Parent.id property '%s' into a UInt64.\n", parentId.c_str());
            return ERROR;
            }
        DgnElementId mappedId = m_importer->m_syncInfo->LookupElement(DgnElementId(id));
        if (!mappedId.IsValid())
            {
            GetLogger().errorv("Failed to remap Parent.id '%s'\n", parentId.c_str());
            return ERROR;
            }
        Utf8PrintfString idStr("%" PRIu64 "", mappedId.GetValue());
        element[BIS_ELEMENT_PROP_Parent]["id"] = idStr.c_str();
        Utf8PrintfString parRelId("%" PRIu64 "", _GetRelationshipClassId().GetValue());
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
            {
            GetLogger().errorv("Failed to parse Category.id property '%s' into a UInt64.\n", catId.c_str());
            return ERROR;
            }
        DgnElementId mappedId = m_importer->m_syncInfo->LookupElement(DgnElementId(id));
        if (!mappedId.IsValid())
            {
            GetLogger().errorv("Failed to remap Category.id '%s'\n", catId.c_str());
            return ERROR;
            }
        Utf8PrintfString idStr("%" PRIu64 "", mappedId.GetValue());
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

    Utf8PrintfString idStr("%" PRIu64 "", mappedId.GetValue());
    element[BIS_ELEMENT_PROP_Model]["id"] = idStr;
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2017
//---------------+---------------+---------------+---------------+---------------+-------
DgnModelId ElementReader::GetMappedModelId(Json::Value& element, Utf8CP propertyName)
    {
    // remap modelId
    Utf8String modelId = element[propertyName]["id"].asString();
    uint64_t id;
    if (Utf8String::IsNullOrEmpty(modelId.c_str()))
        {
        GetLogger().errorv("Model roperty '%s.id' is empty.\n", propertyName);
        return DgnModelId();
        }

    if (SUCCESS != BeStringUtilities::ParseUInt64(id, modelId.c_str()))
        {
        GetLogger().errorv("Failed to parse %s property value '%s' into a UInt64.", propertyName, modelId.c_str());
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

    DgnFontId oldId(font["Id"].asUInt64());
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
            GetLogger().warningv("Unable to locate parent subject with exported id %" PRIu64 ".  Using root subject instead", parentId);
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
            GetLogger().warningv("Unable to locate partition's subject with exported id %" PRIu64 ".  Using root subject instead", subjectId);
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
            GetLogger().errorv("Failed to create a GroupInformationPartition for %" PRIu64 "", oldInstanceId);
            return ERROR;
            }
        newId = gip->GetElementId();
        }
    else if (partitionType.Equals("PhysicalPartition"))
        {
        PhysicalPartitionCPtr pp = PhysicalPartition::CreateAndInsert(*subject, label.c_str(), partition["Descr"].isNull() ? nullptr : partition["Descr"].asString().c_str());
        if (!pp.IsValid())
            {
            GetLogger().errorv("Failed to create a PhysicalPartition for %" PRIu64 "", oldInstanceId);
            return ERROR;
            }
        newId = pp->GetElementId();
        }
    else if (partitionType.Equals("DocumentPartition"))
        {
        DocumentPartitionCPtr dp = DocumentPartition::CreateAndInsert(*subject, label.c_str(), partition["Descr"].isNull() ? nullptr : partition["Descr"].asString().c_str());
        if (!dp.IsValid())
            {
            GetLogger().errorv("Failed to create a DocumentPartition for %" PRIu64 "", oldInstanceId);
            return ERROR;
            }
        newId = dp->GetElementId();
        }
    else if (partitionType.Equals("LinkPartition"))
        {
        LinkPartitionCPtr lp = LinkPartition::CreateAndInsert(*subject, label.c_str(), partition["Descr"].isNull() ? nullptr : partition["Descr"].asString().c_str());
        if (!lp.IsValid())
            {
            GetLogger().errorv("Failed to create a LinkPartition for %" PRIu64 "", oldInstanceId);
            return ERROR;
            }
        newId = lp->GetElementId();
        }
    else
        {
        GetLogger().errorv("Unknown (or empty) partition type '%s' for element %" PRIu64 "", partitionType.c_str(), oldInstanceId);
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
        {
        GetLogger().errorv("Failed to create IECInstance for element %s\n", _GetElementType());
        return ERROR;
        }

    Utf8String idString = element["$ECInstanceId"].asString();
    if (SUCCESS != BeStringUtilities::ParseUInt64(m_instanceId, idString.c_str()))
        return ERROR;

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
    DgnElementPtr dgnElement = GetDgnDb()->Elements().CreateElement(*ecInstance, &stat);

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

    IECInstancePtr ecInstance = _CreateInstance(object);
    if (!ecInstance.IsValid())
        {
        GetLogger().errorv("Failed to create IECInstance for element %s\n", _GetElementType());
        return ERROR;
        }

    Utf8String idString = object["$ECInstanceId"].asString();
    if (SUCCESS != BeStringUtilities::ParseUInt64(m_instanceId, idString.c_str()))
        return ERROR;

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
    ats->SetFontId(DgnFontId(properties["FontId"].asUInt64()));
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

    DgnElementId categorySelectorId = GetMappedElementId(viewDefinition, "CategorySelector");
    if (!categorySelectorId.IsValid())
        return ERROR;
    CategorySelectorPtr categorySelector = GetDgnDb()->Elements().GetForEdit<CategorySelector>(categorySelectorId); // We aren't editing it, but the CreateParams wants a ref, not a const ref
    if (!categorySelector.IsValid())
        return ERROR;

    ECClassCP ecClass = GetClassFromKey(viewDefinition["$ECClassKey"].asString().c_str());
    if (nullptr == ecClass)
        return ERROR;

    if (m_is3d)
        {
        DisplayStyle3dPtr displayStyle = GetDgnDb()->Elements().GetForEdit<DisplayStyle3d>(displayStyleId); // We aren't editing it, but the CreateParams wants a ref, not a const ref
        if (!displayStyle.IsValid())
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

            viewDef = new SpatialViewDefinition(GetDgnDb()->GetDictionaryModel(), dgnCode.GetValueUtf8(), *categorySelector, *displayStyle, *modelSelector, camera);
            }
        else
            viewDef = new OrthographicViewDefinition(GetDgnDb()->GetDictionaryModel(), dgnCode.GetValueUtf8(), *categorySelector, *displayStyle, *modelSelector);

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
        if (ecClass->Is(m_importer->m_sheetViewClass))
            viewDef = new SheetViewDefinition(GetDgnDb()->GetDictionaryModel(), dgnCode.GetValueUtf8(), baseModelId, *categorySelector, *displayStyle);
        else
            viewDef = new DrawingViewDefinition(GetDgnDb()->GetDictionaryModel(), dgnCode.GetValueUtf8(), baseModelId, *categorySelector, *displayStyle);

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
            GetLogger().errorv("Unable to retrieve Category %" PRIu64 "", categoryId.GetValue());
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
        idString.Sprintf("%" PRIu64 "", mappedId.GetValue());
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
    // its id to the new one.  Also create an element mapping as this is needed for CodeScope remappings
    if (m_isDictionary)
        {
        GetSyncInfo()->InsertModel(mapping, GetDgnDb()->GetDictionaryModel().GetModelId(), DgnModelId(ecInstanceId), nullptr);
        SyncInfo::ElementMapping map(DgnElementId(ecInstanceId), GetDgnDb()->GetDictionaryModel().GetModeledElementId());
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

        if (ecSchema->OriginalECXmlVersionLessThan(ECVersion::V3_1) && !ECSchemaConverter::Convert(*ecSchema))
            {
            GetLogger().fatalv("Failed to convert schema %s to EC3.1.  Unable to continue.", schemaName);
            return ERROR;
            }
        
        if (ecSchema->GetName().EqualsIAscii("jclass"))
            {
            ECClassP acadProps = ecSchema->GetClassP("ACAD_PROPERTIES");
            if (nullptr != acadProps)
                {
                acadProps->SetClassModifier(ECClassModifier::Abstract);

                IECInstancePtr mixinInstance = CoreCustomAttributeHelper::CreateCustomAttributeInstance("IsMixin");
                if (!mixinInstance.IsValid())
                    {
                    GetLogger().errorv("Unable to create mixin custom attribute for jclass:ACAD_PROPERTIES");
                    return ERROR;
                    }
                auto& coreCA = mixinInstance->GetClass().GetSchema();
                if (!ECSchema::IsSchemaReferenced(*ecSchema, coreCA))
                    ecSchema->AddReferencedSchema(const_cast<ECSchemaR>(coreCA));

                ECValue appliesToClass("jclass:JSPACE_OBJECT");
                mixinInstance->SetValue("AppliesToEntityClass", appliesToClass);

                if (ECObjectsStatus::Success != acadProps->SetCustomAttribute(*mixinInstance))
                    {
                    GetLogger().errorv("Unable to set mixin custom attribute on jclass:ACAD_PROPERTIES");
                    return ERROR;
                    }
                }
            for (ECClassCP ecClass : ecSchema->GetClasses())
                {
                for (ECClassCP base : ecClass->GetBaseClasses())
                    {
                    if (!base->GetName().EqualsIAscii("ACAD_PROPERTIES"))
                        continue;
                    // Need to ensure that the mixin baseclass is at the end of the list of baseclasses
                    ECClassP nonConstClass = ecSchema->GetClassP(ecClass->GetName().c_str());
                    nonConstClass->RemoveBaseClass(*base);
                    nonConstClass->AddBaseClass(*base);
                    }
                }
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
            Utf8PrintfString error("Failed to map GroupId element id %" PRIu64 ".", group["GroupId"].asUInt64());
            GetLogger().warning(error.c_str());
            continue;
            }

        GenericGroupPtr groupElement = GetDgnDb()->Elements().GetForEdit<GenericGroup>(mappedGroup);
        if (!groupElement.IsValid())
            {
            Utf8PrintfString error("Unable to get GroupInformationElement(%" PRIu64 ").\n%s", mappedGroup.GetValue(), mappedGroup.ToString().c_str());
            GetLogger().warning(error.c_str());
            continue;
            }
        DgnElementId mappedMember = GetSyncInfo()->LookupElement(DgnElementId(group["MemberId"].asUInt64()));
        if (!mappedMember.IsValid())
            {
            Utf8PrintfString error("Failed to map MemberId element id %" PRIu64 ".", group["MemberId"].asUInt64());
            GetLogger().warning(error.c_str());
            continue;
            }
        DgnElementPtr member = GetDgnDb()->Elements().GetForEdit<DgnElement>(mappedMember);
        if (!member.IsValid())
            {
            Utf8PrintfString error("Unable to get member DgnElement(%" PRIu64 ").\n%s", mappedGroup.GetValue(), mappedGroup.ToString().c_str());
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
        GetLogger().error("ElementRefersToElement value should be an array.\n");
        return ERROR;
        }

    for (Json::Value::iterator iter = relationships.begin(); iter != relationships.end(); iter++)
        {
        Json::Value& member = *iter;
        Utf8PrintfString relName("%s.%s", member["Schema"].asString().c_str(), member["Class"].asString().c_str());

        DgnElementId mappedSource = GetSyncInfo()->LookupElement(DgnElementId(member["SourceId"].asUInt64()));
        if (!mappedSource.IsValid())
            {
            Utf8PrintfString error("Failed to map source instance id %" PRIu64 " for ECRelationship '%s'.", member["SourceId"].asUInt64(), relName.c_str());
            GetLogger().warning(error.c_str());
            continue;
            }

        DgnElementId mappedTarget = GetSyncInfo()->LookupElement(DgnElementId(member["TargetId"].asUInt64()));
        if (!mappedTarget.IsValid())
            {
            Utf8PrintfString error("Failed to map target instance id %" PRIu64 " for ECRelationship %s.\n", member["TargetId"].asUInt64(), relName.c_str());
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
        DgnElementId mappedSource = GetSyncInfo()->LookupElement(DgnElementId(link["ElementId"].asUInt64()));
        if (!mappedSource.IsValid())
            {
            Utf8PrintfString error("Failed to map ElementId for ElementHasLinks relationship %" PRIu64 ".", link["ElementId"].asUInt64());
            GetLogger().warning(error.c_str());
            continue;
            }

        DgnElementId mappedLink = GetSyncInfo()->LookupElement(DgnElementId(link["LinkId"].asUInt64()));
        if (!mappedLink.IsValid())
            {
            Utf8PrintfString error("Failed to map LinkId for ElementHasLinks relationship%" PRIu64 ".", link["LinkId"].asUInt64());
            GetLogger().warning(error.c_str());
            continue;
            }
        LinkElementPtr linkElement = GetDgnDb()->Elements().GetForEdit<LinkElement>(mappedLink);
        if (!linkElement.IsValid())
            {
            Utf8PrintfString error("Unable to get LinkElement DgnElement(%" PRIu64 ").\n", linkElement->GetElementId().GetValue());
            GetLogger().warning(error.c_str());
            continue;
            }
        linkElement->AddToSource(mappedSource);
        }
    return SUCCESS;
    }
END_BIM_TELEPORTER_NAMESPACE
