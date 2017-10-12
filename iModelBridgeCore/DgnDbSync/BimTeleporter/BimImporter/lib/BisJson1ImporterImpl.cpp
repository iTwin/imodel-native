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

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC

BEGIN_BIM_TELEPORTER_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2017
//---------------+---------------+---------------+---------------+---------------+-------
BisJson1ImporterImpl::BisJson1ImporterImpl(DgnDb* dgndb) : m_dgndb(dgndb), DgnImportContext(*dgndb, *dgndb), m_isDone(false)
    {
    m_syncInfo = nullptr;
    //BeFileName db = m_dgndb->GetFileName();
    //BeFileName jsonPath(db.GetDirectoryName());
    //jsonPath.AppendString(db.GetFileNameWithoutExtension().c_str());
    //jsonPath.AppendExtension(L"json");
    //if (m_file.Create(jsonPath, true) != BeFileStatus::Success)
    //    {
    //    GetLogger().errorv("Failed to create JSON file %s", jsonPath.GetName());
    //    }

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

    m_orthographicViewClass = nullptr;
    m_sheetViewClass = nullptr;

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
            if (SUCCESS != ImportJson(entry))
                {
                auto jsonString = entry.toStyledString();
                GetLogger().errorv("Failed to import:\n%s\n", jsonString.c_str());
                //return ERROR;
                }
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
    auto jsonString = entry.toStyledString();
    if (m_file.IsOpen())
        {
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
    else if (objectType.Equals(JSON_TYPE_LineStyleProperty))
        reader = new LsComponentReader(this);
    else if (objectType.Equals(JSON_TYPE_FontFaceData))
        reader = new FontFaceReader(this);
    else if (objectType.Equals(JSON_TYPE_Font))
        reader = new FontReader(this);
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
    else if (objectType.Equals(JSON_TYPE_AnnotationTextStyle))
        reader = new AnnotationTextStyleReader(this);
    else if (objectType.Equals(JSON_TYPE_CategorySelector))
        reader = new CategorySelectorReader(this);
    else if (objectType.Equals(JSON_TYPE_ModelSelector))
        reader = new ModelSelectorReader(this);
    else if (objectType.Equals(JSON_TYPE_DisplayStyle))
        reader = new DisplayStyleReader(this);
    else if (objectType.Equals(JSON_TYPE_Texture))
        reader = new TextureReader(this);
    else if (objectType.Equals(JSON_TYPE_LineStyleElement))
        reader = new LineStyleReader(this);
    else if (objectType.Equals(JSON_TYPE_ElementRefersToElement))
        reader = new ElementRefersToElementReader(this);
    else if (objectType.Equals(JSON_TYPE_ElementGroupsMembers))
        reader = new ElementGroupsMembersReader(this);
    else if (objectType.Equals(JSON_TYPE_ElementHasLinks))
        reader = new ElementHasLinksReader(this);
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
    m_dgndb->SaveChanges();
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
// @bsimethod                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
RenderMaterialId BisJson1ImporterImpl::_RemapRenderMaterialId(RenderMaterialId sourceId)
    {
	RenderMaterialId dest = m_remap.Find(sourceId);
    if (!dest.IsValid())
        {
        DgnElementId destElm = m_syncInfo->LookupElement(sourceId);
        if (!destElm.IsValid())
            return RenderMaterialId();
        dest = RenderMaterialId(destElm.GetValue());
        return m_remap.Add(sourceId, dest);
        }
    return dest;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnFontId BisJson1ImporterImpl::_RemapFont(DgnFontId sourceId)
    {
    DgnFontId dest = m_remap.Find(sourceId);
    if (dest.IsValid())
        return dest;

    DgnFontId destFont = m_syncInfo->LookupFont(sourceId);
    if (destFont.IsValid())
        return m_remap.Add(sourceId, destFont);
    
    DgnFontCP srcFont = &T_HOST.GetFontAdmin().GetAnyLastResortFont();
    return GetDgnDb()->Fonts().AcquireId(*srcFont);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
DgnStyleId BisJson1ImporterImpl::_RemapLineStyleId(DgnStyleId sourceId)
    {
    DgnStyleId dest = m_remap.Find(sourceId);
    if (dest.IsValid())
        return dest;

    DgnElementId destStyle = m_syncInfo->LookupElement(sourceId);
    if (destStyle.IsValid())
        return m_remap.Add(sourceId, DgnStyleId(destStyle.GetValue()));

    return DgnStyleId();
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2017
//---------------+---------------+---------------+---------------+---------------+-------
DgnTextureId BisJson1ImporterImpl::_RemapTextureId(DgnTextureId sourceId)
    {
    DgnTextureId dest = m_remap.Find(sourceId);
    if (dest.IsValid())
        return dest;

    DgnElementId destTexture = m_syncInfo->LookupElement(sourceId);
    if (destTexture.IsValid())
        return m_remap.Add(sourceId, DgnTextureId(destTexture.GetValue()));

    return DgnTextureId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool SchemaRemapper::_ResolveClassName(Utf8StringR serializedClassName, ECN::ECSchemaCR ecSchema) const
    {
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool SchemaRemapper::_ResolvePropertyName(Utf8StringR serializedPropertyName, ECN::ECClassCR ecClass) const
    {
    if (!m_convSchema.IsValid())
        {
        ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
        SchemaKey key("ECv3ConversionAttributes", 1, 0);
        m_convSchema = ECSchema::LocateSchema(key, *context);
        if (!m_convSchema.IsValid())
            {
            BeAssert(false);
            return false;
            }
        }

    if (!ECSchema::IsSchemaReferenced(ecClass.GetSchema(), *m_convSchema))
        return true;

    T_propertyNameMappings properties;
    T_ClassPropertiesMap::iterator mappedClassIter = m_renamedClassProperties.find(ecClass.GetFullName());
    if (mappedClassIter == m_renamedClassProperties.end())
        {
        IECInstancePtr renameInstance = ecClass.GetCustomAttributeLocal("ECv3ConversionAttributes", "RenamedPropertiesMapping");
        if (renameInstance.IsValid())
            {
            ECValue v;
            renameInstance->GetValue(v, "PropertyMapping");

            bvector<Utf8String> components;
            BeStringUtilities::Split(v.GetUtf8CP(), ";", components);
            for (Utf8String mapping : components)
                {
                bvector<Utf8String> components2;
                BeStringUtilities::Split(mapping.c_str(), "|", components2);
                bpair<Utf8String, Utf8String> pair(components2[0], components2[1]);
                properties.insert(pair);
                }
            }
        bpair<Utf8String, T_propertyNameMappings> pair2(Utf8String(ecClass.GetFullName()), properties);
        m_renamedClassProperties.insert(pair2);
        }
    else
        properties = mappedClassIter->second;

    T_propertyNameMappings::iterator mappedPropertiesIterator = properties.find(serializedPropertyName);
    if (mappedPropertiesIterator != properties.end())
        {
        serializedPropertyName = mappedPropertiesIterator->second;
        return true;
        }

    for (ECClassP baseClass : ecClass.GetBaseClasses())
        {
        if (_ResolvePropertyName(serializedPropertyName, *baseClass))
            return true;
        }

    return true;
    }


END_BIM_TELEPORTER_NAMESPACE