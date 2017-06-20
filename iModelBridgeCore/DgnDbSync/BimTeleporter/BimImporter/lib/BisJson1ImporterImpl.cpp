/*--------------------------------------------------------------------------------------+
|
|     $Source: BimTeleporter/BimImporter/lib/BisJson1ImporterImpl.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/


#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnPlatformLib.h>
#include <DgnPlatform/GenericDomain.h>
#include <Logging/bentleylogging.h>

#include <BimTeleporter/BisJson1Importer.h>
#include "BisJson1ImporterImpl.h"
#include "SyncInfo.h"
#include "Readers.h"

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
// @bsimethod                                   Carole.MacDonald            03/2017
//---------------+---------------+---------------+---------------+---------------+-------
BisJson1ImporterImpl::BisJson1ImporterImpl(DgnDb* dgndb) : m_dgndb(dgndb), DgnImportContext(*dgndb, *dgndb), m_isDone(false)
    {
    m_syncInfo = nullptr;
    BeFileName db = m_dgndb->GetFileName();
    BeFileName jsonPath(db.GetDirectoryName());
    jsonPath.AppendString(db.GetFileNameWithoutExtension().c_str());
    jsonPath.AppendExtension(L"json");
    if (m_file.Create(jsonPath, true) != BeFileStatus::Success)
        {
        GetLogger().errorv("Failed to create JSON file %s", jsonPath.GetName());
        }

    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2017
//---------------+---------------+---------------+---------------+---------------+-------
BisJson1ImporterImpl::~BisJson1ImporterImpl()
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus BisJson1ImporterImpl::InitializeSchemas()
    {
    m_schemaReadContext = ECN::ECSchemaReadContext::CreateContext();
    m_schemaReadContext->AddSchemaLocater(m_dgndb->GetSchemaLocater());
    m_schemaReadContext->SetResolveConflicts(true);

    bvector<ECSchemaCP> ecSchemas = m_dgndb->Schemas().GetSchemas();
    for (ECN::ECSchemaCP schema : ecSchemas)
        {
        bpair<Utf8String, SchemaKey> pair(schema->GetName(), schema->GetSchemaKey());
        m_schemaNameToKey.insert(pair);
        }

    m_orthographicViewClass = m_dgndb->Schemas().GetClass(BIS_ECSCHEMA_NAME, "OrthographicViewDefinition");

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus BisJson1ImporterImpl::CreateAndAttachSyncInfo()
    {
    if (nullptr == m_syncInfo)
        m_syncInfo = new SyncInfo(*this);
    if (SUCCESS != m_syncInfo->CreateEmptyFile(SyncInfo::GetDbFileName(*m_dgndb)))
        {
        GetLogger().errorv(L"Unable to create syncinfo file at %ls\n", SyncInfo::GetDbFileName(*m_dgndb).GetName());
        return ERROR;
        }
    return AttachSyncInfo();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus BisJson1ImporterImpl::AttachSyncInfo()
    {
    BeFileName syncInfoFileName = SyncInfo::GetDbFileName(*m_dgndb);

    if (BentleyApi::SUCCESS != m_syncInfo->AttachToProject(*m_dgndb, syncInfoFileName))
        {
        BeFileName::BeDeleteFile(syncInfoFileName.c_str());
        GetLogger().errorv(L"Unable to attach syncinfo file at %ls\n", syncInfoFileName.GetName());
        return ERROR;
        }
    BeAssert(m_syncInfo->IsValid());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus BisJson1ImporterImpl::ImportJson(folly::ProducerConsumerQueue<BentleyB0200::Json::Value>& objectQueue)
    {
    Json::Value entry;
    while (!m_isDone)
        {
        while (objectQueue.read(entry))
            {
            ImportJson(entry);
            }
        }
    FinalizeImport();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus BisJson1ImporterImpl::ImportJson(Json::Value& entry)
    {
    if (m_file.IsOpen())
        {
        auto jsonString = entry.toStyledString();
        m_file.Write(nullptr, jsonString.c_str(), static_cast<uint32_t>(jsonString.size()));
        }

    BentleyStatus stat = SUCCESS;
    if (entry.isNull())
        return ERROR;
    Utf8String objectType = entry[JSON_TYPE_KEY].asString();
    if (Utf8String::IsNullOrEmpty(objectType.c_str()))
        return ERROR;
    if (entry[JSON_OBJECT_KEY].isNull())
        return ERROR;

    Reader* reader = nullptr;
    if (objectType.Equals(JSON_TYPE_Schema))
        reader = new SchemaReader(this);
    else if (objectType.Equals(JSON_TYPE_CodeSpec))
        reader = new CodeSpecReader(this);
    else if (objectType.Equals(JSON_TYPE_Model))
        reader = new ModelReader(this, false);
    else if (objectType.Equals(JSON_TYPE_DictionaryModel))
        reader = new ModelReader(this, true);
    else if (objectType.Equals(JSON_TYPE_Element))
        reader = new ElementReader(this);
    else if (objectType.Equals(JSON_TYPE_GeometricElement2d))
        reader = new GeometricElementReader(this, false);
    else if (objectType.Equals(JSON_TYPE_GeometricElement3d))
        reader = new GeometricElementReader(this, true);
    else if (objectType.Equals(JSON_TYPE_GeometryPart))
        reader = new GeometryPartReader(this);
    else if (objectType.Equals(JSON_TYPE_Subject))
        reader = new SubjectReader(this);
    else if (objectType.Equals(JSON_TYPE_Category))
        reader = new CategoryReader(this);
    else if (objectType.Equals(JSON_TYPE_SubCategory))
        {
        bool isDefault = false;
        if (!entry["IsDefaultSubCategory"].isNull())
            isDefault = entry["IsDefaultSubCategory"].asBool();
        reader = new SubCategoryReader(this, isDefault);
        }
    else if (objectType.Equals(JSON_TYPE_ViewDefinition2d))
        reader = new ViewDefinitionReader(this, false);
    else if (objectType.Equals(JSON_TYPE_ViewDefinition3d))
        reader = new ViewDefinitionReader(this, true);
    else if (objectType.Equals(JSON_TYPE_Partition))
        reader = new PartitionReader(this);
    else if (objectType.Equals(JSON_TYPE_CategorySelector))
        reader = new CategorySelectorReader(this);
    else if (objectType.Equals(JSON_TYPE_ModelSelector))
        reader = new ModelSelectorReader(this);
    else if (objectType.Equals(JSON_TYPE_DisplayStyle))
        reader = new DisplayStyleReader(this);
    else if (objectType.Equals(JSON_TYPE_ElementRefersToElement))
        reader = new ElementRefersToElementReader(this);
    else if (objectType.Equals(JSON_TYPE_ElementGroupsMembers))
        reader = new ElementGroupsMembersReader(this);
    if (nullptr == reader)
        return ERROR;

    stat = reader->Read(entry[JSON_OBJECT_KEY]);
    delete reader;
    if (SUCCESS != stat)
        return stat;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2017
//---------------+---------------+---------------+---------------+---------------+-------
void BisJson1ImporterImpl::FinalizeImport()
    {
    m_dgndb->GeoLocation().InitializeProjectExtents();
    m_dgndb->Schemas().CreateClassViewsInDb();
    delete m_syncInfo;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
CodeSpecId BisJson1ImporterImpl::_RemapCodeSpecId(CodeSpecId sourceId)
    {
    CodeSpecId dest = m_remap.Find(sourceId);
    if (!dest.IsValid())
        {
        dest = m_syncInfo->LookupCodeSpec(sourceId);
        if (!dest.IsValid())
            return sourceId;
        return m_remap.Add(sourceId, dest);
        }
    return dest;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnCategoryId BisJson1ImporterImpl::_RemapCategory(DgnCategoryId sourceId)
    {
    DgnCategoryId dest = m_remap.Find(sourceId);
    if (!dest.IsValid())
        {
        dest = m_syncInfo->LookupCategory(sourceId);
        if (!dest.IsValid())
            return DgnCategoryId();
        return m_remap.Add(sourceId, dest);
        }
    return dest;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnSubCategoryId BisJson1ImporterImpl::_RemapSubCategory(DgnCategoryId destCategoryId, DgnSubCategoryId sourceId)
    {
    DgnSubCategoryId dest = m_remap.Find(sourceId);
    if (!dest.IsValid())
        {
        dest = m_syncInfo->LookupSubCategory(destCategoryId, sourceId);
        if (!dest.IsValid())
            return DgnSubCategoryId();
        return m_remap.Add(sourceId, dest);
        }
    return dest;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnFontId BisJson1ImporterImpl::_RemapFont(DgnFontId sourceId)
    {
    DgnFontCP srcFont = &T_HOST.GetFontAdmin().GetAnyLastResortFont();
    return GetDgnDb()->Fonts().AcquireId(*srcFont);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnGeometryPartId BisJson1ImporterImpl::_RemapGeometryPartId(DgnGeometryPartId sourceId)
    {
    DgnGeometryPartId dest = m_remap.Find(sourceId);
    if (!dest.IsValid())
        {
        DgnElementId elemId = m_syncInfo->LookupElement((DgnElementId) sourceId);
        if (!elemId.IsValid())
            return DgnGeometryPartId();
        return m_remap.Add(sourceId, DgnGeometryPartId(elemId.GetValue()));
        }
    return dest;
    }
END_BIM_TELEPORTER_NAMESPACE