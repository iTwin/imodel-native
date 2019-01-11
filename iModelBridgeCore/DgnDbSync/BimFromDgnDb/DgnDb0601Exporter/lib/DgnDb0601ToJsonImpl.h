/*--------------------------------------------------------------------------------------+
|
|     $Source: BimFromDgnDb/DgnDb0601Exporter/lib/DgnDb0601ToJsonImpl.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
#include <BimFromDgnDb/BimFromDgnDb.h>

#include <DgnDb06Api/Bentley/Bentley.h>
#include <DgnDb06Api/Bentley/BeTimeUtilities.h>
#include <DgnDb06Api/DgnPlatform/DgnPlatformApi.h>
#include <DgnDb06Api/DgnPlatform/DgnPlatformLib.h>
#include <DgnDb06Api/Logging/bentleylogging.h>
#include <DgnDb06Api/BeSQLite/L10N.h>

#include <DgnDb06Api/DgnPlatform/DgnProgressMeter.h>

#define DGNDB0601_TO_JSON_NAMESPACE_NAME BimFromDgnDb
#define BEGIN_DGNDB0601_TO_JSON_NAMESPACE namespace BentleyG0601 { namespace Dgn { namespace DGNDB0601_TO_JSON_NAMESPACE_NAME {
#define END_DGNDB0601_TO_JSON_NAMESPACE   } } }

BEGIN_DGNDB0601_TO_JSON_NAMESPACE

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
        WString tmpStr;
        BeFileName::FixPathName(tmpStr, assetsPath.GetName(), false);
        BeFileName tmp(tmpStr);
        m_assetsDirectory = tmp.GetDirectoryName();
        m_assetsDirectory.AppendToPath(L"Assets06");
        }
    };

struct DgnDb0601ToJsonImpl : DgnPlatformLib::Host
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
        ECN::ECClassCP      m_planningModelClass;
        ECN::ECClassCP      m_planningElementClass;
        ECN::ECClassCP      m_workbreakDownClass;
        ECN::ECClassCP      m_activityClass;
        ECN::ECClassCP      m_timeSpanClass;
        ECN::ECClassCP      m_cameraKeyFrameClass;
        ECN::ECClassCP      m_elementAspectClass;
        ECN::ECClassCP      m_elementUniqueAspectClass;
        ECN::ECClassCP      m_elementMultiAspectClass;
        ECN::ECClassCP      m_elementClass;
        ECN::ECClassCP      m_geometric2dClass;
        ECN::ECClassCP      m_geometric3dClass;
        ECN::ECClassCP      m_pointCloudModelClass;
        ECN::ECClassCP      m_threeMxModelClass;
        ECN::ECClassCP      m_rasterFileModelClass;

        Dgn::DgnElementId        m_jobSubjectId;
        Dgn::DgnElementId        m_documentListModelId;
        Dgn::DgnElementId        m_sheetListModelId;
        Dgn::DgnElementId       m_jobDefinitionModelId;
        bmap<Utf8String, Utf8String> m_authorityIds;
        bmap<Utf8String, Dgn::DgnElementId> m_disciplineIds;
        bmap<Utf8String, Dgn::DgnElementId> m_namespaceDefinitionModels;

        bmap<DgnElementId, int> m_insertedElements;
        bmap<BentleyApi::BeSQLite::EC::ECInstanceId, int> m_insertedAspects;
        bmap<DgnModelId, DgnElementId> m_insertedModels;
        bmap<DgnCategoryId, DgnCategoryId> m_mappedDrawingCategories;

        BENTLEY_TRANSLATABLE_STRINGS_START(ProgressMessage, export_progress)
            L10N_STRING(STEP_EXPORT_SCHEMAS)         // =="Exporting ECSchemas"==
            L10N_STRING(STEP_EXPORT_AUTHORITIES)     // =="Exporting Authorities"==
            L10N_STRING(STEP_EXPORT_MODELS)          // =="Exporting Models"==
            BENTLEY_TRANSLATABLE_STRINGS_END

        void LogMessage(BimFromDgnDbLoggingSeverity severity, Utf8CP message, ...);
        void LogMessage(BimFromDgnDbLoggingSeverity severity, WCharCP message, ...);
        void LogPerformanceMessage(StopWatch& stopWatch, Utf8CP message, ...);
        void SendToQueue(Utf8CP json);
        void MakeNavigationProperty(Json::Value& out, Utf8CP propertyName, BeInt64Id id);
        void MakeNavigationProperty(Json::Value& out, Utf8CP propertyName, uint64_t id);
        void MakeNavigationProperty(Json::Value& out, Utf8CP propertyName, Utf8CP id);
        void MakeNavigationProperty(Json::Value& out, Utf8CP propertyName, Json::Value& id);
        void CalculateEntities();
        BentleyStatus ExportFonts();
        BentleyStatus ExportGeometryParts();
        BentleyStatus ExportLineStyles();
        BentleyStatus ExportTextures();
        BentleyStatus ExportAuthorities();
        BentleyStatus ExportViews();
        BentleyStatus ExportCategories();
        BentleyStatus ExportCategories(Utf8CP tableName, Utf8CP bisClassName, Utf8CP bisAuthorityStr, bvector<DgnCategoryId>& duplicateIds);
        DgnElementId CreateModelSelector(ViewControllerCR vc, Utf8CP name);
        DgnElementId CreateCategorySelector(ViewControllerCR vc, Utf8CP name);
        DgnElementId CreateDisplayStyle(ViewControllerCP vc, Utf8CP name, bool is3d, bool isCamera);
        BentleyStatus ExportModels();
        BentleyStatus ExportModel(Utf8CP schemaName, Utf8CP className, Utf8CP whereClause = nullptr);
        BentleyStatus ExportElements();
        BentleyStatus ExportElements(DgnModelId parentModel);
        BentleyStatus ExportElements(Json::Value& out, Utf8CP schemaName, Utf8CP className, DgnModelId parentModel, Utf8CP whereClause = nullptr, bool sendToQueue = true, bool allowDuplicates = false);
        BentleyStatus ExportElementAspects();
        BentleyStatus ExportElementAspects(ECN::ECClassId classId);
        BentleyStatus ExportElementAspects(ECN::ECClassId classId, BentleyApi::BeSQLite::EC::ECInstanceId aspectId);
        BentleyStatus ExportTextAnnotationData();
        BentleyStatus ExportNamedGroups();
        BentleyStatus ExportElementHasLinks();
        BentleyStatus ExportConstraint(ECN::ECClassId constraintClassId, BentleyApi::BeSQLite::EC::ECInstanceId constraintId);
        BentleyStatus ExportLinkTables(Utf8CP schemaName, Utf8CP className, Utf8CP newClassName = nullptr);
        BentleyStatus ExportPropertyData();
        BentleyStatus ExportEmbeddedFiles();
        BentleyStatus ExportExtraTables(Utf8CP alias, Utf8CP className);

        DgnElementId CreateCodeSpec(uint8_t codeSpecType, Utf8CP name);
        DgnElementId CreateSubjectElement(Utf8CP subjectName);
        DgnElementId CreatePartitionElement(DgnModelCR model, DgnElementId subject);
        DgnElementId CreatePartitionElement(Utf8CP partitionName, Utf8CP partitionType, DgnElementId subject);
        DgnElementId CreateDrawingElement(Utf8CP name);
        DgnElementId CreateSheetElement(DgnModelCR model);
        DgnElementId CreateDefinitionModel(Utf8CP modelName);
        BentleyStatus ExportUnits() const;
        BentleyStatus ExportSchemas() const;
        DgnElementId InitListModel(Utf8CP name);
        BentleyStatus InitDrawingListModel();
        BentleyStatus InitSheetListModel();
        BentleyStatus InitJobDefinitionModel();
        Utf8String RemapResourceAuthority(ECN::ECClassCP elementClass);
        void HandleAnnotationTextStyle(Json::Value& obj, DgnElementId id);
        
        // Planning schema specific exports
        BentleyStatus ExportTimelines();

        //! Report progress and detect if user has indicated that he wants to cancel.
        void ReportProgress() const;

        //! Report progress. 
        void ShowProgress() const { if (nullptr != m_meter) m_meter->ShowProgress(); }

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
        DgnDb0601ToJsonImpl(wchar_t const* dbPath, wchar_t const* tempPath, wchar_t const* assetsPath);
        bool OpenDgnDb();
        void SetProgressMeter(DgnProgressMeter* meter) { m_meter = meter; }
        bool ExportDgnDb();
        void SetGeneralLogger(T_LogGeneralMessage logger) { m_logger = logger; }
        void SetPerformanceLogger(T_LogPerformanceMessage logger) { m_performanceLog = logger; }
        void SetQueueWrite(T_QueueJson queue) { QueueJson = queue; }

    };

END_DGNDB0601_TO_JSON_NAMESPACE
