/*--------------------------------------------------------------------------------------+
|
|     $Source: BimTeleporter/BimExporter0601/lib/BisJson1ExporterImpl0601.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
#include <windows.h>
#include <BimTeleporter/BimTeleporter.h>

#include <DgnDb06Api/Bentley/Bentley.h>
#include <DgnDb06Api/Bentley/BeTimeUtilities.h>
#include <DgnDb06Api/DgnPlatform/DgnPlatformApi.h>
#include <DgnDb06Api/DgnPlatform/DgnPlatformLib.h>
#include <DgnDb06Api/Logging/bentleylogging.h>
#include <DgnDb06Api/BeSQLite/L10N.h>

#include <DgnDb06Api/DgnPlatform/DgnProgressMeter.h>

#define BIM_EXPORTER_NAMESPACE_NAME BimTeleporter
#define BEGIN_BIM_EXPORTER_NAMESPACE namespace BentleyG0601 { namespace Dgn { namespace BIM_EXPORTER_NAMESPACE_NAME {
#define END_BIM_EXPORTER_NAMESPACE   } } }

BEGIN_BIM_EXPORTER_NAMESPACE

struct KnownDesktopLocationsAdmin : DgnPlatformLib::Host::IKnownLocationsAdmin
    {
    BeFileName m_tempDirectory;
    BeFileName m_executableDirectory;
    BeFileName m_assetsDirectory;

    virtual BeFileNameCR _GetLocalTempDirectoryBaseName() override { return m_tempDirectory; }
    virtual BeFileNameCR _GetDgnPlatformAssetsDirectory() override { return m_assetsDirectory; }

    //---------------------------------------------------------------------------------------
    // @bsiclass                                                   BentleySystems
    //---------------------------------------------------------------------------------------
    KnownDesktopLocationsAdmin(BeFileName tempPath, BeFileName assetsPath) : m_tempDirectory(tempPath)
        {
        m_assetsDirectory = assetsPath.GetDirectoryName();
        m_assetsDirectory.AppendToPath(L"Assets06");
        }
    };

struct BisJson1ExporterImpl : DgnPlatformLib::Host
    {
    private:
        DgnDbPtr            m_dgndb;
        DgnProgressMeter*   m_meter;
        T_LogGeneralMessage m_logger;
        T_LogPerformanceMessage m_performanceLog;
        T_QueueJson QueueJson;
        BeFileName m_dbPath;
        BeFileName m_tempPath;
        BeFileName m_assetsPath;

        DgnElementId        m_nextAvailableId;
        ECN::ECClassCP      m_viewDefinition3dClass;
        ECN::ECClassCP      m_viewDefinition2dClass;
        ECN::ECClassId      m_spatialViewDefinitionId;
        ECN::ECClassCP      m_cameraViewDefinitionClass;
        ECN::ECClassCP      m_drawingViewDefinitionClass;
        ECN::ECClassCP      m_sheetViewDefinitionClass;
        ECN::ECClassCP      m_categoryClass;
        ECN::ECClassCP      m_subCategoryClass;
        ECN::ECClassCP      m_linkModelClass;
        ECN::ECClassCP      m_annotationTextStyle;
        ECN::ECClassCP      m_textureClass;

        Dgn::DgnElementId        m_jobSubjectId;
        Dgn::DgnElementId        m_documentListModelId;
        Dgn::DgnElementId        m_sheetListModelId;
        Dgn::DgnElementId       m_jobDefinitionModelId;
        bmap<Utf8String, Utf8String> m_authorityIds;

        bmap<DgnElementId, int> m_insertedElements;
        bmap<DgnModelId, int> m_insertedModels;
        bmap<DgnCategoryId, DgnCategoryId> m_mappedDrawingCategories;

        BENTLEY_TRANSLATABLE_STRINGS_START(ProgressMessage, export_progress)
            L10N_STRING(STEP_EXPORT_SCHEMAS)         // =="Exporting ECSchemas"==
            L10N_STRING(STEP_EXPORT_AUTHORITIES)     // =="Exporting Authorities"==
            L10N_STRING(STEP_EXPORT_MODELS)          // =="Exporting Models"==
            BENTLEY_TRANSLATABLE_STRINGS_END

        void LogMessage(TeleporterLoggingSeverity severity, Utf8CP message, ...);
        void LogMessage(TeleporterLoggingSeverity severity, WCharCP message, ...);
        void LogPerformanceMessage(StopWatch& stopWatch, Utf8CP message, ...);
        void SendToQueue(Utf8CP json);
        void MakeNavigationProperty(Json::Value& out, Utf8CP propertyName, BeInt64Id id);
        void MakeNavigationProperty(Json::Value& out, Utf8CP propertyName, uint64_t id);
        void MakeNavigationProperty(Json::Value& out, Utf8CP propertyName, Utf8CP id);
        void MakeNavigationProperty(Json::Value& out, Utf8CP propertyName, Json::Value& id);
        BentleyStatus ExportFonts(Json::Value& out);
        BentleyStatus ExportGeometryParts(Json::Value& out);
        BentleyStatus ExportLineStyles(Json::Value& out);
        BentleyStatus ExportTextures(Json::Value& out);
        BentleyStatus ExportAuthorities(Json::Value& out);
        BentleyStatus ExportViews(Json::Value& out);
        BentleyStatus ExportCategories(Json::Value& out);
        BentleyStatus ExportCategories(Json::Value& out, Utf8CP tableName, Utf8CP bisClassName, Utf8CP bisAuthorityStr, bvector<DgnCategoryId>& duplicateIds);
        DgnElementId CreateModelSelector(Json::Value& out, ViewControllerCR vc, Utf8CP name);
        DgnElementId CreateCategorySelector(Json::Value& out, ViewControllerCR vc, Utf8CP name);
        DgnElementId CreateDisplayStyle(Json::Value& out, ViewControllerCR vc, Utf8CP name, bool is3d);
        BentleyStatus ExportModels(Json::Value& out);
        BentleyStatus ExportModel(Json::Value& out, Utf8CP schemaName, Utf8CP className, Utf8CP whereClause = nullptr);
        BentleyStatus ExportElements(Json::Value& out);
        BentleyStatus ExportElements(Json::Value& out, DgnModelId parentModel);
        BentleyStatus ExportElements(Json::Value& out, Utf8CP schemaName, Utf8CP className, DgnModelId parentModel, Utf8CP whereClause = nullptr, bool sendToQueue = true, bool allowDuplicates = false);
        BentleyStatus ExportNamedGroups(Json::Value& out);
        BentleyStatus ExportElementHasLinks(Json::Value& out);
        BentleyStatus ExportV8Relationships(Json::Value& out);
        DgnElementId CreateCodeSpec(Json::Value& out, uint8_t codeSpecType, Utf8CP name);
        DgnElementId CreateSubjectElement(Utf8CP subjectName, Json::Value& out);
        DgnElementId CreatePartitionElement(DgnModelCR model, DgnElementId subject, Json::Value& out);
        DgnElementId CreatePartitionElement(Utf8CP partitionName, Utf8CP partitionType, DgnElementId subject, Json::Value& out);
        DgnElementId CreateDrawingElement(Json::Value& out, Utf8CP name);
        DgnElementId CreateSheetElement(Json::Value& out, DgnModelCR model);
        BentleyStatus ExportSchemas(Json::Value& out) const;
        DgnElementId InitListModel(Json::Value& out, Utf8CP name);
        BentleyStatus InitDrawingListModel(Json::Value& out);
        BentleyStatus InitSheetListModel(Json::Value& out);
        BentleyStatus InitJobDefinitionModel(Json::Value& out);
        Utf8String RemapResourceAuthority(Json::Value& obj, ECN::ECClassCP elementClass);
        void HandleAnnotationTextStyle(Json::Value& obj, DgnElementId id);

        //! Report progress and detect if user has indicated that he wants to cancel.
        void ReportProgress() const;

        //! Report progress. 
        void ShowProgress() const { m_meter->ShowProgress(); }

        //! Specify how many steps remain in the current task
        void AddSteps(int32_t n) const;
        //! Set the name of the current step. The progress meter is expected to update its display and decrement the number of remaining steps as a result.
        void SetStepName(ProgressMessage::StringId, ...) const;
        //! Specify how many tasks remain in the conversion
        void AddTasks(int32_t n) const;
        //! Set the name of the current task. The progress meter is expected to update its display and decrement the number of remaining steps as a result.
        void SetTaskName(ProgressMessage::StringId, ...) const;
        //! Get the progress meter, if any.

        virtual void                        _SupplyProductName(Utf8StringR name) override { name.assign("BimTeleporter"); }
        virtual IKnownLocationsAdmin&       _SupplyIKnownLocationsAdmin() override { return *new KnownDesktopLocationsAdmin(m_tempPath, m_assetsPath); };
        virtual BeSQLite::L10N::SqlangFiles _SupplySqlangFiles() override;


    public:
        BisJson1ExporterImpl(wchar_t const* dbPath, wchar_t const* tempPath, wchar_t const* assetsPath);
        ~BisJson1ExporterImpl();
        bool OpenDgnDb();
        void SetProgressMeter(DgnProgressMeter* meter) { m_meter = meter; }
        bool ExportDgnDb();
        void SetGeneralLogger(T_LogGeneralMessage logger) { m_logger = logger; }
        void SetPerformanceLogger(T_LogPerformanceMessage logger) { m_performanceLog = logger; }
        void SetQueueWrite(T_QueueJson queue) { QueueJson = queue; }

    };

END_BIM_EXPORTER_NAMESPACE
