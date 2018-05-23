/*--------------------------------------------------------------------------------------+
|
|     $Source: BimTeleporter/BimImporter/lib/BisJson1ImporterImpl.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
#include <BimTeleporter/BimTeleporter.h>
#include <Bentley/Bentley.h>
#include <Bentley/BentleyAllocator.h>
#include <Bentley/WString.h>
#include <DgnPlatform/DgnPlatform.h>
#include <BeSQLite/L10N.h>

#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnProgressMeter.h>
#include <folly/ProducerConsumerQueue.h>

#include "SyncInfo.h"
BEGIN_BIM_TELEPORTER_NAMESPACE

BENTLEY_TRANSLATABLE_STRINGS_START(BimUpgrader, bim_import)
    L10N_STRING(TASK_CREATING_THUMBNAIL)            // =="View: %s"==
    L10N_STRING(TASK_IMPORTING_SCHEMA)              // =="%s"==
    L10N_STRING(TASK_IMPORTING_RELATIONSHIP)        // =="ECRelationship: %s"==
    L10N_STRING(TASK_ELEMENT_GROUPS_MEMBERS)        // =="ElementGroupsMembers"==
    L10N_STRING(STEP_GENERATING_THUMBNAILS)         // =="Generating Thumbnails"==
    L10N_STRING(STEP_IMPORTING_SCHEMAS)             // =="Importing Schemas"==
    L10N_STRING(STEP_IMPORTING_ELEMENTS)            // =="Importing Elements"==
BENTLEY_TRANSLATABLE_STRINGS_END

struct Reader;

//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            01/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct SchemaRemapper : ECN::IECSchemaRemapper
    {
    private:
        typedef bmap<Utf8String, Utf8String> T_propertyNameMappings;
        typedef bmap<Utf8String, T_propertyNameMappings> T_ClassPropertiesMap;
        mutable ECN::ECSchemaPtr m_convSchema;
        mutable T_ClassPropertiesMap m_renamedClassProperties;

        virtual bool _ResolvePropertyName(Utf8StringR serializedPropertyName, ECN::ECClassCR ecClass) const override;
        virtual bool _ResolveClassName(Utf8StringR serializedClassName, ECN::ECSchemaCR ecSchema) const override;

    public:
        explicit SchemaRemapper() {}
        ~SchemaRemapper() {}
    };


//---------------------------------------------------------------------------------------
// @bsiclass                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
struct BisJson1ImporterImpl : DgnImportContext
    {
    friend struct SyncInfo;
    friend struct Reader;
    friend struct ElementReader;
    friend struct SchemaReader;
    friend struct ViewDefinitionReader;

    private:
        DgnDbPtr            m_dgndb;
        DgnProgressMeter*   m_meter;
        bmap<Utf8String, ECN::SchemaKey> m_schemaNameToKey;
        ECN::ECClassCP m_orthographicViewClass;
        ECN::ECClassCP m_sheetViewClass;
        BeFile m_file;
        bool m_isDone;
        SchemaRemapper m_remapper;
        uint32_t m_entityCount;
        StopWatch m_importTimer;

    protected:
        ECN::ECSchemaReadContextPtr m_schemaReadContext;
        SyncInfo* m_syncInfo;

        static NativeLogging::ILogger& GetLogger() { return *NativeLogging::LoggingManager::GetLogger("BimTeleporter"); }
        DgnDbP GetDgnDb()
            {
            return m_dgndb.get();
            }

        SchemaRemapper& GetRemapper() { return m_remapper; }
        BentleyStatus CreateSyncInfo();

    protected:
        CodeSpecId _RemapCodeSpecId(CodeSpecId sourceId) override;
        DgnGeometryPartId _RemapGeometryPartId(DgnGeometryPartId sourceId) override;
        DgnCategoryId _RemapCategory(DgnCategoryId sourceId) override;
        DgnSubCategoryId _RemapSubCategory(DgnCategoryId destCategoryId, DgnSubCategoryId sourceId) override;
        //DgnClassId _RemapClassId(DgnClassId sourceId) override;
        RenderMaterialId _RemapRenderMaterialId(RenderMaterialId sourceId) override;
        DgnTextureId _RemapTextureId(DgnTextureId sourceId) override;
        //DgnDbStatus _RemapGeometryStreamIds(GeometryStreamR geom) override;
        DgnFontId _RemapFont(DgnFontId) override;
        DgnStyleId _RemapLineStyleId(DgnStyleId sourceId) override;
        DgnElementId _RemapAnnotationStyleId(DgnElementId sourceId) override;

        BentleyStatus ImportJson(Json::Value& jsonInput);
        void GenerateThumbnails();

    public:
        BisJson1ImporterImpl(DgnDb* dgndb, bool setQuietAssertions = false);
        ~BisJson1ImporterImpl();
        BentleyStatus InitializeSchemas();
        BentleyStatus CreateAndAttachSyncInfo();
        BentleyStatus AttachSyncInfo();
        BentleyStatus ImportJson(folly::ProducerConsumerQueue<BentleyB0200::Json::Value>& objectQueue, folly::Future<bool>& exporterFuture);
        void AddToQueue(const char* entry);
        void SetDone() { m_isDone = true; }
        void FinalizeImport();
        void ShowProgress() { if (nullptr != m_meter) m_meter->ShowProgress(); }
        void SetTaskName(BimUpgrader::StringId stringId, ...) const;
        void SetStepName(BimUpgrader::StringId stringId, ...) const;

    };


END_BIM_TELEPORTER_NAMESPACE