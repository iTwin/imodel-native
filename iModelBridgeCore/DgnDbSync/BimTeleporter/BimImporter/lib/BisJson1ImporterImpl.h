/*--------------------------------------------------------------------------------------+
|
|     $Source: BimTeleporter/BimImporter/lib/BisJson1ImporterImpl.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
#include <BimTeleporter/BimTeleporter.h>
#include <Bentley/Bentley.h>
#include <Bentley/BentleyAllocator.h>
#include <Bentley/WString.h>
#include <DgnPlatform/DgnPlatform.h>

#include <DgnPlatform/DgnPlatformApi.h>
#include <folly/ProducerConsumerQueue.h>

#include "SyncInfo.h"
BEGIN_BIM_TELEPORTER_NAMESPACE

struct Reader;

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
        bmap<Utf8String, ECN::SchemaKey> m_schemaNameToKey;
        ECN::ECClassCP m_orthographicViewClass;
        BeFile m_file;
        bool m_isDone;

    protected:
        ECN::ECSchemaReadContextPtr m_schemaReadContext;
        SyncInfo* m_syncInfo;

        static NativeLogging::ILogger& GetLogger() { return *NativeLogging::LoggingManager::GetLogger("BimTeleporter"); }
        DgnDbP GetDgnDb()
            {
            return m_dgndb.get();
            }

        BentleyStatus CreateSyncInfo();

    protected:
        CodeSpecId _RemapCodeSpecId(CodeSpecId sourceId) override;
        DgnGeometryPartId _RemapGeometryPartId(DgnGeometryPartId sourceId) override;
        DgnCategoryId _RemapCategory(DgnCategoryId sourceId) override;
        DgnSubCategoryId _RemapSubCategory(DgnCategoryId destCategoryId, DgnSubCategoryId sourceId) override;
        //DgnClassId _RemapClassId(DgnClassId sourceId) override;
        //DgnMaterialId _RemapMaterialId(DgnMaterialId sourceId) override;
        //DgnTextureId _RemapTextureId(DgnTextureId sourceId) override;
        //DgnDbStatus _RemapGeometryStreamIds(GeometryStreamR geom) override;
        DgnFontId _RemapFont(DgnFontId) override;
        //DgnStyleId _RemapLineStyleId(DgnStyleId sourceId) override;
        BentleyStatus ImportJson(Json::Value& jsonInput);

    public:
        BisJson1ImporterImpl(DgnDb* dgndb);
        ~BisJson1ImporterImpl();
        BentleyStatus InitializeSchemas();
        BentleyStatus CreateAndAttachSyncInfo();
        BentleyStatus AttachSyncInfo();
        BentleyStatus ImportJson(folly::ProducerConsumerQueue<BentleyB0200::Json::Value>& objectQueue);
        void AddToQueue(const char* entry);
        void SetDone() { m_isDone = true; }
        void FinalizeImport();
    };


END_BIM_TELEPORTER_NAMESPACE