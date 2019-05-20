/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <Bentley/Base64Utilities.h>
#include "DgnDb0601ToJsonImpl.h"
#include <limits>
#include <DgnDb06Api/DgnPlatform/WebMercator.h>
#include <DgnDb06Api/DgnPlatform/DgnIModel.h>
#include <DgnDb06Api/DgnPlatform/DgnGeoCoord.h>
#include <DgnDb06Api/Planning/PlanningApi.h>
#include <DgnDb06Api/ECObjects/ECJsonUtilities.h>
#include <DgnDb06Api/PointCloudSchema/PointCloudSchemaApi.h>
#include <DgnDb06Api/ThreeMx/ThreeMxApi.h>
#include <DgnDb06Api/RasterSchema/RasterSchemaApi.h>
#include <DgnDb06Api/RasterSchema/RasterFileHandler.h>
#include <DgnDb06Api/DgnPlatform/JsonUtils.h>

DGNDB06_USING_NAMESPACE_BENTLEY
DGNDB06_USING_NAMESPACE_BENTLEY_SQLITE
DGNDB06_USING_NAMESPACE_BENTLEY_SQLITE_EC
DGNDB06_USING_NAMESPACE_BENTLEY_EC
DGNDB06_USING_NAMESPACE_BENTLEY_DGN
DGNDB06_USING_NAMESPACE_BENTLEY_PLANNING

static Utf8CP const JSON_TYPE_KEY = "Type";
static Utf8CP const JSON_OBJECT_KEY = "Object";
static Utf8CP const JSON_ACTION_KEY = "Action";
static Utf8CP const JSON_ACTION_INSERT = "Insert";
static Utf8CP const JSON_TYPE_Font = "Font";
static Utf8CP const JSON_TYPE_FontFaceData = "FontFaceData";
static Utf8CP const JSON_TYPE_LineStyleProperty = "LineStyleProperty";
static Utf8CP const JSON_TYPE_Model = "Model";
static Utf8CP const JSON_TYPE_CategorySelector = "CategorySelector";
static Utf8CP const JSON_TYPE_ModelSelector = "ModelSelector";
static Utf8CP const JSON_TYPE_DisplayStyle = "DisplayStyle";
static Utf8CP const JSON_TYPE_DictionaryModel = "DictionaryModel";
static Utf8CP const JSON_TYPE_CodeSpec = "CodeSpec";
static Utf8CP const JSON_TYPE_Schema = "Schema";
static Utf8CP const JSON_TYPE_Element = "Element";
static Utf8CP const JSON_TYPE_ElementMultiAspect = "ElementMultiAspect";
static Utf8CP const JSON_TYPE_ElementUniqueAspect = "ElementUniqueAspect";
static Utf8CP const JSON_TYPE_GeometricElement2d = "GeometricElement2d";
static Utf8CP const JSON_TYPE_GeometricElement3d = "GeometricElement3d";
static Utf8CP const JSON_TYPE_GeometryPart = "GeometryPart";
static Utf8CP const JSON_TYPE_Subject = "Subject";
static Utf8CP const JSON_TYPE_Partition = "Partition";
static Utf8CP const JSON_TYPE_Category = "Category";
static Utf8CP const JSON_TYPE_SubCategory = "SubCategory";
static Utf8CP const JSON_TYPE_ViewDefinition3d = "ViewDefinition3d";
static Utf8CP const JSON_TYPE_ViewDefinition2d = "ViewDefinition2d";
static Utf8CP const JSON_TYPE_LinkTable = "LinkTable";
static Utf8CP const JSON_TYPE_ElementGroupsMembers = "ElementGroupsMembers";
static Utf8CP const JSON_TYPE_ElementHasLinks = "ElementHasLinks";
static Utf8CP const JSON_TYPE_AnnotationTextStyle = "AnnotationTextStyle";
static Utf8CP const JSON_TYPE_Texture = "Texture";
static Utf8CP const JSON_TYPE_WorkBreakdown = "WorkBreakdown";
static Utf8CP const JSON_TYPE_Activity = "Activity";
static Utf8CP const JSON_TYPE_Baseline = "Baseline";
/*unused*/ //static Utf8CP const JSON_TYPE_TimeSpan = "TimeSpan";
static Utf8CP const JSON_TYPE_PropertyData = "PropertyData";
static Utf8CP const JSON_TYPE_TextAnnotationData = "TextAnnotationData";
static Utf8CP const JSON_TYPE_PointCloudModel = "PointCloudModel";
static Utf8CP const JSON_TYPE_ThreeMxModel = "ThreeMxModel";
static Utf8CP const JSON_TYPE_RasterFileModel = "RasterFileModel";
static Utf8CP const JSON_TYPE_EmbeddedFile = "EmbeddedFile";

static Utf8CP const  BIS_ELEMENT_PROP_CodeSpec="CodeSpec";
static Utf8CP const  BIS_ELEMENT_PROP_CodeScope="CodeScope";
static Utf8CP const  BIS_ELEMENT_PROP_CodeValue = "CodeValue";
static Utf8CP const  BIS_ELEMENT_PROP_Model = "Model";
static Utf8CP const  BIS_ELEMENT_PROP_Parent = "Parent";

static Utf8CP const  BIS_MODEL_PROP_ModeledElement = "ModeledElement";

static Utf8CP const JSON_INSTANCE_ID = "id";
static Utf8CP const JSON_CLASSNAME = "className";

#define EMBEDDED_FACE_DATA_PROP_NS "dgn_Font"
#define EMBEDDED_FACE_DATA_PROP_NAME "EmbeddedFaceData"
static const PropertySpec EMBEDDED_FACE_DATA_PROPERTY_SPEC(EMBEDDED_FACE_DATA_PROP_NAME, EMBEDDED_FACE_DATA_PROP_NS);

BEGIN_DGNDB0601_TO_JSON_NAMESPACE

static const wchar_t s_spinner[] = L" /-\\|";
static const size_t s_spinnerSize = _countof(s_spinner) - 1;

//=======================================================================================
// A quick and dirty progress meter.
// @bsiclass                                                    Sam.Wilson  07/14
//=======================================================================================
struct PrintfProgressMeter : BentleyApi::Dgn::DgnProgressMeter
    {
    DEFINE_T_SUPER(DgnProgressMeter)

    protected:
        Utf8String m_stepName;
        Utf8String m_taskName;
        Utf8String m_lastMessage;
        double m_timeOfLastUpdate;
        double m_timeOfLastSpinnerUpdate;
        uint32_t m_spinCount;

        void UpdateDisplay0(Utf8StringCR msgLeft)
            {
            m_lastMessage = msgLeft;

            // Display the number of tasks remaining. Not all major tasks have a task count.
            Utf8String tbd;
            if (m_stepsRemaining || m_tasksRemaining)
                tbd = Utf8PrintfString(":%d/%d", m_stepsRemaining, m_tasksRemaining);

            // Display the spinner and the task.
            Utf8PrintfString msg("[%c] %-123.123s %-16.16s", s_spinner[m_spinCount%s_spinnerSize], msgLeft.c_str(), tbd.c_str());
            printf("%s\r", msg.c_str());
            }

        void UpdateDisplay()
            {
            auto now = BeTimeUtilities::QuerySecondsCounter();

            if ((now - m_timeOfLastUpdate) < 1.0)
                return;

            m_timeOfLastUpdate = now;

            UpdateDisplay0(FmtMessage());
            }

        bool HasDescription() const
            {
            return m_taskName.find(':') != Utf8String::npos;
            }

        Utf8String FmtMessage() const
            {
            Utf8String msg(m_stepName);
            msg.append(": ");
            msg.append(m_taskName);
            return msg;
            }

        void ForceNextUpdateToDisplay() { m_timeOfLastUpdate = m_timeOfLastSpinnerUpdate = 0; }
        void _Hide() override
            {
            Utf8PrintfString msg("    %-123.123s %-16.16s", "", "");
            printf("%s\r", msg.c_str());
            }

        Abort _ShowProgress() override
            {
            if (m_aborted)
                return ABORT_Yes;

            auto now = BeTimeUtilities::QuerySecondsCounter();

            if ((now - m_timeOfLastSpinnerUpdate) < 0.25) // don't do printf's more than a few times per second -- too slow and not useful
                return ABORT_No;

            m_timeOfLastSpinnerUpdate = now;

            m_spinCount++;

            bool justShowSpinner = false;

            if ((now - m_timeOfLastUpdate) < 0.5)
                justShowSpinner = true;         // don't push out full messages more than a couple times per second -- too slow and not useful
            else
                justShowSpinner = (FmtMessage() == m_lastMessage);

            if (justShowSpinner)
                {
                printf("[%c]\r", s_spinner[m_spinCount%s_spinnerSize]);
                return ABORT_No;
                }

            ForceNextUpdateToDisplay();
            UpdateDisplay();
            return ABORT_No;
            }

        void _SetCurrentStepName(Utf8CP stepName) override
            {
            T_Super::_SetCurrentStepName(stepName); // decrements step count

            if (NULL == stepName)
                {
                m_stepName.clear();
                return;
                }
            if (m_stepName.Equals(stepName))
                return;

            m_stepName = stepName;
            m_taskName.clear();
            m_spinCount = 0;
            ForceNextUpdateToDisplay();
            UpdateDisplay();
            }

        void _SetCurrentTaskName(Utf8CP taskName) override
            {
            T_Super::_SetCurrentTaskName(taskName); // decrements task count

            if (taskName && m_taskName == taskName)
                return;

            m_taskName = taskName ? taskName : "";
            m_spinCount = 0;
            ForceNextUpdateToDisplay();
            UpdateDisplay();
            }

    public:
        PrintfProgressMeter() : BentleyApi::Dgn::DgnProgressMeter(), m_timeOfLastUpdate(0), m_timeOfLastSpinnerUpdate(0), m_spinCount(0) {}
    };

static const size_t ID_STRINGBUFFER_LENGTH = std::numeric_limits<uint64_t>::digits + 1; //+1 for the trailing 0 character

Utf8String IdToString(int64_t id)
    {
    Utf8Char idStrBuffer[ID_STRINGBUFFER_LENGTH];
    BeStringUtilities::FormatUInt64(idStrBuffer, ID_STRINGBUFFER_LENGTH, id, (HexFormatOptions) ((int) HexFormatOptions::IncludePrefix | (int) HexFormatOptions::Uppercase));
    return Utf8String(idStrBuffer);
    }

Utf8String IdToString(uint64_t id)
    {
    Utf8Char idStrBuffer[ID_STRINGBUFFER_LENGTH];
    BeStringUtilities::FormatUInt64(idStrBuffer, ID_STRINGBUFFER_LENGTH, id, (HexFormatOptions) ((int) HexFormatOptions::IncludePrefix | (int) HexFormatOptions::Uppercase));
    return Utf8String(idStrBuffer);
    }

Utf8String IdToString(Utf8CP idString)
    {
    uint64_t id;
    BeStringUtilities::ParseUInt64(id, idString);
    return IdToString(id);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnDb0601ToJsonImpl::DgnDb0601ToJsonImpl(wchar_t const* dbPath, wchar_t const* tempPath, wchar_t const* assetsPath) : m_dbPath(dbPath), m_tempPath(tempPath), m_assetsPath(assetsPath)
    {
    static PrintfProgressMeter printfProgress;
    SetProgressMeter(&printfProgress);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
L10N::SqlangFiles DgnDb0601ToJsonImpl::_SupplySqlangFiles()
    {
    BeFileName sqlangFile(GetIKnownLocationsAdmin().GetDgnPlatformAssetsDirectory());
    sqlangFile.AppendToPath(L"sqlang");
    sqlangFile.AppendToPath(L"DgnPlatform_en.sqlang.db3");

    return L10N::SqlangFiles(sqlangFile);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
void DgnDb0601ToJsonImpl::LogMessage(BimFromDgnDbLoggingSeverity severity, Utf8CP message, ...)
    {
    va_list args;
    va_start(args, message);
    Utf8PrintfString msg(message, args);
    va_end(args);
    m_logger(severity, msg.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
void DgnDb0601ToJsonImpl::LogMessage(BimFromDgnDbLoggingSeverity severity, WCharCP message, ...)
    {
    va_list args;
    va_start(args, message);
    WPrintfString msg(message, args);
    va_end(args);
    m_logger(severity, Utf8String(msg.c_str()).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
void DgnDb0601ToJsonImpl::LogPerformanceMessage(StopWatch& stopWatch, Utf8CP description, ...)
    {
    stopWatch.Stop();
    va_list args;
    va_start(args, description);
    Utf8String formattedDescription;
    formattedDescription.VSprintf(description, args);
    va_end(args);

    Utf8PrintfString message("%s|%.0f millisecs", formattedDescription.c_str(), stopWatch.GetElapsedSeconds() * 1000.0);
    m_performanceLog(message.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
void DgnDb0601ToJsonImpl::SendToQueue(Json::Value& json, bool doReplace)
    {
    Utf8String jsonStr = Json::FastWriter::ToString(json);
    if (doReplace)
        jsonStr.ReplaceAll("dgn.", "BisCore.");
    (QueueJson)(jsonStr.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool DgnDb0601ToJsonImpl::OpenDgnDb()
    {
    DgnPlatformLib::Initialize(*this, false);
    DgnDomains::RegisterDomain(Planning::PlanningDomain::GetDomain());
    DbResult dbStatus;
    DgnDb::OpenParams openParams(DgnDb::OpenMode::Readonly);
    BeFileName dgndbFileName;

    if (m_dbPath.GetExtension().EqualsI(L"imodel"))
        {
        BimFormatVersion versionInfo = BimFormatVersion::Extract(m_dbPath);
        if (!versionInfo.IsValid() || !versionInfo.IsCurrent())
            {
            LogMessage(BimFromDgnDbLoggingSeverity::LOG_ERROR, L"Failed to open imodel '%ls'", m_dbPath.GetName());
            return false;
            }

        BeFileName tempPathname;
        BentleyStatus status = T_HOST.GetIKnownLocationsAdmin().GetLocalTempDirectory(tempPathname, nullptr);
        if (SUCCESS != status)
            {
            LogMessage(BimFromDgnDbLoggingSeverity::LOG_ERROR, "Failed to retrieve temporary path for extracting imodel");
            return false;
            }

        dgndbFileName = tempPathname.Combine({L"Bentley", L"DgnDb0601ToBimConverter", m_dbPath.GetFileNameWithoutExtension().AppendA(".i.idgndb").c_str()});
        if (!dgndbFileName.GetDirectoryName().DoesPathExist())
            {
            BeFileName::CreateNewDirectory(dgndbFileName.GetDirectoryName().c_str());
            }

        BentleyApi::BeSQLite::DbResult dbResult;
        StopWatch timer(true);
        DgnIModel::ExtractUsingDefaults(dbResult, dgndbFileName, m_dbPath, true);

        if (BentleyApi::BeSQLite::DbResult::BE_SQLITE_OK != dbResult)
            {
            LogMessage(BimFromDgnDbLoggingSeverity::LOG_ERROR, "Failed to extract dgndb from imodel");
            return false;
            }
        LogPerformanceMessage(timer, "Extract dgndb from imodel");


        }
    else
        dgndbFileName = m_dbPath;

    m_dgndb = DgnDb::OpenDgnDb(&dbStatus, dgndbFileName, openParams);
    if (!m_dgndb.IsValid())
        {
        LogMessage(BimFromDgnDbLoggingSeverity::LOG_ERROR, L"Failed to open DgnDb '%ls'", dgndbFileName.GetName());
        return false;
        }

    m_nextAvailableId = DgnElementId(*m_dgndb, "dgn_Element", "Id");
    m_nextAvailableId.UseNext(*m_dgndb);

    m_elementClass = m_dgndb->Schemas().GetECClass(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_Element);
    m_geometric2dClass = m_dgndb->Schemas().GetECClass(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_GeometricElement2d);
    m_geometric3dClass = m_dgndb->Schemas().GetECClass(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_GeometricElement3d);
    m_elementAspectClass = m_dgndb->Schemas().GetECClass(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_ElementAspect);
    m_elementUniqueAspectClass = m_dgndb->Schemas().GetECClass(DGN_ECSCHEMA_NAME, "ElementUniqueAspect");
    m_elementMultiAspectClass = m_dgndb->Schemas().GetECClass(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_ElementMultiAspect);
    m_viewDefinition3dClass = m_dgndb->Schemas().GetECClass(DGN_ECSCHEMA_NAME, "ViewDefinition3d");
    m_viewDefinition2dClass = m_dgndb->Schemas().GetECClass(DGN_ECSCHEMA_NAME, "ViewDefinition2d");
    m_spatialViewDefinitionId = m_dgndb->Schemas().GetECClassId(DGN_ECSCHEMA_NAME, "SpatialViewDefinition");
    m_cameraViewDefinitionClass = m_dgndb->Schemas().GetECClass(DGN_ECSCHEMA_NAME, "CameraViewDefinition");
    m_sheetViewDefinitionClass = m_dgndb->Schemas().GetECClass(DGN_ECSCHEMA_NAME, "SheetViewDefinition");
    m_drawingViewDefinitionClass = m_dgndb->Schemas().GetECClass(DGN_ECSCHEMA_NAME, "DrawingViewDefinition");
    m_subCategoryClass = m_dgndb->Schemas().GetECClass(DGN_ECSCHEMA_NAME, "SubCategory");
    m_categoryClass = m_dgndb->Schemas().GetECClass(DGN_ECSCHEMA_NAME, "Category");
    m_linkModelClass = m_dgndb->Schemas().GetECClass(DGN_ECSCHEMA_NAME, "LinkModel");
    m_annotationTextStyle = m_dgndb->Schemas().GetECClass(DGN_ECSCHEMA_NAME, "AnnotationTextStyle");
    m_textureClass = m_dgndb->Schemas().GetECClass(DGN_ECSCHEMA_NAME, "Texture");
    m_planningModelClass = m_dgndb->Schemas().GetECClass("Planning", "PlanningModel");
    m_planningElementClass = m_dgndb->Schemas().GetECClass("Planning", "PlanningElement");
    m_workbreakDownClass = m_dgndb->Schemas().GetECClass("Planning", "WorkBreakdown");
    m_activityClass = m_dgndb->Schemas().GetECClass("Planning", "Activity");
    m_timeSpanClass = m_dgndb->Schemas().GetECClass("Planning", "TimeSpan");
    m_cameraKeyFrameClass = m_dgndb->Schemas().GetECClass("Planning", "CameraKeyFrame");
    m_pointCloudModelClass = m_dgndb->Schemas().GetECClass("PointCloud", "PointCloudModel");
    m_threeMxModelClass = m_dgndb->Schemas().GetECClass("ThreeMx", "ThreeMxModel");
    m_rasterFileModelClass = m_dgndb->Schemas().GetECClass("Raster", "RasterFileModel");

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
bool DgnDb0601ToJsonImpl::ExportDgnDb()
    {
    if (!OpenDgnDb())
        return false;

    m_meter->AddSteps(9);

    SetStepName(DgnDb0601ToJsonImpl::ProgressMessage::STEP_EXPORT_SCHEMAS());
    StopWatch timer(true);
    StopWatch totalTimer(true);
    BentleyStatus stat = SUCCESS;
    if (SUCCESS != (stat = ExportUnits()))
        return false;

    if (SUCCESS != (stat = ExportSchemas()))
        return false;

    LogPerformanceMessage(timer, "Export Schemas");
    CalculateEntities();

    timer.Start();
    if (SUCCESS != (stat = ExportFonts()))
        return false;
    LogPerformanceMessage(timer, "Export Fonts");

    SetStepName(DgnDb0601ToJsonImpl::ProgressMessage::STEP_EXPORT_AUTHORITIES());
    timer.Start();
    if (SUCCESS != (stat = ExportAuthorities()))
        return false;
    LogPerformanceMessage(timer, "Export Authorities");

    SetStepName(DgnDb0601ToJsonImpl::ProgressMessage::STEP_EXPORT_MODELS());
    timer.Start();
    if (SUCCESS != (stat = ExportModels()))
        return false;
    LogPerformanceMessage(timer, "Export Models");

    timer.Start();
    if (SUCCESS != (stat = ExportCategories()))
        return false;
    LogPerformanceMessage(timer, "Export Categories");

    timer.Start();
    if (SUCCESS != (stat = ExportViews()))
        return false;
    LogPerformanceMessage(timer, "Export Views");

    timer.Start();
    if (SUCCESS != (stat = ExportGeometryParts()))
        return false;
    LogPerformanceMessage(timer, "Export GeomParts");

    timer.Start();
    if (SUCCESS != (stat = ExportLineStyles()))
        return false;
    LogPerformanceMessage(timer, "Export LineStyles");

    timer.Start();
    if (SUCCESS != (stat = ExportTextures()))
        return false;
    LogPerformanceMessage(timer, "Export Textures");

    //if (m_dgndb->Schemas().ContainsECSchema("Planning"))
    //    {
    //    if (SUCCESS != (stat = ExportTimelines()))
    //        return false;
    //    }

    if (SUCCESS != (stat = ExportExtraTables("rv", "VisualizationRuleSet")))
        return false;

    timer.Start();
    if (SUCCESS != (stat = ExportElements()))
        return false;
    ReportProgress();
    LogPerformanceMessage(timer, "Export Elements");

    timer.Start();
    if (SUCCESS != (stat = ExportElementAspects()))
        return false;
    LogPerformanceMessage(timer, "Export ElementAspects");

    timer.Start();
    if (SUCCESS != (stat = ExportTextAnnotationData()))
        return false;
    LogPerformanceMessage(timer, "Export TextAnnotationData");

    timer.Start();
    if (SUCCESS != (stat = ExportNamedGroups()))
        return false;
    LogPerformanceMessage(timer, "Export Named Groups");

    timer.Start();
    if (SUCCESS != (stat = ExportElementHasLinks()))
        return false;
    LogPerformanceMessage(timer, "Export ElementHasLinks");

    timer.Start();
    if (SUCCESS != (stat = ExportLinkTables("generic", "ElementRefersToElement")))
        return false;

    if (m_dgndb->Schemas().ContainsECSchema("Planning"))
        {
        if (SUCCESS != (stat = ExportLinkTables("Planning", "ActivityAffectsElements")))
            return false;
        if (SUCCESS != (stat = ExportLinkTables("Planning", "ActivityHasConstraint")))
            return false;
        //if (SUCCESS != (stat = ExportLinkTables("Planning", "WorkBreakdownHasTimeSpans", "WorkBreakdownOwnsTimeSpans")))
        //    return false;
        //if (SUCCESS != (stat = ExportLinkTables("Planning", "ActivityHasTimeSpans", "ActivityOwnsTimeSpans")))
        //    return false;
        }

    LogPerformanceMessage(timer, "Export EC Relationships");

    ExportPropertyData();

    timer.Start();
    ExportEmbeddedFiles();
    LogPerformanceMessage(timer, "Export Embedded Files");
    LogPerformanceMessage(totalTimer, "Total export time");

    m_dgndb->CloseDb();
    m_dgndb = nullptr;
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2018
//---------------+---------------+---------------+---------------+---------------+-------
void DgnDb0601ToJsonImpl::CalculateEntities()
    {
    Utf8String sql("SELECT SUM(rows) as total FROM (");
    sql.append("SELECT count(*) as rows FROM dgn_Element ");
    sql.append("UNION ALL SELECT count(*) as rows FROM dgn_Authority ");
    sql.append("UNION ALL SELECT count(*) as rows FROM dgn_Model ");
    sql.append("UNION ALL SELECT count(*) as rows FROM dgn_Font ");
    sql.append("UNION ALL SELECT count(*) as rows FROM dgn_ElementGroupsMembers ");
    sql.append("UNION ALL SELECT count(*) as rows FROM dgn_TextAnnotationData ");
    sql.append("UNION ALL SELECT count(*) as rows FROM _dgn_ElementAspect ");
    if (m_dgndb->Schemas().ContainsECSchema("Planning"))
        sql.append("UNION ALL SELECT count(*) as rows FROM bp_ActivityAffectsElements ");
    sql.append(") u");
    
    Statement stmt;
    if (BE_SQLITE_OK != (stmt.Prepare(*m_dgndb, sql.c_str())))
        return;

    while (BE_SQLITE_ROW == stmt.Step())
        {
        auto entry = Json::Value(Json::ValueType::objectValue);
        int64_t id = stmt.GetValueInt64(0);
        entry["elementCount"] = id;
        SendToQueue(entry);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DgnDb0601ToJsonImpl::ExportFonts()
    {
    Statement stmt;
    Utf8PrintfString sql("SELECT Id,SubId,StrData,RawSize FROM be_Prop WHERE Namespace='%s' AND Name='%s'", EMBEDDED_FACE_DATA_PROP_NS, EMBEDDED_FACE_DATA_PROP_NAME);
    if (BE_SQLITE_OK != (stmt.Prepare(*m_dgndb, sql.c_str())))
        {
        LogMessage(BimFromDgnDbLoggingSeverity::LOG_ERROR, "Unable to prepare statement to retrieve font properties");
        return ERROR;
        }
    while (BE_SQLITE_ROW == stmt.Step())
        {
        auto entry = Json::Value(Json::ValueType::objectValue);
        entry[JSON_TYPE_KEY] = JSON_TYPE_FontFaceData;
        entry[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
        entry[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
        auto& row = entry[JSON_OBJECT_KEY];
        row.clear();
        int64_t id = stmt.GetValueInt64(0);
        row[JSON_INSTANCE_ID] = IdToString(id).c_str();
        row["SubId"] = IdToString(stmt.GetValueInt64(1)).c_str();
        row["StrData"] = stmt.GetValueText(2);
        int rawSize = stmt.GetValueInt(3);
        row["RawSize"] = rawSize;

        bvector<Byte> data;
        data.resize(rawSize);
        m_dgndb->QueryProperty(&data[0], (uint32_t) data.size(), EMBEDDED_FACE_DATA_PROPERTY_SPEC, id, 0);
        Utf8String str;
        Base64Utilities::Encode(str, data.data(), rawSize);
        row["Base64EncodedData"] = str.c_str();
        row["Base64EncodedSize"] = str.SizeInBytes();
        SendToQueue(entry);
        }

    Statement stmt2;
    Utf8String sql2("SELECT Id,Type,Name,Metadata FROM dgn_Font");
    if (BE_SQLITE_OK != (stmt2.Prepare(*m_dgndb, sql2.c_str())))
        {
        LogMessage(BimFromDgnDbLoggingSeverity::LOG_ERROR, "Unable to prepare statement to retrieve fonts");
        return ERROR;
        }

    while (BE_SQLITE_ROW == stmt2.Step())
        {
        auto entry = Json::Value(Json::ValueType::objectValue);
        entry[JSON_TYPE_KEY] = JSON_TYPE_Font;
        entry[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
        entry[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
        auto& row = entry[JSON_OBJECT_KEY];
        row.clear();
        row[JSON_INSTANCE_ID] = IdToString(stmt2.GetValueInt64(0)).c_str();
        row["Type"] = stmt2.GetValueInt(1);
        row["Name"] = stmt2.GetValueText(2);

        ByteCP data = static_cast<ByteCP>(stmt2.GetValueBlob(3));
        size_t dataSize = stmt2.GetColumnBytes(3);
        Utf8String str;
        Base64Utilities::Encode(str, data, dataSize);
        row["Metadata"] = str.c_str();
        SendToQueue(entry);
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DgnDb0601ToJsonImpl::ExportGeometryParts()
    {
    auto out = Json::Value(Json::ValueType::objectValue);
    // These need to come first as the LineStyle components contain mapped ids.  And LineStyleElements using LineStyle components.
    return ExportElements(out, DGN_ECSCHEMA_NAME, "GeometryPart", m_dgndb->GetDictionaryModel().GetModelId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DgnDb0601ToJsonImpl::ExportTextures()
    {
    auto out = Json::Value(Json::ValueType::objectValue);
    // These need to come first as the GeomParts contain mapped ids
    return ExportElements(out, DGN_ECSCHEMA_NAME, "Texture", m_dgndb->GetDictionaryModel().GetModelId());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DgnDb0601ToJsonImpl::ExportLineStyles()
    {
    Statement stmt;
    Utf8String sql("SELECT Id, Name, StrData FROM be_Prop WHERE Namespace='dgn_LStyle'");
    if (BE_SQLITE_OK != (stmt.Prepare(*m_dgndb, sql.c_str())))
        {
        LogMessage(BimFromDgnDbLoggingSeverity::LOG_ERROR, "Unable to prepare statement to retrieve line style properties");
        return ERROR;
        }
    while (BE_SQLITE_ROW == stmt.Step())
        {
        auto entry = Json::Value(Json::ValueType::objectValue);
        entry[JSON_TYPE_KEY] = JSON_TYPE_LineStyleProperty;
        entry[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
        entry[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
        auto& row = entry[JSON_OBJECT_KEY];
        row.clear();
        int64_t id = stmt.GetValueInt64(0);
        row[JSON_INSTANCE_ID] = IdToString(id).c_str();
        row["Name"] = stmt.GetValueText(1);
        row["StrData"] = stmt.GetValueText(2);
        SendToQueue(entry);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DgnDb0601ToJsonImpl::ExportAuthorities()
    {
    Statement stmt;
    Utf8PrintfString sql("SELECT Id, Properties FROM dgn_Authority");
    if (BE_SQLITE_OK != (stmt.Prepare(*m_dgndb, sql.c_str())))
        {
        LogMessage(BimFromDgnDbLoggingSeverity::LOG_ERROR, "Unable to prepare statement to retrieve authorities");
        return ERROR;
        }
    while (BE_SQLITE_ROW == stmt.Step())
        {
        DgnAuthorityId id = stmt.GetValueId<DgnAuthorityId>(0);
        DgnAuthorityCPtr authority = m_dgndb->Authorities().GetAuthority(id);
        
        //All of the Dgn* authorities have been changed.
        if (authority->GetName().Equals("DgnComponent") ||
            authority->GetName().Equals("DgnModels") ||
            authority->GetName().Equals("DgnCategories") ||
            authority->GetName().Equals("DgnResources") ||
            authority->GetName().Equals("DgnColors"))
            continue;

        auto entry = Json::Value(Json::ValueType::objectValue);

        ECClassCP authorityClass = m_dgndb->Schemas().GetECClass(authority->GetClassId());

        entry[JSON_TYPE_KEY] = JSON_TYPE_CodeSpec;
        entry[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
        entry[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
        auto& row = entry[JSON_OBJECT_KEY];
        row.clear();
        if (authorityClass->GetName().Equals("NamespaceAuthority"))
            row["CodeSpecType"] = 1;
        else 
            row["CodeSpecType"] = 1;
        row[JSON_INSTANCE_ID] = IdToString(id.GetValue()).c_str();
        if (authority->GetName().Equals("Local"))
            row["Name"] = "bis:NullCodeSpec";
        else if (authority->GetName().Equals("DgnGeometryPart"))
            row["Name"] = "bis:GeometryPart";
        else if (authority->GetName().Equals("DgnMaterials"))
            row["Name"] = "bis:MaterialElement";
        else
            row["Name"] = authority->GetName().c_str();
        row["Properties"] = stmt.GetValueText(1);
        SendToQueue(entry);
        }

    // Need to make some "fake" authorities that were added in BisCore
    CreateCodeSpec(2, "bis:AnnotationFrameStyle");
    CreateCodeSpec(2, "bis:AnnotationLeaderStyle");
    CreateCodeSpec(2, "bis:AnnotationTextStyle");
    CreateCodeSpec(2, "bis:CategorySelector");
    CreateCodeSpec(2, "bis:ColorBook");
    CreateCodeSpec(2, "bis:DisplayStyle");
    CreateCodeSpec(2, "bis:Drawing");
    CreateCodeSpec(2, "bis:DrawingCategory");
    CreateCodeSpec(2, "bis:GraphicalType2d");
    CreateCodeSpec(2, "bis:LineStyle");
    CreateCodeSpec(2, "bis:LinkElement");
    CreateCodeSpec(2, "bis:ModelSelector");
    CreateCodeSpec(2, "bis:PhysicalType");
    CreateCodeSpec(2, "bis:Sheet");
    CreateCodeSpec(2, "bis:SpatialCategory");
    CreateCodeSpec(2, "bis:SpatialLocationType");
    CreateCodeSpec(2, "bis:TextAnnotationSeed");
    CreateCodeSpec(2, "bis:Texture");
    CreateCodeSpec(2, "bis:ViewDefinition");


    CreateCodeSpec(3, "bis:InformationPartitionElement");
    CreateCodeSpec(3, "bis:Subject");
    CreateCodeSpec(3, "bis:SubCategory");

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnElementId DgnDb0601ToJsonImpl::CreateCodeSpec(uint8_t codeSpecType, Utf8CP name)
    {
    auto entry = Json::Value(Json::ValueType::objectValue);
    entry[JSON_TYPE_KEY] = JSON_TYPE_CodeSpec;
    entry[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
    entry[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
    auto& row = entry[JSON_OBJECT_KEY];
    row.clear();
    row["CodeSpecType"] = codeSpecType;
    row["Name"] = name;
    m_nextAvailableId = DgnElementId(m_nextAvailableId.GetValue() + 1);
    row[JSON_INSTANCE_ID] = IdToString(m_nextAvailableId.GetValue()).c_str();

    m_authorityIds[name] = IdToString(m_nextAvailableId.GetValue());
    SendToQueue(entry);
    return m_nextAvailableId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            12/2016
//---------------+---------------+---------------+---------------+---------------+-------
Utf8String DgnDb0601ToJsonImpl::RemapResourceAuthority(ECN::ECClassCP elementClass)
    {
    Utf8String authority;
    if (elementClass->GetName().Equals("AnnotationTextStyle"))
        authority = m_authorityIds["bis:AnnotationTextStyle"];
    else if (elementClass->GetName().Equals("AnnotationFrameStyle"))
        authority = m_authorityIds["bis:AnnotationFrameStyle"];
    else if (elementClass->GetName().Equals("AnnotationLeaderStyle"))
        authority = m_authorityIds["bis:AnnotationLeaderStyle"];
    else if (elementClass->GetName().Equals("TextAnnotationSeed"))
        authority = m_authorityIds["bis:TextAnnotationSeed"];
    else if (elementClass->GetName().Equals("Texture"))
        authority = m_authorityIds["bis:Texture"];
    else if (elementClass->GetName().Equals("TrueColor"))
        authority = m_authorityIds["bis:TrueColor"];
    else if (elementClass->GetName().Equals("ViewDefinition"))
        authority = m_authorityIds["bis:ViewDefinition"];
    else if (elementClass->GetName().Equals("LineStyle"))
        authority = m_authorityIds["bis:LineStyle"];
    if (!Utf8String::IsNullOrEmpty(authority.c_str()))
        return authority;

    for (ECN::ECClassCP base : elementClass->GetBaseClasses())
        {
        authority = RemapResourceAuthority(base);
        if (!Utf8String::IsNullOrEmpty(authority.c_str()))
            return authority;
        }
    return authority;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnElementId DgnDb0601ToJsonImpl::CreateDisplayStyle(ViewControllerCP vc, Utf8CP name, bool is3d, bool isCamera)
    {
    Render::ViewFlags viewFlags = vc->GetViewFlags();

    auto displayStyle = Json::Value(Json::ValueType::objectValue);
    displayStyle[JSON_TYPE_KEY] = JSON_TYPE_DisplayStyle;
    displayStyle[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
    displayStyle[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
    auto& row = displayStyle[JSON_OBJECT_KEY];
    m_nextAvailableId = DgnElementId(m_nextAvailableId.GetValue() + 1);
    row[JSON_INSTANCE_ID] = IdToString(m_nextAvailableId.GetValue());
    row["Name"] = name;
    row["ViewFlags"] = Json::Value(Json::ValueType::objectValue);

    // ViewFlags Json has changed in bim0200, so we need to manually create the Json
    auto& vf = row["ViewFlags"];
    viewFlags.ToBaseJson(vf);
    vf.removeMember("construct");
    vf.removeMember("fill");
    if (!viewFlags.m_constructions) vf["noConstruct"] = true;
    if (!viewFlags.m_fill) vf["noFill"] = true;

    if (is3d)
        {
        viewFlags.To3dJson(vf);
        vf.removeMember("ignoreLighting");
        vf.removeMember("noClipVol");
        if (!viewFlags.m_noClipVolume) vf["clipVol"] = true;
        if (viewFlags.m_ignoreLighting) vf["noLighting"] = true;
        }

    row["BackgroundColor"] = vc->GetBackgroundColor().GetValue();
    row["Is3d"] = is3d;
    if (isCamera)
        {
        CameraViewControllerCP camera = dynamic_cast<CameraViewControllerCP>(vc);
        if (nullptr != camera)
            {
            row["IsEnvironmentEnabled"] = camera->IsEnvironmentEnabled();
            if (camera->IsEnvironmentEnabled())
                {
                row["Environment"] = Json::Value(Json::ValueType::objectValue);
                auto& env = row["Environment"];
                CameraViewController::EnvironmentDisplay display = camera->GetEnvironmentDisplay();
                if (camera->IsGroundPlaneEnabled())
                    {
                    env["GroundPlaneEnabled"] = true;
                    env["GroundPlane"] = Json::Value(Json::ValueType::objectValue);
                    auto& ground = env["GroundPlane"];
                    ground["elevation"] = display.m_groundPlane.m_elevation;
                    ground["aboveColor"] = display.m_groundPlane.m_aboveColor.GetValue();
                    ground["belowColor"] = display.m_groundPlane.m_belowColor.GetValue();

                    }
                if (camera->IsSkyBoxEnabled())
                    {
                    env["SkyBoxEnabled"] = true;
                    env["SkyBox"] = Json::Value(Json::ValueType::objectValue);
                    auto& skyBox = env["SkyBox"];
                    skyBox["jpegFile"] = display.m_skybox.m_jpegFile.c_str();
                    skyBox["zenithColor"] = display.m_skybox.m_zenithColor.GetValue();
                    skyBox["nadirColor"] = display.m_skybox.m_nadirColor.GetValue();
                    skyBox["groundColor"] = display.m_skybox.m_groundColor.GetValue();
                    skyBox["skyColor"] = display.m_skybox.m_skyColor.GetValue();
                    skyBox["groundExponent"] = display.m_skybox.m_groundExponent;
                    skyBox["skyExponent"] = display.m_skybox.m_skyExponent;
                    }
                }
            }
        }
    MakeNavigationProperty(row, BIS_ELEMENT_PROP_Model, m_jobDefinitionModelId);
    SendToQueue(displayStyle);

    return m_nextAvailableId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnElementId DgnDb0601ToJsonImpl::CreateCategorySelector(ViewControllerCR vc, Utf8CP name)
    {
    // Need to export the categories before we can create the category selector
    for (DgnCategoryId id : vc.GetViewedCategories())
        {
        if (m_insertedElements.end() == m_insertedElements.find(id))
            {

            DgnCategoryCPtr cat = DgnCategory::QueryCategory(id, *m_dgndb);
            if (!cat.IsValid())
                continue;
            Utf8PrintfString whereClause(" AND ECInstanceId=%" PRIu64, cat->GetElementId().GetValue());
            auto out = Json::Value(Json::ValueType::objectValue);
            ExportElements(out, cat->GetElementClass()->GetSchema().GetName().c_str(), cat->GetElementClass()->GetName().c_str(), cat->GetModelId(), whereClause.c_str(), false);
            auto& entry = out[JSON_OBJECT_KEY];
            auto& codeSpec = entry[BIS_ELEMENT_PROP_CodeSpec] = Json::Value(Json::ValueType::objectValue);
            if (vc.IsDrawingView() || vc.IsSheetView())
                {
                entry[JSON_CLASSNAME] = "BisCore.DrawingCategory";
                codeSpec["id"] = m_authorityIds["bis:DrawingCategory"].c_str();
                }
            else
                {
                entry[JSON_CLASSNAME] = "BisCore.SpatialCategory";
                codeSpec["id"] = m_authorityIds["bis:SpatialCategory"].c_str();
                }
            SendToQueue(out);
            }

        for (DgnSubCategoryId subCatId : DgnSubCategory::QuerySubCategories(*m_dgndb, id))
            {
            if (m_insertedElements.end() != m_insertedElements.find(subCatId))
                continue;
            DgnSubCategoryCPtr subCat = DgnSubCategory::QuerySubCategory(subCatId, *m_dgndb);
            Utf8PrintfString subCatWhereClause(" AND ECInstanceId=%" PRIu64, subCatId.GetValue());
            auto subCatJson = Json::Value(Json::ValueType::objectValue);
            ExportElements(subCatJson, subCat->GetElementClass()->GetSchema().GetName().c_str(), subCat->GetElementClass()->GetName().c_str(), subCat->GetModelId(), subCatWhereClause.c_str(), true);
            }
        }

    auto categorySelector = Json::Value(Json::ValueType::objectValue);
    categorySelector[JSON_TYPE_KEY] = JSON_TYPE_CategorySelector;
    categorySelector[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
    categorySelector[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
    auto& row = categorySelector[JSON_OBJECT_KEY];
    row.clear();
    m_nextAvailableId = DgnElementId(m_nextAvailableId.GetValue() + 1);
    row[JSON_INSTANCE_ID] = IdToString(m_nextAvailableId.GetValue());
    row["Name"] = name;
    row["Categories"] = Json::Value(Json::ValueType::arrayValue);
    MakeNavigationProperty(row, "DefinitionModel", m_jobDefinitionModelId.GetValue());
    DgnElementId categorySelectorId = m_nextAvailableId;
    for (DgnCategoryId id : vc.GetViewedCategories())
        {
        Json::Value category(Json::ValueType::uintValue);
        category = IdToString(id.GetValue());
        if (vc.IsDrawingView() || vc.IsSheetView())
            {
            if (m_mappedDrawingCategories.find(id) != m_mappedDrawingCategories.end())
                category = IdToString(m_mappedDrawingCategories[id].GetValue());
            }
        row["Categories"].append(category);
        }
    SendToQueue(categorySelector);
    return categorySelectorId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnElementId DgnDb0601ToJsonImpl::CreateModelSelector(ViewControllerCR vc, Utf8CP name)
    {
    auto modelSelector = Json::Value(Json::ValueType::objectValue);
    modelSelector[JSON_TYPE_KEY] = JSON_TYPE_ModelSelector;
    modelSelector[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
    modelSelector[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
    auto& row = modelSelector[JSON_OBJECT_KEY];
    row.clear();
    m_nextAvailableId = DgnElementId(m_nextAvailableId.GetValue() + 1);
    row[JSON_INSTANCE_ID] = IdToString(m_nextAvailableId.GetValue());
    row["Name"] = name;
    row["Models"] = Json::Value(Json::ValueType::arrayValue);
    for (DgnModelId id : vc.GetViewedModels())
        {
        Json::Value model(Json::ValueType::uintValue);
        model = IdToString(id.GetValue());
        row["Models"].append(model);
        }
    MakeNavigationProperty(row, "DefinitionModel", m_jobDefinitionModelId.GetValue());
    SendToQueue(modelSelector);
    return m_nextAvailableId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnElementId DgnDb0601ToJsonImpl::CreateDrawingElement(Utf8CP name)
    {
    auto drawingElement = Json::Value(Json::ValueType::objectValue);
    drawingElement[JSON_TYPE_KEY] = JSON_TYPE_Element;
    drawingElement[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
    drawingElement[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
    auto& obj = drawingElement[JSON_OBJECT_KEY];
    obj.clear();
    m_nextAvailableId = DgnElementId(m_nextAvailableId.GetValue() + 1);

    obj[JSON_INSTANCE_ID] = IdToString(m_nextAvailableId.GetValue()).c_str();
    obj[JSON_CLASSNAME] = "BisCore.Drawing";

    Utf8String codeNS = IdToString(m_documentListModelId.GetValue());
    MakeNavigationProperty(obj, BIS_ELEMENT_PROP_CodeSpec, m_authorityIds["bis:Drawing"].c_str());
    MakeNavigationProperty(obj, BIS_ELEMENT_PROP_CodeScope, codeNS.c_str());

    obj[BIS_ELEMENT_PROP_CodeValue] = name;
    obj["UserLabel"] = name;
    MakeNavigationProperty(obj, BIS_ELEMENT_PROP_Model, codeNS.c_str());
    SendToQueue(drawingElement);
    return m_nextAvailableId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnElementId DgnDb0601ToJsonImpl::CreateSheetElement(DgnModelCR model)
    {
    auto sheetElement = Json::Value(Json::ValueType::objectValue);
    sheetElement[JSON_TYPE_KEY] = JSON_TYPE_Element;
    sheetElement[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
    sheetElement[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
    auto& obj = sheetElement[JSON_OBJECT_KEY];
    obj.clear();
    m_nextAvailableId = DgnElementId(m_nextAvailableId.GetValue() + 1);

    obj[JSON_INSTANCE_ID] = IdToString(m_nextAvailableId.GetValue()).c_str();
    obj[JSON_CLASSNAME] = "BisCore.Sheet";

    Utf8String codeNS = IdToString(m_sheetListModelId.GetValue());
    MakeNavigationProperty(obj, BIS_ELEMENT_PROP_CodeSpec, m_authorityIds["bis:Sheet"].c_str());
    MakeNavigationProperty(obj, BIS_ELEMENT_PROP_CodeScope, codeNS.c_str());
    obj[BIS_ELEMENT_PROP_CodeValue] = model.GetName().c_str();
    obj["UserLabel"] = model.GetName().c_str();
    MakeNavigationProperty(obj, BIS_ELEMENT_PROP_Model, codeNS.c_str());

    obj["Height"] = model.ToSheetModel()->GetSize().y;
    obj["Width"] = model.ToSheetModel()->GetSize().x;

    SendToQueue(sheetElement);
    return m_nextAvailableId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DgnDb0601ToJsonImpl::ExportViews()
    {
    auto iter = ViewDefinition::MakeIterator(*m_dgndb);
    for (auto const& entry : iter)
        {
        ViewDefinitionCPtr view = ViewDefinition::QueryView(entry.GetId(), *m_dgndb);
        if (!view.IsValid())
            continue;

        ViewControllerPtr vc = ViewDefinition::LoadViewController(entry.GetId(), *m_dgndb);
        if (!vc.IsValid())
            continue;

        DgnElementId categorySelectorId = CreateCategorySelector(*vc, view->GetName().c_str());
        DgnElementId displayStyle = CreateDisplayStyle(vc.get(), view->GetName().c_str(), view->GetElementClass()->Is(m_viewDefinition3dClass), view->GetElementClass()->Is(m_cameraViewDefinitionClass));
        DgnElementId modelSelectorId;
        if (view->GetElementClass()->Is(m_viewDefinition3dClass))
            modelSelectorId = CreateModelSelector(*vc, view->GetName().c_str());

        Utf8PrintfString whereClause(" AND ECInstanceId=%" PRIu64, view->GetElementId().GetValue());
        auto row = Json::Value(Json::ValueType::objectValue);

        ExportElements(row, view->GetElementClass()->GetSchema().GetName().c_str(), view->GetElementClass()->GetName().c_str(), view->GetModelId(), whereClause.c_str(), false);
        auto& obj = row[JSON_OBJECT_KEY];
        MakeNavigationProperty(obj, "CategorySelector", categorySelectorId.GetValue());
        MakeNavigationProperty(obj, "DisplayStyle", displayStyle.GetValue());
        MakeNavigationProperty(obj, BIS_ELEMENT_PROP_CodeSpec, m_authorityIds["bis:ViewDefinition"].c_str());
        MakeNavigationProperty(obj, BIS_ELEMENT_PROP_Model, m_jobDefinitionModelId);

        if (vc->HasSubCategoryOverride())
            {
            auto const& settings = vc->GetSettings();
            auto const& subcatJson = settings["subCategories"];
            obj["overrides"] = subcatJson;
            }

        if (obj.isMember("Source"))
            {
            if ((DgnViewSource) (obj["Source"].asInt()) == DgnViewSource::Private)
                obj["IsPrivate"] = true;
            else
                obj["IsPrivate"] = false;
            obj.removeMember("Source");
            }

        if (view->GetElementClass()->Is(m_viewDefinition3dClass))
            {
            row[JSON_TYPE_KEY] = JSON_TYPE_ViewDefinition3d;
            obj.removeMember("BaseModelId");

            MakeNavigationProperty(obj, "ModelSelector", modelSelectorId.GetValue());
            obj["Origin"] = Json::Value(Json::ValueType::objectValue);
            obj["Origin"]["x"] = vc->GetOrigin().x;
            obj["Origin"]["y"] = vc->GetOrigin().y;
            obj["Origin"]["z"] = vc->GetOrigin().z;

            obj["Extents"] = Json::Value(Json::ValueType::objectValue);
            obj["Extents"]["x"] = vc->GetDelta().x;
            obj["Extents"]["y"] = vc->GetDelta().y;
            obj["Extents"]["z"] = vc->GetDelta().z;

            RotMatrix rot = vc->GetRotation();
            YawPitchRollAngles ypr;
            YawPitchRollAngles::TryFromRotMatrix(ypr, rot);
            obj["Yaw"] = ypr.GetYaw().Degrees();
            obj["Pitch"] = ypr.GetPitch().Degrees();
            obj["Roll"] = ypr.GetRoll().Degrees();

            if (view->GetElementClass()->Is(m_cameraViewDefinitionClass))
                {
                CameraViewControllerCP cvc = dynamic_cast<CameraViewControllerCP>(vc.get());
                obj[JSON_CLASSNAME] = "BisCore.SpatialViewDefinition";

                obj["EyePoint"] = Json::Value(Json::ValueType::objectValue);
                obj["EyePoint"]["x"] = cvc->GetEyePoint().x;
                obj["EyePoint"]["y"] = cvc->GetEyePoint().y;
                obj["EyePoint"]["z"] = cvc->GetEyePoint().z;

                obj["FocusDistance"] = cvc->GetFocusDistance();
                obj["LensAngle"] = cvc->GetLensAngle();
                obj["IsCameraOn"] = cvc->IsCameraOn();
                }
            else
                {
                obj[JSON_CLASSNAME] = "BisCore.OrthographicViewDefinition";
                }
            }
        else if (view->GetElementClass()->Is(m_viewDefinition2dClass))
            {
            row[JSON_TYPE_KEY] = JSON_TYPE_ViewDefinition2d;
            Json::Value tmp(obj["BaseModelId"]);
            MakeNavigationProperty(obj, "BaseModel", tmp);

            obj.removeMember("BaseModelId");
            obj["Origin"] = Json::Value(Json::ValueType::objectValue);
            obj["Origin"]["x"] = vc->GetOrigin().x;
            obj["Origin"]["y"] = vc->GetOrigin().y;

            obj["Extents"] = Json::Value(Json::ValueType::objectValue);
            obj["Extents"]["x"] = vc->GetDelta().x;
            obj["Extents"]["y"] = vc->GetDelta().y;

            DVec3d xColumn; 
            RotMatrix rot = vc->GetRotation();
            rot.GetColumn(xColumn, 0);
            obj["RotationAngle"] = atan2(xColumn.y, xColumn.x);

            if (view->GetElementClass()->Is(m_drawingViewDefinitionClass))
                obj[JSON_CLASSNAME] = "BisCore.DrawingViewDefinition";
            else if (view->GetElementClass()->Is(m_sheetViewDefinitionClass))
                obj[JSON_CLASSNAME] = "BisCore.SheetViewDefinition";
            }
        SendToQueue(row);
        }
        return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DgnDb0601ToJsonImpl::ExportCategories()
    {
    bvector<DgnCategoryId> duplicateIds;
    // First, need to create a definition model for each Namespace used in the Categories for those categories that are in the default DictionaryModel
    Utf8PrintfString("select DISTINCT Code.Namespace from [dgn].[Category] WHERE ModelId = %" PRIu64, m_dgndb->GetDictionaryModel().GetModelId().GetValue());
    CachedECSqlStatementPtr statement = m_dgndb->GetPreparedECSqlStatement("select DISTINCT Code.Namespace from [dgn].[Category] WHERE ModelId = 1");
    if (!statement.IsValid())
        {
        LogMessage(BimFromDgnDbLoggingSeverity::LOG_FATAL, "DgnDb0601ToJson: (Export Categories) Unable to get cached statement ptr.");
        return ERROR;
        }

    while (BE_SQLITE_ROW == statement->Step())
        {
        Utf8String ns = statement->GetValueText(0);
        if (Utf8String::IsNullOrEmpty(ns.c_str()))
            continue;
        DgnElementId modelId = CreateDefinitionModel(ns.c_str());
        m_namespaceDefinitionModels[ns] = modelId;
        }

    ExportCategories("dgn_GeometricElement3d", "BisCore.SpatialCategory", m_authorityIds["bis:SpatialCategory"].c_str(), duplicateIds);

    Utf8PrintfString sql("select DISTINCT g.CategoryId FROM dgn_GeometricElement3d g INTERSECT select DISTINCT g2.CategoryId FROM dgn_GeometricElement2d g2");
    Statement stmt;
    auto stat = stmt.Prepare(*m_dgndb, sql.c_str());
    if (DbResult::BE_SQLITE_OK != stat)
        {
        Utf8String error;
        error.Sprintf("Failed get prepared SQL statement for SELECTing duplicate categories: %s", sql.c_str());
        LogMessage(BimFromDgnDbLoggingSeverity::LOG_WARNING, error.c_str());
        return ERROR;
        }

    while (BE_SQLITE_ROW == stmt.Step())
        {
        DgnCategoryId categoryId = stmt.GetValueId<DgnCategoryId>(0);
        duplicateIds.push_back(categoryId);
        }
    ExportCategories("dgn_GeometricElement2d", "BisCore.DrawingCategory", m_authorityIds["bis:DrawingCategory"].c_str(), duplicateIds);
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DgnDb0601ToJsonImpl::ExportCategories(Utf8CP tableName, Utf8CP bisClassName, Utf8CP bisAuthorityStr, bvector<DgnCategoryId>& duplicateIds)
    {
    Utf8PrintfString sql("select DISTINCT g.CategoryId, e.ModelId,  s.NAME, c.NAME FROM ec_Schema s, ec_Class c, %s g, dgn_Element e WHERE g.CategoryId = e.Id AND e.ECClassId = c.[Id] and c.[SchemaId] = s.[Id]", tableName);
    Statement stmt;
    auto stat = stmt.Prepare(*m_dgndb, sql.c_str());
    if (DbResult::BE_SQLITE_OK != stat)
        {
        Utf8String error;
        error.Sprintf("Failed get prepared SQL statement for SELECTing categories: %s", sql.c_str());
        LogMessage(BimFromDgnDbLoggingSeverity::LOG_WARNING, error.c_str());
        return ERROR;
        }

    while (BE_SQLITE_ROW == stmt.Step())
        {
        DgnCategoryId categoryId = stmt.GetValueId<DgnCategoryId>(0);
        if (m_insertedElements.find(DgnElementId(categoryId.GetValue())) != m_insertedElements.end())
            {
            if (std::find(duplicateIds.begin(), duplicateIds.end(), categoryId) == duplicateIds.end())
                continue;
            }
        if (m_mappedDrawingCategories.find(categoryId) != m_mappedDrawingCategories.end())
            continue;

        Utf8PrintfString whereClause(" AND ECInstanceId=%" PRIu64, categoryId.GetValue());
        auto entry = Json::Value(Json::ValueType::objectValue);
        ExportElements(entry, stmt.GetValueText(2), stmt.GetValueText(3), stmt.GetValueId<DgnModelId>(1), whereClause.c_str(), false, true);
        auto& obj = entry[JSON_OBJECT_KEY];
        obj[JSON_CLASSNAME] = bisClassName;
        if (std::find(duplicateIds.begin(), duplicateIds.end(), categoryId) != duplicateIds.end())
            {
            m_nextAvailableId = DgnElementId(m_nextAvailableId.GetValue() + 1);
            obj[JSON_INSTANCE_ID] = IdToString(m_nextAvailableId.GetValue()).c_str();
            m_mappedDrawingCategories[categoryId] = DgnCategoryId(m_nextAvailableId.GetValue());
            }
        MakeNavigationProperty(obj, BIS_ELEMENT_PROP_CodeSpec, bisAuthorityStr);
        SendToQueue(entry);

        for (DgnSubCategoryId subCatId : DgnSubCategory::QuerySubCategories(*m_dgndb, categoryId))
            {
            if (m_insertedElements.end() != m_insertedElements.find(subCatId))
                continue;
            DgnSubCategoryCPtr subCat = DgnSubCategory::QuerySubCategory(subCatId, *m_dgndb);
            Utf8PrintfString subCatWhereClause(" AND ECInstanceId=%" PRIu64, subCatId.GetValue());
            auto subCatJson = Json::Value(Json::ValueType::objectValue);
            ExportElements(subCatJson, subCat->GetElementClass()->GetSchema().GetName().c_str(), subCat->GetElementClass()->GetName().c_str(), subCat->GetModelId(), subCatWhereClause.c_str(), true);
            }

        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2018
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DgnDb0601ToJsonImpl::ExportUnits()
    {
    auto firstModel = m_dgndb->Models().GetModel(m_dgndb->Models().QueryFirstModelId());
    if (firstModel != nullptr)
        {
        GeometricModelP geometricModel = firstModel->ToGeometricModelP();
        if (nullptr != geometricModel)
            {
            GeometricModel::DisplayInfo& displayInfo = geometricModel->GetDisplayInfoR();
            UnitDefinitionCR mu = displayInfo.GetMasterUnits();

            auto units = Json::Value(Json::ValueType::objectValue);
            auto system = mu.GetSystem();
            if (UnitSystem::English == system)
                units["masterUnit"] = "English";
            else if (UnitSystem::Metric == system)
                units["masterUnit"] = "Metric";
            else if (UnitSystem::USSurvey == system)
                units["masterUnit"] = "USSurvey";
            else
                units["masterUnit"] = "Undefined";
            SendToQueue(units);
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DgnDb0601ToJsonImpl::ExportSchemas()
    {
    bvector<Utf8String> knownSchemas = {"Bentley_Standard_Classes", "Bentley_Standard_CustomAttributes", "CoreCustomAttributes", "ECDbMap", "ECDbFileInfo", "ECDbSystem","ECDb_FileInfo", "ECDb_System", "EditorCustomAttributes", "Generic", "MetaSchema", "dgn", "PointCloud", "Raster", "ThreeMx", "ECv3ConversionAttributes"};
    ECSchemaCP dgnSchema = m_dgndb->Schemas().GetECSchema("dgn");
    ECSchemaP nonConstDgn = const_cast<ECSchemaP>(dgnSchema);
    ECSchemaCP genericSchema = m_dgndb->Schemas().GetECSchema("generic");
    ECClassCP defaultBase = genericSchema->GetClassCP("ElementRefersToElement");
    ECClassCP defaultAspectBase = genericSchema->GetClassCP("ElementOwnsMultiAspect");
    ECClassCP aspectClass = dgnSchema->GetClassCP("ElementMultiAspect");
    bvector<Utf8String> alreadyVisited;

    ECN::ECSchemaReadContextPtr schemaReadContext;
    schemaReadContext = ECN::ECSchemaReadContext::CreateContext();
    schemaReadContext->AddSchemaLocater(m_dgndb->GetSchemaLocater());

    bvector<ECSchemaCP> ecSchemas = m_dgndb->Schemas().GetECSchemas();

    int schemaCount = 0;
    for (ECN::ECSchemaCP ischema : ecSchemas)
        {
        if (knownSchemas.end() != std::find(knownSchemas.begin(), knownSchemas.end(), ischema->GetName()))
            continue;
        schemaCount++;
        }
    auto entryCount = Json::Value(Json::ValueType::objectValue);
    entryCount["schemaCount"] = schemaCount;
    SendToQueue(entryCount);

    auto entry = Json::Value(Json::ValueType::objectValue);
    entry[JSON_TYPE_KEY] = JSON_TYPE_Schema;
    entry[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::arrayValue);
    entry[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
    auto& schemas = entry[JSON_OBJECT_KEY];

    for (ECN::ECSchemaCP ischema : ecSchemas)
        {
        bvector<ECSchemaCP> orderedSchemas;
        if (knownSchemas.end() != std::find(knownSchemas.begin(), knownSchemas.end(), ischema->GetName()))
            continue;
        ischema->FindAllSchemasInGraph(orderedSchemas);

        for (ECN::ECSchemaCP schema : orderedSchemas)
            {
            if (knownSchemas.end() != std::find(knownSchemas.begin(), knownSchemas.end(), schema->GetName()))
                continue;
            if (alreadyVisited.end() != std::find(alreadyVisited.begin(), alreadyVisited.end(), schema->GetName()))
                continue;
            Utf8String copyXml;
            if (ECN::SchemaWriteStatus::Success != schema->WriteToXmlString(copyXml, 3, 0))
                continue;

            ECSchemaPtr copied = nullptr;
            ECN::SchemaReadStatus readStat = ECN::ECSchema::ReadFromXmlString(copied, copyXml.c_str(), *schemaReadContext);
            if (ECN::SchemaReadStatus::Success != readStat)
                continue;

            if (!ECN::ECSchema::IsSchemaReferenced(*copied, *dgnSchema))
                {
                copied->AddReferencedSchema(*nonConstDgn);
                }

            for (ECN::ECClassCP ecClass : copied->GetClasses())
                {
                ECN::ECClassP nonConstClass = const_cast<ECClassP>(ecClass);
                // Calculated ECProperties are not read-only in bis
                for (ECN::ECPropertyP prop : nonConstClass->GetProperties(false))
                    {
                    if (prop->IsCalculated())
                        prop->SetIsReadOnly(false);
                    // BisCore renames several properties on core classes.  This could cause a conflict with an inherited class's property, so we need to rename it
                    if (nonConstClass->Is(m_elementClass) && (prop->GetName().EqualsIAscii("Model") || prop->GetName().EqualsIAscii("Parent") || prop->GetName().EqualsIAscii("ID")))
                        {
                        nonConstClass->RenameConflictProperty(prop, true);
                        }
                    if (prop->GetName().EqualsIAscii("Category") && (ecClass->Is(m_geometric2dClass) || ecClass->Is(m_geometric3dClass)))
                        nonConstClass->RenameConflictProperty(prop, true);

                    prop->RemoveCustomAttribute("PropertyMap");
                    if (prop->GetCustomAttributeLocal("EditorCustomAttributes", "StandardValues").IsValid())
                        {
                        ECN::PrimitiveECPropertyP primProp = prop->GetAsPrimitivePropertyP();
                        if (nullptr != primProp && primProp->GetType() == ECN::PrimitiveType::PRIMITIVETYPE_String)
                            primProp->RemoveCustomAttribute("EditorCustomAttributes", "StandardValues");
                        else
                            {
                            ECN::ArrayECPropertyP arrayProp = prop->GetAsArrayPropertyP();
                            if (nullptr != arrayProp && arrayProp->GetIsPrimitiveArray() && arrayProp->GetPrimitiveElementType() == ECN::PrimitiveType::PRIMITIVETYPE_String)
                                arrayProp->RemoveCustomAttribute("EditorCustomAttributes", "StandardValues");
                            }
                        }
                    }

                nonConstClass->RemoveCustomAttribute("ForeignKeyRelationshipMap");
                nonConstClass->RemoveCustomAttribute("ClassMap");

                if (!ecClass->IsRelationshipClass())
                    continue;
                
                if (nonConstClass->GetBaseClasses().size() > 1)
                    {
                    if (!(*nonConstClass->GetBaseClasses().begin())->Is(defaultBase))
                        continue;
                    nonConstClass->RemoveBaseClass(*(*nonConstClass->GetBaseClasses().begin()));
                    }
                
                ECClassCP base = defaultBase;
                for (ECN::ECEntityClassP constraint : nonConstClass->GetRelationshipClassP()->GetTarget().GetClasses())
                    {
                    if (constraint->Is(aspectClass))
                        {
                        base = defaultAspectBase;
                        break;
                        }
                    }
                nonConstClass->AddBaseClass(*base);
                }
            Utf8String schemaXml;
            if (ECN::SchemaWriteStatus::Success != copied->WriteToXmlString(schemaXml, 3, 0))
                continue;
            schemaXml.ReplaceAll("dgn:", "bis:");
            schemaXml.ReplaceAll("name=\"dgn\" version=\"02.00.00\" prefix=\"dgn\"", "name=\"BisCore\" version=\"01.00\" prefix=\"bis\"");
            schemaXml.ReplaceAll("xmlns=\"dgn.02.00\"", "xmlns=\"BisCore.01.00\"");
            schemaXml.ReplaceAll("name=\"ECDbMap\" version=\"01.00.01\"", "name=\"ECDbMap\" version=\"02.00\"");
            schemaXml.ReplaceAll("generic:ElementRefersToElement", "bis:ElementRefersToElements");
            schemaXml.ReplaceAll("ECStructArrayProperty", "ECProperty");
            schemaXml.ReplaceAll("generic:PhysicalObject", "bis:PhysicalElement");
            schemaXml.ReplaceAll("generic:SpatialGroup", "bis:GroupInformationElement");
            schemaXml.ReplaceAll("generic:MultiAspect", "bis:ElementMultiAspect");
            schemaXml.ReplaceAll("PropertyStatementType", "AutoHandledProperty");
            // There is no Authority class in bis.  Nor is there a way to remove a class from the schema.  Therefore, we just make it a DefinitionElement
            schemaXml.ReplaceAll("<BaseClass>bis:Authority</BaseClass>", "<BaseClass>bis:DefinitionElement</BaseClass>");

            Json::Value schemaJson(Json::ValueType::objectValue);
            schemaJson[schema->GetFullSchemaName().c_str()] = schemaXml.c_str();
            alreadyVisited.push_back(schema->GetName());
            schemas.append(schemaJson);
            }
        }
    SendToQueue(entry);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnElementId DgnDb0601ToJsonImpl::InitListModel(Utf8CP name)
    {
    DgnElementId partitionId = CreatePartitionElement(name, "DocumentPartition", m_jobSubjectId);
    auto entry = Json::Value(Json::ValueType::objectValue);
    entry[JSON_TYPE_KEY] = JSON_TYPE_Model;
    entry[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
    entry[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
    auto& obj = entry[JSON_OBJECT_KEY];
    obj.clear();
    obj[JSON_CLASSNAME] = "BisCore.DocumentListModel";

    MakeNavigationProperty(obj, BIS_MODEL_PROP_ModeledElement, partitionId.GetValue());

    m_nextAvailableId = DgnElementId(m_nextAvailableId.GetValue() + 1);
    obj[JSON_INSTANCE_ID] = IdToString(partitionId.GetValue()).c_str();
    m_insertedModels[DgnModelId(partitionId.GetValue())] = partitionId;
    SendToQueue(entry);
    return partitionId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DgnDb0601ToJsonImpl::InitDrawingListModel()
    {
    m_documentListModelId = InitListModel("Converted Drawings");
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DgnDb0601ToJsonImpl::InitSheetListModel()
    {
    m_sheetListModelId = InitListModel("Converted Sheets");
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            04/2018
//---------------+---------------+---------------+---------------+---------------+-------
DgnElementId DgnDb0601ToJsonImpl::CreateDefinitionModel(Utf8CP modelName)
    {
    DgnElementId definitionModelId = CreatePartitionElement(modelName, "DocumentPartition", m_jobSubjectId);
    auto entry = Json::Value(Json::ValueType::objectValue);
    entry[JSON_TYPE_KEY] = JSON_TYPE_Model;
    entry[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
    entry[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
    auto& obj = entry[JSON_OBJECT_KEY];
    obj.clear();
    obj[JSON_CLASSNAME] = "BisCore.DefinitionModel";

    MakeNavigationProperty(obj, BIS_MODEL_PROP_ModeledElement, definitionModelId.GetValue());
    m_insertedModels[DgnModelId(definitionModelId.GetValue())] = definitionModelId;

    m_nextAvailableId = DgnElementId(m_nextAvailableId.GetValue() + 1);
    obj[JSON_INSTANCE_ID] = IdToString(definitionModelId.GetValue()).c_str();
    SendToQueue(entry);
    return definitionModelId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2018
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DgnDb0601ToJsonImpl::InitJobDefinitionModel()
    {
    m_jobDefinitionModelId = CreateDefinitionModel("Job Definition Model");
    if (m_jobDefinitionModelId.IsValid())
        return SUCCESS;
    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            08/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DgnDb0601ToJsonImpl::ExportModels()
    {
    Utf8PrintfString subjectName("DgnDb0601: %s", Utf8String(m_dgndb->GetFileName().GetFileNameWithoutExtension().c_str()).c_str());
    m_jobSubjectId = CreateSubjectElement(subjectName.c_str());
    InitDrawingListModel();
    InitSheetListModel();
    InitJobDefinitionModel();

    Statement stmt;
    Utf8PrintfString sql("SELECT s.NAME, c.NAME from ec_Schema s, ec_Class c WHERE s.ID = c.[SchemaId] and c.Id in (SELECT DISTINCT ECClassId FROM %s_%s)", DGN_ECSCHEMA_NAME, DGN_CLASSNAME_Model);
    if (BE_SQLITE_OK != (stmt.Prepare(*m_dgndb, sql.c_str())))
        {
        LogMessage(BimFromDgnDbLoggingSeverity::LOG_ERROR, "Unable to prepare statement to retrieve DgnModels");
        return ERROR;
        }

    BentleyStatus stat = SUCCESS;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        if (SUCCESS != (stat = ExportModel(stmt.GetValueText(0), stmt.GetValueText(1))))
            break;
        }
    return stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DgnDb0601ToJsonImpl::ExportModel(Utf8CP schemaName, Utf8CP className, Utf8CP whereClause)
    {
    Utf8PrintfString ecSql("SELECT ECInstanceId, * FROM ONLY [%s].[%s]", schemaName, className);
    if (nullptr != whereClause)
        ecSql.append(whereClause);

    CachedECSqlStatementPtr statement = m_dgndb->GetPreparedECSqlStatement(ecSql.c_str());
    if (!statement.IsValid())
        {
        Utf8PrintfString error("DgnDb0601ToJson: (Export Model) Unable to get cached statement ptr for \"%s\".", ecSql.c_str());
        LogMessage(BimFromDgnDbLoggingSeverity::LOG_FATAL, error.c_str());
        return ERROR;
        }
    JsonECSqlSelectAdapter jsonAdapter(*statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));

    while (BE_SQLITE_ROW == statement->Step())
        {
        // First need to create a SubjectElement that will be used for the ModelModelsElement relationship
        ECInstanceId actualElementId = statement->GetValueId<ECInstanceId>(0);
        if (m_insertedModels.find(DgnModelId(actualElementId.GetValue())) != m_insertedModels.end())
            continue;

        DgnModelPtr model = m_dgndb->Models().GetModel(DgnModelId(actualElementId.GetValue()));
        bool isPointCloud = nullptr != m_pointCloudModelClass && m_dgndb->Schemas().GetECClass(model->GetClassId())->Is(m_pointCloudModelClass);
        bool isThreeMx = nullptr != m_threeMxModelClass && m_dgndb->Schemas().GetECClass(model->GetClassId())->Is(m_threeMxModelClass);
        bool isRaster = nullptr != m_rasterFileModelClass && m_dgndb->Schemas().GetECClass(model->GetClassId())->Is(m_rasterFileModelClass);

        DgnElementId modeledElementId;
        if (model->IsSheetModel())
            modeledElementId = CreateSheetElement(*model);
        else if (model->Is2dModel())
            modeledElementId = CreateDrawingElement(model->GetName().c_str());
        else if (!model->IsDictionaryModel() && !isPointCloud && !isThreeMx && !isRaster)
            modeledElementId = CreatePartitionElement(*model, DgnElementId());
        
        auto entry = Json::Value(Json::ValueType::objectValue);
        if (model->IsDictionaryModel())
            entry[JSON_TYPE_KEY] = JSON_TYPE_DictionaryModel;
        else
            entry[JSON_TYPE_KEY] = JSON_TYPE_Model;

        entry[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
        entry[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
        auto& obj = entry[JSON_OBJECT_KEY];
        obj.clear();
        jsonAdapter.GetRowInstance(obj);

        if (nullptr != dynamic_cast<WebMercator::WebMercatorModel*>(model.get()))
            continue;

        // Certain classes are now abstract in BisCore, so they need to be converted to the derived concrete classes
        if (model->IsGroupInformationModel())
            obj[JSON_CLASSNAME] = "Generic.GroupModel";
        else if (isPointCloud)
            {
            entry[JSON_TYPE_KEY] = JSON_TYPE_PointCloudModel;
            obj["PointCloudModel"] = Json::Value(Json::ValueType::objectValue);
            auto& pc = obj["PointCloudModel"];
            Json::Value pcOld;
            Json::Reader::Parse(obj["Properties"].asCString(), pcOld);
            pc["Color"] = pcOld["PointCloudModel"]["Color"].asUInt();
            pc["Density"] = pcOld["PointCloudModel"]["Density"].asFloat();

            Json::Value fileId;
            Json::Reader::Parse(pcOld["PointCloudModel"]["FileId"].asCString(), fileId);

            pc["FileUri"] = fileId["localFile"]["fullPath"].asCString();
            if (pcOld["PointCloudModel"].isMember("Description"))
                pc["Description"] = pcOld["PointCloudModel"]["Description"].asCString();
            if (pcOld["PointCloudModel"].isMember("Wkt"))
                pc["Wkt"] = pcOld["PointCloudModel"]["Wkt"].asCString();
            Transform stw;
            JsonUtils::TransformFromJson(stw, pcOld["PointCloudModel"]["SceneToWorld"]);
            JsonUtils::TransformToJson(pc["SceneToWorld"], stw);
            }
        else if (isThreeMx)
            {
            entry[JSON_TYPE_KEY] = JSON_TYPE_ThreeMxModel;
            Json::Value properties;
            Json::Reader::Parse(obj["Properties"].asCString(), properties);
            obj["SceneFile"] = properties["SceneFile"].asCString();
            obj["SceneName"] = model->GetName();

            if (properties.isMember("Location"))
                {
                Transform location;
                JsonUtils::TransformFromJson(location, properties["Location"]);
                JsonUtils::TransformToJson(obj["Location"], location);
                }
            }
        else if (isRaster)
            {
            entry[JSON_TYPE_KEY] = JSON_TYPE_RasterFileModel;
            Json::Value properties;
            Json::Reader::Parse(obj["Properties"].asCString(), properties);

            Json::Value fileId;
            Json::Reader::Parse(properties["fileId"].asCString(), fileId);
            obj["FileUri"] = fileId["localFile"]["fullPath"].asCString();

            Transform transform;
            JsonUtils::TransformFromJson(transform, properties["transform"]);
            JsonUtils::TransformToJson(obj["transform"], transform);

            DRange2d range;
            JsonUtils::DPoint2dFromJson(range.low, properties["bbox"]["low"]);
            JsonUtils::DPoint2dFromJson(range.high, properties["bbox"]["high"]);
            JsonUtils::DPoint2dToJson(obj["bbox"]["low"], range.low);
            JsonUtils::DPoint2dToJson(obj["bbox"]["high"], range.high);
            obj[JSON_CLASSNAME] = "Raster.RasterFileModel";
            }
        else if (model->IsSpatialModel())
            {
            Utf8String tmp(obj["$ECClassKey"].asString());
            tmp.ReplaceAll("dgn.SpatialModel", "BisCore.PhysicalModel");
            obj[JSON_CLASSNAME] = tmp.c_str();
            }
        else if (model->IsSheetModel())
            {
            Utf8String tmp(obj["$ECClassKey"].asString());
            tmp.ReplaceAll("dgn.SheetModel", "BisCore.SheetModel");
            obj[JSON_CLASSNAME] = tmp.c_str();
            obj.removeMember("SheetSize");
            }
        else if (nullptr != model->ToGeometricModel2d())
            { 
            Utf8String tmp(obj["$ECClassKey"].asString());
            tmp.ReplaceAll("dgn.GeometricModel2d", "BisCore.DrawingModel");
            obj[JSON_CLASSNAME] = tmp.c_str();
            }
        else
            {
            Utf8String tmp(obj["$ECClassKey"].asString());
            tmp.ReplaceAll("dgn.", "BisCore.");
            obj[JSON_CLASSNAME] = tmp.c_str();
            }

        obj.removeMember("Code");
        obj.removeMember("Label");
        obj.removeMember("DependencyIndex");
        obj.removeMember("$ECClassKey");
        obj.removeMember("$ECInstanceLabel");
        obj.removeMember("$ECClassId");
        obj.removeMember("$ECClassLabel");

        obj[JSON_INSTANCE_ID] = IdToString(obj["$ECInstanceId"].asString().c_str()).c_str();

        obj.removeMember("$ECInstanceId");

        if (obj.isMember("Visibility"))
            {
            obj["IsPrivate"] = obj["Visibility"].asInt() == 0;
            obj.removeMember("Visibility");
            }

        if (obj.isMember("Properties"))
            {
            if (!obj["Properties"].isNull() && !Utf8String::IsNullOrEmpty(obj["Properties"].asString().c_str()))
                {
                Utf8String properties = obj["Properties"].asString();
                if (properties.Contains("DisplayInfo"))
                    {
                    Utf8String tmp = obj["Properties"].asString();
                    tmp.ReplaceAll("DisplayInfo", "formatter");
                    obj["JsonProperties"] = tmp.c_str();
                    }
                else
                    obj["JsonProperties"] = properties.c_str();
                }
            obj.removeMember("Properties");
            }

        if (modeledElementId.IsValid())
            {
            MakeNavigationProperty(obj, BIS_MODEL_PROP_ModeledElement, modeledElementId.GetValue());
            m_insertedModels[model->GetModelId()] = modeledElementId;
            }
        else
            {
            m_insertedModels[model->GetModelId()] = DgnElementId(m_dgndb->GetDictionaryModel().GetModelId().GetValue());
            }

        SendToQueue(entry);

        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DgnDb0601ToJsonImpl::ExportElements()
    {
    BentleyStatus stat = SUCCESS;
    DgnModels::Iterator modelsIterator = m_dgndb->Models().MakeIterator();
    for (DgnModels::Iterator::Entry modelEntry : modelsIterator)
        {
        DgnModelPtr model = m_dgndb->Models().GetModel(modelEntry.GetModelId());
        if (SUCCESS != (stat = ExportElements(model->GetModelId())))
            break;
        }
    return stat;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DgnDb0601ToJsonImpl::ExportElements(DgnModelId parentModel)
    {
    Statement stmt;
    Utf8CP sql = "SELECT s.NAME, c.NAME from ec_Schema s, ec_Class c WHERE s.ID = c.[SchemaId] and c.Id in (SELECT DISTINCT ECClassId FROM dgn_Element WHERE ModelId=?)";
    if (BE_SQLITE_OK != (stmt.Prepare(*m_dgndb, sql)))
        {
        LogMessage(BimFromDgnDbLoggingSeverity::LOG_ERROR, "Unable to prepare statement to retrieve element classes for model");
        return ERROR;
        }
    stmt.BindId(1, parentModel);

    BentleyStatus stat = SUCCESS;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        auto element = Json::Value(Json::ValueType::objectValue);
        if (SUCCESS != (stat = ExportElements(element, stmt.GetValueText(0), stmt.GetValueText(1), parentModel)))
            break;
        }
    return stat;
    }

void RenameConflictMembers(Json::Value& obj, Utf8CP prefix, Utf8CP member)
    {
    for (auto const& id : obj.getMemberNames())
        {
        if (0 == BeStringUtilities::Stricmp(id.c_str(), member))
            {
            Utf8PrintfString tmp("%s_%s_", prefix, member);
            obj[tmp.c_str()] = obj[id];
            obj.removeMember(id);
            return;
            }
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DgnDb0601ToJsonImpl::ExportElements(Json::Value& entry, Utf8CP schemaName, Utf8CP className, DgnModelId parentModel, Utf8CP whereClause, bool sendToQueue, bool allowDuplicates)
    {
    Utf8PrintfString ecSql("SELECT ECInstanceId, * FROM ONLY [%s].[%s] WHERE ModelId=?", schemaName, className);
    if (nullptr != whereClause)
        ecSql.append(whereClause);
    CachedECSqlStatementPtr statement = m_dgndb->GetPreparedECSqlStatement(ecSql.c_str());
    if (!statement.IsValid())
        {
        Utf8PrintfString error("DgnDb0601ToJson: (Export Elements) Unable to get cached statement ptr for \"%s\".", ecSql.c_str());
        LogMessage(BimFromDgnDbLoggingSeverity::LOG_FATAL, error.c_str());
        return ERROR;
        }
    statement->BindId(1, parentModel);
    JsonECSqlSelectAdapter jsonAdapter(*statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));
    jsonAdapter.SetStructArrayAsString(true);
    jsonAdapter.SetPreferNativeDgnTypes(true);
    Utf8String prefix = m_dgndb->Schemas().GetECSchema(schemaName)->GetNamespacePrefix();

    ECClassCP ecClass = m_dgndb->Schemas().GetECClass(schemaName, className);
    bvector<Utf8CP> navPropNames;
    for (ECPropertyCP prop : ecClass->GetProperties(true))
        {
        if (prop->GetClass().GetSchema().GetName().Equals("dgn"))
            continue;

        if (prop->GetIsNavigation())
            navPropNames.push_back(prop->GetName().c_str());
        }

    while (BE_SQLITE_ROW == statement->Step())
        {
        ECInstanceId actualElementId = statement->GetValueId<ECInstanceId>(0);
        if (!allowDuplicates && m_insertedElements.end() != m_insertedElements.find(DgnElementId(actualElementId.GetValue())))
            continue;

        DgnElementCPtr element = m_dgndb->Elements().GetElement(DgnElementId(actualElementId.GetValue()));
        if (m_insertedModels.find(element->GetModelId()) == m_insertedModels.end())
            {
            ECClassCP modelClass = m_dgndb->Schemas().GetECClass(element->GetModel()->GetClassId());
            Utf8PrintfString whereClause(" WHERE ECInstanceId=%" PRIu64, element->GetModelId().GetValue());
            if (SUCCESS != ExportModel(modelClass->GetSchema().GetName().c_str(), modelClass->GetName().c_str(), whereClause.c_str()))
                return ERROR;
            }

        if (element->GetParentId().IsValid() && m_insertedElements.end() == m_insertedElements.find(element->GetParentId()))
            {
            DgnElementCPtr parent = m_dgndb->Elements().GetElement(element->GetParentId());
            Utf8PrintfString whereClause(" AND ECInstanceId=%" PRIu64, parent->GetElementId().GetValue());
            auto parentObj = Json::Value(Json::ValueType::objectValue);
            if (SUCCESS != ExportElements(parentObj, parent->GetElementClass()->GetSchema().GetName().c_str(), parent->GetElementClass()->GetName().c_str(), parent->GetModelId(), whereClause.c_str()))
                return ERROR;
            }

        entry[JSON_TYPE_KEY] = JSON_TYPE_Element;
        entry[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
        entry[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
        Json::Value obj = Json::Value(Json::ValueType::objectValue);
        obj.clear();
        jsonAdapter.GetRowInstance(obj);
        RenameConflictMembers(obj, prefix.c_str(), "ID");

        obj[JSON_INSTANCE_ID] = IdToString(obj["$ECInstanceId"].asString().c_str()).c_str();
        obj.removeMember("$ECInstanceId");

        for (Utf8CP navName : navPropNames)
            {
            if (obj.isMember(navName) && !obj[navName].isNull())
                {
                MakeNavigationProperty(obj, navName, IdToString(obj[navName].asCString()).c_str());
                obj.removeMember(navName);
                }
            }

        MakeNavigationProperty(obj, BIS_ELEMENT_PROP_CodeSpec, IdToString(obj["Code"]["AuthorityId"].asString().c_str()).c_str());

        // In general, scope things to the model it lives in
        MakeNavigationProperty(obj, BIS_ELEMENT_PROP_CodeScope, m_insertedModels[element->GetModelId()].GetValue());

        obj[BIS_ELEMENT_PROP_CodeValue] = obj["Code"]["Value"];
        obj.removeMember("LastMod");
        obj["UserLabel"] = obj["Label"];
        obj.removeMember("Label");
        obj["JsonProperties"] = Utf8String(obj["UserProperties"].asString().c_str()).c_str();
        obj.removeMember("UserProperties");

        RenameConflictMembers(obj, prefix.c_str(), "Model");
        RenameConflictMembers(obj, prefix.c_str(), "Description");
        RenameConflictMembers(obj, prefix.c_str(), "Parent");
        RenameConflictMembers(obj, prefix.c_str(), "Category");

        if (obj.isMember("ParentId"))
            {
            if (!obj["ParentId"].isNull())
                MakeNavigationProperty(obj, BIS_ELEMENT_PROP_Parent, IdToString(obj["ParentId"].asString().c_str()).c_str());
            obj.removeMember("ParentId");
            }

        if (obj.isMember("ModelId"))
            {
            if (!obj["ModelId"].isNull())
                MakeNavigationProperty(obj, BIS_ELEMENT_PROP_Model, IdToString(obj["ModelId"].asString().c_str()).c_str());

            obj.removeMember("ModelId");
            }

        uint64_t oldCatId = -1;
        if (obj.isMember("CategoryId"))
            {
            if (!obj["CategoryId"].isNull())
                {
                MakeNavigationProperty(obj, "Category", IdToString(obj["CategoryId"].asString().c_str()).c_str());
                BE_STRING_UTILITIES_UTF8_SSCANF(obj["CategoryId"].asCString(), "%" PRId64, &oldCatId);
                }
            obj.removeMember("CategoryId");
            }

        if (obj.isMember("Descr"))
            {
            obj["Description"] = obj["Descr"].asString();
            obj.removeMember("Descr");
            }
        if (element->GetElementClass()->Is(m_subCategoryClass))
            {
            entry[JSON_TYPE_KEY] = JSON_TYPE_SubCategory;
            DgnSubCategoryCPtr sub = m_dgndb->Elements().Get<DgnSubCategory>(element->GetElementId());
            if (sub->IsDefaultSubCategory())
                entry["IsDefaultSubCategory"] = true;
            else
                entry["IsDefaultSubCategory"] = false;
            MakeNavigationProperty(obj, BIS_ELEMENT_PROP_CodeScope, element->GetParentId());
            }
        else if (element->GetElementClass()->Is(m_categoryClass))
            {
            entry[JSON_TYPE_KEY] = JSON_TYPE_Category;
            // This gets renamed to the correct Spatial/DrawingCategory classname in the actual ExportCategories method
            obj[JSON_CLASSNAME] = "BisCore.SpatialCategory";
            obj[BIS_ELEMENT_PROP_CodeSpec]["id"] = m_authorityIds["bis:SpatialCategory"].c_str();
            obj.removeMember("Scope");
            Utf8String ns = obj["Code"]["Namespace"].asString();
            if (Utf8String::IsNullOrEmpty(ns.c_str()))
                {
                MakeNavigationProperty(obj, BIS_ELEMENT_PROP_CodeScope, m_jobDefinitionModelId);
                MakeNavigationProperty(obj, BIS_ELEMENT_PROP_Model, m_jobDefinitionModelId);
                }
            else
                {
                MakeNavigationProperty(obj, BIS_ELEMENT_PROP_CodeScope, m_namespaceDefinitionModels[ns]);
                MakeNavigationProperty(obj, BIS_ELEMENT_PROP_Model, m_namespaceDefinitionModels[ns]);
                }
            }
        else if (element->IsGeometricElement())
            {
            if (element->Is2d())
                {
                entry[JSON_TYPE_KEY] = JSON_TYPE_GeometricElement2d;
                DgnCategoryId cat(oldCatId);
                if (m_mappedDrawingCategories.find(cat) != m_mappedDrawingCategories.end())
                    {
                    MakeNavigationProperty(obj, "Category", m_mappedDrawingCategories[cat].GetValue());
                    }
                }
            else
                entry[JSON_TYPE_KEY] = JSON_TYPE_GeometricElement3d;
            // The GeomStream retrieved directly from the database is apparently incomplete.  Need to actually get it off the element
            GeometryStreamCR geom = element->ToGeometrySource()->GetGeometryStream();
            if (geom.HasData())
                {
                Utf8String encode;
                Base64Utilities::Encode(encode, geom.GetData(), geom.GetSize());
                obj["GeometryStream"] = encode.c_str();
                }
            }
        else if (nullptr != element->ToGeometryPart())
            {
            entry[JSON_TYPE_KEY] = JSON_TYPE_GeometryPart;
            GeometryStreamCR geom = element->ToGeometryPart()->GetGeometryStream();
            Utf8String encode;
            Base64Utilities::Encode(encode, geom.GetData(), geom.GetSize());
            obj["GeometryStream"] = encode.c_str();
            MakeNavigationProperty(obj, BIS_ELEMENT_PROP_CodeScope, m_jobDefinitionModelId);
            MakeNavigationProperty(obj, BIS_ELEMENT_PROP_Model, m_jobDefinitionModelId);
            }
        else if (element->IsGroupInformationElement())
            {
            Utf8String tmp = obj["$ECClassKey"].asString();
            tmp.ReplaceAll("Generic.SpatialGroup", "Generic.Group");
            tmp.ReplaceAll("Generic.GraphicGroup2d", "Generic.Group");
            obj[JSON_CLASSNAME] = tmp.c_str();
            }
        else if (element->GetElementClass()->Is(m_textureClass))
            {
            DgnTextureCPtr texture = m_dgndb->Elements().Get<DgnTexture>(element->GetElementId());
            Render::ImageSourceCR data = texture->GetImageSource();
            ByteStream const& stream = data.GetByteStream();
            Utf8String encode;
            Base64Utilities::Encode(encode, stream.GetData(), stream.GetSize());
            obj["Data"] = encode.c_str();
            entry[JSON_TYPE_KEY] = JSON_TYPE_Texture;
            obj["Height"] = texture->GetHeight();
            obj["Width"] = texture->GetWidth();
            obj["Format"] = (uint32_t) data.GetFormat();
            }
        else if (element->GetElementClass()->Is(m_planningElementClass))
            {
            if (obj.isMember("PlanId"))
                {
                MakeNavigationProperty(obj, "Plan", IdToString(obj["PlanId"].asCString()).c_str());
                obj.removeMember("PlanId");
                if (obj["Plan"]["id"] == obj["id"])
                    obj.removeMember("Plan");
                }
            if (element->GetElementClass()->Is(m_workbreakDownClass))
                entry[JSON_TYPE_KEY] = JSON_TYPE_WorkBreakdown;
            else if (element->GetElementClass()->Is(m_activityClass))
                entry[JSON_TYPE_KEY] = JSON_TYPE_Activity;
            }
        else if (element->GetElementClass()->Is(m_timeSpanClass))
            {
            if (obj.isMember("TimelineId"))
                {
                MakeNavigationProperty(obj, "Baseline", IdToString(obj["TimelineId"].asCString()).c_str());
                obj.removeMember("Baseline");
                }
            }
        else if (element->GetElementClass()->Is(m_cameraKeyFrameClass))
            {
            if (obj.isMember("AnimationId"))
                obj.removeMember("AnimationId");
            }

        if (element->GetCodeAuthority()->GetName().Equals("DgnResources"))
            {
            Utf8String authority = RemapResourceAuthority(element->GetElementClass());
            if (!Utf8String::IsNullOrEmpty(authority.c_str()))
                obj[BIS_ELEMENT_PROP_CodeSpec]["id"] = authority.c_str();
            }
        else if (element->GetCodeAuthority()->GetName().Equals("DgnV8"))
            {
            Utf8PrintfString value("%s-%s", obj["Code"]["Namespace"].asCString(), obj["Code"]["Value"].asCString());
            obj[BIS_ELEMENT_PROP_CodeValue] = value.c_str();
            }
        else if (element->GetCodeAuthority()->GetName().Equals("ConstructionPlanning_WorkAreaHierarchy"))
            {
            if (element->GetParentId().IsValid())
                MakeNavigationProperty(obj, BIS_ELEMENT_PROP_CodeScope, element->GetParentId());
            }
        else if (element->GetCodeAuthority()->GetName().Equals("ConstructionPlanning_Discipline"))
            {
            if (obj["Code"].isMember("Namespace") && !obj["Code"]["Namespace"].isNull() && !Utf8String::IsNullOrEmpty(obj["Code"]["Namespace"].asCString()))
                {
                bvector<Utf8String> ns;
                BeStringUtilities::Split(obj["Code"]["Namespace"].asCString(), "/", ns);
                Utf8String discipline = ns[0];
                if (m_disciplineIds.find(discipline) != m_disciplineIds.end())
                    MakeNavigationProperty(obj, BIS_ELEMENT_PROP_CodeScope, m_disciplineIds[discipline]);
                }
            else
                m_disciplineIds[obj["Code"]["Value"].asString()] = element->GetElementId();
            }

        if (obj.isMember("Data"))
            {
            if (element->GetElementClass()->GetName().EqualsI("MaterialElement"))
                {
                Utf8PrintfString assets("{\"materialAssets\":%s}", obj["Data"].asString().c_str());
                assets.ReplaceAll("RenderMaterial", "renderMaterial");
                obj["JsonProperties"] = assets.c_str();
                obj.removeMember("Data");

                // This is a bit of a hack.  But we sometimes end up with duplicate MaterialElements that differ by CodeNamespace.
                if (!Utf8String::IsNullOrEmpty(element->GetCode().GetNamespace().c_str()))
                    {
                    Utf8PrintfString value("%s-%s", obj["Code"]["Namespace"].asCString(), obj["Code"]["Value"].asCString());
                    obj[BIS_ELEMENT_PROP_CodeValue] = value.c_str();
                    }
                }
            else if (element->GetElementClass()->Is(m_annotationTextStyle))
                {
                entry[JSON_TYPE_KEY] = JSON_TYPE_AnnotationTextStyle;
                HandleAnnotationTextStyle(obj, element->GetElementId());
                obj.removeMember("Data");
                }
            }
        obj.removeMember("Code");
        if (element->GetElementClass()->GetName().EqualsI("MaterialElement"))
            obj[JSON_CLASSNAME] = "BisCore.RenderMaterial";

        if (obj.isMember("$ECClassKey") && !obj.isMember(JSON_CLASSNAME))
            {
            Utf8String tmp = obj["$ECClassKey"].asString();
            obj[JSON_CLASSNAME] = tmp.c_str();
            }

        if (obj.isMember("MODEL"))
            obj.removeMember("MODEL");
        obj.removeMember("$ECClassKey");
        obj.removeMember("$ECClassId");
        obj.removeMember("$ECClassLabel");
        obj.removeMember("$ECInstanceLabel");
        entry[JSON_OBJECT_KEY] = obj;
        m_insertedElements[element->GetElementId()] = 1;
        if (sendToQueue)
            SendToQueue(entry, true);
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2018
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DgnDb0601ToJsonImpl::ExportTimelines()
    {
    CachedECSqlStatementPtr statement = m_dgndb->GetPreparedECSqlStatement("select Label, PlanId from only [bp].[Timeline];");
    if (!statement.IsValid())
        {
        LogMessage(BimFromDgnDbLoggingSeverity::LOG_FATAL, "DgnDb0601ToJson: (Export Timelines) Unable to get cached statement ptr.");
        return ERROR;
        }

    JsonECSqlSelectAdapter jsonAdapter(*statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));
    jsonAdapter.SetStructArrayAsString(true);

    while (BE_SQLITE_ROW == statement->Step())
        {
        DgnElementId planId = statement->GetValueId<DgnElementId>(1);
        if (m_insertedElements.end() == m_insertedElements.find(planId))
            {
            DgnElementCPtr planEl = m_dgndb->Elements().GetElement(planId);
            Utf8PrintfString whereClause(" AND ECInstanceId=%" PRIu64, planId.GetValue());
            auto elem = Json::Value(Json::ValueType::objectValue);
            if (SUCCESS != ExportElements(elem, planEl->GetElementClass()->GetSchema().GetName().c_str(), planEl->GetElementClass()->GetName().c_str(), planEl->GetModelId(), whereClause.c_str()))
                return ERROR;
            }

        auto baseline = Json::Value(Json::ValueType::objectValue);
        baseline[JSON_TYPE_KEY] = JSON_TYPE_Baseline;
        baseline[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
        baseline[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
        auto& obj = baseline[JSON_OBJECT_KEY];
        obj.clear();
        obj["Label"] = statement->GetValueText(0);
        MakeNavigationProperty(obj, "Plan", planId);
        SendToQueue(baseline);
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            01/2019
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DgnDb0601ToJsonImpl::ExportExtraTables(Utf8CP alias, Utf8CP className)
    {
    Utf8PrintfString tableName("%s_%s", alias, className);
    if (!m_dgndb->TableExists(tableName.c_str()))
        return SUCCESS;

    Utf8PrintfString ecSql("SELECT ECInstanceId, * FROM ONLY [%s].[%s]", alias, className);
    CachedECSqlStatementPtr statement = m_dgndb->GetPreparedECSqlStatement(ecSql.c_str());
    if (!statement.IsValid())
        {
        Utf8PrintfString error("DgnDb0601ToJson: (Export Extra Tables) Unable to get cached statement ptr for \"%s\".", ecSql.c_str());
        LogMessage(BimFromDgnDbLoggingSeverity::LOG_FATAL, error.c_str());
        return ERROR;
        }

    JsonECSqlSelectAdapter jsonAdapter(*statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));
    jsonAdapter.SetStructArrayAsString(true);
    jsonAdapter.SetPreferNativeDgnTypes(true);

    while (BE_SQLITE_ROW == statement->Step())
        {
        auto entry = Json::Value(Json::ValueType::objectValue);
        entry[JSON_TYPE_KEY] = JSON_TYPE_Element;
        entry[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
        entry[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
        Json::Value obj = Json::Value(Json::ValueType::objectValue);
        obj.clear();
        jsonAdapter.GetRowInstance(obj);

        obj[JSON_INSTANCE_ID] = IdToString(obj["$ECInstanceId"].asString().c_str()).c_str();
        obj.removeMember("$ECInstanceId");

        if (obj.isMember("$ECClassKey") && !obj.isMember(JSON_CLASSNAME))
            {
            Utf8String tmp = obj["$ECClassKey"].asString();
            obj[JSON_CLASSNAME] = tmp.c_str();
            }

        obj.removeMember("$ECClassKey");
        obj.removeMember("$ECClassId");
        obj.removeMember("$ECClassLabel");
        obj.removeMember("$ECInstanceLabel");

        MakeNavigationProperty(obj, BIS_ELEMENT_PROP_Model, m_jobDefinitionModelId);
        MakeNavigationProperty(obj, BIS_ELEMENT_PROP_CodeSpec, "1");
        MakeNavigationProperty(obj, BIS_ELEMENT_PROP_CodeScope, m_jobDefinitionModelId);

        entry[JSON_OBJECT_KEY] = obj;
        SendToQueue(entry);

        }
    return SUCCESS;

    }
//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2018
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DgnDb0601ToJsonImpl::ExportElementAspects()
    {
    Statement stmt;
    Utf8PrintfString sql("SELECT DISTINCT ECClassId from _dgn_ElementAspect");
    if (BE_SQLITE_OK != (stmt.Prepare(*m_dgndb, sql.c_str())))
        {
        LogMessage(BimFromDgnDbLoggingSeverity::LOG_ERROR, "Unable to prepare statement to retrieve element aspect classes");
        return ERROR;
        }

    while (BE_SQLITE_ROW == stmt.Step())
        {
        ECClassId derived = stmt.GetValueId<ECClassId>(0);
        ExportElementAspects(derived, ECInstanceId());
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2018
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DgnDb0601ToJsonImpl::ExportElementAspects(ECClassId classId, ECInstanceId aspectId)
    {
    ECClassCP ecClass = m_dgndb->Schemas().GetECClass(classId);

    if (ecClass->GetName().Equals("ElementExternalKey"))
        return SUCCESS;
    //if (ecClass->GetName().Equals("Timeline"))
    //    return SUCCESS;

    Utf8PrintfString ecSql("SELECT * FROM ONLY [%s].[%s]", ecClass->GetSchema().GetName().c_str(), ecClass->GetName().c_str());
    if (aspectId.IsValid())
        {
        Utf8PrintfString append(" WHERE ECInstanceId=%" PRIu64, aspectId.GetValue());
        ecSql.append(append.c_str());
        }
    CachedECSqlStatementPtr statement = m_dgndb->GetPreparedECSqlStatement(ecSql.c_str());
    if (!statement.IsValid())
        {
        Utf8PrintfString error("DgnDb0601ToJson: (Export Element Aspects) Unable to get cached statement ptr for \"%s\".", ecSql.c_str());
        LogMessage(BimFromDgnDbLoggingSeverity::LOG_FATAL, error.c_str());
        return ERROR;
        }
    JsonECSqlSelectAdapter jsonAdapter(*statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));
    jsonAdapter.SetStructArrayAsString(true);
    jsonAdapter.SetPreferNativeDgnTypes(true);
    Utf8CP typeKey = ecClass->Is(m_elementMultiAspectClass) ? JSON_TYPE_ElementMultiAspect : JSON_TYPE_ElementUniqueAspect;

    while (BE_SQLITE_ROW == statement->Step())
        {
        auto entry = Json::Value(Json::ValueType::objectValue);
        entry[JSON_TYPE_KEY] = typeKey;

        entry[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
        entry[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
        Json::Value obj = Json::Value(Json::ValueType::objectValue);
        obj.clear();
        jsonAdapter.GetRowInstance(obj);

        obj[JSON_INSTANCE_ID] = IdToString(obj["$ECInstanceId"].asString().c_str()).c_str();
        if (obj.isMember("$ECClassKey") && !obj.isMember(JSON_CLASSNAME))
            {
            Utf8String tmp = obj["$ECClassKey"].asString();
            obj[JSON_CLASSNAME] = tmp.c_str();
            }

        if (obj.isMember("ElementId"))
            {
            MakeNavigationProperty(obj, "Element", IdToString(obj["ElementId"].asCString()).c_str());
            obj.removeMember("ElementId");
            }
            // ElementUniqueAspect uses the same id for the aspect id and the element id
        else
            {
            MakeNavigationProperty(obj, "Element", IdToString(obj["$ECInstanceId"].asCString()).c_str());
            }


        if (obj[JSON_CLASSNAME].asString().Equals("Planning.TimeSpan"))
            {
            MakeNavigationProperty(obj, "Baseline", IdToString(obj["TimelineId"].asCString()).c_str());
            obj.removeMember("TimelineId");
            MakeNavigationProperty(obj, "Element", IdToString(obj["ParentId"].asCString()).c_str());
            uint64_t elementId;
            BeStringUtilities::ParseUInt64(elementId, obj["ParentId"].asCString());

            obj.removeMember("ParentId");
            ECClassCP ecClass = m_dgndb->Elements().GetElement(DgnElementId(elementId))->GetElementClass();
            if (ecClass->Is(m_activityClass))
                obj["Element"]["relClassName"] = "Planning.ActivityOwnsTimeSpans";
            else
                obj["Element"]["relClassName"] = "Planning.WorkBreakdownOwnsTimeSpans";

            entry[JSON_TYPE_KEY] = JSON_TYPE_ElementMultiAspect;
            }
        else if (ecClass->GetName().Equals("Timeline"))
            {
            obj[JSON_CLASSNAME] = "Planning.Baseline";
            MakeNavigationProperty(obj, "Element", IdToString(obj["PlanId"].asCString()).c_str());
            obj.removeMember("PlanId");
            obj["Element"]["relClassName"] = "Planning.PlanOwnsBaselines";
            entry[JSON_TYPE_KEY] = JSON_TYPE_ElementMultiAspect;
            }

        obj.removeMember("$ECClassKey");
        obj.removeMember("$ECClassId");
        obj.removeMember("$ECClassLabel");
        obj.removeMember("$ECInstanceLabel");
        entry[JSON_OBJECT_KEY] = obj;
        SendToQueue(entry, true);

        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2018
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DgnDb0601ToJsonImpl::ExportTextAnnotationData()
    {

    Statement stmt;
    Utf8PrintfString sql("SELECT ECInstanceId, TextAnnotation FROM dgn_TextAnnotationData WHERE TextAnnotation is NOT NULL ");
    if (BE_SQLITE_OK != (stmt.Prepare(*m_dgndb, sql.c_str())))
        {
        LogMessage(BimFromDgnDbLoggingSeverity::LOG_ERROR, "Unable to prepare statement to retrieve TextAnnotationData");
        return ERROR;
        }
    while (BE_SQLITE_ROW == stmt.Step())
        {
        auto entry = Json::Value(Json::ValueType::objectValue);
        entry[JSON_TYPE_KEY] = JSON_TYPE_TextAnnotationData;
        entry[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
        entry[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
        auto& row = entry[JSON_OBJECT_KEY];
        row.clear();
        int64_t id = stmt.GetValueInt64(0);
        MakeNavigationProperty(row, "Element", id);

        ByteCP data = static_cast<ByteCP>(stmt.GetValueBlob(1));
        size_t dataSize = stmt.GetColumnBytes(1);
        Utf8String str;
        Base64Utilities::Encode(str, data, dataSize);
        row["TextAnnotation"] = str.c_str();
        SendToQueue(entry);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
void DgnDb0601ToJsonImpl::HandleAnnotationTextStyle(Json::Value& obj, DgnElementId id)
    {
    AnnotationTextStyleCPtr ats = m_dgndb->Elements().Get<AnnotationTextStyle>(id);
    if (!ats.IsValid())
        return;

    obj["ExplicitProperties"] = Json::Value(Json::ValueType::objectValue);
    auto& props = obj["ExplicitProperties"];
    props["AnnotationColorType"] = (int) ats->GetColorType();
    props["ColorValue"] = ats->GetColorValue().GetValue();
    props["FontId"] = IdToString(ats->GetFontId().GetValue()).c_str();
    props["Height"] = ats->GetHeight();
    props["LineSpacingFactor"] = ats->GetLineSpacingFactor();
    props["IsBold"] = ats->IsBold();
    props["IsItalic"] = ats->IsItalic();
    props["IsUnderlined"] = ats->IsUnderlined();
    props["StackedFractionScale"] = ats->GetStackedFractionScale();
    props["StackedFractionType"] = (int) ats->GetStackedFractionType();
    props["SubScriptOffsetFactor"] = ats->GetSubScriptOffsetFactor();
    props["SubScriptOffsetSclae"] = ats->GetSubScriptScale();
    props["WidthFactor"] = ats->GetWidthFactor();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnElementId DgnDb0601ToJsonImpl::CreateSubjectElement(Utf8CP subjectName)
    {
    auto entry = Json::Value(Json::ValueType::objectValue);
    entry[JSON_TYPE_KEY] = JSON_TYPE_Subject;
    entry[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
    entry[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
    auto& subject = entry[JSON_OBJECT_KEY];
    subject.clear();

    m_nextAvailableId = DgnElementId(m_nextAvailableId.GetValue() + 1);
    subject[JSON_INSTANCE_ID] = IdToString(m_nextAvailableId.GetValue()).c_str();
    subject["Label"] = subjectName;
    subject["Parent"] = -1;
    SendToQueue(entry);

    return m_nextAvailableId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnElementId DgnDb0601ToJsonImpl::CreatePartitionElement(DgnModelCR model, DgnElementId subject)
    {
    Utf8String partitionType = nullptr;
    if (model.IsGroupInformationModel())
        partitionType = "GroupInformationPartition";
    else if (model.IsSpatialModel())
        partitionType = "PhysicalPartition";
    else if (model.IsSheetModel() || model.Is2dModel())
        partitionType = "DocumentPartition";
    else if (m_dgndb->Schemas().GetECClass(model.GetClassId())->Is(m_linkModelClass))
        partitionType = "LinkPartition";
    else if (model.IsDefinitionModel())
        partitionType = "DefinitionPartition";
    else if (nullptr != m_planningModelClass && m_dgndb->Schemas().GetECClass(model.GetClassId())->Is(m_planningModelClass))
        partitionType = "PlanningPartition";
    else
        {
        LogMessage(BimFromDgnDbLoggingSeverity::LOG_WARNING, "Unknown model type %s", typeid(model).name());
        }

    return CreatePartitionElement(model.GetName().c_str(), partitionType.c_str(), subject);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnElementId DgnDb0601ToJsonImpl::CreatePartitionElement(Utf8CP partitionName, Utf8CP partitionType, DgnElementId subject)
    {
    auto entry = Json::Value(Json::ValueType::objectValue);
    entry[JSON_TYPE_KEY] = JSON_TYPE_Partition;
    entry[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
    entry[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
    auto& partition = entry[JSON_OBJECT_KEY];
    partition.clear();

    m_nextAvailableId = DgnElementId(m_nextAvailableId.GetValue() + 1);
    partition[JSON_INSTANCE_ID] = IdToString(m_nextAvailableId.GetValue()).c_str();
    partition["Label"] = partitionName;
    if (subject.IsValid())
        partition["Subject"] = IdToString(subject.GetValue()).c_str();
    partition["PartitionType"] = partitionType;
    SendToQueue(entry);

    return m_nextAvailableId;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DgnDb0601ToJsonImpl::ExportNamedGroups()
    {
    Statement stmt;
    Utf8CP sql = "SELECT GroupId, MemberId, MemberPriority from dgn_ElementGroupsMembers";
    if (BE_SQLITE_OK != (stmt.Prepare(*m_dgndb, sql)))
        {
        LogMessage(BimFromDgnDbLoggingSeverity::LOG_ERROR, "Unable to prepare statement to retrieve element groups elements instances");
        return ERROR;
        }

    while (BE_SQLITE_ROW == stmt.Step())
        {
        auto elemGroupsMembers = Json::Value(Json::ValueType::objectValue);
        elemGroupsMembers[JSON_TYPE_KEY] = JSON_TYPE_ElementGroupsMembers;
        elemGroupsMembers[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
        elemGroupsMembers[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
        auto& group = elemGroupsMembers[JSON_OBJECT_KEY];

        group["GroupId"] = IdToString(stmt.GetValueId<DgnElementId>(0).GetValue()).c_str();
        group["MemberId"] = IdToString(stmt.GetValueId<DgnElementId>(1).GetValue()).c_str();
        group["MemberPriority"] = stmt.GetValueInt(2);
        SendToQueue(elemGroupsMembers);
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            05/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DgnDb0601ToJsonImpl::ExportElementHasLinks()
    {

    Statement stmt;
    Utf8CP sql = "SELECT ElementId, LinkId from dgn_ElementHasLinks";
    if (BE_SQLITE_OK != (stmt.Prepare(*m_dgndb, sql)))
        {
        LogMessage(BimFromDgnDbLoggingSeverity::LOG_ERROR, "Unable to prepare statement to retrieve element has links instances");
        return ERROR;
        }


    while (BE_SQLITE_ROW == stmt.Step())
        {
        auto elemHasLinks = Json::Value(Json::ValueType::objectValue);
        elemHasLinks[JSON_TYPE_KEY] = JSON_TYPE_ElementHasLinks;
        elemHasLinks[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
        elemHasLinks[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
        auto& group = elemHasLinks[JSON_OBJECT_KEY];

        group["ElementId"] = IdToString(stmt.GetValueId<DgnElementId>(0).GetValue()).c_str();
        group["LinkId"] = IdToString(stmt.GetValueId<DgnElementId>(1).GetValue()).c_str();
        SendToQueue(elemHasLinks);
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2018
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DgnDb0601ToJsonImpl::ExportConstraint(ECClassId constraintClassId, ECInstanceId constraintId)
    {
    ECClassCP constraintClass = m_dgndb->Schemas().GetECClass(constraintClassId);
    DgnElementId dgnId(constraintId.GetValue());
    
    if (constraintClass->Is(m_elementClass))
        {
        if (m_insertedElements.end() == m_insertedElements.find(dgnId))
            {
            DgnElementCPtr constraintEl = m_dgndb->Elements().GetElement(dgnId);
            Utf8PrintfString whereClause(" AND ECInstanceId=%" PRIu64, constraintId.GetValue());
            auto out = Json::Value(Json::ValueType::objectValue);
            if (SUCCESS != ExportElements(out, constraintClass->GetSchema().GetName().c_str(), constraintClass->GetName().c_str(), constraintEl->GetModelId(), whereClause.c_str()))
                return ERROR;
            }
        }
    else if (constraintClass->Is(m_elementAspectClass))
        {
        //if (m_insertedAspects.end() == m_insertedAspects.find(constraintId))
        //    {
        //    ExportElementAspects(constraintClassId, constraintId);
        //    }
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DgnDb0601ToJsonImpl::ExportLinkTables(Utf8CP schemaName, Utf8CP className, Utf8CP newClassName)
    {
    if (!m_dgndb->Schemas().ContainsECSchema(schemaName))
        return SUCCESS;
    if (nullptr == m_dgndb->Schemas().GetECSchema(schemaName)->GetClassCP(className))
        return SUCCESS;

    //Utf8PrintfString ecSql("SELECT s.Name, c.Name, r.SourceECInstanceId, r.TargetECInstanceId FROM [%s].[%s] r, [MetaSchema].[Class] c, [MetaSchema].[Schema] s WHERE c.[Id] = r.ECClassId AND s.Id = c.[SchemaId]", schemaName, className);
    Utf8PrintfString ecSql("SELECT ECClassId, SourceECInstanceId, SourceECClassId, TargetECInstanceId, TargetECClassId from [%s].[%s] ORDER BY ECClassId", schemaName, className);
    CachedECSqlStatementPtr stmt = m_dgndb->GetPreparedECSqlStatement(ecSql.c_str());

    if (!stmt.IsValid())
        {
        LogMessage(BimFromDgnDbLoggingSeverity::LOG_ERROR, "Unable to prepare statement to retrieve generic relationship instances");
        return ERROR;
        }

    while (BE_SQLITE_ROW == stmt->Step())
        {
        auto entry = Json::Value(Json::ValueType::objectValue);
        entry[JSON_TYPE_KEY] = JSON_TYPE_LinkTable;
        entry[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
        entry[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
        auto&  relationship = entry[JSON_OBJECT_KEY];
        relationship.clear();

        ECInstanceId sourceId = stmt->GetValueId<ECInstanceId>(1);
        ECInstanceId targetId = stmt->GetValueId<ECInstanceId>(3);
        ExportConstraint(stmt->GetValueId<ECClassId>(2), sourceId);
        ExportConstraint(stmt->GetValueId<ECClassId>(4), targetId);

        ECClassCP relClass = m_dgndb->Schemas().GetECClass(stmt->GetValueId<ECClassId>(0));
        relationship["Schema"] = relClass->GetSchema().GetName().c_str();
        if (nullptr == newClassName)
            relationship["Class"] = relClass->GetName().c_str();
        else
            relationship["Class"] = newClassName;
        relationship["SourceId"] = IdToString(sourceId.GetValue()).c_str();
        relationship["TargetId"] = IdToString(targetId.GetValue()).c_str();
        SendToQueue(entry);
        }
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            03/2018
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DgnDb0601ToJsonImpl::ExportPropertyData()
    {
    PropertySpec    prop = DgnViewProperty::DefaultView();
    DgnElementId    existingId;
    if (m_dgndb->QueryProperty(&existingId, sizeof(existingId), prop) != DbResult::BE_SQLITE_ROW || !existingId.IsValid())
        return SUCCESS;

    auto entry = Json::Value(Json::ValueType::objectValue);
    entry[JSON_TYPE_KEY] = JSON_TYPE_PropertyData;
    entry[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
    entry[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
    auto& propData = entry[JSON_OBJECT_KEY];
    propData.clear();

    MakeNavigationProperty(propData, "DefaultView", existingId);
    uint32_t propSize;
    if (m_dgndb->QueryPropertySize(propSize, DgnProjectProperty::DgnGCS()) == BeSQLite::BE_SQLITE_ROW)
        {

        ScopedArray<Byte> buffer(propSize);
        m_dgndb->QueryProperty(buffer.GetData(), propSize, DgnProjectProperty::DgnGCS());
        propData["GCS"] = Json::Value(Json::ValueType::objectValue);
        ECN::ECJsonUtilities::BinaryToJson(propData["GCS"], buffer.GetData(), propSize);

        DPoint3d globalOrigin;
        globalOrigin = m_dgndb->Units().GetGlobalOrigin();
        propData["globalOrigin"] = Json::Value(Json::ValueType::objectValue);
        ECN::ECJsonUtilities::Point3DToJson(propData["globalOrigin"], globalOrigin);
        }

    Utf8String value;
    if (BE_SQLITE_ROW == m_dgndb->QueryProperty(value, DgnProjectProperty::Extents()))
        propData["projectExtents"] = value;
    SendToQueue(entry);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2018
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DgnDb0601ToJsonImpl::ExportEmbeddedFiles()
    {
    DbEmbeddedFileTable& embeddedFileTable = m_dgndb->EmbeddedFiles();

    DbEmbeddedFileTable::Iterator iter = embeddedFileTable.MakeIterator();

    for (auto const& file : iter)
        {
        bvector<Byte> buffer;
        embeddedFileTable.Read(buffer, file.GetNameUtf8());

        auto entry = Json::Value(Json::ValueType::objectValue);
        entry[JSON_TYPE_KEY] = JSON_TYPE_EmbeddedFile;
        entry[JSON_OBJECT_KEY] = Json::Value(Json::ValueType::objectValue);
        entry[JSON_ACTION_KEY] = JSON_ACTION_INSERT;
        auto& fileData = entry[JSON_OBJECT_KEY];
        fileData.clear();
        fileData["data"] = Json::Value(Json::ValueType::objectValue);
        ECN::ECJsonUtilities::BinaryToJson(fileData["data"], buffer.data(), buffer.size());
        fileData["name"] = file.GetNameUtf8();
        Utf8String fileType(file.GetTypeUtf8());
        if (fileType.EqualsI("ExtraFile"))
            fileType = Utf8String(BeFileName::GetExtension(WString(file.GetNameUtf8(), BentleyCharEncoding::Utf8).c_str()));
        fileData["type"] = fileType.c_str();
        SendToQueue(entry);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            07/2014
//---------------+---------------+---------------+---------------+---------------+-------
void DgnDb0601ToJsonImpl::ReportProgress() const
    {
    if (nullptr != m_meter && DgnProgressMeter::ABORT_Yes == m_meter->ShowProgress())
        {
        //OnFatalError();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnDb0601ToJsonImpl::SetStepName(ProgressMessage::StringId stringNum, ...) const
    {
    if (nullptr == m_meter)
        return;

    Utf8String fmt = ProgressMessage::GetString(stringNum);
    if (fmt.length() == 0)
        return;

    va_list args;
    va_start(args,stringNum);

    Utf8String value;
    value.VSprintf(fmt.c_str(), args);
    va_end(args);

    m_meter->SetCurrentStepName(value.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2017
//---------------+---------------+---------------+---------------+---------------+-------
void DgnDb0601ToJsonImpl::MakeNavigationProperty(Json::Value& out, Utf8CP propertyName, BeInt64Id id)
    {
    MakeNavigationProperty(out, propertyName, id.GetValue());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2017
//---------------+---------------+---------------+---------------+---------------+-------
void DgnDb0601ToJsonImpl::MakeNavigationProperty(Json::Value& out, Utf8CP propertyName, uint64_t id)
    {
    MakeNavigationProperty(out, propertyName, IdToString(id).c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2017
//---------------+---------------+---------------+---------------+---------------+-------
void DgnDb0601ToJsonImpl::MakeNavigationProperty(Json::Value& out, Utf8CP propertyName, Utf8CP id)
    {
    auto& navObj = out[propertyName] = Json::Value(Json::ValueType::objectValue);
    navObj["id"] = id;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2017
//---------------+---------------+---------------+---------------+---------------+-------
void DgnDb0601ToJsonImpl::MakeNavigationProperty(Json::Value& out, Utf8CP propertyName, Json::Value& id)
    {
    auto& navObj = out[propertyName] = Json::Value(Json::ValueType::objectValue);
    navObj["id"] = id;
    }

END_DGNDB0601_TO_JSON_NAMESPACE
